#include "CANFD.h"
#include <iostream>
#include "CANSec.h"
#include <cstring>
#include <span>
#include <iomanip>

void ListeningTry(CANFDStruct myCAN) {
    std::cout<<"Geldi geldi"<<std::endl;
    std::cout<<"ID:"<<myCAN.CANID<<std::endl;
    std::cout<<"Received length: "<<static_cast<int>(myCAN.LENGTH)<<std::endl;

    // Extract ciphertext and tag from received data
    // Format: [ciphertext (variable length)] + [tag (16 bytes)]
    if (myCAN.LENGTH < 16) {
        std::cerr << "Received data too short (need at least 16 bytes for tag)" << std::endl;
        return;
    }

    int ciphertext_len = myCAN.LENGTH - 16;
    std::array<__uint8_t, 32> ciphertext{};
    std::array<__uint8_t, 16> tag{};
    
    // Copy ciphertext
    std::memcpy(ciphertext.data(), myCAN.DATA, ciphertext_len);
    // Copy tag (last 16 bytes)
    std::memcpy(tag.data(), myCAN.DATA + ciphertext_len, 16);

    std::cout << "Extracted ciphertext length: " << ciphertext_len << std::endl;

    CANSec cansec;
    std::array<__uint8_t,32> key = {};
    cansec.setKey(key);
    // Set the same nonce as the sender (must match!)
    std::array<__uint8_t,12> nonce = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
    cansec.setNonce(nonce);

    std::array<__uint8_t,32> plaintext{};
    int plaintext_len = 0;

    cansec.DecryptMessage(
        std::span<__uint8_t>(ciphertext.data(), ciphertext_len),
        ciphertext_len,
        std::span<__uint8_t>(plaintext.data(), plaintext.size()),
        plaintext_len,
        std::span<__uint8_t>(tag.data(), tag.size())
    );

    std::cout << "Decrypted plaintext length: " << plaintext_len << std::endl;
    std::cout << "Plaintext (hex): ";
    for (int i = 0; i < plaintext_len; i++) {
        std::cout << std::hex << static_cast<int>(plaintext[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

int main() {
    CANFD my_CANFD;
    my_CANFD.CreateSocket("AleynamSocket");
    my_CANFD.setNetworkInterfaceUp("Aleyna","AleynamSocket");

    my_CANFD.setID(0x13);

    my_CANFD.ReceiveMessage("AleynamSocket", ListeningTry);

    while (1) {

    }
}