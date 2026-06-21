#pragma once

#include <mbedtls/aes.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/md.h>
#include <esp_random.h>
#include <string>
#include <cstring>
#include <algorithm>

class EncryptionUtils
{
public:
    static constexpr size_t KEY_SIZE = 16;
    static constexpr size_t IV_SIZE  = 16;

    // Derives a 128-bit AES key from a password via PBKDF2-HMAC-SHA256.
    // Uses a fixed application salt so all devices derive the same key from the same password.
    // Empty password must never be passed — callers must check EncryptionEnabled() first.
    static void DeriveKey(const std::string& password, uint8_t key[KEY_SIZE])
    {
        static constexpr uint8_t SALT[]     = "CelestialWayfinder-LoRa-v1";
        static constexpr size_t  SALT_LEN   = sizeof(SALT) - 1;
        static constexpr uint32_t ITERATIONS = 10000;

        mbedtls_pkcs5_pbkdf2_hmac_ext(
            MBEDTLS_MD_SHA256,
            reinterpret_cast<const uint8_t*>(password.c_str()), password.size(),
            SALT, SALT_LEN,
            ITERATIONS,
            KEY_SIZE, key);
    }

    // Fills iv[IV_SIZE] with cryptographically random bytes from the ESP32 hardware RNG.
    static void GenerateIV(uint8_t iv[IV_SIZE])
    {
        static_assert(IV_SIZE % 4 == 0, "IV_SIZE must be a multiple of 4");
        for (size_t i = 0; i < IV_SIZE; i += 4)
        {
            uint32_t r = esp_random();
            memcpy(iv + i, &r, 4);
        }
    }

    // AES-128-CBC + PKCS7 padding. out must be at least inLen + 16 bytes.
    static bool Encrypt(const uint8_t* in, size_t inLen,
                        uint8_t* out, size_t& outLen,
                        const uint8_t key[KEY_SIZE], const uint8_t iv[IV_SIZE])
    {
        size_t padLen    = KEY_SIZE - (inLen % KEY_SIZE);
        size_t paddedLen = inLen + padLen;

        if (paddedLen > MAX_PAYLOAD_SIZE) { return false; }

        uint8_t padded[MAX_PAYLOAD_SIZE];
        memcpy(padded, in, inLen);
        memset(padded + inLen, static_cast<uint8_t>(padLen), padLen);

        uint8_t ivCopy[IV_SIZE];
        memcpy(ivCopy, iv, IV_SIZE);

        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_enc(&ctx, key, 128);
        mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, paddedLen, ivCopy, padded, out);
        mbedtls_aes_free(&ctx);

        outLen = paddedLen;
        return true;
    }

    // AES-128-CBC + PKCS7 strip. inLen must be a multiple of 16.
    static bool Decrypt(const uint8_t* in, size_t inLen,
                        uint8_t* out, size_t& outLen,
                        const uint8_t key[KEY_SIZE], const uint8_t iv[IV_SIZE])
    {
        if (inLen == 0 || inLen % KEY_SIZE != 0) { return false; }

        uint8_t ivCopy[IV_SIZE];
        memcpy(ivCopy, iv, IV_SIZE);

        mbedtls_aes_context ctx;
        mbedtls_aes_init(&ctx);
        mbedtls_aes_setkey_dec(&ctx, key, 128);
        mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, inLen, ivCopy, in, out);
        mbedtls_aes_free(&ctx);

        uint8_t padLen = out[inLen - 1];
        if (padLen == 0 || padLen > KEY_SIZE) { return false; }

        outLen = inLen - padLen;
        return true;
    }

private:
    static constexpr size_t MAX_PAYLOAD_SIZE = 528;
};
