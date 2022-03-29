#include "driverlib.h"
#include "device.h"
#include "board.h"

#include <stdint.h>

#include "lan9252_hbi.h"

#define EPWM1_TIMER_TBPRD  65535U
#define EPWM1_MAX_CMPB     100U

uint32_t duty;

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

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    for(;;)
    {
        duty = lan9252_hbi_indirect_read16(0x1000);
        EPWM_setCounterCompareValue(myEPWM0_BASE,
                                    EPWM_COUNTER_COMPARE_B,
                                    duty);
        SysCtl_delay(1000);
    }
}
