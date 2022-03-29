#include "lan9252_hbi.h"

#define ECAT_CSR_CMD    0x0304
#define ECAT_CSR_DATA   0x0300

#define NS_DELAY    asm(" RPT #3 || NOP")

#define ALELO_MASK  (0x01UL << 14)
#define WR_MASK     (0x01UL << 13)
#define RD_MASK     (0x01UL << 7)
#define CS_MASK     (0x01UL << 1)

#define AD_MASK     0x0000FFFF

void lan9252_hbi_init(){

    // Custom GPIO Setup
    // GPIO[0..15] : LAN9252 HBI AD[0..15] (Output / Input)
    // GPIO46 : LAN9252 ALELO (Output)
    // GPIO33 : LAN9252 CS (Output)
    // GPIO39 : LAN9252 RD (Output)
    // GPIO45 : LAN9252 WR (Output)
    volatile uint32_t *gpioA_BaseAddr;
    volatile uint32_t *gpioA_DataReg;

    volatile uint32_t *gpioB_BaseAddr;
    volatile uint32_t *gpioB_DataReg;

    uint32_t gpioA_mask = AD_MASK;
    uint32_t gpioB_mask = CS_MASK | RD_MASK | WR_MASK | ALELO_MASK;

    gpioA_BaseAddr = (uint32_t *)GPIOCTRL_BASE;
    gpioA_DataReg = (uint32_t *)GPIODATA_BASE;

    gpioB_BaseAddr = (uint32_t *)((uintptr_t)GPIOCTRL_BASE) + GPIO_CTRL_REGS_STEP;
    gpioB_DataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) + GPIO_DATA_REGS_STEP;

    EALLOW;

    gpioA_BaseAddr[GPIO_GPxDIR_INDEX] |= gpioA_mask; // Set GPIO[0..15] to output
    gpioA_BaseAddr[GPIO_GPxODR_INDEX] &= ~gpioA_mask; // Set GPIO[0..15] to normal output (not open drain)
    gpioA_BaseAddr[GPIO_GPxPUD_INDEX] |= gpioA_mask; // Disable GPIO[0..15] pull up
    gpioA_BaseAddr[GPIO_GPxQSEL_INDEX] &= ~gpioA_mask; // Set GPIO[0..15] qualification type to sync
    gpioA_DataReg[GPIO_GPxDAT_INDEX] &= ~gpioA_mask; // Set GPIO[0..15] to zero at default

    gpioB_BaseAddr[GPIO_GPxDIR_INDEX] |= gpioB_mask; // Extra pin to output
    gpioB_BaseAddr[GPIO_GPxODR_INDEX] &= ~gpioB_mask; // Set extra pin to normal output (not open drain)
    gpioB_BaseAddr[GPIO_GPxPUD_INDEX] |= gpioB_mask; // Disable extra pin pull up
    gpioB_BaseAddr[GPIO_GPxQSEL_INDEX] &= ~gpioB_mask; // Set extra pin qualification type to sync
    gpioB_DataReg[GPIO_GPxDAT_INDEX] &= ~gpioB_mask; // Set extra pin to zero at default

    EDIS;

}

inline uint16_t lan9252_hbi_direct_read16(uint16_t address){ // only word address access

    uint16_t data;

    // GPIO[0..15] : LAN9252 HBI AD[0..15] (Output / Input)
    // GPIO46 : LAN9252 ALELO (Output)
    // GPIO33 : LAN9252 CS (Output)
    // GPIO39 : LAN9252 RD (Output)
    // GPIO45 : LAN9252 WR (Output)

    volatile uint32_t *gpioA_BaseAddr = (uint32_t *)GPIOCTRL_BASE;
    volatile uint32_t *gpioA_DataReg = (uint32_t *)GPIODATA_BASE;
    volatile uint32_t *gpioB_DataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) + GPIO_DATA_REGS_STEP;

    EALLOW;

    // [[ Phase 1 ]]
    // ALELO (GPIO46) = 1
    // CS (GPIO33) = 0
    // RD (GPIO39) = 0
    // WR (GPIO45) = 0
    // AD[0:15] (GPIO[0:15]) = Address
    gpioA_DataReg[GPIO_GPxSET_INDEX] = (uint32_t)address >> 1;
    gpioA_DataReg[GPIO_GPxCLEAR_INDEX] = ~((uint32_t)address >> 1) & AD_MASK;
    gpioB_DataReg[GPIO_GPxSET_INDEX] = ALELO_MASK;
    gpioB_DataReg[GPIO_GPxCLEAR_INDEX] = CS_MASK | RD_MASK | WR_MASK;
    NS_DELAY;

    // [[ Phase 2 ]]
    // ALELO (GPIO46) = 0
    gpioB_DataReg[GPIO_GPxCLEAR_INDEX] = ALELO_MASK;
    //NS_DELAY; // > 5ns ?

    // [[ Phase 3 ]]
    // AD[0:15] to zero
    // AD[0:15] to input mode
    // CS (GPIO33) = 1
    // RD (GPIO39) = 1
    gpioA_DataReg[GPIO_GPxCLEAR_INDEX] = AD_MASK;
    gpioA_BaseAddr[GPIO_GPxDIR_INDEX] &= ~AD_MASK;
    gpioB_DataReg[GPIO_GPxSET_INDEX] = CS_MASK | RD_MASK;
    NS_DELAY;

    // [[ Phase 4 ]]
    // Read data
    // CS (GPIO33) = 0
    // RD (GPIO39) = 0
    // AD[0:15] to output
    data = (uint16_t)gpioA_DataReg[GPIO_GPxDAT_INDEX];
    gpioB_DataReg[GPIO_GPxCLEAR_INDEX] = CS_MASK | RD_MASK;
    gpioA_BaseAddr[GPIO_GPxDIR_INDEX] |= AD_MASK;

    EDIS;

    return data;

}

inline void lan9252_hbi_direct_write16(uint16_t address, uint16_t data){ // only word address access

    // GPIO[0..15] : LAN9252 HBI AD[0..15] (Output / Input)
    // GPIO46 : LAN9252 ALELO (Output)
    // GPIO33 : LAN9252 CS (Output)
    // GPIO39 : LAN9252 RD (Output)
    // GPIO45 : LAN9252 WR (Output)

    volatile uint32_t *gpioA_DataReg = (uint32_t *)GPIODATA_BASE;
    volatile uint32_t *gpioB_DataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) + GPIO_DATA_REGS_STEP;

    // [[ Phase 1 ]]
    // ALELO (GPIO46) = 1
    // CS (GPIO33) = 0
    // RD (GPIO39) = 0
    // WR (GPIO45) = 0
    // AD[0:15] (GPIO[0:15]) = Address
    gpioA_DataReg[GPIO_GPxSET_INDEX] = (uint32_t)address >> 1;
    gpioA_DataReg[GPIO_GPxCLEAR_INDEX] = ~((uint32_t)address >> 1) & AD_MASK;
    gpioB_DataReg[GPIO_GPxSET_INDEX] = ALELO_MASK;
    gpioB_DataReg[GPIO_GPxCLEAR_INDEX] = CS_MASK | RD_MASK | WR_MASK;
    NS_DELAY;

    // [[ Phase 2 ]]
    // ALELO (GPIO46) = 0
    gpioB_DataReg[GPIO_GPxCLEAR_INDEX] = ALELO_MASK;
    //NS_DELAY; // > 5ns ?

    // [[ Phase 3 ]]
    // AD[0:15] to zero
    // CS (GPIO33) = 1
    // WR (GPIO45) = 1
    gpioA_DataReg[GPIO_GPxCLEAR_INDEX] = AD_MASK;
    gpioB_DataReg[GPIO_GPxSET_INDEX] = CS_MASK | WR_MASK;
    //NS_DELAY;

    // [[ Phase 4 ]]
    // write data
    gpioA_DataReg[GPIO_GPxSET_INDEX] = (uint32_t)data;
    gpioA_DataReg[GPIO_GPxCLEAR_INDEX] = ~((uint32_t)data) & AD_MASK;
    NS_DELAY;

    // [[ Phase 5]
    // CS (GPIO33) = 0
    // WR (GPIO45) = 0
    // AD[0:15] to zero
    gpioB_DataReg[GPIO_GPxCLEAR_INDEX] = CS_MASK | WR_MASK;
    gpioA_DataReg[GPIO_GPxCLEAR_INDEX] = AD_MASK;

}

uint16_t lan9252_hbi_indirect_read16(uint16_t address){ // Figure 12-10

    uint16_t rx;

    lan9252_hbi_direct_write16(ECAT_CSR_CMD, address);
    //NS_DELAY;
    lan9252_hbi_direct_write16(ECAT_CSR_CMD + 2, 0xC004);
    //NS_DELAY;
    rx = lan9252_hbi_direct_read16(ECAT_CSR_DATA);

    return rx;

}

uint32_t lan9252_hbi_indirect_read32(uint16_t address){ // Figure 12-10

    uint16_t rx[2];

    lan9252_hbi_direct_write16(ECAT_CSR_CMD, address);
    //NS_DELAY;
    lan9252_hbi_direct_write16(ECAT_CSR_CMD + 2, 0xC004);
    //NS_DELAY;
    rx[0] = lan9252_hbi_direct_read16(ECAT_CSR_DATA);
    //NS_DELAY;
    rx[1] = lan9252_hbi_direct_read16(ECAT_CSR_DATA + 2);

    return ((uint32_t)rx[1] << 16) | (uint32_t)rx[0];

}
