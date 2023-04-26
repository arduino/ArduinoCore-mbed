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

#define SE05X_EC_KEY_RAW_LENGTH          64
#define SE05X_EC_KEY_HEADER_LENGTH       27
#define SE05X_EC_KEY_DER_LENGTH          SE05X_EC_KEY_HEADER_LENGTH + SE05X_EC_KEY_RAW_LENGTH
#define SE05X_EC_SIGNATURE_RAW_LENGTH    64
#define SE05X_EC_SIGNATURE_HEADER_LENGTH 6
#define SE05X_EC_SIGNATURE_DER_LENGTH    SE05X_EC_SIGNATURE_HEADER_LENGTH + SE05X_EC_SIGNATURE_RAW_LENGTH
#define SE05X_SHA256_LENGTH              32
#define SE05X_SN_LENGTH                  18
#define SE05X_DER_BUFFER_SIZE            256
#define SE05X_TEMP_OBJECT                9999

SE05XClass::SE05XClass()
: _cipher_type {kSSS_CipherType_EC_NIST_P}
, _algorithm_type {kAlgorithm_SSS_ECDSA_SHA256}
, _key_size_bits {256}
{

}

SE05XClass::~SE05XClass()
{

}

static void getECKeyXyValuesFromDER(byte* derKey, size_t derLen, byte* rawKey)
{
  memcpy(rawKey, &derKey[derLen - SE05X_EC_KEY_RAW_LENGTH], SE05X_EC_KEY_RAW_LENGTH);
}

static void setECKeyXyVauesInDER(const byte* rawKey, byte* derKey)
{
  static const byte ecc_der_header_nist256[SE05X_EC_KEY_HEADER_LENGTH] =
  {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
    0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
    0x42, 0x00, 0x04
  };

  memcpy(&derKey[0], &ecc_der_header_nist256[0], SE05X_EC_KEY_HEADER_LENGTH);
  memcpy(&derKey[SE05X_EC_KEY_HEADER_LENGTH], &rawKey[0], SE05X_EC_KEY_RAW_LENGTH);
}

static void getECSignatureRsValuesFromDER(byte* derSignature, size_t derLen, byte* rawSignature)
{
  byte rLen;
  byte sLen;

  rLen = derSignature[3];
  sLen = derSignature[3 + rLen + 2];

  byte * out = rawSignature;

  if(rLen == (SE05X_EC_SIGNATURE_RAW_LENGTH / 2))
  {
    memcpy(out, &derSignature[4], (SE05X_EC_SIGNATURE_RAW_LENGTH / 2));
  }
  else if ((rLen == ((SE05X_EC_SIGNATURE_RAW_LENGTH / 2) + 1)) && (derSignature[4] == 0))
  {
    memcpy(out, &derSignature[5], (SE05X_EC_SIGNATURE_RAW_LENGTH / 2));
  }

  out += (SE05X_EC_SIGNATURE_RAW_LENGTH / 2);

  if(sLen == (SE05X_EC_SIGNATURE_RAW_LENGTH / 2))
  {
    memcpy(out, &derSignature[3 + rLen + 3], (SE05X_EC_SIGNATURE_RAW_LENGTH / 2));
  }
  else if ((sLen == ((SE05X_EC_SIGNATURE_RAW_LENGTH / 2) + 1)) && (derSignature[3 + rLen + 3] == 0))
  {
    memcpy(out, &derSignature[3 + rLen + 4], (SE05X_EC_SIGNATURE_RAW_LENGTH / 2));
  }
}

static void setECSignatureRsValuesInDER(const byte* rawSignature, byte* signature)
{
    byte rLen = (SE05X_EC_SIGNATURE_RAW_LENGTH / 2);
    byte sLen = (SE05X_EC_SIGNATURE_RAW_LENGTH / 2);
    byte rawSignatureLen = SE05X_EC_SIGNATURE_RAW_LENGTH;

    signature[0] = 0x30;
    signature[1] = (uint8_t)(rawSignatureLen + 4);
    signature[2] = 0x02;
    signature[3] = (uint8_t)rLen;
    memcpy(&signature[4], &rawSignature[0], rLen);
    signature[3 + rLen + 1] = 0x02;
    signature[3 + rLen + 2] = (uint8_t)sLen;
    memcpy(&signature[3 + rLen + 3], &rawSignature[rLen], sLen);
}

int SE05XClass::begin()
{
    sss_status_t status;

    memset(&_boot_ctx, 0, sizeof(ex_sss_boot_ctx_t));

    se05x_ic_power_on();

    if (nLog_Init() != 0) {
        SE05X_PRINT_ERROR("Lock initialisation failed");
        return 0;
    }

    if (kStatus_SSS_Success != ex_sss_boot_open(&_boot_ctx, "portName")) {
        SE05X_PRINT_ERROR("ex_sss_session_open Failed");
        return 0;
    }

    if (kStatus_SSS_Success != ex_sss_key_store_and_object_init(&_boot_ctx)) {
        SE05X_PRINT_ERROR("ex_sss_key_store_and_object_init Failed");
        return 0;
    }

    return 1;
}

void SE05XClass::end()
{
    se05x_ic_power_off();
}

int SE05XClass::writeConfiguration(const byte data[])
{
    _cipher_type = (sss_cipher_type_t)data[0];
    _algorithm_type = (sss_algorithm_t)(data[1] << 8 | data[2]);
    _key_size_bits = (size_t)(data[3] << 8 | data[4]);
    return 1;
}

int SE05XClass::readConfiguration(byte data[])
{
    data[0] = (byte)_cipher_type;
    data[1] = (byte)_algorithm_type >> 8;
    data[2] = (byte)_algorithm_type;
    data[3] = (byte)_key_size_bits >> 8;
    data[4] = (byte)_key_size_bits;
    return 1;
}

String SE05XClass::serialNumber()
{
    String result = (char*)NULL;
    byte UID[SE05X_SN_LENGTH];
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
    size_t       derSzBits;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Pair, kKeyObject_Mode_Persistent, _cipher_type)) {
        return 0;
    }

    status = sss_key_store_generate_key(&_boot_ctx.ks, &keyObject, _key_size_bits, NULL);

    if (status == kStatus_SSS_Success) {
        derSzBits = pubKeyDerMaxLen * 8;
        * pubKeyDerLen = pubKeyDerMaxLen;
        status = sss_key_store_get_key(&_boot_ctx.ks, &keyObject, pubKeyDer, pubKeyDerLen, &derSzBits);
    }

    if (status != kStatus_SSS_Success) {
        SE05X_PRINT_ERROR("sss_key_store_get_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::generatePrivateKey(int slot, byte publicKey[])
{
    byte publicKeyDer[SE05X_DER_BUFFER_SIZE];
    size_t publicKeyDerLen;

    if ((_cipher_type != kSSS_CipherType_EC_NIST_P) || (_algorithm_type != kAlgorithm_SSS_ECDSA_SHA256)) {
        return 0;
    }

    if (!generatePrivateKey(slot, publicKeyDer, sizeof(publicKeyDer), &publicKeyDerLen)) {
        return 0;
    }

    getECKeyXyValuesFromDER(publicKeyDer, publicKeyDerLen, publicKey);
    return 1;
}

int SE05XClass::generatePublicKey(int keyId, byte pubKeyDer[], size_t pubKeyDerMaxLen, size_t * pubKeyDerlen)
{
    sss_status_t status;
    sss_object_t keyObject;
    size_t       derSzBits;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Pair, kKeyObject_Mode_Persistent, _cipher_type)) {
        return 0;
    }

    derSzBits = pubKeyDerMaxLen * 8;
    * pubKeyDerlen = pubKeyDerMaxLen;
    status = sss_key_store_get_key(&_boot_ctx.ks, &keyObject, pubKeyDer, pubKeyDerlen, &derSzBits);

    if (status != kStatus_SSS_Success) {
        SE05X_PRINT_ERROR("sss_key_store_get_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::generatePublicKey(int slot, byte publicKey[])
{
    byte publicKeyDer[SE05X_DER_BUFFER_SIZE];
    size_t publicKeyDerLen;

    if ((_cipher_type != kSSS_CipherType_EC_NIST_P) || (_algorithm_type != kAlgorithm_SSS_ECDSA_SHA256)) {
        return 0;
    }

    if (!generatePublicKey(slot, publicKeyDer, sizeof(publicKeyDer), &publicKeyDerLen)) {
        return 0;
    }

    getECKeyXyValuesFromDER(publicKeyDer, publicKeyDerLen, publicKey);
    return 1;
}

int SE05XClass::importPublicKey(int keyId, const byte pubKeyDer[], size_t pubKeyDerLen)
{
    sss_status_t        status;
    sss_object_t        keyObject;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Public, kKeyObject_Mode_Persistent, _cipher_type)) {
        return 0;
    }

    status = sss_key_store_set_key(&_boot_ctx.ks, &keyObject, pubKeyDer, pubKeyDerLen, _key_size_bits, NULL, 0);

    if(status != kStatus_SSS_Success )  {
        SE05X_PRINT_ERROR("sss_key_store_set_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::beginSHA256()
{
    sss_status_t        status;

    status = sss_digest_context_init(&_digest_ctx, &_boot_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);

    if (status != kStatus_SSS_Success) {
        SE05X_PRINT_ERROR("sss_digest_context_init Failed!!!");
        return 0;
    }

    status = sss_digest_init(&_digest_ctx);

    if (status != kStatus_SSS_Success) {
        SE05X_PRINT_ERROR("sss_digest_init Failed!!!");
        return 0;
    }

    return 1;
}

int SE05XClass::updateSHA256(const byte in[], size_t inLen)
{
    sss_status_t        status;

    status =  sss_digest_update(&_digest_ctx, in, inLen);

    if (status != kStatus_SSS_Success) {
        SE05X_PRINT_ERROR("sss_digest_update Failed!!!");
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
        SE05X_PRINT_ERROR("sss_digest_context_init Failed!!!");
        return 0;
    }

    * outLen = outMaxLen;
    status = sss_digest_one_go(&_digest_ctx, in, inLen, out, outLen);
    sss_digest_context_free(&_digest_ctx);
    if (status != kStatus_SSS_Success) {
        SE05X_PRINT_ERROR("sss_digest_one_go Failed!!!");
        return 0;
    }

    return 1;
}

int SE05XClass::Sign(int keyId, const byte hash[], size_t hashLen, byte sig[], size_t sigMaxLen, size_t * sigLen)
{
    sss_status_t        status;
    sss_object_t        keyObject;
    sss_asymmetric_t    ctx_asymm;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Private, kKeyObject_Mode_Persistent, _cipher_type)) {
        return 0;
    }

    status = sss_asymmetric_context_init(&ctx_asymm,
                                         &_boot_ctx.session,
                                         &keyObject,
                                         _algorithm_type,
                                         kMode_SSS_Sign);

    if(status != kStatus_SSS_Success)  {
        SE05X_PRINT_ERROR("sss_asymmetric_context_init Failed");
        return 0;
    }

    * sigLen = sigMaxLen;
    if(kStatus_SSS_Success != sss_asymmetric_sign_digest(&ctx_asymm, (uint8_t *)hash, hashLen, (uint8_t *)sig, sigLen)) {
        SE05X_PRINT_ERROR("sss_asymmetric_sign_digest Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::ecSign(int slot, const byte message[], byte signature[])
{
    byte signatureDer[SE05X_DER_BUFFER_SIZE];
    size_t signatureDerLen;

    if ((_cipher_type != kSSS_CipherType_EC_NIST_P) || (_algorithm_type != kAlgorithm_SSS_ECDSA_SHA256)) {
        return 0;
    }

    if (!Sign(slot, message, SE05X_SHA256_LENGTH, signatureDer, sizeof(signatureDer), &signatureDerLen)) {
        return 0;
    }

    /* Get r s values from DER buffer */
    getECSignatureRsValuesFromDER(signatureDer, signatureDerLen, signature);
    return 1;
}

int SE05XClass::Verify(int keyId, const byte hash[], size_t hashLen, const byte sig[], size_t sigLen)
{
    sss_status_t        status;
    sss_object_t        keyObject;
    sss_asymmetric_t    ctx_asymm;

    if(!initObject(keyId, &keyObject, kSSS_KeyPart_Public, kKeyObject_Mode_Persistent, _cipher_type)) {
        return 0;
    }

    status = sss_asymmetric_context_init(&ctx_asymm,
                                         &_boot_ctx.session,
                                         &keyObject,
                                         _algorithm_type,
                                         kMode_SSS_Verify);

    if(status != kStatus_SSS_Success)  {
        SE05X_PRINT_ERROR("sss_asymmetric_context_init Failed");
        return 0;
    }

    if(kStatus_SSS_Success != sss_asymmetric_verify_digest(&ctx_asymm, (uint8_t *)hash, hashLen, (uint8_t *)sig, sigLen)) {
        SE05X_PRINT_ERROR("sss_asymmetric_verify_digest Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::ecdsaVerify(const byte message[], const byte signature[], const byte pubkey[])
{
    byte pubKeyDER[SE05X_EC_KEY_DER_LENGTH];
    byte signatureDER[SE05X_EC_SIGNATURE_DER_LENGTH];
    int  result;

    if ((_cipher_type != kSSS_CipherType_EC_NIST_P) || (_algorithm_type != kAlgorithm_SSS_ECDSA_SHA256)) {
        return 0;
    }

    setECKeyXyVauesInDER(pubkey, pubKeyDER);
    if (!importPublicKey(SE05X_TEMP_OBJECT, pubKeyDER, sizeof(pubKeyDER))) {
        return 0;
    }

    setECSignatureRsValuesInDER(signature, signatureDER);

    result = Verify(SE05X_TEMP_OBJECT, message, SE05X_SHA256_LENGTH, signatureDER, SE05X_EC_SIGNATURE_DER_LENGTH);

    if (!deleteBinaryObject(SE05X_TEMP_OBJECT)) {
        return 0;
    }
    return result;
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
        SE05X_PRINT_ERROR("sss_key_store_get_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::readSlot(int slot, byte data[], int length)
{
    size_t              binSizeBits;
    return readBinaryObject(slot, data, length, &binSizeBits);
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
        SE05X_PRINT_ERROR("sss_key_store_set_key Failed");
        return 0;
    }

    return 1;
}

int SE05XClass::writeSlot(int slot, const byte data[], int length)
{
    if (existsBinaryObject(slot)) {
        if (!deleteBinaryObject(slot)) {
            return 0;
        }
    }
    return writeBinaryObject(slot, data, length);
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
        SE05X_PRINT_ERROR("sss_key_store_erase_key Failed");
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
        SE05X_PRINT_ERROR("sss_key_object_init Failed");
        return 0;
    }

    if(kStatus_SSS_Success != sss_key_object_get_handle(object, objectId)) {
        SE05X_PRINT_ERROR("sss_key_object_get_handle Failed");
        return 0;
    }

    return 1;
}

ex_sss_boot_ctx_t* SE05XClass::getDeviceCtx(void) {
    return &_boot_ctx;
}

int SE05XClass::initObject(size_t objectId, sss_object_t * object, sss_key_part_t objectPart, sss_key_object_mode_t objectMode, sss_cipher_type_t objectChiper) 
{
    if (getObjectHandle(objectId, object)) {
        return 1;
    }

    if(kStatus_SSS_Success != sss_key_object_allocate_handle(object, objectId, objectPart, objectChiper, 0, objectMode)) {
        SE05X_PRINT_ERROR("sss_key_object_allocate_handle Failed");
        return 0;
    }
    return 1;
}


SE05XClass SE05X;
