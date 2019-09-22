#include <msp430.h> 

/**
 * main.c
 */
int main(void)
{
    int i;
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	/*Exercise 1*/
/*
	CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
	CSCTL1 &= ~DCORSEL; // DCORSEL set to 0
	CSCTL1 |= DCOFSEL_3; // enables 8MHz DCO (pg16 ds [ds = datasheet]
//	CSCTL1 |= DCOFSEL0 + DCOFSEL1; // alternate method to set 8MHz; (pg81 ug) for high high
	CSCTL2 |= SELS__DCOCLK; // sets SMCLK to run off DCO (pg82 ug), 011b = DCOCLK
//	CSCTL2 |= SELS0 + SELS1; // alternate method to set SMCLK to run off DCO; (pg82 ug) for low high high
	CSCTL3 |= DIVS0 + DIVS2; // sets SMCLK divider to /32 (pg83 ug); 101 = high low high, DIVS1 is low
//	CSCTL3 |= DIVS__32; // alternate method to /32
	P3DIR |= BIT4; // change P3.4 direction
	P3SEL1 |= BIT4; // enable P3.4 to output SMCLK (pg81 ds)
	P3SEL0 |= BIT4; // enable P3.4 to outputSMCLK (pg81 ds)
*/
	/*Test to get output on TP10*/
//	PJDIR = BIT0;
//	PJSEL1 &= ~BIT0;
//	PJSEL0 |= BIT0;

	/*Exercise 2*/
/*
//	PJDIR |= BIT0 + BIT1 + BIT2 + BIT3; // change PJ directions
//	P3DIR |= BIT4 + BIT5 + BIT6 + BIT7; // change P3 directions
    PJDIR |= 0x0F; // 00001111 // unsure LSB
    P3DIR |= 0xF0; // 11110000

//	PJOUT |= BIT0 + BIT3;
//	P3OUT |= BIT6 + BIT7;
    PJOUT |= 0x09; // 00001001
    P3OUT |= 0xC0; // 11000000

	while(1) {
	    PJOUT ^= 0x06; // 00000110
	    P3OUT ^= 0x30; // 00110000
//	    PJOUT ^= BIT1;
//	    PJOUT ^= BIT2;
//	    P3OUT ^= BIT4;
//	    P3OUT ^= BIT5;

        for (i = 0; i < 20000; i++)
            _NOP(); // one way to delay
	}
*/

	/*Exercise 3*/
    P4DIR |= BIT0;
    P4IN |= BIT0;
	return 0;
}
