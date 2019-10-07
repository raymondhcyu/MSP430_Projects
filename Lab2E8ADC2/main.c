//#include <msp430.h>
//#include <msp430fr5739.h>
//
//volatile unsigned int temperature = 0;
//volatile char result[4];
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
//void setClk(void);
//void setTimer(void);
//void setUART(void);
//void setLEDs();
//void setADC(void);
//
//int main(void)
//{
//	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
//
//    // Set P2.7 to output HIGH to power temperature sensor
//    P2DIR |= BIT7;
//    P2OUT |= BIT7;
//
//	setClk();
//	setTimer();
//	setUART();
//	setLEDs();
//	setADC();
//
//    _EINT(); // enable global interrupts
//
//    while(1) {
//        // Poll temperature sensor
////        PJOUT = BIT0; // turn off all LEDs
////        if (degC > startTemp)
////            PJOUT |= BIT1;
////        if (degC > startTemp + 1)
////            PJOUT |= BIT2;
////        if (degC > startTemp + 2)
////            PJOUT |= BIT3;
////        if (degC > startTemp + 3)
////            P3OUT |= BIT4;
////        if (degC > startTemp + 4)
////            P3OUT |= BIT5;
////        if (degC > startTemp + 5)
////            P3OUT |= BIT6;
////        if (degC > startTemp + 6)
////            P3OUT |= BIT7;
//    }
//
//	return 0;
//}
//
//#pragma vector = ADC10_VECTOR
//__interrupt void ADC10_ISR(void) {
//   ADC10CTL0 &= ~ADC10ENC;
//   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_10;
//   temperature = ADC10MEM0; // already 8 bits set by ADC10CTL2
//   ADC10CTL0 |= ADC10ENC | ADC10SC;
//}
//
//#pragma vector = TIMER1_B1_VECTOR
//__interrupt void TIMER1_B1_ISR(void) {
//    sendInt(temperature);
//    newLine();
//    TB1CCTL1 &= ~CCIFG; // reset flag
//}
//
//void setClk() {
//    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
//    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
//    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
//    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
//    CSCTL3 |= DIVS__8; // set SMCLK divider to /8
//}
//
//void setTimer() {
//    // Set timer B
//    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
//    TB1CCTL1 = OUTMOD_3 + CCIE; // set/reset and interrupt enable (ug375, ug366 diagrams)
//
//    // Set 25Hz waves (draw up graph to show)
//    TB1CCR0 = 40000 - 1; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions; subtract one since it counts more
//    TB1CCR1 = 20000; // 50% duty cycle
//}
//
//void setUART() {
//    // Configure UART on P2.0 and 2.1
//    P2SEL0 &= ~(BIT0 + BIT1); // set to 00 ds74
//    P2SEL1 |= BIT0 + BIT1; // set to 11 ds74
//    UCA0CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
//    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
////    UCA0MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF700; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
//    UCA0BRW = 52; // ug490 and ug497, bit clock prescaler ***Why is this 52 for both 9600 and 57600 baud?
//    UCA0IE |= UCRXIE; // enable UART RX interrupt
//}
//
//void setLEDs() {
//    PJDIR |= 0x0F; // 00001111 // unsure LSB
//    P3DIR |= 0xF0; // 11110000
//
//    // Toggle to show ready
//    PJOUT |= 0x0F;
//    P3OUT |= 0xF0;
//    __delay_cycles(2000000);
//    PJOUT &= ~0x0F; // turn off
//    P3OUT &= ~0xF0; // turn off
//}
//
//void setADC() {
//    // Enable ADC_B (ug449)
//    ADC10CTL0 &= ~ADC10ENC;                        // ensure ENC is clear
//    ADC10CTL0 = ADC10ON + ADC10SHT_5;
//    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
//    ADC10CTL2 &= ~ADC10RES; // 8 bit ADC out
//    ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_10; // ADC10INCH for temperature
//    ADC10IV = 0x00;    // clear all ADC12 channel int flags
//    ADC10IE |= ADC10IE0;  // enable ADC10 interrupts
//
//    ADC10CTL0 |= ADC10ENC | ADC10SC; // start the first sample. If this is not done the ADC10 interrupt will not trigger.
//}

#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

#define CALADC10_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
#define CALADC10_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C

volatile long temp;
volatile long degC;
volatile char charArr[2];

// CHANGE THESE VALUES TO DEMO DIFFERENT SECTION
int demoTemp = 1;
int demoBuffer = 1;
int demoPacket = 0;

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    char* array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // This is important, see the enqueue
    queue->array = (char*) malloc(queue->capacity * sizeof(char));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue* queue)
{  return (queue->size == queue->capacity);  }

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{  return (queue->size == 0); }

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, char item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

// Function to remove an item from queue.
// It changes front and size
char dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    char item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

struct Queue* queue;

void toArray(int number)
    {
        int n = log10(number) + 1;
        int i;
        for ( i = 0; i < n; ++i, number /= 10 )
        {
            charArr[i] = (char)(number % 10);
        }
        __no_operation();
    }

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop Watch Dog timer

    queue = createQueue(50);

    // Configure clocks
    CSCTL0 = 0xA500;                        // Write password to modify CS registers
    CSCTL1 = DCOFSEL0 + DCOFSEL1;           // DCO = 8 MHz
    CSCTL2 = SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO

    // Set LEDs to outputs
   PJDIR = 0x0F;
   P3DIR = 0xF0;
   PJOUT = 0;
   P3OUT = 0;

   // Configure ports for UART
   P2SEL0 &= ~(BIT0 + BIT1);
   P2SEL1 |= BIT0 + BIT1;

   // Configure UART0
   UCA0CTLW0 = UCSSEL0;   // Run the UART using ACLK
   //UCA0MCTLW = UCOS16 + UCBRF1 + UCBRF3 + 0xF700; // Baud rate = 57.6Kb
   UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900;
   UCA0BRW = 52;
   UCA0IE |= UCRXIE; // Enable UART Rx interrupt

   if (demoPacket) {
   // Configure P3.4 and M3.5 to output captures for timerB
       P3SEL1 &= ~BIT4 + ~BIT5;
       P3SEL0 |= BIT4 + BIT5;

       TB1CTL = TBSSEL1 +  MC0; // Up mode and source SMCLK
       TB1CCR0 = 2000;
       TB1CCR1 = 1000;
       TB1CCTL1 = OUTMOD_3;
       //TB1CCR2 = 1500; // 25% duty cycle
       //TB1CCTL2 = OUTMOD_3;

   }

    // Configure the accelerometer
    //P2DIR |= BIT7; // Output high to the accelerometer
    //P2OUT |= BIT7;

    // Configure ADC
    ADC10CTL0 = ADC10SHT_8 + ADC10ON;         // 16 ADC10CLKs; ADC ON,x`erature sample period>30us
    ADC10CTL1 = ADC10SHP + ADC10CONSEQ_0;     // s/w trig, single ch/conv
    ADC10CTL2 = ADC10RES;                     // 10-bit conversion results
    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_10;  // ADC input ch A10 => temp sense

    // Configure internal reference
    while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
    REFCTL0 |= REFVSEL_0+REFON;               // Select internal ref = 1.5V

    ADC10IE |= ADC10IE0; // Interrupt enabled

     __delay_cycles(400);                      // Delay for Ref to settle

     // Global interrupt enable
     _EINT();

    while(1) {
        ADC10CTL0 |= ADC10ENC + ADC10SC;
        __bis_SR_register(LPM4_bits + GIE);     // LPM4 with interrupts enabled
        degC = (temp - CALADC10_15V_30C) *  (85-30)/(CALADC10_15V_85C-CALADC10_15V_30C) +30;


        if (demoTemp) {
            // Turn off all the LEDs (except for first LED)
            PJOUT = BIT0;
            P3OUT = 0;

            int startTemp = 24;

            if (degC > startTemp) {
                PJOUT |= BIT1;
            }

            if (degC > startTemp + 1) {
                PJOUT |= BIT2;
            }

            if (degC > startTemp + 2) {
                PJOUT |= BIT3;
            }

            if (degC > startTemp + 3) {
                P3OUT |= BIT4;
            }

            if (degC > startTemp + 4) {
                P3OUT |= BIT5;
            }

            if (degC > startTemp + 5) {
                P3OUT |= BIT6;
            }

            if (degC > startTemp + 6) {
                P3OUT |= BIT7;
            }

            while (!(UCA0IFG & UCTXIFG));

            charArr[1] = 0;
            charArr[0] = 0;


            int degrees = (int)degC;
            toArray(degrees);

            UCA0TXBUF = charArr[1] + '0';
            while (!(UCA0IFG & UCTXIFG));

            UCA0TXBUF = charArr[0] + '0';
            while (!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = '\n';

            while (!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = '\r';
        }

        if (demoBuffer) {
            // Print out the contents of the circular buffer
            int j;
            for (j = 0; j < queue->size; ++j) {
                while (!(UCA0IFG & UCTXIFG));
                int idx = queue->front + j;
                if (idx >= queue->capacity) {
                    idx -= queue->capacity;
                }
                UCA0TXBUF = queue->array[idx];
            }
            if (j) {
            while (!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = '\n';

            while (!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = '\r';
            }
        }

        if (demoPacket) {
            if (queue->size >= 5) {
                char startByte = dequeue(queue);
                if (startByte == 255) {
                    char commandByte = dequeue(queue);
                    char dataByte1 = dequeue(queue);
                    char dataByte2 = dequeue(queue);
                    char escapeByte = dequeue(queue);

                    if (escapeByte == 1) {
                        dataByte1 = 255;
                    } else if (escapeByte == 2) {
                        dataByte2 = 255;
                    } else if (escapeByte == 3) {
                        dataByte1 = 255;
                        dataByte2 = 255;
                    }
                    int data = dataByte1 << 4 + dataByte2;

                    // Commands 0 to 7 toggle the LEDs
                    if (commandByte == 0) {
                        PJOUT ^= BIT0;
                    } else if (commandByte == 1) {
                        PJOUT ^= BIT1;
                    } else if (commandByte == 2) {
                        PJOUT ^= BIT2;
                    } else if (commandByte == 3) {
                        PJOUT ^= BIT3;
                    } else if (commandByte == 4) {
                        P3OUT ^= BIT4;
                    } else if (commandByte == 5) {
                        P3OUT ^= BIT5;
                    } else if (commandByte == 6) {
                        P3OUT ^= BIT6;
                    } else if (commandByte == 7) {
                        P3OUT ^= BIT7;
                    } else {
                        // If we receive any other command, change the frequency of the timer
                     uint8_t timerCap = 2000000 / data / 2;
                     uint8_t timerSwitch = timerCap / 2;
                     TB1CCR0 = timerCap;
                     TB1CCR1 = timerSwitch;
                    }
                }
            }
        }


        __delay_cycles(100000);
    }

    return 0;
}

// ADC10 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(ADC10IV,12))
  {
    case  0: break;                          // No interrupt
    case  2: break;                          // conversion result overflow
    case  4: break;                          // conversion time overflow
    case  6: break;                          // ADC10HI
    case  8: break;                          // ADC10LO
    case 10: break;                          // ADC10IN
    case 12: temp = ADC10MEM0;
             __bic_SR_register_on_exit(LPM4_bits);
             break;                          // Clear CPUOFF bit from 0(SR)
    default: break;
  }
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte = 0;
    RxByte = UCA0RXBUF;                     // Get the new byte from the Rx buffer
    if (RxByte == '\r') {
       char popByte = dequeue(queue);
       while (!(UCA0IFG & UCTXIFG));
       UCA0TXBUF = popByte;
       while (!(UCA0IFG & UCTXIFG));
       UCA0TXBUF = '\n';
       while (!(UCA0IFG & UCTXIFG));
       UCA0TXBUF = '\r';
    } else {
        enqueue(queue, RxByte);
    }

}
