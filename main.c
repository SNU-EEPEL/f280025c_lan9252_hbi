#include "driverlib.h"
#include "device.h"
#include "board.h"

#include <stdint.h>

#include "lan9252_hbi.h"

#define EPWM1_TIMER_TBPRD  4999U // 10kHz up-down count = 100MHz / 2 / 10k - 1 = 4999

uint16_t duty, tmp;

__interrupt void sync0InterruptHandler(void){
    duty = (uint16_t)lan9252_hbi_indirect_read16(0x1000);

    // LAN9252 TABLE 5-2
    // Before reading the same register or any other register affected by the write
    // minimum 45ns is required
    NS_DELAY_BTW_RW;

    lan9252_hbi_indirect_write16(0x100C, tmp);

    EPWM_setCounterCompareValue(myEPWM0_BASE,EPWM_COUNTER_COMPARE_B,duty);

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

void main(void)
{
    Device_init();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    Board_init();

    lan9252_hbi_init();

    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // Set-up TBCLK
    EPWM_setTimeBasePeriod(myEPWM0_BASE, EPWM1_TIMER_TBPRD);
    EPWM_setPhaseShift(myEPWM0_BASE, 0U);
    EPWM_setTimeBaseCounter(myEPWM0_BASE, 0U);

    // Set Compare values
    EPWM_setCounterCompareValue(myEPWM0_BASE,
                                EPWM_COUNTER_COMPARE_B,
                                0);

    // Set up counter mode
    EPWM_setTimeBaseCounterMode(myEPWM0_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_disablePhaseShiftLoad(myEPWM0_BASE);
    EPWM_setClockPrescaler(myEPWM0_BASE,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

    // Set up shadowing
    EPWM_setCounterCompareShadowLoadMode(myEPWM0_BASE,
                                         EPWM_COUNTER_COMPARE_B,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);

    // Set actions
    EPWM_setActionQualifierAction(myEPWM0_BASE,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
    EPWM_setActionQualifierAction(myEPWM0_BASE,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);

    // Set Phase shift
    EPWM_enablePhaseShiftLoad(myEPWM0_BASE);
    EPWM_setPhaseShift(myEPWM0_BASE, 0);
    EPWM_setSyncInPulseSource(myEPWM0_BASE,
                              EPWM_SYNC_IN_PULSE_SRC_INPUTXBAR_OUT5);

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // SYNC0 Interrupt enable
    GPIO_setInterruptType(GPIO_INT_XINT1, GPIO_INT_TYPE_RISING_EDGE);
    GPIO_setInterruptPin(sync0GPIO, GPIO_INT_XINT1);
    GPIO_enableInterrupt(GPIO_INT_XINT1);

    Interrupt_register(INT_XINT1, &sync0InterruptHandler);
    Interrupt_enable(INT_XINT1);

    // Enable interrupt
    Interrupt_enableMaster();

    for(;;)
    {
    }
}
