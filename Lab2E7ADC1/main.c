//#include <msp430.h>
//#include <msp430fr5739.h>
//
//volatile unsigned int xAcc = 0;
//volatile unsigned int yAcc = 0;
//volatile unsigned int zAcc = 0;
//
//volatile char result[4];
//volatile unsigned int shouldSendData = 0; // marker
//
///* Sauce to convert int to string for UART: https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/291574?CCS-function-for-Integer-to-string-conversion-for-UART-output*/
//void itoa(long unsigned int value, volatile char* result, int base) {
//      // Check that base is valid
//      if (base < 2 || base > 36) { *result = '\0';}
//
//      char* ptr = result, *ptr1 = result, tmp_char;
//      int tmp_value;
//
//      do {
//        tmp_value = value;
//        value /= base;
//        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
//      } while ( value );
//
//      // Apply negative sign
//      if (tmp_value < 0) *ptr++ = '-';
//      *ptr-- = '\0';
//      while(ptr1 < ptr) {
//        tmp_char = *ptr;
//        *ptr--= *ptr1;
//        *ptr1++ = tmp_char;
//      }
//}
//
//void sendInt(int num) {
//    itoa(num, result, 10);
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = result[0];
//
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = result[1];
//
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = result[2];
//
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = result[3];
//}
//
//void sendComma() {
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = ',';
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = ' ';
//}
//
//void newLine() {
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = '\n';
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = '\r';
//}
//
///**
// * main.c
// */
//
///*
//1. Configure P2.7 to output high to power the accelerometer.
//2. Set up the ADC to sample from ports A12, A13, and A14.
//3. Write code to sample data from all three accelerometer axes.
//4. Bit shift the 10-bit result to an 8-bit value.
//5. Set up a timer interrupt to trigger an interrupt every 40 ms (25 Hz).
//6. Using timer interrupt, transmit result using UART with 255 as start byte. Data packet should look like 255, X-axis, Y-axis, Z-axis. Check that transmission is active serial terminal.
//7. Sample ADC inside timer interrupt service routine and transmit result to the PC. Check transmission rate using an oscilloscope by probing the UART Tx port, P3.4.
// * */
//
//int main(void)
//{
//	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
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
//    TB1CCTL1 = OUTMOD_3 + CCIE; // set/reset and interrupt enable (ug375, ug366 diagrams)
//
//    // Set 25Hz waves (draw up graph to show)
//    TB1CCR0 = 40000 - 1; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions; subtract one since it counts more
//    TB1CCR1 = 20000; // 50% duty cycle
//
//    // Set P2.7 to output HIGH
//	P2DIR |= BIT7;
//	P2OUT |= BIT7;
//
//	// Set P3.0, 3.1, 3.2 to output A12, A13, A14
//	P3SEL0 |= BIT0 + BIT1 + BIT2; // ds80
//	P3SEL1 |= BIT0 + BIT1 + BIT2;
//
//    // Set P3.4 to be Timer B output for verification (ds81)
//    P3DIR |= BIT4;
//    P3OUT &= ~BIT4;
//    P3SEL0 |= BIT4;
//    P3SEL1 &= ~BIT4;
//
//	// Enable ADC_B (ug449)
//	ADC10CTL0 &= ~ADC10ENC; // initialize ADC10ENC to 0
//	ADC10CTL0 |= ADC10ON; // turn on
//	ADC10CTL1 |= ADC10SSEL_3; // select SMCLK as source
//
//	// Configure UART
//    P2SEL0 &= ~(BIT0 + BIT1); // set to 00 ds74
//    P2SEL1 |= BIT0 + BIT1; // set to 11 ds74
//    UCA0CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
//    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
////    UCA0MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF700; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
//    UCA0BRW = 52; // ug490 and ug497, bit clock prescaler ***Why is this 52 for both 9600 and 57600 baud?
//    UCA0IE |= UCRXIE; // enable UART RX interrupt
//
//    _EINT(); // enable global interrupts
//
//	while(1) {
//	    // Set ADC to receive A12, 13, 14 inputs
//	    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_14; // receive A14 input; AVSS = GND; ug455
//	    ADC10CTL0 |= ADC10ENC + ADC10SC; // enable conversion and start ug449
//	    while((ADC10IFG & ADC10IFG0) == 0); // wait while flag not set
//	    ADC10CTL0 &= ~(ADC10ENC + ADC10SC); // turn off to reconfigure for next input
//	    xAcc = ADC10MEM0 >> 2; // copy ADC output to x; bitshift by 2 so 8 bit output instead of 10 bit ADC
//
//        ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_13; // receive A13 input; AVSS = GND; ug455
//        ADC10CTL0 |= ADC10ENC + ADC10SC; // enable conversion and start ug449
//        while((ADC10IFG & ADC10IFG0) == 0); // wait while flag not set
//        ADC10CTL0 &= ~(ADC10ENC + ADC10SC); // turn off to reconfigure for next input
//        yAcc = ADC10MEM0 >> 2; // copy ADC output to x; bitshift by 2 so 8 bit output instead of 10 bit ADC
//
//        ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_12; // receive A12 input; AVSS = GND; ug455
//        ADC10CTL0 |= ADC10ENC + ADC10SC; // enable conversion and start ug449
//        while((ADC10IFG & ADC10IFG0) == 0); // wait while flag not set
//        ADC10CTL0 &= ~(ADC10ENC + ADC10SC); // turn off to reconfigure for next input
//        zAcc = ADC10MEM0 >> 2; // copy ADC output to x; bitshift by 2 so 8 bit output instead of 10 bit ADC
//	}
//
//	return 0;
//}
//
//#pragma vector = TIMER1_B1_VECTOR
//__interrupt void TIMER1_B1_ISR(void) {
//    sendInt(255); // start bit 255
//    sendComma();
//    sendInt(xAcc);
//    sendComma();
//    sendInt(yAcc);
//    sendComma();
//    sendInt(zAcc);
//    newLine();
//    TB1CCTL1 &= ~CCIFG; // reset flag
//}

#include <msp430.h> 
#include <msp430fr5739.h>

volatile unsigned int x = 0;
volatile unsigned int y = 0;
volatile unsigned int z = 0;


volatile char result[4];

volatile unsigned int shouldSendData = 0;

void itoa(long unsigned int value, volatile char* result, int base) {
      // check that the base if valid
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

int main(void)
{

    // Exercise 7: ADC I
        // 1. Configure P2.7 to output high to power the accelerometer.
        // 2. Set up the ADC to sample from ports A12, A13, and A14.
        // 3. Write code to sample data from all three accelerometer axes.
        // 4. Bit shift the 10-bit result to an 8-bit value.
        // 5. Set up a timer interrupt to trigger an interrupt every 40 ms (25 Hz).
        // 6. Using the timer interrupt, transmit the result using the UART with 255 as the start byte. The data packet
        // should look like 255, X-axis, Y-axis, Z-axis. Check that the transmission is active using CCS
        // Terminal/PuTTY/ MECH 423 Serial Communicator.
        // 7. Sample the ADC inside the timer interrupt service routine and transmit the result to the PC. Check the
        // transmission rate using an oscilloscope by probing the UART Tx port, P3.4.
        //8. Test your MSP430 code using your C# program from Lab 1.

    // CLOCK SETUP
    CSCTL0 = 0xA500; // Password for clock
    CSCTL1 = DCOFSEL0 + DCOFSEL1; // Set the frequency to 8 MHz
    CSCTL2 = SELA0 + SELA1 + SELS0 + SELS1 + SELM0 + SELM1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO

    // Configure the ports for UART
    P2SEL0 &= ~(BIT0 + BIT1);
    P2SEL1 |= BIT0 + BIT1;

    UCA0CTLW0 = UCSSEL0; // Configure the UART to use ACLK ATTENTION: |= on this wont make it work!
    UCA0MCTLW |= UCOS16 + UCBRF0 + 0x4900; // Baud = 9600
    //UCA0MCTLW = UCOS16 + UCBRF1 + UCBRF3 + 0xF700; // Baud rate = 57.6Kb
    UCA0BRW = 52;
    UCA0IE |= UCRXIE; // Receive interrupt enable

    // Output high to the Accel
    P2DIR = BIT7;
    P2OUT = BIT7;

    ADC10CTL0 &= ~(ADC10ENC); // Ensure that ADC10ENC is 0
    ADC10CTL1 |= ADC10SSEL0; // Set ACLK as source
    ADC10CTL2 = 0;
    ADC10CTL0 |= ADC10ON;


    //setup timer for 40ms interrupt
    TB0CTL = TBSSEL_2 + ID__1 + MC_1;   //Set clock to read from SMCLK, divide by 1, operate in up mode
    TB0CCTL1 = CCIE;
    TB0CCR0 = 40000;                     //Every time it counts 1000 it switches the bit, so a count of 2000 is a period. At 1MHZ -> 500HZ
    TB0CCR1 = 20000;

__enable_interrupt();             //enable Global interrupts
    int i = 0;
while (1) {
    ADC10MCTL0 = ADC10INCH_14 + ADC10SREF_1; // set the input channel to 14
    ADC10CTL0 |= ADC10ENC + ADC10SC; // Start converting
    ADC10CTL0 &= ~(ADC10SC);
    while((ADC10IFG & ADC10IFG0)==0); // wait for converter to finish
    ADC10CTL0 &= ~(ADC10ENC);
    x = ADC10MEM0; // copy the converted value to x

    ADC10MCTL0 = ADC10INCH_13 + ADC10SREF_1; // set the input channel to 13
    ADC10CTL0 |= ADC10ENC + ADC10SC; // Start converting
    ADC10CTL0 &= ~(ADC10SC);
    while((ADC10IFG & ADC10IFG0)==0); // wait for converter to finish
    ADC10CTL0 &= ~(ADC10ENC);
    y = ADC10MEM0; // copy the converted value to y

    ADC10MCTL0 = ADC10INCH_12 + ADC10SREF_1; // set the input channel to 12
    ADC10CTL0 |= ADC10ENC + ADC10SC; // Start converting
    ADC10CTL0 &= ~(ADC10SC);
    while((ADC10IFG & ADC10IFG0)==0); // wait for converter to finish
    ADC10CTL0 &= ~(ADC10ENC);
    z = ADC10MEM0; // copy the converted value to z
    }
return 0;
}

#pragma vector = TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void){
    sendInt(255);
    sendComma();
    sendInt(x);
    sendComma();
    sendInt(y);
    sendComma();
    sendInt(z);
    newLine();
    TB0CCTL1 &= ~BIT0; // reset interrupt flag
}

