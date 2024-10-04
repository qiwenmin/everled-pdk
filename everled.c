#include <stdint.h>
#include <stdio.h>
#include "easypdk/pdk.h"

#define PULSE_PIN 7

unsigned char __sdcc_external_startup(void) {
    EASY_PDK_INIT_SYSCLOCK_1MHZ();
    EASY_PDK_CALIBRATE_IHRC(1000000, 3000);
    return 0;
}

void main(void) {
    PADIER = 0; // disable pins as wakeup source
    PBDIER = 0; // Also port B needs to be disabled even if it is not connected
                // to the outside of the package. touching the package can introduce
                // glitches and wake up the device
    INTEN = 0;  // Make sure all interrupts are disabled
    INTRQ = 0;

    PAPH = 0;   // Disable all pull up resistors
    PA = 0;     // All pins output low
    PAC = 0xFF; // Set all pins output

    MISC = MISC_FAST_WAKEUP_ENABLE; // Enable faster wakeup (45 clocks instead of 3000)

    // Pulse frequency: ~50KHz / 4 / 256 = ~49Hz
    T16M = T16_CLK_ILRC | T16_CLK_DIV4 | T16_INTSRC_8BIT;
    T16C = 0;

    for (;;) {
        // Pulse for about 3uS
        __set1io(PA, PULSE_PIN);
        __nop();
        __nop();
        __set0io(PA, PULSE_PIN);

        // Turn off IHRC to save power
        EASY_PDK_INIT_SYSCLOCK_ILRC();
        CLKMD &= ~CLKMD_ENABLE_IHRC;

        // Wait for the next pulse
        __stopexe();

        // Turn on IHRC again
        EASY_PDK_INIT_SYSCLOCK_1MHZ();
    }
}
