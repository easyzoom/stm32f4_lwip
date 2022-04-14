#ifndef __CRC_H__
#define __CRC_H__

#include "cmsis_os.h"

extern const uint16_t crc16Table[256];

uint16_t crc16(uint8_t *data, uint32_t length, uint16_t init);

#endif
