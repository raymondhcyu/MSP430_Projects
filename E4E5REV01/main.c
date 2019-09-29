//#include <msp430.h>
//
//// Define global vars for interrupt
//volatile unsigned int rise;
//volatile unsigned int fall;
//volatile unsigned int freq = 0; // falling - rising
//volatile unsigned int mark = 0; // record if rising or falling
//volatile int currentFreq;
//
///**
// * main.c
// */
//int main(void)
//{
//  WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
//
//    // Set clock
//    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
//    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
//    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
//    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
//    CSCTL3 |= DIVS__8; // set SMCLK divider to /8
//
//    // Set timer B
//    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
//
//    // Set mode (ug375, ug366 diagrams)
//    TB1CCTL1 = OUTMOD_3; // set capture/compare register to set/reset (ug375)
//    TB1CCTL2 = OUTMOD_3; // set capture/compare register to set/reset (ug375)
//
//    // Set 500Hz waves (draw up graph to show)
//    TB1CCR0 = 2000; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions
//    TB1CCR1 = 1000; // 50% duty cycle, TB1.1 50%
//    TB1CCR2 = 1500; // 25% duty cycle, TB1.2 25%
//
//    // Set P3.4 and P3.5 to be Timer B output and LED output (P1 only has Timer A and no LED); um14, 17
//    // BIT4 is TB1.1, BIT5 is TB1.2, should be dimmer
//    P3DIR |= BIT4 + BIT5;
//    P3OUT &= ~(BIT4 + BIT5);
//    P3SEL0 |= BIT4 + BIT5;
//    P3SEL1 &= ~(BIT4 + BIT5);
//
//    /*Exercise 6: Timer II
//    1. Set up timer A to measure the length of time of a pulse from a rising edge to a falling edge.
//    2. Connect the timer output from the previous exercise to the input of this timer.
//    3. Using the debugger to check the measured 16-bit value.*/
//
//    // Set timer A
//    TA0CTL |= TASSEL1 + MC0; // select SMCLK source, initialize up mode (ug349)
//
//    // Set mode (ug351, ug366 diagrams)
//    TA0CCTL0 |= CM0 + CM1 + SCS + CAP + CCIE; // capture on rise + falling edge, synchronize capture with timer clock, set to capture, enable interrupt
//
//    // Set P1.2 to receiver Timer B input (ds7 P1.2 has TA1.1)
//    P1DIR &= ~BIT6; // set P1.2 to input ug293
//    P1SEL0 |= BIT6;
//    P1SEL1 |= BIT6;
//
//    // Set P3.7 to be freq output
//    P3DIR |= BIT7;
//    P3OUT &= BIT7;
//
////    _EINT(); // enable global interrupts
//    __enable_interrupt(); // Enable Global interrupts
//
//  return 0;
//}
//
//#pragma vector = TIMER0_A0_VECTOR
//__interrupt void Timer_A(void)
//{
//    currentFreq = TA0CCR0;
//    if (mark == 0)
//    {
//        rise = currentFreq;
//        mark = 1;
//        P3OUT ^= BIT7;
//    }
//    else
//    {
//        fall = currentFreq;
//        mark = 0;
//    }
//    freq = fall - rise;
//}

#include <msp430.h>


/**
 * main.c
 */

volatile unsigned int idx = 0;
volatile unsigned int risingEdge = 0;
volatile unsigned int fallingEdge = 0;
volatile int frequency = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Exercise 4: UART
    // 1. Configure the UART to operate at 9600, 8, N, 1.
    // 2. Set up P2.0 and P2.1 for UART communications.
    // 3. Write a program to periodically transmit the letter ‘a’ to the serial port.
    // 4. Check the transmission using the CCS terminal in debug (or PuTTY).
    // 5. Enable UART receive interrupt. Enable global interrupt.
    // 6. Set up an interrupt service routine so that when a single byte is received, the same byte is transmitted
    // by back (or echoed) to the serial port. Check using CCS terminal/PuTTY.
    // 7. In addition to echoing, also transmit the next byte in the ASCII table. Check again using PuTTY.
    // 8. Add code to turn on LED1 when a ‘j’ is received and turn off LED1 when ‘k’ is received.
    // 9. Change the UART speed to 57.6 kbaud and make sure the code in exercise 4.7 and 4.8 still works.

    // CLOCK SETUP
    CSCTL0 = 0xA500; // Password for clock
    CSCTL1 &= ~DCORSEL; // Ensure that DCORSEL is set to 0
    CSCTL1 = DCOFSEL0 + DCOFSEL1; // Set the frequency to 8 MHz
    CSCTL2 = SELA0 + SELA1 + SELS0 + SELS1 + SELM0 + SELM1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO

    // SET LEDS AS OUTPUTS
    PJDIR |= BIT0 + BIT1 + BIT2 + BIT3; // Set them to outputs
    P3DIR |= BIT4 + BIT5 + BIT6 + BIT7; // Set them to outputs

    // Turn off the LEDs
    PJOUT &= ~(BIT0 + BIT1 + BIT1 + BIT3);
    P3OUT &= ~(BIT4 + BIT5 + BIT6 + BIT7);

    // Configure the ports for UART
    P2SEL0 &= ~(BIT0 + BIT1);
    P2SEL1 |= BIT0 + BIT1;

    UCA0CTLW0 = UCSSEL0; // Configure the UART to use ACLK ATTENTION: |= on this wont make it work!
    // Page 490 user manual, for Baud Rate = 9600, 8Mhz
    // UCOS16 = 1
    // UCBRx = 52
    // UCBRFx = 1
    // UCBRSx = 0x49
    UCA0MCTLW |= UCOS16 + UCBRF0 + 0x4900; // Baud = 9600
    //UCA0MCTLW = UCOS16 + UCBRF1 + UCBRF3 + 0xF700; // Baud rate = 57.6Kb
    UCA0BRW = 52;
    UCA0IE |= UCRXIE; // Receive interrupt enable

    // Exercise 5: Timer I
    // 1. Set up Timer B in the “up count” mode.
    // 2. Configure TB1.1 to produce a 500 Hz square wave. You may need to use frequency dividers when setting
    // up the clock and the timer. Output on P3.4. Verify using an oscilloscope. LED5 should also be lit.
    // 3. Configure TB1.2 to produce a 500 Hz square wave at 25% duty cycle. Output on P3.5. Verify using an
    // oscilloscope. LED6 should be lit. Verify that LED6 is dimmer than LED5.

    // CONFIGURE TIMER B 500HZ
    TB1CTL |= TBSSEL1 + MC0; // Set the source as SMCLK and put in up mode
    // By default, SMCLK divides by 8 (page 83 user manual)
    TB1CCR0 = 2000; // (1/500) / (1 / (8000000 / 8)) 500 is desired frequency, 8000000 is DCO freq, 8 is divider for SMCLK
    TB1CCR1 = 1000; // 50% duty cycle
    TB1CCR2 = 1500; // 25% duty cycle
    TB1CCTL1 = OUTMOD_3; // Page 366 user manual shows diagram
    TB1CCTL2 = OUTMOD_3; // set / reset capture mode

    // OUTPUT TIMER B on P3.4 and P3.5
    P3DIR |= BIT4 + BIT5;
    P3SEL0 |= BIT4 + BIT5;
    P3SEL1 &= ~(BIT4 + BIT5);

    // Exercise 6: Timer II
    // 1. Set up timer A to measure the length of time of a pulse from a rising edge to a falling edge.
    // 2. Connect the timer output from the previous exercise to the input of this timer.
    // 3. Using the debugger to check the measured 16-bit value.

    // CONFIGURE TIMER A
    TA0CTL |= TBSSEL1 + MC0; // Source to SMCLK, continuous mode (0xFFFF)
    TA0CCTL0 |= CM0 + CM1 + SCS + CAP + CCIE; // Capture on rising and falling edge, synchronous, set it to capture mode, Interrupt enable,
    // On page 7 of data sheet, we can see that TA0.0 is connected to P1.6

    // Configure P1.6 to capture from timer --> datasheet pg 73
    P1DIR &= ~BIT6; // Set Dir to input
    P1SEL0 |= BIT6;
    P1SEL1 |= BIT6;

    __enable_interrupt(); // Enable Global interrupts

    int j = 0;
    int i = 0;
    while(1) {
        if (++j > 25)
            j = 0;
        while (!(UCA0IFG & UCTXIFG)); // If the buffer is transmitting data,wait
        UCA0TXBUF = 'a' + j;
        for (i=0;i<20000;i++)
            _NOP();
    }

    return 0;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char rxByte = UCA0RXBUF; // Save the Received Byte
    // Toggle LEDS
    if (rxByte == '0') {
        PJOUT ^= BIT0;
    }
    if (rxByte == '1') {
        PJOUT ^= BIT1;
    }
    if (rxByte == '2') {
        PJOUT ^= BIT2;
    }
    if (rxByte == '3') {
        PJOUT ^= BIT3;
    }
    if (rxByte == '4') {
        P3OUT ^= BIT4;
    }
    if (rxByte == '5') {
        P3OUT ^= BIT5;
    }
    if (rxByte == '6') {
        P3OUT ^= BIT6;
    }
    if (rxByte == '7') {
        P3OUT ^= BIT7;
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    int val = TA0CCR0;
    if (idx == 0) {
        risingEdge = val;
        idx = 1;
        PJOUT ^= BIT0;
    } else {
        fallingEdge = val;
        idx = 0;
    }
    frequency = fallingEdge - risingEdge;
}
