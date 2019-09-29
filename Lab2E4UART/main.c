#include <msp430.h> 
#define RXD BIT0
#define TXD BIT1

/**
 * main.c
 */
int main(void)
{
    int i;
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
	// Set clock
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)

    // Configure P2.0 and P2.1
    P2SEL0 &= ~(RXD + TXD); // set to 00 ds74
    P2SEL1 |= RXD + TXD; // set to 11 ds74

    // Set LED to flash when RX
    PJDIR |= RXD + TXD;
    PJOUT |= RXD + TXD;
    PJOUT &= ~(RXD + TXD);

    // Configure UART
    /*
     * = means set bit high, but everything else low
     * |= means set bit high, but don't touch else
     * ug487 formula
     * N = clock / baud e.g. 8000000/57600 = 138.9
     * UCBRx = int(138.9/16) = 9
     */
    UCA0CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
//    UCA0MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF7; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
    UCA0BRW = 52; // ug490 and ug497, bit clock prescaler
    UCA0IE |= UCRXIE; // enable UART RX interrupt

    _EINT(); // enable global interrupts

    while(1)
    {
        while (!(UCA0IFG & UCTXIFG)); // uca0ifg & uctxifg tells you value of interrupt flag, if 0, not it, then 1
        UCA0TXBUF = 'A'; // char A in transmit buffer, can also put in ASCII
        PJOUT ^= BIT0;
        for (i = 0; i <20000; i++) // delay
            _NOP();
    }

	return 0;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte = 0;
    RxByte = UCA0RXBUF; // get new byte from Rx buffer
    while (!(UCA0IFG & UCTXIFG)); // wait until previous Tx finished
    UCA0TXBUF = RxByte; // "echo back received byte", +1 will give next byte, if type c get d, type d, get e, etc.
    PJOUT ^= BIT0;
}
