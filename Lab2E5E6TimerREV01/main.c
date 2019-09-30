#include <msp430.h>

// Define global vars for interrupt
unsigned int rise = 0;
unsigned int fall = 0;
int freq = 0; // falling - rising
int mark = 0; // record if rising or falling

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

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
    TB1CCR0 = 2000 - 1; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions; subtract one since it counts more
    TB1CCR1 = 1000; // 50% duty cycle, TB1.1 50%
    TB1CCR2 = 1500; // 25% duty cycle, TB1.2 25%

    // Set P3.4 and P3.5 to be Timer B output and LED output (P1 only has Timer A and no LED); um14, 17
    // BIT4 is TB1.1, BIT5 is TB1.2, should be dimmer
    P3DIR |= BIT4 + BIT5;
    P3OUT &= ~(BIT4 + BIT5);
    P3SEL0 |= BIT4 + BIT5;
    P3SEL1 &= ~(BIT4 + BIT5);

    // Set timer TA0.0 b/c it has it's own vector b/c "special"
    TA0CTL |= TASSEL1 + MC1 + TAIE; // select SMCLK source, initialize continuous mode (ug349) to go up to 0xFFFF, assuming that TA0CCR0 within

    // Set mode (ug351, ug366 diagrams)
    TA0CCTL0 |= CM_3 + SCS + CAP + CCIE; // capture on rise + falling edge, synchronize capture with timer clock, set to capture, enable interrupt
    TA0CCTL0 &= ~(CCIS0 + CCIS1);

    // Set P1.6 to receiver Timer B input (ds7 P1.6 has TA0.0)
    P1DIR &= ~BIT6; // set P1.6 to input ug293
    P1SEL0 |= BIT6;
    P1SEL1 &= ~BIT6;

    _EINT(); // enable global interrupts

    return 0;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
//    if ((TA0CCTL0 & CM_3) == CM_1) // if rising edge
//    {
//        rise = TA0R; // timer counter
//        TA0CCTL0 &= ~CM_1; // clear rising edge capture to capture falling edge next
//        TA0CCTL0 |= CM_2; // falling edge
//    }
//
//    else if ((TA0CCTL0 & CM_3) == CM_2) // if falling edge
//    {
//        if ((TA0CCTL0 & COV) == COV) // check for overflow
//        {
//            rise = rise - 0xFFFF;
//            TA0CCTL0 &= ~COV; // reset overflow flag
//        }
//        fall = TA0R;
//        TA0CCTL0 &= ~CM_2; // clear falling edge capture to capture rising edge next
//        TA0CCTL0 |= CM_1; // rising edge
//
//        freq = fall - rise;
//    }
//
//    TA0CCTL0 &= CCIFG; // clear flag

    int currentFreq = TA0CCR0;
    if (mark == 0)
    {
        rise = currentFreq;
        mark = 1;
    }
    else
    {
        fall = currentFreq;
        mark = 0;
    }
    freq = fall - rise;
    if (freq < 0)
        freq = freq * -1;
}
