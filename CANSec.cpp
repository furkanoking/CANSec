#include "CANSec.h"
#include "openssl/aes.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <cstring>
#include <utility>

CANSec::CANSec() {
    NonceValueGenerator();
}

CANSec::~CANSec() = default;

void CANSec::setReceivedEvent(std::function<void()> ReceivedFunction) {
    m_funcReceivedFunction = std::move(ReceivedFunction);
}

void CANSec::receiveMessage() const  {
    if (m_funcReceivedFunction) {
        m_funcReceivedFunction();
    }
}

void CANSec::NonceValueGenerator() {
    if ( RAND_bytes(m_arrNonceValue.data(),(12*sizeof(__uint8_t)) ) != 1 ) {
        std::cerr << "Error generating random bytes for nonce." << std::endl;
    }
}

void CANSec::EncryptMessage(const std::span<__uint8_t> plaintext,
                        const int &plaintext_len,
                        std::span<__uint8_t> ciphertext,
                        int& ciphertext_len,
                        std::span<__uint8_t> tag) const
{
    int len = 0;
    int ciphertext_len_local = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Error creating cipher context." << std::endl;
    }

    // Initialize encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        std::cerr << "Error initializing encryption." << std::endl;
    }

    // Set key and nonce
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, m_arrKey.data(), m_arrNonceValue.data()) != 1) {
        std::cerr << "Error setting key and nonce." << std::endl;
    }

    // Encrypt plaintext
    if (plaintext_len > 0 && EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext_len) != 1) {
            std::cerr << "Error during encryption." << std::endl;
        }
    ciphertext_len_local += len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + ciphertext_len, &len) != 1) {
        std::cerr << "Error finalizing encryption." << std::endl;
    }

    // Calculate the tag
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
        std::cerr << "Error getting tag." << std::endl;
    }

    ciphertext_len_local += len;
    ciphertext_len = ciphertext_len_local;
}

void CANSec::DecryptMessage(const std::span<__uint8_t> ciphertext,
                        const int &ciphertext_len,
                        std::span<__uint8_t> plaintext,
                        int& plaintext_len,
                        std::span<__uint8_t> expected_tag) const
{
    int len = 0;
    int plaintext_len_local = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Error creating cipher context." << std::endl;
    }

    // Initialize decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        std::cerr << "Error initializing decryption." << std::endl;
    }

    // Set key and nonce
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, m_arrKey.data(), m_arrNonceValue.data()) != 1) {
        std::cerr << "Error setting key and nonce." << std::endl;
    }

    // Decrypt ciphertext
    if (ciphertext_len > 0 && EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext_len) != 1) {
            std::cerr << "Error during decryption." << std::endl;
        }
    plaintext_len_local += len;

    // Set expected tag value
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16,  expected_tag.data()) != 1 ) {
        std::cerr << "Error setting tag." << std::endl;
    }

    // Finalize decryption
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_len_local, &len) != 1) {
        std::cerr << "Decryption failed: tag mismatch." << std::endl;
    }

    plaintext_len_local += len;
    plaintext_len = plaintext_len_local;
}