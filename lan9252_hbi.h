/*
 * lan9225_hbi.h
 *
 *  Created on: 2022. 3. 24.
 *      Author: YJ
 */

#ifndef LAN9252_HBI_H_
#define LAN9252_HBI_H_

#include "device.h"

// LAN9252 TABLE 5-2
// Before reading the same register or any other register affected by the write
// minimum 45ns is required
#define NS_DELAY_BTW_RW    asm(" RPT #8 || NOP")

// LAN9252 basic functions
extern void lan9252_hbi_init();
extern inline uint16_t lan9252_hbi_direct_read16(uint16_t address);
extern inline void lan9252_hbi_direct_write16(uint16_t address, uint16_t data);

// LAN9252 complex functions
extern uint16_t lan9252_hbi_indirect_read16(uint16_t address);
extern uint32_t lan9252_hbi_indirect_read32(uint16_t address);

extern void lan9252_hbi_indirect_write16(uint16_t address, uint16_t data);
extern void lan9252_hbi_indirect_write32(uint16_t address, uint32_t data);

#endif /* LAN9252_HBI_H_ */
