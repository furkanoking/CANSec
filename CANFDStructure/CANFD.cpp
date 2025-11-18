#include "CANFD.h"
#include <sys/socket.h>
#include <net/if.h>
#include <cstring>
#include <iostream>
#include <linux/can.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>

#include "GlobalThreads.h"
#include "Tipler.h"

CANFD::~CANFD() {
    if (m_threadReceive.joinable()) {
        m_threadReceive.join();
    }

    // Clean up resources if necessary
    std::scoped_lock<std::mutex> lock(m_mutexSocketMap);
    for (const auto& [name,socket] : m_mapSocket) {
        close(socket);
    }

}

bool CANFD::setNetworkInterfaceUp(const std::string &interfaceName, const std::string &portName) {
    // Implementation to set up the network interface
    // Validate interface name length to prevent buffer overflow
    if (interfaceName.length() >= IFNAMSIZ) {
        std::cerr << "Interface name too long (max " << (IFNAMSIZ - 1) << " characters): " << interfaceName << std::endl;
        return false;
    }
    
    ifreq the_ifreq{};
    std::strcpy(the_ifreq.ifr_name,interfaceName.c_str());

    int socket_fd = -1;
    {
        std::scoped_lock<std::mutex> lock(m_mutexSocketMap);
        auto it = m_mapSocket.find(portName);
        if (it == m_mapSocket.end()) {
            std::cerr << "Socket not found: " << portName << std::endl;
            return false;
        }
        socket_fd = it->second;
    }

    try {
        if (int interfaceCheck = ioctl(socket_fd, SIOCGIFFLAGS, &the_ifreq); interfaceCheck < 0) {
            std::cerr<<" Interface is not valid"<<std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error checking interface: " << e.what() << std::endl;
        return false;
    }

    //
    sockaddr_can the_sockaddr_can{};
    the_sockaddr_can.can_family = AF_CAN;
    the_sockaddr_can.can_ifindex = the_ifreq.ifr_ifindex;

    if (bind(socket_fd, reinterpret_cast<sockaddr *>(&the_sockaddr_can), sizeof(the_sockaddr_can)) < 0) {
        std::cerr<<"Error binding socket"<<std::endl;
        return false;
    }
    else {
        return true;
    }
}

bool CANFD::CreateSocket(const std::string &socketname) {

    if (const int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW); sock < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }

    else {
        // Enable CAN FD supporta
        if (int enable_canfd = 1; setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd)) < 0) {
            std::cerr << "Error enabling CAN FD support on socket" << std::endl;
            close(sock);
            return false;
        }

        std::scoped_lock<std::mutex> lock(m_mutexSocketMap);
        m_mapSocket[socketname] = sock;
        return true;
    }
}

void CANFD::ReceiveMessage(const std::string &socketname, std::function<void(CANFDStruct)>& callback) {
    // Pass callback by value to avoid use-after-free: the thread gets its own copy
    // Pass socketname by value for the same reason
    std::jthread ThreadListening(&CANFD::ThreadReceiveMessage, this, socketname, callback);

    // Move the thread to the global thread variable. This will ensure that only one background listening thread is active at any time
    backgroundListeningThread = std::move(ThreadListening);
}

void CANFD::ThreadReceiveMessage(const std::string &socketname, std::function<void( CANFDStruct)> callback) {
    int socket_value{-99};

    {
        std::scoped_lock<std::mutex> lock(m_mutexSocketMap);
        try {
            socket_value = m_mapSocket.at(socketname);
        } catch (const std::exception& e) {
            std::cerr << "Error getting socket value: " << e.what() << std::endl;
            return;
        }
    }

    canfd_frame the_listening_frame{};
    CANFDStruct the_CANFD{};

    while (true) {
        if (int nybets_read = read(socket_value, &the_listening_frame, sizeof(canfd_frame)); nybets_read > 0) {
            if (the_listening_frame.can_id != m_iID) {
                std::cout<<"Receiving message is not suitable ID. It will be not read"<<std::endl;
                continue;
            }

            the_CANFD.CANID = the_listening_frame.can_id;
            the_CANFD.LENGTH = the_listening_frame.len;
            the_CANFD.FLAGS = the_listening_frame.flags;
            std::memcpy(the_CANFD.DATA, the_listening_frame.data, the_listening_frame.len);

            // Call customer callback first (if provided) - allows immediate processing
            if (callback) {
                try {
                    callback(the_CANFD);
                } catch (const std::exception& e) {
                    std::cerr << "Error in customer callback: " << e.what() << std::endl;
                }
            }

            // Add received data to queue with mutex protection (for later retrieval if needed)
            setReceievedData(the_CANFD);
        }
    }
}

void CANFD::setReceievedData(const CANFDStruct &data) {
    std::scoped_lock<std::mutex> lock(m_mutexQueue);
    m_queueReceivedData.push(data);
}


void CANFD::SendMessage(const std::string &socketname, const int ID, const int frame_len, const char* data) {
    // Copy data to avoid dangling pointer: the thread gets its own copy
    std::vector<uint8_t> data_copy(data, data + frame_len); //TODO is it necessary? 
    std::jthread ThreadSending(&CANFD::ThreadSendMessage, this, socketname, ID, frame_len, std::move(data_copy));
    backgroundSendingThread = std::move(ThreadSending);
}

void CANFD::ThreadSendMessage(const std::string &socketname, const int ID, const int frame_len, std::vector<uint8_t> data) {
    int socket_value{-99};
    {
        std::scoped_lock<std::mutex> lock(m_mutexSocketMap);
        
        try {
            socket_value = m_mapSocket.at(socketname);
        } catch (const std::exception& e) {
            std::cerr << "Error getting socket value: " << e.what() << std::endl;
            return;
        }
        
        }
    canfd_frame the_sending_frame{};
    the_sending_frame.can_id = ID;
    the_sending_frame.len = frame_len;
    std::memcpy(the_sending_frame.data, data.data(), frame_len);

    std::cout<<"Trying to send message"<<std::endl;

    if (write(socket_value, &the_sending_frame, sizeof(canfd_frame)) < 0) {
        std::cerr << "Error sending message" << std::endl;
        return;
    }
}
