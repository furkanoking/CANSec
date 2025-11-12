#ifndef TIPLER_H
#define TIPLER_H
#include <sys/types.h>

constexpr int MAX_CANFD_DATA_LEN = 64;

struct CANFDStruct {
    u_int32_t CANID;
    u_int8_t LENGTH;
    u_int8_t FLAGS;
    u_int8_t DATA[MAX_CANFD_DATA_LEN] __attribute__((aligned(8)));
};
#endif