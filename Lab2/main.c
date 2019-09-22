#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	/*Exercise 1*/
	CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
	CSCTL1 &= ~DCORSEL; // DCORSEL set to 0
	CSCTL1 |= DCOFSEL_3; // enables 8MHz DCO (pg16 ds [ds = datasheet]
//	CSCTL1 |= DCOFSEL1 + DCOFSEL0; //alternate method? to set 8MHz; (pg81 ug) for high high
	CSCTL2 |= SELS__DCOCLK; // sets SMCLK to run off DCO (pg82 ug), 011b = DCOCLK
//	CSCTL2 |= SELS1 + SELS0; // alternate method? to set SMCLK to run off DCO; (pg82 ug) for low high high
	CSCTL3 |= DIVS2 + DIVS0; // sets SMCLK divider to /32 (pg83 ug); 101 = high low high, DIVS1 is low
	P3DIR = BIT3; // change P3.4 direction
	P3OUT |= BIT3; // set P3.4 as output
	P3SEL1 |= BIT3; // enable P3.4 to output SMCLK (pg81 ds)
	P3SEL0 |= BIT3; // enable P3.4 to outputSMCLK (pg81 ds)

	/*Exercise 2*/

	return 0;
}
