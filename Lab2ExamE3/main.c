/* Lab 2 Exam Exercise 3 */

#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

void setClk(void);
void setTimer(void);
void setLEDs(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    _EINT(); // enable global interrupts

    setLEDs();
    setClk();
    setTimer();

    return 0;
}

void setLEDs() {
    PJDIR |= 0x0F; // 00001111 // unsure LSB
    P3DIR |= 0xF0; // 11110000

    // Toggle to show ready
    PJOUT |= 0x0F;
    P3OUT |= 0xF0;
    __delay_cycles(500000);
    PJOUT &= ~0x0F; // turn off
    P3OUT &= ~0xF0; // turn off
}

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__8; // set SMCLK divider to /8; 1MHz out
}

void setTimer() {
    // Set P3.4 and P3.5 to be Timer B output and LED output; um14, 17
    P3DIR |= BIT4 + BIT5;
    P3OUT &= ~(BIT4 + BIT5);
    P3SEL0 |= BIT4 + BIT5;
    P3SEL1 &= ~(BIT4 + BIT5);

    // Set timer B
    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
    TB1CCTL1 = OUTMOD_3; // set/reset and interrupt enable (ug375, ug366 diagrams)
    TB1CCTL2 = OUTMOD_3; // set capture/compare register to set/reset (ug375)

    // Set 500Hz waves (draw up graph to show)
    TB1CCR0 = 2000 - 1; // = (CLK/divider)/target = (8E6/8)/500; subtract one since it counts more
    TB1CCR1 = 1000; // 50% duty cycle
    TB1CCR2 = 1500; // 25% duty cycle
}
