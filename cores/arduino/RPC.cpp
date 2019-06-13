#include "RPC.h"

int RPC::rpmsg_recv_service_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv)
{
  RPC* rpc = (RPC*)priv;
  service_request* s = ((service_request *) data);
  if (s->code == REQUEST_REBOOT) {
    NVIC_SystemReset();
  }
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

int RPC::begin() {

  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /*HW semaphore Notification enable*/
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

  /* Inilitize the mailbox use notify the other core on new message */
  MAILBOX_Init();

  /* Inilitize OpenAmp and libmetal libraries */
  if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL) !=  0) {
    return 0;
  }

  rp_endpoints[0].priv = this;
  rp_endpoints[1].priv = this;

  /* create a endpoint for rmpsg communication */
  int status = OPENAMP_create_endpoint(&rp_endpoints[0], "service", RPMSG_ADDR_ANY,
                                   rpmsg_recv_service_callback, NULL);
  if (status < 0)
  {
    return 0;
  }

  /* create a endpoint for raw rmpsg communication */
  status = OPENAMP_create_endpoint(&rp_endpoints[1], "raw", RPMSG_ADDR_ANY,
                                   rpmsg_recv_raw_callback, NULL);
  if (status < 0)
  {
    return 0;
  }

  eventThread = new rtos::Thread(osPriorityNormal);
  eventThread->start(callback(&eventQueue, &events::EventQueue::dispatch_forever));
  ticker.attach(eventQueue.event(&OPENAMP_check_for_message), 0.02f);

  initialized = true;
  return 1;
}

size_t RPC::write(const uint8_t* buf, size_t len) {
  OPENAMP_send(&rp_endpoints[1], buf, len);
  return len;
}

size_t RPC::write(uint8_t c) {
  OPENAMP_send(&rp_endpoints[1], &c, 1);
  return 1;
}

int RPC::request(service_request* s) {
  return OPENAMP_send(&rp_endpoints[0], s, sizeof(*s) + s->length);
}

arduino::RPC RPC1;
