/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBEDTLS_LORA_CONFIG_H
#define MBEDTLS_LORA_CONFIG_H

/*
 * Configure mbedtls for LoRa stack.
 *
 * These settings are just a customization of the main mbedtls config
 * mbed-os/features/mbedtls/inc/mbedtls/config.h
 */

// Ensure LoRa required ciphers are enabled
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_CMAC_C

// Reduce ROM usage by optimizing some mbedtls features.
// These are only reference configurations for this LoRa example application.
// Other LoRa applications might need different configurations.
#define MBEDTLS_AES_FEWER_TABLES

#undef MBEDTLS_GCM_C
#undef MBEDTLS_CHACHA20_C
#undef MBEDTLS_CHACHAPOLY_C
#undef MBEDTLS_POLY1305_C

#endif /* MBEDTLS_LORA_CONFIG_H */
