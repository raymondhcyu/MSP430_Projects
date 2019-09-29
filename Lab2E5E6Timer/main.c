#include <msp430.h>

// Define global vars for interrupt
volatile unsigned int rise;
volatile unsigned int fall;
volatile unsigned int freq = 0; // falling - rising
volatile unsigned int mark = 0; // record if rising or falling
volatile int currentFreq;

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

    // Set mode (ug375, ug366 diagrams)
    TB1CCTL1 = OUTMOD_3; // set capture/compare register to set/reset (ug375)
    TB1CCTL2 = OUTMOD_3; // set capture/compare register to set/reset (ug375)

    // Set 500Hz waves (draw up graph to show)
    TB1CCR0 = 2000; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions
    TB1CCR1 = 1000; // 50% duty cycle, TB1.1 50%
    TB1CCR2 = 1500; // 25% duty cycle, TB1.2 25%

    // Set P3.4 and P3.5 to be Timer B output and LED output (P1 only has Timer A and no LED); um14, 17
    // BIT4 is TB1.1, BIT5 is TB1.2, should be dimmer
    P3DIR |= BIT4 + BIT5;
    P3OUT &= ~(BIT4 + BIT5);
    P3SEL0 |= BIT4 + BIT5;
    P3SEL1 &= ~(BIT4 + BIT5);

    // Set timer A
    TA1CTL |= TASSEL1 + MC0; // select SMCLK source, initialize up mode (ug349)

    // Set mode (ug351, ug366 diagrams)
    TA1CCTL1 = CM0 + CM1 + SCS + CAP + CCIE; // capture on rise + falling edge, synchronize capture with timer clock, set to capture, enable interrupt

    // Set P1.2 to receiver Timer B input (ds7 P1.2 has TA1.1)
    P1DIR &= ~BIT2; // set P1.2 to input ug293
    P1SEL0 &= ~BIT2;
    P1SEL1 = BIT2;

    // Set P3.7 to be freq output
    P3DIR |= BIT7;
    P3OUT &= BIT7;

    _EINT(); // enable global interrupts

	return 0;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A(void)
{
    currentFreq = TA1CCR0;
    if (mark == 0)
    {
        rise = currentFreq;
        mark = 1;
        P3OUT ^= BIT7;
    }
    else
    {
        fall = currentFreq;
        mark = 0;
    }
    freq = fall - rise;
}
