#include <msp430.h>


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    // Set clock
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__8; // set SMCLK divider to /8

    // Set timer B
    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)

    // Set timer A
    TA1CTL |= TASSEL1 + MC0; // select SMCLK source, initialize up mode (ug349)

    // Set output modes (ug366 diagrams)
    TB1CCTL1 = OUTMOD_3; // set capture/compare register to set/reset (ug375)
    TB1CCTL2 = OUTMOD_3; // set capture/compare register to set/reset (ug375)

    // Set 500Hz waves (draw up graph to show)
    TB1CCR0 = 2000; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions
    TB1CCR1 = 1000; // 50% duty cycle, TB1.1 50%
    TB1CCR2 = 1500; // 25% duty cycle, TB1.2 25%

    // Set P3 to be Timer B output and LED output (P1 only has Timer A and no LED); um14, 17
    // BIT4 is TB1.1, BIT5 is TB1.2, should be dimmer
    P3DIR |= BIT4 + BIT5;
    P3OUT &= ~(BIT4 + BIT5);
    P3SEL0 |= BIT4 + BIT5;
    P3SEL1 &= ~(BIT4 + BIT5);

	return 0;
}
