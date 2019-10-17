/* Lab 2 Exam Exercise 5 */
/* Not submitted */

#include <msp430.h>
volatile unsigned int circBuffer[50];
volatile unsigned int start=0;
volatile unsigned int end=0;
volatile int state = 1;
volatile unsigned int currentPeriod = 0xFFFF;
volatile int packetProgress = 0;

void setLEDs(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    setLEDs();

    // ######### Configuring the ADC ###########
    // Configure clock source
    CSCTL0_H = 0xA5; // unlock clock registers
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;  // 8MHz DCO
    CSCTL2 = SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // DCO = ACLK = SMCLK = MCLK

    // ######### Power on Accelerometer #########
    P2DIR |= BIT7;
    P2OUT |= BIT7;
    P2SEL0 &= (~BIT7);
    P2SEL1 &= (~BIT7);

    P3DIR |= (BIT4); // P3.4 TB1.1
      P3SEL0 |= BIT4;

    // Configure ports for UART
    P2SEL0 &= ~(BIT0 + BIT1);
    P2SEL1 |= BIT0 + BIT1;

    // ######## Configuring Timer for UART Transmission at 25Hz ########
    TA1CCR0 = 10000; // 40000 * 1us = 40ms period in Toggle mode
    TA1CCTL0 |= OUTMOD_2; // Output = Toggle mode
    TA1CTL |= TASSEL__ACLK + ID_3 + MC_1 + TACLR; // ACLK, fr/8, Up, Interrupt enabled, Enable

    TB1CCR0 = 0xFFFF; // Timer B Period
    TB1CCTL1 = OUTMOD_3 + CCIS_1;  // Set/Reset
    TB1CCR1 = 0xEFFF;     // CCR1 PWM duty cycle  (50%)                        // CCR2 PWM duty cycle
    TB1CTL = TBSSEL__ACLK + MC_1 + ID_3; // 1MHz signal
    TB1CTL |= TBCLR;         // Enable

    // ######## Configuring UART ############
    // Configure UART0 for 9600 Baud
    UCA0CTLW0 = UCSSEL0;                    // Run the UART using ACLK
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900;   // Baud rate = 9600 from an 8 MHz clock
    UCA0BRW = 52;
    UCA0IE |= UCRXIE;
    __enable_interrupt();

    while(1){
        if(state == 2){
            TB1CTL |= TBCLR;         // Enable
            __delay_cycles(800000);
            TB1CCR1 = currentPeriod >> 8;
            __delay_cycles(400000);
            TB1CCR1 = currentPeriod >> 7;
            __delay_cycles(100000);
            TB1CCR1 = currentPeriod >> 6;
            __delay_cycles(100000);
            TB1CCR1 = currentPeriod >> 5;
            __delay_cycles(100000);
            TB1CCR1 = currentPeriod >> 4;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 3;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 2;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 1;
            __delay_cycles(100000);
            TB1CCR1 = currentPeriod;
            __delay_cycles(300000);
        }
        else if(state == 1){
            TB1CTL |= TBCLR;         // Enable
            __delay_cycles(100000);
            TB1CCR1 = currentPeriod;
            __delay_cycles(200000);
            TB1CCR1 = currentPeriod >> 1;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 2;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 3;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 4;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 5;
            __delay_cycles(300000);
            TB1CCR1 = currentPeriod >> 6;
            __delay_cycles(400000);
            TB1CCR1 = currentPeriod >> 7;
            __delay_cycles(400000);
            TB1CCR1 = currentPeriod >> 8;
            __delay_cycles(700000);
            __no_operation();
            }
        else if(state == 4){
//            TB1CTL &= ~TBCLR;         // Enable
        }
    }

}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{

    if(UCA0IFG & UCRXIFG){
        // Update packet progress
        if(UCA0RXBUF == 255){ // If packet-header detected, reset progress
            packetProgress = 1;
        }
        else{
            packetProgress += 1;
        }

        // Fill circular buffer
        if((UCA0RXBUF == 13) && (start != end))
        {
            while (!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = circBuffer[start];
            start++;

            if(start >= 50)
                start == 0;
        }
        else if (UCA0RXBUF != 13){
            if(end >= 50)
                end = 0;

            circBuffer[end] = UCA0RXBUF;
            end++;
        }

        if(packetProgress == 3){
            // Process packet
            while(circBuffer[start] != 255){
                // Find start byte
                start++;
                if(start == 50)
                    start = 0;
            }

            int command, data1;
            command = circBuffer[(start+1)%50];
            data1 = circBuffer[(start+2)%50];
            start += 3;
            start = (start%50);

            // Execute command
            if((command == 1) || (command == 2) || (command == 4)){
                while (!(UCA0IFG & UCTXIFG));
                UCA0TXBUF = command;
                // COMMAND 1 - Dark to Bright
                state = command;
            }

            currentPeriod = (data1 << 8);
            packetProgress == 0;
        }
    }
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
