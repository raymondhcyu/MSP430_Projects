#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	/*Exercise 1*/
	CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
	CSCTL1 |= DCOFSEL3; // enables 8MHz DCO (pg16 ds [ds = datasheet])
	CSCTL2 |= SELS__DCOCLK; // sets SMCLK to run off DCO (pg82 ug), 011b = DCOCLK (unsure how to set this another way)
	CSCTL3 &= ~DIVS__5; // sets divider to /32 or 2^5 (pg83 ug)
	P3DIR |= BIT3; // change P3.4 direction
	P3OUT |= BIT3; // set P3.4 as output

	return 0;
}
