#include "stm32h7xx_hal.h"
#include "openamp.h"
#include "mbox_hsem.h"
#include "rsc_table.h"

static struct rpmsg_endpoint rp_endpoints[4];

typedef struct _service_request {
  uint8_t* data;
} service_request;

static int rpmsg_recv_callback(struct rpmsg_endpoint *ept, void *data,
                size_t len, uint32_t src, void *priv)
{
  if (strcmp(ept->name, "cm7tocm4") == 0) {
    #if 0
      printf("Got RPC response: ");
      for (int i=0; i<len; i++) {
        printf("[%02X]", ((uint8_t*)data)[i]);
      }
      printf("\n");
    #endif
      // TODO: pass the result to the caller
  }
  if (strcmp(ept->name, "cm4tocm7") == 0) {
    #if 0
      printf("Got RPC request: %d\n\r", (*(int*)data));
      for (int i=0; i<len; i++) {
        printf("[%02X]", ((uint8_t*)data)[i]);
      }
      printf("\n");

      // calling add(6,19)
      //uint8_t test_buf[10] = {0x94, 0x00, 0x00,  0xA3,0x61,0x64,0x64,0x92,0x06,0x13};
      //OPENAMP_send(&rp_endpoints[0], test_buf, sizeof(test_buf));

      // responding to add(6,19)
      uint8_t test_buf[5] = {0x94,0x01,0x00,0xC0,((uint8_t*)data)[9] + ((uint8_t*)data)[8]};
      OPENAMP_send(&rp_endpoints[1], test_buf, sizeof(test_buf));

      printf("After responding\n\r");
    #endif

      // TODO: start the repl, execute the required function, respond on the same endpoint
  }
  if (strcmp(ept->name, "raw") == 0) {
	  int i = 0;
	  while (i < len) {
		putc(((char*)data)[i++], stdout);
	  }
  }
  return 0;
}

void service_destroy_cb(struct rpmsg_endpoint *ept)
{
  /* this function is called while remote endpoint as been destroyed, the 
   * service is no more available
   */
}

void new_service_cb(struct rpmsg_device *rdev, const char *name, uint32_t dest)
{
  int idx = -1;
  if (strcmp(name, "cm7tocm4") == 0) {
    idx = 0;
  }
  if (strcmp(name, "cm4tocm7") == 0) {
    idx = 1;
  }
  if (strcmp(name, "raw") == 0) {
    idx = 2;
  }
  /* create a endpoint for rmpsg communication */
  OPENAMP_create_endpoint(&rp_endpoints[idx], name, dest, rpmsg_recv_callback,
                          service_destroy_cb);
}

void OpenAMP_MPU_Config(void)
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
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


int openamp_enable() {

  //resource_table_load_from_flash();
  //HAL_SYSCFG_EnableCM4BOOT();
  HAL_RCCEx_EnableBootCore(RCC_BOOT_C2);

  /* Initialize the mailbox use notify the other core on new message */
  MAILBOX_Init();

  /* Initialize OpenAmp and libmetal libraries */
  if (MX_OPENAMP_Init(RPMSG_MASTER, new_service_cb) !=  HAL_OK) {
    printf("openAMP init failed\n\rNo RPC is available\n\r");
    return 0;
  }

  /* Initialize the rpmsg endpoint to set default addresses to RPMSG_ADDR_ANY */
  rpmsg_init_ept(&rp_endpoints[0], "cm7tocm4", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                 NULL, NULL);

  rpmsg_init_ept(&rp_endpoints[1], "cm4tocm7", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                 NULL, NULL);

  rpmsg_init_ept(&rp_endpoints[2], "raw", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                 NULL, NULL);

  printf("EP0 @ %X\n\r", (unsigned int)&rp_endpoints[0]);
  printf("EP1 @ %X\n\r", (unsigned int)&rp_endpoints[1]);
  printf("EP2 @ %X\n\r", (unsigned int)&rp_endpoints[2]);

  

  /*
   * The rpmsg service is initiate by the remote processor, on H7 new_service_cb
   * callback is received on service creation. Wait for the callback
   */
  printf("%d\n", HAL_GetTick());
  OPENAMP_Wait_EndPointready(&rp_endpoints[0], HAL_GetTick() + 200);
  OPENAMP_Wait_EndPointready(&rp_endpoints[1], HAL_GetTick() + 50);
  OPENAMP_Wait_EndPointready(&rp_endpoints[2], HAL_GetTick() + 50);
  printf("%d\n", HAL_GetTick());

  // Send first dummy message to enable the channel
  int message = 0x00;
  OPENAMP_send(&rp_endpoints[0], &message, sizeof(message));
  OPENAMP_send(&rp_endpoints[1], &message, sizeof(message));
  OPENAMP_send(&rp_endpoints[2], &message, sizeof(message));

  return 0;
}

int openamp_disable() {
  /* Deinitialize OpenAMP */
  OPENAMP_DeInit();
  return 0;
}

void openamp_raw_send(void* buf, size_t len) {
  OPENAMP_send(&rp_endpoints[2], buf, len);
}

#if 0
void HSEM1_IRQHandler(void)
{
  HAL_HSEM_IRQHandler();
}
#endif
