#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

volatile unsigned int rise = 0;
volatile unsigned int fall = 0;
volatile int freq = 0; // falling - rising
volatile int mark = 0; // record if rising or falling

volatile int state = 0; // for singled LED switch cycling

void newLine() {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\n';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\r';
}

char error[] = "Error";
// Sauce: https://forum.43oh.com/topic/1643-g2553-hardware-uart-hello-world-example/
void errorMessage(void) {
    unsigned int i = 0;
    for (i = 0; i < 5; i++){
            while (!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = error[i];
        }
}

void setClk(void);
void setLEDs(void);
void setUART(void);
void setTimer(void);
void setCapture(void);

// Output SMCLK, MCLK, and ACLK on PJ.0, PJ.1, PJ.2
void test1(void);
// Cycle LEDs one at a time
void test2(void);
// Incrementally flash single LED
void flashIt(void);
// Switch trigger action; cycle single LED to MSB with each button press (high to low)
void switchIt(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    _EINT(); // enable global interrupts

    setClk();
    setLEDs();

//    test1();
//    test2();
    switchIt();
//    setUART();
//    setTimer();
//    setCapture();
//    flashIt();

//    while(1) {
//        while (!(UCA0IFG & UCTXIFG)); // wait until the previous Tx is finished
//        UCA0TXBUF = freq;
//        newLine();
//        PJOUT ^= BIT0;
//        __delay_cycles(100000); // to avoid spamming serial reader
//    }

    return 0;
}

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__32; // set SMCLK divider to /32
}

void setLEDs() {
    PJDIR |= 0x0F; // 00001111 // unsure LSB
    P3DIR |= 0xF0; // 11110000

    // Toggle to show ready
    PJOUT |= 0x0F;
    P3OUT |= 0xF0;
    __delay_cycles(1000000);
    PJOUT &= ~0x0F; // turn off
    P3OUT &= ~0xF0; // turn off
}

void test1() {
    PJSEL1 &= ~BIT0;
    PJSEL0 |= BIT0;

    PJSEL1 &= ~BIT1;
    PJSEL0 |= BIT1;

    PJSEL1 &= ~BIT2;
    PJSEL0 |= BIT2;
}

void test2() {
//    PJOUT |= 0x09; // 1001
//    P3OUT |= 0xC0; // 1100 with MSB being LED7 and LED6 so C
//    PJOUT |= BIT0 + BIT3;
//    P3OUT |= BIT6 + BIT7;

    const unsigned long flashDelay = 500000;
    int count = 1;

    PJOUT |= BIT0;

    while (PJOUT != BIT3) {
        __delay_cycles(flashDelay);
        PJOUT = PJOUT << 1;
    }

    PJOUT &= ~0x0F;
    P3OUT |= BIT4;

    while (P3OUT != BIT7) {
        __delay_cycles(flashDelay);
        P3OUT = P3OUT << 1;
    }

    __delay_cycles(flashDelay);
    P3OUT &= ~0xF0;

//    for (count; count > 0; count--) {
//        PJOUT |= BIT0;
//        __delay_cycles(flashDelay);
//        PJOUT &= ~BIT0;
//        PJOUT |= BIT1;
//        __delay_cycles(flashDelay);
//        PJOUT &= ~BIT1;
//        PJOUT |= BIT2;
//        __delay_cycles(flashDelay);
//        PJOUT &= ~BIT2;
//        PJOUT |= BIT3;
//        __delay_cycles(flashDelay);
//        PJOUT &= ~BIT3;
//    }
}

void switchIt(void) {
    P4DIR &= ~BIT1;
    P4OUT |= BIT1;
    P4SEL0 &= ~BIT1; // ds84
    P4SEL1 &= ~BIT1;
    P4REN |= BIT1; // enable pullup

    P4IES |= BIT1; // high to low transition
    P4IE |= BIT1; // enable interrupts
    P4IFG &= ~BIT1; // reset flag

    // Single cycle stuff
    PJOUT |= BIT0;
}

#pragma vector = PORT4_VECTOR; // Port_4 b/c P4
__interrupt void Port4_ISR(void) {
    if (PJOUT != BIT3)
        PJOUT = PJOUT << 1;

    else if (PJOUT == BIT3) {
        PJOUT &= ~0x0F;
        P3OUT |= BIT4;
        state = 1;
    }

    if (state == 0) {
        if (P3OUT != BIT7) {
            P3OUT = P3OUT << 1;
        }

        else if (P3OUT == BIT7) {
            P3OUT &= ~BIT7;
            PJOUT |= BIT0;
        }
    }

    state = 0;
    P4IFG &= ~BIT1; // clear flag
}

unsigned int i;

void setUART(void) {
    // Configure UART on P2.0 and 2.1
    P2SEL0 &= ~(BIT0 + BIT1); // set to 00 ds74
    P2SEL1 |= BIT0 + BIT1; // set to 11 ds74
    UCA0CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
//    UCA0MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF700; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
    UCA0BRW = 52; // ug490 and ug497, bit clock prescaler ***Why is this 52 for both 9600 and 57600 baud?
    UCA0IE |= UCRXIE; // enable UART RX interrupt
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte = 0;

    RxByte = UCA0RXBUF; // get val from RX buffer
//    RxByte = RxByte + 1; // cycle next code
//    UCA0TXBUF = RxByte; // "echo back received byte"

    while (!(UCA0IFG & UCTXIFG)); // wait until the previous Tx is finished
    if (RxByte == 'j')
        PJOUT ^= BIT0;

    if (RxByte == 'k')
        PJOUT ^= BIT1;

    if (RxByte == 'l') {
        P3OUT ^= BIT7;
        newLine();
        errorMessage();
    }
}

void setTimer() {
    // Set P3.4 to be Timer B output and LED output; um14, 17
    P3DIR |= BIT4;
    P3OUT &= ~(BIT4);
    P3SEL0 |= BIT4;
    P3SEL1 &= ~(BIT4);

    // Set timer B
    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
    TB1CCTL1 = OUTMOD_3; // set/reset and interrupt enable (ug375, ug366 diagrams)
    TB1CCTL2 = OUTMOD_3; // set capture/compare register to set/reset (ug375)

    // Set 1000Hz waves (draw up graph to show)
    TB1CCR0 = 250 - 1; // = (CLK/divider)/target = (8E6/32)/1000; subtract one since it counts more
    TB1CCR1 = 125; // 50% duty cycle
//    TB1CCR2 = 37500; // 25% duty cycle
}

void setCapture() {
    // NOTE: Need to connect jumper from P3.4 or P3.5 to P1.6 to receive clock
    TA0CTL |= TASSEL1 + MC1 + TAIE; // select SMCLK source, initialize continuous mode (ug349) to go up to 0xFFFF, assuming that TA0CCR0 within

    TA0CCTL0 |= CM_3 + SCS + CAP + CCIE; // capture on rise + falling edge, synchronize capture with timer clock, set to capture, enable interrupt
    TA0CCTL0 &= ~(CCIS0 + CCIS1);

    // Set P1.6 to receiver Timer B input (ds7 P1.6 has TA0.0)
    P1DIR &= ~BIT6; // set P1.6 to input ug293
    P1SEL0 |= BIT6;
    P1SEL1 &= ~BIT6;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
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

unsigned int flashFreq = 5;

void flashIt(void) {
    while (1) {
        TB1CCR0 = (250000 / flashFreq) - 1; // = (CLK/divider)/target = (8E6/32)/5; subtract one since it counts more
        TB1CCR1 = TB1CCR0 / 2; // 50% duty cycle

        __delay_cycles(200000);

        flashFreq = flashFreq + 5;
        if (flashFreq > 100) {
            flashFreq = 5;
        }
    }

}
