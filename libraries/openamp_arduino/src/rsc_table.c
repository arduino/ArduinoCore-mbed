/**
  ******************************************************************************
  * @file    rsc_table.c
  * @author  MCD Application Team
  * @brief   Ressource table
  *
  *   This file provides a default resource table requested by remote proc to
  *  load the elf file. It also allows to add debug trace using a shared buffer.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#if defined(__ICCARM__) || defined (__CC_ARM)
#include <stddef.h> /* needed  for offsetof definition*/
#endif
#include "rsc_table.h"
#include "openamp/open_amp.h"

#if defined (__LOG_TRACE_IO_)
extern char system_log_buf[];
#endif

void resource_table_init(int RPMsgRole, void **table_ptr, int *length) {
    (void)RPMsgRole;
    volatile struct shared_resource_table *resource_table = SHM_RSC_ADDR;

    #ifdef CORE_CM7
    memset(resource_table, 0, SHM_RSC_SIZE);
    resource_table->num = 1;
    resource_table->version = 1;
    resource_table->offset[0] = offsetof(struct shared_resource_table, vdev);
    #if defined (__LOG_TRACE_IO_)
    resource_table->offset[1] = offsetof(struct shared_resource_table, cm_trace);
    #endif

    resource_table->vring0.da = VRING_TX_ADDRESS;
    resource_table->vring0.align = VRING_ALIGNMENT;
    resource_table->vring0.num = VRING_NUM_BUFFS;
    resource_table->vring0.notifyid = VRING0_ID;

    resource_table->vring1.da = VRING_RX_ADDRESS;
    resource_table->vring1.align = VRING_ALIGNMENT;
    resource_table->vring1.num = VRING_NUM_BUFFS;
    resource_table->vring1.notifyid = VRING1_ID;

    #if defined (__LOG_TRACE_IO_)
	resource_table->cm_trace.type;
	resource_table->cm_trace.da;
	resource_table->cm_trace.len;
	resource_table->cm_trace.reserved = 0;
	resource_table->cm_trace.name = (uint8_t[]){"cm_trace"};
    #endif

    resource_table->vdev.type = RSC_VDEV;
    resource_table->vdev.id = VIRTIO_ID_RPMSG;
    resource_table->vdev.num_of_vrings=VRING_COUNT;
    resource_table->vdev.dfeatures = (1 << VIRTIO_RPMSG_F_NS);
    #else
    // For CM4, wait until the resource_table is initialized by the host
    while(resource_table->vring1.da != VRING_RX_ADDRESS) {

    }
    #endif

    *length = SHM_RSC_SIZE;
    *table_ptr = resource_table;
}
