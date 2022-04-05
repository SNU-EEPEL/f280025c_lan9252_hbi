/*
 * lan9225_hbi.h
 *
 *  Created on: 2022. 3. 24.
 *      Author: YJ
 */

#ifndef LAN9252_HBI_H_
#define LAN9252_HBI_H_

#include "device.h"

// LAN9252 basic functions
extern void lan9252_hbi_init();
extern inline uint16_t lan9252_hbi_direct_read16(uint16_t address);
extern inline void lan9252_hbi_direct_write16(uint16_t address, uint16_t data);

// LAN9252 complex functions
extern uint16_t lan9252_hbi_indirect_read16(uint16_t address);
extern uint32_t lan9252_hbi_indirect_read32(uint16_t address);

#endif /* LAN9252_HBI_H_ */
