#define LED_BUILTIN (16*('I'-'A')+15)

extern "C" {
#define boolean   boolean_t
#include "openamp.h"
}

#define RPMSG_SERVICE_NAME              "openamp_cm4_serial"

static  uint32_t message;
static volatile int message_received;
static volatile unsigned int received_data;

static struct rpmsg_endpoint rp_endpoint;

HSEM_TypeDef * HSEM_DEBUG = HSEM;

static int rpmsg_recv_callback(struct rpmsg_endpoint *ept, void *data,
                               size_t len, uint32_t src, void *priv)
{
  received_data = *((unsigned int *) data);
  message_received = 1;

  if (received_data == 0x7F7F7F7F) {
    NVIC_SystemReset();
  }

  return 0;
}

unsigned int receive_message(void)
{
  while (message_received == 0)
  {
    OPENAMP_check_for_message();
  }
  message_received = 0;

  return received_data;
}

void setup()
{
  int32_t status = 0;

#if 1
  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /*HW semaphore Notification enable*/
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

  /* Inilitize the mailbox use notify the other core on new message */
  MAILBOX_Init();

  /* Inilitize OpenAmp and libmetal libraries */
  if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL) !=  0) {
    while (1) {}
  }

  /* create a endpoint for rmpsg communication */
  status = OPENAMP_create_endpoint(&rp_endpoint, RPMSG_SERVICE_NAME, RPMSG_ADDR_ANY,
                                   rpmsg_recv_callback, NULL);
  if (status < 0)
  {
    while (1) {}
  }
#endif

  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {

#if 1
  OPENAMP_check_for_message();
  message++;
  OPENAMP_send(&rp_endpoint, &message, sizeof(message));
#endif

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
