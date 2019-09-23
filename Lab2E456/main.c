#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
    /*Exercise 4*/
    /*
1. Configure the UART to operate at 9600, 8, N, 1.
2. Set up P2.0 and P2.1 for UART communications.
3. Write a program to periodically transmit the letter ‘a’ to the serial port.
4. Check the transmission using the CCS terminal in debug (or PuTTY).
5. Enable UART receive interrupt. Enable global interrupt.
6. Set up an interrupt service routine so that when a single byte is received, the same byte is transmitted by back (or echoed) to the serial port. Check using CCS terminal/PuTTY.
7. In addition to echoing, also transmit the next byte in the ASCII table. Check again using PuTTY.
8. Add code to turn on LED1 when a ‘j’ is received and turn off LED1 when ‘k’ is received.
9. Change the UART speed to 57.6 kbaud and make sure the code in exercise 4.7 and 4.8 still works.
     */
    // UART diagram: ug477, N formula ug487
	// Set async clock
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for high high
    CSCTL2 |= SELA0 + SELA1; // set ACLK to run off DCO; (pg82 ug) for low high high
    CSCTL3 |= DIVA0 + DIVA2; // sets ACLK divider to /32 (pg83 ug); 101 = high low high, DIVS1 is low
    UCA0CTLW0 = UCSSEL0; // 01b for async (pg495 ug)

	return 0;
}
