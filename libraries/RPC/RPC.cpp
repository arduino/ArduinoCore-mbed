#include "RPC_internal.h"

static struct rpmsg_endpoint rp_endpoints[4];

void rpc::client::post(RPCLIB_MSGPACK::sbuffer *buffer) {
#ifdef CORE_CM7
  RPC1.write(ENDPOINT_CM7TOCM4, (const uint8_t*)buffer->data(), buffer->size());
#else
  RPC1.write(ENDPOINT_CM4TOCM7, (const uint8_t*)buffer->data(), buffer->size());
#endif
}

int RPC::rpmsg_recv_cm7tocm4_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv)
{
  // This fuction gets called when we are the rpc server and need to execute a function
  RPC* rpc = (RPC*)priv;
  memcpy(rpc->pac_.buffer(), (const void*)data, len);
  rpc->pac_.buffer_consumed(len);

  osSignalSet(rpc->dispatcherThreadId, 0x1);

  return 0;
}

int RPC::rpmsg_recv_cm4tocm7_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv)
{
  // This fuction gets called when we want to retrieve the rpc response (as clients)
  RPC* rpc = (RPC*)priv;
  memcpy(rpc->pac_.buffer(), (const void*)data, len);
  rpc->pac_.buffer_consumed(len);

  osSignalSet(rpc->dispatcherThreadId, 0x2);

  return 0;
}

int RPC::rpmsg_recv_raw_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv)
{
  RPC* rpc = (RPC*)priv;
  uint8_t* buf = (uint8_t*)data;
  for (int i=0; i<len; i++) {
    rpc->rx_buffer.store_char(buf[i]);
  }
  // call attached function
  if (rpc->_rx) {
    rpc->_rx.call();
  }

  return 0;
}

void RPC::new_service_cb(struct rpmsg_device *rdev, const char *name, uint32_t dest)
{
  int idx = -1;
  if (strcmp(name, "cm7tocm4") == 0) {
    OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_CM7TOCM4], name, dest, rpmsg_recv_cm4tocm7_callback, NULL);
  }
  if (strcmp(name, "cm4tocm7") == 0) {
    OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_CM4TOCM7], name, dest, rpmsg_recv_cm7tocm4_callback, NULL);
  }
  if (strcmp(name, "raw") == 0) {
    OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_RAW], name, dest, rpmsg_recv_raw_callback, NULL);
  }
}

osThreadId eventHandlerThreadId;

void eventHandler() {
  eventHandlerThreadId = osThreadGetId();
  while (1) {
    osEvent v = osSignalWait(0, osWaitForever);
#ifdef CORE_CM4
    delay(50);
#endif
    OPENAMP_check_for_message();
  }
}

#ifdef CORE_CM4
int RPC::begin() {

  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /*HW semaphore Notification enable*/
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

  eventThread = new rtos::Thread(osPriorityHigh);
  eventThread->start(&eventHandler);

  /* Inilitize OpenAmp and libmetal libraries */
  if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL) !=  0) {
    return 0;
  }

  rp_endpoints[0].priv = this;
  rp_endpoints[1].priv = this;
  rp_endpoints[2].priv = this;

  /* create a endpoint for rmpsg communication */
  int status = OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_CM7TOCM4], "cm7tocm4", RPMSG_ADDR_ANY,
                                   rpmsg_recv_cm7tocm4_callback, NULL);
  if (status < 0)
  {
    return 0;
  }

  status = OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_CM4TOCM7], "cm4tocm7", RPMSG_ADDR_ANY,
                                   rpmsg_recv_cm4tocm7_callback, NULL);
  if (status < 0)
  {
    return 0;
  }

  /* create a endpoint for raw rmpsg communication */
  status = OPENAMP_create_endpoint(&rp_endpoints[ENDPOINT_RAW], "raw", RPMSG_ADDR_ANY,
                                   rpmsg_recv_raw_callback, NULL);
  if (status < 0)
  {
    return 0;
  }

  dispatcherThread = new rtos::Thread(osPriorityNormal);
  dispatcherThread->start(mbed::callback(this, &RPC::dispatch));

  initialized = true;
  pac_.reserve_buffer(1024);

  return 1;
}
#endif

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
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER7;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

int RPC::begin() {

	OpenAMP_MPU_Config();

	//resource_table_load_from_flash();
	//HAL_SYSCFG_EnableCM4BOOT();
	HAL_RCCEx_EnableBootCore(RCC_BOOT_C2);

	eventThread = new rtos::Thread(osPriorityHigh);
	eventThread->start(&eventHandler);

	/* Initialize OpenAmp and libmetal libraries */
	if (MX_OPENAMP_Init(RPMSG_MASTER, new_service_cb) !=  HAL_OK) {
	printf("openAMP init failed\n\rNo RPC is available\n\r");
	return 0;
	}

	/* Initialize the rpmsg endpoint to set default addresses to RPMSG_ADDR_ANY */
	rpmsg_init_ept(&rp_endpoints[0], "cm7tocm4", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, NULL, NULL);

	rpmsg_init_ept(&rp_endpoints[1], "cm4tocm7", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, NULL, NULL);

	rpmsg_init_ept(&rp_endpoints[2], "raw", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, NULL, NULL);

	rp_endpoints[0].priv = this;
	rp_endpoints[1].priv = this;
	rp_endpoints[2].priv = this;

	/*
	* The rpmsg service is initiate by the remote processor, on H7 new_service_cb
	* callback is received on service creation. Wait for the callback
	*/
	OPENAMP_Wait_EndPointready(&rp_endpoints[0], HAL_GetTick() + 500);
	OPENAMP_Wait_EndPointready(&rp_endpoints[1], HAL_GetTick() + 500);
	OPENAMP_Wait_EndPointready(&rp_endpoints[2], HAL_GetTick() + 500);

	// Send first dummy message to enable the channel
	int message = 0x00;
	OPENAMP_send(&rp_endpoints[0], &message, sizeof(message));
	OPENAMP_send(&rp_endpoints[1], &message, sizeof(message));
	OPENAMP_send(&rp_endpoints[2], &message, sizeof(message));

	dispatcherThread = new rtos::Thread(osPriorityNormal);
	dispatcherThread->start(mbed::callback(this, &RPC::dispatch));

	initialized = true;
	pac_.reserve_buffer(1024);
	return 1;
}
#endif

void RPC::dispatch() {

  dispatcherThreadId = osThreadGetId();

  while (true) {
    osEvent v = osSignalWait(0, osWaitForever);

    if (v.status == osEventSignal) {
       if (v.value.signals & 0x1) {
        RPCLIB_MSGPACK::unpacked result;
        while (pac_.next(result)) {
          auto msg = result.get();
          auto resp = rpc::detail::dispatcher::dispatch(msg, true);
          auto data = resp.get_data();
          if (resp.is_empty()) {
            //printf("no response\n");
          } else {
#ifdef CORE_CM7
            write(ENDPOINT_CM4TOCM7, (const uint8_t*)data.data(), data.size());
#else
            write(ENDPOINT_CM7TOCM4, (const uint8_t*)data.data(), data.size());
#endif
          }
        }
      }
      if (v.value.signals & 0x2) {
        RPCLIB_MSGPACK::unpacked result;
        while (pac_.next(result)) {
          auto r = rpc::detail::response(std::move(result));
          auto id = r.get_id();
          // fill the correct client stuff
          int i = 0;
          for (i = 0; i<10; i++) {
            if (clients[i] != NULL && (int)clients[i]->callThreadId == id) {
              break;
            }
          }
          clients[i]->result = std::move(*r.get_result());
          // Unlock callThreadId thread
          osSignalSet(clients[i]->callThreadId, 0x1);
        }
      }
    }
  }
}

size_t RPC::write(const uint8_t* buf, size_t len) {
  OPENAMP_send(&rp_endpoints[ENDPOINT_RAW], buf, len);
  return len;
}

size_t RPC::write(uint8_t c) {
  OPENAMP_send(&rp_endpoints[ENDPOINT_RAW], &c, 1);
  return 1;
}

size_t RPC::write(enum endpoints_t ep, const uint8_t* buf, size_t len) {
  int ret = OPENAMP_send(&rp_endpoints[ep], buf, len);
  return len;
}

arduino::RPC RPC1;
