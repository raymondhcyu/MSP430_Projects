#include <msp430.h> 

/**
 * main.c
 * In class demo 20 September 2019
 * Attempt to blink LED
 * LEDs connected to PJ.0, PJ.1, PJ.3, etc. (check hand notes b/c ports are 8 bits, redundant mapping)
 *
 */
int main(void)
{
    int i;
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// Access J.0 ports and change direction; BIT0 generic for 0x01
	PJDIR |= BIT0 + BIT1 + BIT2 + BIT3;

//    P3DIR |= BIT4;
//    P3DIR |= BIT5;
//    P3DIR |= BIT6;
//    P3DIR |= BIT7;

    // Configure PJ and P3 ports (LEDs) as outputs
	PJOUT |= BIT0 + BIT1 + BIT2 + BIT3;
//	PJOUT |= BIT1;
//    PJOUT |= BIT2;
//    PJOUT |= BIT3;

//    P3OUT |= BIT4;
//    P3OUT |= BIT5;
//    P3OUT |= BIT6;
//    P3OUT |= BIT7;

	while(1) {
	    PJOUT ^= BIT0; // toggle bit
	    for (i = 0; i < 20000; i++)
	        _NOP(); // one way to delay
        PJOUT ^= BIT1;
        for (i = 0; i < 20000; i++)
            _NOP(); // one way to delay
        PJOUT ^= BIT2;
        for (i = 0; i < 20000; i++)
            _NOP(); // one way to delay
        PJOUT ^= BIT3;
        for (i = 0; i < 20000; i++)
            _NOP(); // one way to delay
//        P3OUT ^= BIT4;
//        for (i = 0; i < 20000; i++)
//            _NOP(); // one way to delay
//        P3OUT ^= BIT5;
//        for (i = 0; i < 20000; i++)
//            _NOP(); // one way to delay
//        P3OUT ^= BIT6;
//        for (i = 0; i < 20000; i++)
//            _NOP(); // one way to delay
//        P3OUT ^= BIT7;
	}

	return 0;
}
