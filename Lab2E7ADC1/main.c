#include <msp430.h> 
#include <msp430fr5739.h>

volatile unsigned int xAcc = 0;
volatile unsigned int yAcc = 0;
volatile unsigned int zAcc = 0;

/**
 * main.c
 */

/*
1. Configure P2.7 to output high to power the accelerometer.
2. Set up the ADC to sample from ports A12, A13, and A14.
3. Write code to sample data from all three accelerometer axes.
4. Bit shift the 10-bit result to an 8-bit value.
5. Set up a timer interrupt to trigger an interrupt every 40 ms (25 Hz).
6. Using the timer interrupt, transmit the result using the UART with 255 as the start byte. The data packet should look like 255, X-axis, Y-axis, Z-axis. Check that the transmission is active using CCS Terminal/PuTTY/ MECH 423 Serial Communicator.
7. Sample the ADC inside the timer interrupt service routine and transmit the result to the PC. Check the transmission rate using an oscilloscope by probing the UART Tx port, P3.4.
 * */

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
    // Set clock
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__8; // set SMCLK divider to /8

    // Set P2.7 to output HIGH
	P2DIR |= BIT7;
	P2OUT |= BIT7;

	// Enable ADC_B (ug449)
	ADC10CTL0 &= ~ADC10ENC; // initialize ADC10ENC to 0
	ADC10CTL0 |= ADC10ON; // turn on
	ADC10CTL1 |= ADC10SSEL_3; // select SMCLK as source

    _EINT(); // enable global interrupts

	while(1) {
	    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_14; // receive A14 input; AVSS = GND; ug455
	    ADC10CTL0 |= ADC10ENC + ADC10SC; // enable conversion and start ug449
	    while((ADC10IFG & ADC10IFG0) == 0); // wait while flag not set
	    ADC10CTL0 &= ~(ADC10ENC + ADC10SC); // turn off to reconfigure for next input
	    xAcc = ADC10MEM0 >> 2; // copy ADC output to x; bitshift by 2 so 8 bit output instead of 10 bit ADC

	}

	return 0;
}
