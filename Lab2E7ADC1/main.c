#include <msp430.h>
#include <msp430fr5739.h>

volatile unsigned int xAcc = 0;
volatile unsigned int yAcc = 0;
volatile unsigned int zAcc = 0;

volatile char result[4];
volatile unsigned int shouldSendData = 0; // marker

/* Sauce to convert int to string for UART: https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/291574?CCS-function-for-Integer-to-string-conversion-for-UART-output*/
void itoa(long unsigned int value, volatile char* result, int base) {
      // Check that base is valid
      if (base < 2 || base > 36) { *result = '\0';}

      char* ptr = result, *ptr1 = result, tmp_char;
      int tmp_value;

      do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
      } while ( value );

      // Apply negative sign
      if (tmp_value < 0) *ptr++ = '-';
      *ptr-- = '\0';
      while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
      }
}

void sendInt(int num) {
    itoa(num, result, 10);
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[0];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[1];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[2];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[3];
}

void sendComma() {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = ',';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = ' ';
}

void newLine() {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\n';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\r';
}

/**
 * main.c
 */

/*
1. Configure P2.7 to output high to power the accelerometer.
2. Set up the ADC to sample from ports A12, A13, and A14.
3. Write code to sample data from all three accelerometer axes.
4. Bit shift the 10-bit result to an 8-bit value.
5. Set up a timer interrupt to trigger an interrupt every 40 ms (25 Hz).
6. Using timer interrupt, transmit result using UART with 255 as start byte. Data packet should look like 255, X-axis, Y-axis, Z-axis. Check that transmission is active serial terminal.
7. Sample ADC inside timer interrupt service routine and transmit result to the PC. Check transmission rate using an oscilloscope by probing the UART Tx port, P3.4.
 * */

void setClk(void);
void setTimer(void);
void setUART(void);
void readX(void);
void readY(void);
void readZ(void);

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	setClk();
	setTimer();
	setUART();

    // Set P2.7 to output HIGH
	P2DIR |= BIT7;
	P2OUT |= BIT7;

	// Set P3.0, 3.1, 3.2 to output A12, A13, A14
	P3DIR &= ~(BIT0 + BIT1 + BIT2);
	P3SEL0 |= BIT0 + BIT1 + BIT2; // ds80
	P3SEL1 |= BIT0 + BIT1 + BIT2;

    // Set P3.4 to be Timer B output for verification (ds81)
    P3DIR |= BIT4;
    P3OUT &= ~BIT4;
    P3SEL0 |= BIT4;
    P3SEL1 &= ~BIT4;

	// Enable ADC_B (ug449)
	ADC10CTL0 &= ~ADC10ENC; // initialize ADC10ENC to 0
	ADC10CTL0 |= ADC10ON; // turn on
	ADC10CTL1 |= ADC10SSEL_3; // select SMCLK as source
	ADC10CTL2 &= ~ADC10RES; // 8 bit resolution ug453

    _EINT(); // enable global interrupts

	while(1) {
        // Set ADC to receive A12, 13, 14 inputs
        readX();
//      readY();
//      readZ();
	    }

	return 0;
}

#pragma vector = TIMER1_B1_VECTOR
__interrupt void TIMER1_B1_ISR(void) {
    sendInt(255); // start bit 255
    sendComma();
    sendInt(xAcc);
//    sendComma();
//    sendInt(yAcc);
//    sendComma();
//    sendInt(zAcc);
    newLine();
    TB1CCTL1 &= ~CCIFG; // reset flag
}

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__8; // set SMCLK divider to /8
}

void setTimer() {
    // Set timer B
    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
    TB1CCTL1 = OUTMOD_3 + CCIE; // set/reset and interrupt enable (ug375, ug366 diagrams)

    // Set 25Hz waves (draw up graph to show)
    TB1CCR0 = 40000 - 1; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions; subtract one since it counts more
    TB1CCR1 = 20000; // 50% duty cycle
}

void setUART() {
    // Configure UART on P2.0 and 2.1
    P2SEL0 &= ~(BIT0 + BIT1); // set to 00 ds74
    P2SEL1 |= BIT0 + BIT1; // set to 11 ds74
    UCA0CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
//    UCA0MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF700; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
    UCA0BRW = 52; // ug490 and ug497, bit clock prescaler ***Why is this 52 for both 9600 and 57600 baud?
    UCA0IE |= UCRXIE; // enable UART RX interrupt
}

void readX() {
    ADC10CTL0 &= ~ADC10ENC; // initialize ADC10ENC to 0
    xAcc = ADC10MEM0 >> 2; // copy ADC output to x; bitshift by 2 so 8 bit output instead of 10 bit ADC
    ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_14; // receive A14 input; AVSS = GND; ug455
    ADC10CTL0 |= ADC10ENC + ADC10SC; // enable conversion and start ug449
//    while((ADC10CTL1 & ADC10BUSY) != 0);
    while((ADC10IFG & ADC10IFG0) == 0); // wait while flag not set
//    ADC10CTL0 &= ~(ADC10ENC + ADC10SC); // turn off to reconfigure for next input
}

void readY() {
    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_13; // receive A13 input; AVSS = GND; ug455
    yAcc = ADC10MEM0; // copy ADC output to y; bitshift by 2 so 8 bit output instead of 10 bit ADC
    ADC10CTL0 |= ADC10ENC + ADC10SC; // enable conversion and start ug449
    while((ADC10IFG & ADC10IFG0) == 0); // wait while flag not set
    ADC10CTL0 &= ~(ADC10ENC + ADC10SC); // turn off to reconfigure for next input
}

void readZ() {
    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_12; // receive A12 input; AVSS = GND; ug455
    zAcc = ADC10MEM0; // copy ADC output to z; bitshift by 2 so 8 bit output instead of 10 bit ADC
    ADC10CTL0 |= ADC10ENC + ADC10SC; // enable conversion and start ug449
    while((ADC10IFG & ADC10IFG0) == 0); // wait while flag not set
    ADC10CTL0 &= ~(ADC10ENC + ADC10SC); // turn off to reconfigure for next input
}
