// Copyright (c) 2024 Arduino SA
// SPDX-License-Identifier: MPL-2.0
#include "RPC.h"

typedef struct {
    const char *name;
    bool ready;
    struct rpmsg_endpoint rpmsg;
} endpoint_t;

typedef enum {
    ENDPOINT_ID_RAW,
    ENDPOINT_ID_RPC,
    ENDPOINT_ID_MAX
} endpoint_id_t;

typedef enum {
    MSGPACK_ID_REQUEST,
    MSGPACK_ID_RESPONSE,
    MSGPACK_ID_NOTIFY,
} msgpack_id_t;

static endpoint_t endpoints[ENDPOINT_ID_MAX] = {
    [ENDPOINT_ID_RAW] = { .name = "raw", .ready = false, .rpmsg = { 0 } }, 
    [ENDPOINT_ID_RPC] = { .name = "rpc", .ready = false, .rpmsg = { 0 } }, 
};

arduino::RPCClass RPC;
static rtos::Mutex mutex;
osThreadId eventHandlerThreadId;

void RPCClass::new_service_cb(struct rpmsg_device *rdev, const char *name, uint32_t dest) {
    uint8_t buffer[1] = {0};
    for (size_t i=0; i<ENDPOINT_ID_MAX; i++) {
        endpoint_t *ep = &endpoints[i];
        if (strcmp(name, ep->name) == 0) {
            OPENAMP_create_endpoint(&ep->rpmsg, name, dest, rpmsg_recv_callback, NULL);
            OPENAMP_send(&ep->rpmsg, buffer, sizeof(buffer));
            break;
        }
    }
}

int RPCClass::rpmsg_recv_callback(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv) {
    #ifdef CORE_CM4
    for (size_t i=0; i<ENDPOINT_ID_MAX; i++) {
        endpoint_t *ep = &endpoints[i];
        if (ept == &ep->rpmsg && !ep->ready) {
            ep->ready = true;
            return 0;
        }
    }
    #endif

    if (ept == &endpoints[ENDPOINT_ID_RAW].rpmsg) {
        // data on raw endpoint
        if (RPC.raw_callback) {
            RPC.raw_callback.call((uint8_t *) data, len);
        } else {
            for (size_t i=0; i<len; i++) {
                RPC.rx_buffer.store_char(((uint8_t *) data)[i]);
            }
        }
        return 0;
    }

    uint8_t msgpack_type = ((uint8_t *) data)[1];
    switch (msgpack_type) {
        case MSGPACK_ID_REQUEST:
        case MSGPACK_ID_NOTIFY:
            RPC.request((uint8_t *) data, len);
            break;
        case MSGPACK_ID_RESPONSE:
            RPC.response((uint8_t *) data, len);
            break;
    }

    return 0;
}

void eventHandler() {
    eventHandlerThreadId = osThreadGetId();
    while (1) {
        osSignalWait(0, osWaitForever);
        mutex.lock();
        OPENAMP_check_for_message();
        mutex.unlock();
    }
}

#ifdef CORE_CM7
static void mpu_config(void) {
	// Disable the MPU
	HAL_MPU_Disable();

	// Disable caching for the shared memory region.
	MPU_Region_InitTypeDef MPU_InitStruct;
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = D3_SRAM_BASE;
	MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER15;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	// Enable the MPU
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

#define return_if_not_ok(x) do { int ret = x ; if (ret != HAL_OK) return; } while (0);

static void cm4_kick() {
    // If CM4 is already booted, disable auto-boot and reset.
    FLASH_OBProgramInitTypeDef OBInit;

    OBInit.Banks = FLASH_BANK_1;
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
    
    bootM4();
}

int RPCClass::begin() {
    mpu_config();

    eventThread = new rtos::Thread(osPriorityHigh, 16*1024, nullptr, "rpc_evt");
    eventThread->start(&eventHandler);
    // Allow the event thread to run once to set the thread ID, and get into a known state.
    osDelay(1);

    // Initialize OpenAmp and libmetal libraries
    if (MX_OPENAMP_Init(RPMSG_HOST, new_service_cb) !=  HAL_OK) {
        return 0;
    }

    // Boot the CM4.
    cm4_kick();

    // Wait for the remote to announce the services with a timeout.
    for (uint32_t millis_start = millis(); ; ) {
        size_t ep_count = 0;
        for (size_t i=0; i<ENDPOINT_ID_MAX; i++) {
            ep_count += (endpoints[i].rdev != NULL);
        }
        if (ep_count == ENDPOINT_ID_MAX) {
            break;
        }
        if ((millis() - millis_start) >= 5000) {
            return 0;
        }
        osDelay(10);
    }
    return 1;
}
#endif

#ifdef CORE_CM4
#if (CM4_BINARY_START >= 0x60000000) && (CM4_BINARY_START < 0xe0000000)
class M4Init {
public:
    M4Init() {
        // If the Cortex-M4 core is booting from SDRAM, the memory region must be
        // configured as Strongly Ordered. Note that the Cortex-M4 core does not
        // seem to implement speculative prefetching, so there is no need to protect
        // the whole region from speculative prefetching with a second MPU region.
        HAL_MPU_Disable();
        MPU_Region_InitTypeDef MPU_InitStruct;
        MPU_InitStruct.Number = MPU_REGION_NUMBER1;
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.BaseAddress = CM4_BINARY_START;
        MPU_InitStruct.Size = MPU_REGION_SIZE_1MB;
        MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
        MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
        MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
        MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
        MPU_InitStruct.SubRegionDisable = 0x00;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);
        HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    }
};

M4Init __m4init __attribute__ ((init_priority (101)));
#endif

int RPCClass::begin() {
    eventThread = new rtos::Thread(osPriorityHigh, 16*1024, nullptr, "rpc_evt");
    eventThread->start(&eventHandler);
    // Allow the event thread to run once to set the thread ID, and get into a known state.
    osDelay(1);

    // Initialize OpenAmp and libmetal libraries
    if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL) !=  0) {
        return 0;
    }

    // Create endpoints.
    for (size_t i=0; i<ENDPOINT_ID_MAX; i++) {
        endpoint_t *ep = &endpoints[i];
        if (OPENAMP_create_endpoint(&ep->rpmsg, ep->name, RPMSG_ADDR_ANY, rpmsg_recv_callback, NULL) < 0) {
            return 0;
        }
    }

    // Wait for endpoints to be ready first by the host before allowing
    // the remote to use the endpoints.
    for (uint32_t millis_start = millis(); ; ) {
        size_t ep_count = 0;
        for (size_t i=0; i<ENDPOINT_ID_MAX; i++) {
            ep_count += endpoints[i].ready;
        }
        if (ep_count == ENDPOINT_ID_MAX) {
            break;
        }
        if ((millis() - millis_start) >= 5000) {
            return 0;
        }
        osDelay(10);
    }

    return 1;
}
#endif

void RPCClass::response(uint8_t *buf, size_t len) {
    unpacker.reset();
    unpacker.reserve_buffer(len);
    memcpy(unpacker.buffer(), buf, len);
    unpacker.buffer_consumed(len);

    RPCLIB_MSGPACK::unpacked result;
    while (unpacker.next(result)) {
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

void RPCClass::request(uint8_t *buf, size_t len) {
    unpacker.reset();
    unpacker.reserve_buffer(len); 
    memcpy(unpacker.buffer(), buf, len);
    unpacker.buffer_consumed(len);

    RPCLIB_MSGPACK::unpacked result;
    while (unpacker.next(result)) {
        auto msg = result.get();
        auto resp = rpc::detail::dispatcher::dispatch(msg, false);
        auto data = resp.get_data();
        if (!resp.is_empty()) {
            endpoint_t *ep = &endpoints[ENDPOINT_ID_RPC];
            OPENAMP_send(&ep->rpmsg, data.data(), data.size());
        }
    }
}

size_t RPCClass::write(uint8_t c) {
    return write(&c, 1, true);
}

void rpc::client::write(RPCLIB_MSGPACK::sbuffer *buffer) {
    RPC.write((const uint8_t *) buffer->data(), buffer->size(), false);
}

size_t RPCClass::write(const uint8_t *buf, size_t len, bool raw) {
    mutex.lock();
    OPENAMP_send(&endpoints[raw ? ENDPOINT_ID_RAW : ENDPOINT_ID_RPC].rpmsg, buf, len);
    mutex.unlock();
    return len;
}
