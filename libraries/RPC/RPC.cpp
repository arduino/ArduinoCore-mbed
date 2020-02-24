#include "RPC_internal.h"

void rpc::client::getResult(RPCLIB_MSGPACK::object_handle& res) {
  RPC1.getResult(res);
}

void rpc::client::post(RPCLIB_MSGPACK::sbuffer *buffer) {
  RPC1.write(ENDPOINT_CM4TOCM7, (const uint8_t*)buffer->data(), buffer->size());
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
  return 0;
}

#ifdef CORE_CM4
int RPC::begin() {

  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /*HW semaphore Notification enable*/
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

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

  eventThread = new rtos::Thread(osPriorityNormal);
  eventThread->start(callback(&eventQueue, &events::EventQueue::dispatch_forever));
  ticker.attach(eventQueue.event(&OPENAMP_check_for_message), 0.02f);

  dispatcherThread = new rtos::Thread(osPriorityNormal);
  dispatcherThread->start(mbed::callback(this, &RPC::dispatch));

  initialized = true;
  pac_.reserve_buffer(1024);

  return 1;
}
#endif

#ifdef CORE_CM7

extern "C" {
	int openamp_enable();
	void OpenAMP_MPU_Config(void);
}

int RPC::begin() {

	OpenAMP_MPU_Config();
	openamp_enable();

	eventThread = new rtos::Thread(osPriorityNormal);
	eventThread->start(callback(&eventQueue, &events::EventQueue::dispatch_forever));
	ticker.attach(eventQueue.event(&OPENAMP_check_for_message), 0.005f);

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
          auto resp = server.dispatch(msg, true);
          auto data = resp.get_data();
          if (resp.is_empty()) {
            //printf("no response\n");
          } else {
            // TODO: Post data to endpoint
            write(ENDPOINT_CM7TOCM4, (const uint8_t*)data.data(), data.size());
            // printf("result: %d\n", resp.get_result()->as<int>());
          }
        }
      }
      if (v.value.signals & 0x2) {
        RPCLIB_MSGPACK::unpacked result;
        while (pac_.next(result)) {
          auto r = rpc::detail::response(std::move(result));
          auto id = r.get_id();
          call_result = std::move(*r.get_result());
          // Unlock callThreadId thread
          osSignalSet(client.callThreadId, 0x1);
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
  printf("OPENAMP_send on ep %d returned %d\n", ep, ret);
  return len;
}

arduino::RPC RPC1;
