#ifndef _MDIO_H_
#define _MDIO_H_

#include <stdint.h>


extern uint16_t mdio_read_regs(uint8_t phyAddr, uint8_t regAddr, uint16_t* data);
extern uint16_t mdio_write_regs(uint8_t phyAddr, uint8_t regAddr, uint16_t data);

#endif

