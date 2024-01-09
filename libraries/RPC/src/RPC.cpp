#include "RPC.h"

static struct rpmsg_endpoint rp_endpoints[4];

enum endpoints_t {
  ENDPOINT_RAW = 0,
  ENDPOINT_RESPONSE = 1
};

void rpc::client::send_msgpack(RPCLIB_MSGPACK::sbuffer *buffer) {
  OPENAMP_send(&rp_endpoints[ENDPOINT_RAW], (const uint8_t*)buffer->data(), buffer->size());
}

static RingBufferN<RPMSG_BUFFER_SIZE> intermediate_buffer;
static RingBufferN<RPMSG_BUFFER_SIZE> intermediate_buffer_resp;
//static uint8_t intermediate_buffer_resp[256];
static rtos::Mutex rx_mtx;

static bool _init_recv_message = true;

int RPCClass::rpmsg_recv_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv)
{
  RPCClass* rpc = (RPCClass*)priv;

#ifdef CORE_CM4
      if (_init_recv_message) {
        _init_recv_message = false;
        return 0;
      }
#endif

  rx_mtx.lock();
  for (size_t i = 0; i < len; i++) {
    intermediate_buffer.store_char(((uint8_t*)data)[i]);
  }
  rx_mtx.unlock();

  //memcpy(intermediate_buffer, data, len);

  osSignalSet(rpc->dispatcherThreadId, len);

  return 0;
}

static bool _init_resp_message = true;

int RPCClass::rpmsg_recv_response_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv)
{
  RPCClass* rpc = (RPCClass*)priv;

#ifdef CORE_CM4
      if (_init_resp_message) {
        _init_resp_message = false;
        return 0;
      }
#endif

  rx_mtx.lock();
  for (size_t i = 0; i < len; i++) {
    intermediate_buffer_resp.store_char(((uint8_t*)data)[i]);
  }
  //memcpy(intermediate_buffer_resp, data, len);
  rx_mtx.unlock();

  osSignalSet(rpc->responseThreadId, len);

  return 0;
}

void RPCClass::new_service_cb(struct rpmsg_device *rdev, const char *name, uint32_t dest)
{
  if (strcmp(name, "raw") == 0) {
    OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_RAW], name, dest, rpmsg_recv_callback, NULL);
  }
  if (strcmp(name, "response") == 0) {
    OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_RESPONSE], name, dest, rpmsg_recv_response_callback, NULL);
  }
}

osThreadId eventHandlerThreadId;

void eventHandler() {
  eventHandlerThreadId = osThreadGetId();
  while (1) {
    osSignalWait(0, osWaitForever);
    OPENAMP_check_for_message();
  }
}

#ifdef CORE_CM7

static void OpenAMP_MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as WT for SDRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = D3_SRAM_BASE;
	MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER7;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

#define return_if_not_ok(x) do { int ret = x ; if (ret != HAL_OK) return; } while (0);

static void disableCM4Autoboot() {
  FLASH_OBProgramInitTypeDef OBInit;
  OBInit.Banks     = FLASH_BANK_1;
  HAL_FLASHEx_OBGetConfig(&OBInit);
  if (OBInit.USERConfig & FLASH_OPTSR_BCM4) {
    OBInit.OptionType = OPTIONBYTE_USER;
    OBInit.USERType = OB_USER_BCM4;
    OBInit.USERConfig = 0;
    return_if_not_ok(HAL_FLASH_OB_Unlock());
    return_if_not_ok(HAL_FLASH_Unlock());
    return_if_not_ok(HAL_FLASHEx_OBProgram(&OBInit));
    return_if_not_ok(HAL_FLASH_OB_Launch());
    return_if_not_ok(HAL_FLASH_OB_Lock());
    return_if_not_ok(HAL_FLASH_Lock());
    printf("CM4 autoboot disabled\n");
    NVIC_SystemReset();
    return;
  }
}

int RPCClass::begin(long unsigned int np, uint16_t nd) {

	OpenAMP_MPU_Config();

	//resource_table_load_from_flash();
	//HAL_SYSCFG_EnableCM4BOOT();

  // Ideally this should execute only once
  disableCM4Autoboot();

  eventThread = new rtos::Thread(osPriorityHigh, 4096, nullptr, "rpc_evt");
  eventThread->start(&eventHandler);

  dispatcherThread = new rtos::Thread(osPriorityNormal, 4096, nullptr, "rpc_dispatch");
  dispatcherThread->start(mbed::callback(this, &RPCClass::dispatch));

  responseThread = new rtos::Thread(osPriorityNormal, 4096, nullptr, "rpc_response");
  responseThread->start(mbed::callback(this, &RPCClass::response));

	/* Initialize OpenAmp and libmetal libraries */
	if (MX_OPENAMP_Init(RPMSG_MASTER, new_service_cb) !=  HAL_OK) {
	 return 0;
	}

  //metal_set_log_handler(metal_default_log_handler);

	/* Initialize the rpmsg endpoint to set default addresses to RPMSG_ADDR_ANY */
  rpmsg_init_ept(&rp_endpoints[ENDPOINT_RAW], "raw", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, NULL, NULL);
  rpmsg_init_ept(&rp_endpoints[ENDPOINT_RESPONSE], "response", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, NULL, NULL);

  rp_endpoints[ENDPOINT_RAW].priv = this;
  rp_endpoints[ENDPOINT_RESPONSE].priv = this;

  bootM4();

	/*
	* The rpmsg service is initiate by the remote processor, on H7 new_service_cb
	* callback is received on service creation. Wait for the callback
	*/
  auto err = OPENAMP_Wait_EndPointready(&rp_endpoints[ENDPOINT_RAW], millis() + 500);
  err |= OPENAMP_Wait_EndPointready(&rp_endpoints[ENDPOINT_RESPONSE], millis() + 500);

  if (err == 0) {
    initialized = true;
  } else {
    return 0;
  }

	// Send first dummy message to enable the channel
	uint8_t message = 0x00;
  write(ENDPOINT_RAW, &message, sizeof(message));
  write(ENDPOINT_RESPONSE, &message, sizeof(message));

	return 1;
}

#endif


#ifdef CORE_CM4

int RPCClass::begin(long unsigned int np, uint16_t nd) {

  eventThread = new rtos::Thread(osPriorityHigh, 4096, nullptr, "rpc_evt");
  eventThread->start(&eventHandler);

  dispatcherThread = new rtos::Thread(osPriorityNormal, 4096, nullptr, "rpc_dispatch");
  dispatcherThread->start(mbed::callback(this, &RPCClass::dispatch));

  responseThread = new rtos::Thread(osPriorityNormal, 4096, nullptr, "rpc_response");
  responseThread->start(mbed::callback(this, &RPCClass::response));

  /* Initialize OpenAmp and libmetal libraries */
  if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL) !=  0) {
    return 0;
  }

  rp_endpoints[ENDPOINT_RAW].priv = this;
  rp_endpoints[ENDPOINT_RESPONSE].priv = this;

  /* create a endpoint for raw rmpsg communication */
  int status = OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_RAW], "raw", RPMSG_ADDR_ANY,
                                   rpmsg_recv_callback, NULL);
  if (status < 0) {
    return 0;
  }

  status = OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_RESPONSE], "response", RPMSG_ADDR_ANY,
                                   rpmsg_recv_response_callback, NULL);
  if (status < 0) {
    return 0;
  }

  initialized = true;

  return 1;
}

#endif

using raw_call_t = std::tuple<RPCLIB_MSGPACK::object>;

void RPCClass::response() {
  responseThreadId = osThreadGetId();

  for (int i = 0; i< 10; i++) {
    clients[i] = NULL;
  }

  while (true) {
    osSignalWait(0, osWaitForever);

{

      RPCLIB_MSGPACK::unpacker pac;

      rx_mtx.lock();
      int len = intermediate_buffer_resp.available();
      for (int i = 0; i < len; i++) {
        pac.buffer()[i] = intermediate_buffer_resp.read_char();
      }
      pac.buffer_consumed(len);
      rx_mtx.unlock();

      //memcpy(pac.buffer(), intermediate_buffer_resp, v.value.signals);
      //pac.buffer_consumed(v.value.signals);

      RPCLIB_MSGPACK::unpacked result;
      while (pac.next(result)) {
        auto r = rpc::detail::response(std::move(result));
        auto id = r.get_id();
        // fill the correct client stuff
        rpc::client* client = NULL;
        for (int i = 0; i < 10; i++) {
          if (clients[i] != NULL) {
            if ((uint)clients[i]->callThreadId == id) {
              client = clients[i];
              break;
            }
          }
        }
        if (client != NULL) {
          client->result = std::move(*r.get_result());
          // Unlock callThreadId thread
          osSignalSet(client->callThreadId, 0x1);
        }
      }
    }
  }
}

void RPCClass::dispatch() {

  dispatcherThreadId = osThreadGetId();

  while (true) {
    osSignalWait(0, osWaitForever);

{
    RPCLIB_MSGPACK::unpacker pac;
    rx_mtx.lock();
    int len = intermediate_buffer.available();
    for (int i = 0; i< len; i++) {
      pac.buffer()[i] = intermediate_buffer.read_char();
    }
    pac.buffer_consumed(len);
    rx_mtx.unlock();

    //memcpy(pac.buffer(), intermediate_buffer, v.value.signals);
    //pac.buffer_consumed(v.value.signals);

    RPCLIB_MSGPACK::unpacked result;
    while (pac.next(result)) {
      auto msg = result.get();
      if (msg.via.array.size == 1) {
        // raw array
        raw_call_t arr;
        msg.convert(arr);

        std::vector<uint8_t> buf;
        std::get<0>(arr).convert(buf);

        for (size_t i = 0; i < buf.size(); i++) {
          rx_buffer.store_char(buf[i]);
        }
        // call attached function
        if (_rx) {
          _rx.call();
        }
      }

      if (msg.via.array.size > 2) {
        auto resp = rpc::detail::dispatcher::dispatch(msg, true);
        auto data = resp.get_data();
        if (resp.is_empty()) {
          //printf("no response\n");
        } else {
          OPENAMP_send(&rp_endpoints[ENDPOINT_RESPONSE], (const uint8_t*)data.data(), data.size());
        }
      }
    }
  }
}
}


size_t RPCClass::write(uint8_t c) {
  write(&c, 1);
  return 1;
}

size_t RPCClass::write(const uint8_t* buf, size_t len) {
  return write(ENDPOINT_RAW, buf, len);
}

size_t RPCClass::write(uint8_t ep, const uint8_t* buf, size_t len) {

  std::vector<uint8_t> tx_buffer;
  for (size_t i = 0; i < len; i++) {
    tx_buffer.push_back(buf[i]);
  }
  auto call_obj = std::make_tuple(tx_buffer);

  auto buffer = new RPCLIB_MSGPACK::sbuffer;
  RPCLIB_MSGPACK::pack(*buffer, call_obj);

  OPENAMP_send(&rp_endpoints[ep], (const uint8_t*)buffer->data(), buffer->size());
  delete buffer;
  return len;
}

arduino::RPCClass RPC;