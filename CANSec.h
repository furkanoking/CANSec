#ifndef CANSECKOD_LIBRARY_H
#define CANSECKOD_LIBRARY_H
#include <functional>
#include <array>
#include <span>

class CANSec {
public:

    CANSec();
    ~CANSec();

    void sendMessage(const int iID, const bool bIsEncrypted, const unsigned char* cpData, const int iDataLength);
    void setReceivedEvent(std::function<void()> ReceivedFunction);
private:

    std::function<void()> m_funcReceivedFunction; // Callback function for received messages
    void receiveMessage() const ;
    int m_iCounter{0};
    void EncryptMessage(const std::span<__uint8_t> plaintext, const int& plaintext_len, std::span<__uint8_t> ciphertext, int& ciphertext_len,std::span<__uint8_t> tag) const;
    void NonceValueGenerator();
    std::array<__uint8_t,12> m_arrNonceValue{};
    std::array<__uint8_t,32> m_arrKey{};

    void DecryptMessage(const std::span<__uint8_t> ciphertext, const int& ciphertext_len, std::span<__uint8_t> plaintext, int& plaintext_len, const std::span<__uint8_t> expected_tag) const;


};
#endif //CANSECKOD_LIBRARY_H