/*
  SE05X.cpp
  Copyright (c) 2022 Arduino SA.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "SE05X.h"

SE05XClass::SE05XClass()
{

}

SE05XClass::~SE05XClass()
{

}

int SE05XClass::begin()
{
    sss_status_t status;

    memset(&_boot_ctx, 0, sizeof(ex_sss_boot_ctx_t));

    se05x_ic_power_on();

    if (nLog_Init() != 0) {
        LOG_E("Lock initialisation failed");
        return 0;
    }

    if (kStatus_SSS_Success != ex_sss_boot_open(&_boot_ctx, "portName")) {
        LOG_E("ex_sss_session_open Failed");
        return 0;
    }

    if (kStatus_SSS_Success != ex_sss_key_store_and_object_init(&_boot_ctx)) {
        LOG_E("ex_sss_key_store_and_object_init Failed");
        return 0;
    }

    return 1;
}

void SE05XClass::end()
{
    se05x_ic_power_off();
}

String SE05XClass::serialNumber()
{
    String result = (char*)NULL;
    byte UID[18];
    size_t uidLen = 18;

    sss_session_prop_get_au8(&_boot_ctx.session, kSSS_SessionProp_UID, UID, &uidLen);

    result.reserve(uidLen*2);

    for (int i = 0; i < uidLen; i++) {
        byte b = UID[i];

        if (b < 16) {
          result += "0";
        }
        result += String(b, HEX);
    }

    result.toUpperCase();

    return result;
}

long SE05XClass::random(long max)
{
  return random(0, max);
}

long SE05XClass::random(long min, long max)
{
  if (min >= max)
  {
    return min;
  }

  long diff = max - min;

  long r;
  random((byte*)&r, sizeof(r));

  if (r < 0) {
    r = -r;
  }

  r = (r % diff);

  return (r + min);
}

int SE05XClass::random(byte data[], size_t length)
{
    sss_rng_context_t rng;

    if(kStatus_SSS_Success != sss_rng_context_init(&rng, &_boot_ctx.session)) {
        return 0;
    }

    if(kStatus_SSS_Success != sss_rng_get_random(&rng, data, length)) {
        return 0;
    }

    return 1;
}

int SE05XClass::generatePrivateKey(int keyId, byte pubKeyDer[], size_t pubKeyDerMaxLen, size_t * pubKeyDerLen)
{
    sss_status_t status;
    sss_object_t keyObject;
    size_t       keySizeBits;
    size_t       derSzBits;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Pair, kKeyObject_Mode_Persistent, kSSS_CipherType_EC_NIST_P)) {
        return 0;
    }

    keySizeBits = 256;
    status = sss_key_store_generate_key(&_boot_ctx.ks, &keyObject, keySizeBits, NULL);

    if (status == kStatus_SSS_Success) {
        derSzBits = pubKeyDerMaxLen * 8;
        * pubKeyDerLen = pubKeyDerMaxLen;
        status = sss_key_store_get_key(&_boot_ctx.ks, &keyObject, pubKeyDer, pubKeyDerLen, &derSzBits);
    }

    if (status != kStatus_SSS_Success) {
        LOG_E("sss_key_store_get_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::generatePublicKey(int keyId, byte pubKeyDer[], size_t pubKeyDerMaxLen, size_t * pubKeyDerlen)
{
    sss_status_t status;
    sss_object_t keyObject;
    size_t       derSzBits;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Pair, kKeyObject_Mode_Persistent, kSSS_CipherType_EC_NIST_P)) {
        return 0;
    }

    derSzBits = pubKeyDerMaxLen * 8;
    * pubKeyDerlen = pubKeyDerMaxLen;
    status = sss_key_store_get_key(&_boot_ctx.ks, &keyObject, pubKeyDer, pubKeyDerlen, &derSzBits);

    if (status != kStatus_SSS_Success) {
        LOG_E("sss_key_store_get_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::importPublicKey(int keyId, const byte pubKeyDer[], size_t pubKeyDerLen)
{
    sss_status_t        status;
    sss_object_t        keyObject;
    size_t              keySizeBits;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Public, kKeyObject_Mode_Persistent, kSSS_CipherType_EC_NIST_P)) {
        return 0;
    }

    keySizeBits = 256;
    status = sss_key_store_set_key(&_boot_ctx.ks, &keyObject, pubKeyDer, pubKeyDerLen, keySizeBits, NULL, 0);

    if(status != kStatus_SSS_Success )  {
        LOG_E("sss_key_store_set_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::beginSHA256()
{
    sss_status_t        status;

    status = sss_digest_context_init(&_digest_ctx, &_boot_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);

    if (status != kStatus_SSS_Success) {
        LOG_E("sss_digest_context_init Failed!!!");
        return 0;
    }

    return 1;
}

int SE05XClass::updateSHA256(const byte in[], size_t inLen)
{
    sss_status_t        status;

    status =  sss_digest_update(&_digest_ctx, in, inLen);

    if (status != kStatus_SSS_Success) {
        LOG_E("sss_digest_update Failed!!!");
        return 0;
    }

    return 1;
}

int SE05XClass::endSHA256(byte out[], size_t * outLen)
{
    sss_status_t        status;

    status = sss_digest_finish(&_digest_ctx, out, outLen);
    sss_digest_context_free(&_digest_ctx);
    if (status != kStatus_SSS_Success) {
        return 0;
    }

    return 1;
}

int SE05XClass::SHA256(const byte in[], size_t inLen, byte out[], size_t outMaxLen, size_t * outLen)
{
    sss_status_t        status;

    status = sss_digest_context_init(&_digest_ctx, &_boot_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);
    if (status != kStatus_SSS_Success) {
        LOG_E("sss_digest_context_init Failed!!!");
        return 0;
    }

    * outLen = outMaxLen;
    status = sss_digest_one_go(&_digest_ctx, in, inLen, out, outLen);
    sss_digest_context_free(&_digest_ctx);
    if (status != kStatus_SSS_Success) {
        LOG_E("sss_digest_one_go Failed!!!");
        return 0;
    }

    return 1;
}

int SE05XClass::Sign(int keyId, const byte hash[], size_t hashLen, byte sig[], size_t sigMaxLen, size_t * sigLen)
{
    sss_status_t        status;
    sss_object_t        keyObject;
    sss_asymmetric_t    ctx_asymm;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Private, kKeyObject_Mode_Persistent, kSSS_CipherType_EC_NIST_P)) {
        return 0;
    }

    status = sss_asymmetric_context_init(&ctx_asymm,
                                         &_boot_ctx.session,
                                         &keyObject,
                                         kAlgorithm_SSS_ECDSA_SHA256,
                                         kMode_SSS_Sign);

    if(status != kStatus_SSS_Success)  {
        LOG_E("sss_asymmetric_context_init Failed");
        return 0;
    }

    * sigLen = sigMaxLen;
    if(kStatus_SSS_Success != sss_asymmetric_sign_digest(&ctx_asymm, (uint8_t *)hash, hashLen, (uint8_t *)sig, sigLen)) {
        LOG_E("sss_asymmetric_sign_digest Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::Verify(int keyId, const byte hash[], size_t hashLen, byte sig[], size_t sigLen)
{
    sss_status_t        status;
    sss_object_t        keyObject;
    sss_asymmetric_t    ctx_asymm;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Public, kKeyObject_Mode_Persistent, kSSS_CipherType_EC_NIST_P)) {
        return 0;
    }

    status = sss_asymmetric_context_init(&ctx_asymm,
                                         &_boot_ctx.session,
                                         &keyObject,
                                         kAlgorithm_SSS_ECDSA_SHA256,
                                         kMode_SSS_Verify);

    if(status != kStatus_SSS_Success)  {
        LOG_E("sss_asymmetric_context_init Failed");
        return 0;
    }

    if(kStatus_SSS_Success != sss_asymmetric_verify_digest(&ctx_asymm, (uint8_t *)hash, hashLen, (uint8_t *)sig, sigLen)) {
        LOG_E("sss_asymmetric_verify_digest Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::readBinaryObject(int objectId, byte data[], size_t dataMaxLen, size_t * length)
{
    sss_status_t        status;
    sss_object_t        binObject;
    size_t              binSizeBits;

    if(!initObject(objectId, &binObject, kSSS_KeyPart_Default, kKeyObject_Mode_Persistent, kSSS_CipherType_Binary)) {
        return 0;
    }

    * length = dataMaxLen;
    status = sss_key_store_get_key(&_boot_ctx.ks, &binObject, data, length, &binSizeBits);
    if(status != kStatus_SSS_Success )  {
        LOG_E("sss_key_store_get_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::writeBinaryObject(int objectId, const byte data[], size_t length)
{
    sss_status_t        status;
    sss_object_t        binObject;

    if(!initObject(objectId, &binObject, kSSS_KeyPart_Default, kKeyObject_Mode_Persistent, kSSS_CipherType_Binary)) {
        return 0;
    }

    status = sss_key_store_set_key(&_boot_ctx.ks, &binObject, data, length, length * 8, NULL, 0);
    if(status != kStatus_SSS_Success )  {
        LOG_E("sss_key_store_set_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::existsBinaryObject(int objectId)
{
    sss_object_t        binObject;

    if(!getObjectHandle(objectId, &binObject)) {
        return 0;
    }

    return 1;
}

int SE05XClass::deleteBinaryObject(int objectId)
{
    sss_status_t        status;
    sss_object_t        binObject;

    if(!initObject(objectId, &binObject, kSSS_KeyPart_Default, kKeyObject_Mode_Persistent, kSSS_CipherType_Binary)) {
        return 0;
    }

    status = sss_key_store_erase_key(&_boot_ctx.ks, &binObject);
    if(status != kStatus_SSS_Success )  {
        LOG_E("sss_key_store_erase_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::deleteAllObjects(void)
{
    sss_se05x_session_t *pSession = (sss_se05x_session_t *)&_boot_ctx.session;

    if(SW_OK != Se05x_API_DeleteAll_Iterative(&pSession->s_ctx)) {
        return 0;
    }

    return 1;
}

int SE05XClass::getObjectHandle(int objectId, sss_object_t * object)
{
    sss_status_t status;

    if(kStatus_SSS_Success != sss_key_object_init(object, &_boot_ctx.ks)) {
        LOG_E("sss_key_object_init Failed");
        return 0;
    }

    if(kStatus_SSS_Success != sss_key_object_get_handle(object, objectId)) {
        LOG_E("sss_key_object_get_handle Failed");
        return 0;
    }

    return 1;
}

ex_sss_boot_ctx_t* SE05XClass::getDeviceCtx(void) {
    return &_boot_ctx;
}

int SE05XClass::initObject(size_t objectId, sss_object_t * object, sss_key_part_t objectPart, sss_key_object_mode_t objectMode, sss_cipher_type_t objectChiper) 
{
    sss_status_t status;

    if(kStatus_SSS_Success != sss_key_object_init(object, &_boot_ctx.ks)) {
        LOG_E("sss_key_object_init Failed");
        return 0;
    }

    status = sss_key_object_get_handle(object, objectId);

    if(status != kStatus_SSS_Success )  {
        LOG_E("sss_key_object_get_handle Failed");
        status = sss_key_object_allocate_handle(object,
                                                objectId,
                                                objectPart,
                                                objectChiper,
                                                0, // Unused
                                                objectMode);
    }

    if(status != kStatus_SSS_Success)  {
        LOG_E("sss_key_object_allocate_handle Failed");
        return 0;
    }

    return 1;
}


SE05XClass SE05X;
