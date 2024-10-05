#include <stdint.h>
#include <stdio.h>
#include "easypdk/pdk.h"

#define PULSE_PIN 7

#define USE_ASSEMBLY 0

unsigned char __sdcc_external_startup(void) {
    EASY_PDK_INIT_SYSCLOCK_1MHZ();
    EASY_PDK_CALIBRATE_IHRC(1000000, 3000);
    return 0;
}

void main(void) {
#if !(USE_ASSEMBLY)
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
        // Pulse for about 4uS
        __set1io(PA, PULSE_PIN);
        __nop();
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
#else // USE_ASSEMBLY
    // Below is the optimized code in assembly
    __asm
        mov a, #0
        mov.io __padier, a  // disable pins as wakeup source
        mov.io __pbdier, a  // Also port B needs to be disabled even if it is not connected
                            // to the outside of the package. touching the package can introduce
                            // glitches and wake up the device
        mov.io __inten, a   // Make sure all interrupts are disabled
        mov.io __intrq, a

        mov.io __paph, a    // Disable all pull up resistors
        mov.io __pa, a      // All pins output low

        mov a, #0xFF        // Set all pins output
        mov.io __pac, a

        mov a, #MISC_FAST_WAKEUP_ENABLE // Enable faster wakeup (45 clocks instead of 3000)
        mov.io __misc, a

        // Pulse frequency: ~50KHz / 4 / 256 = ~49Hz
        mov a, #(T16_CLK_ILRC | T16_CLK_DIV4 | T16_INTSRC_8BIT)
        mov.io __t16m, a

        clear p
        stt16 p

    loop:
        // Pulse for about 4uS
        set1.io __pa, #PULSE_PIN
        nop
        nop
        nop
        set0.io __pa, #PULSE_PIN

        // Turn off IHRC to save power
        mov a, #(CLKMD_ENABLE_ILRC | CLKMD_ENABLE_IHRC | CLKMD_ILRC)
        mov.io __clkmd, a
        set0.io __clkmd, #4

        // Wait for the next pulse
        stopexe
        nop

        // Turn on IHRC again
        mov a, #(CLKMD_ENABLE_ILRC | CLKMD_ENABLE_IHRC | CLKMD_IHRC_DIV16)
        mov.io __clkmd, a

        goto loop
    __endasm;
#endif // USE_ASSEMBLY
}
