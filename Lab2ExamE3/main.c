/* Lab 2 Exam Exercise 3 */

#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

volatile uint16_t accTotal = 0;
volatile unsigned int xAcc = 0;
volatile unsigned int yAcc = 0;
volatile unsigned int zAcc = 0;
volatile unsigned int ADC_counter = 0; // increment x, y, z axis
volatile char result[4];

/* Sauce to convert int to string for UART: https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/291574?CCS-function-for-Integer-to-string-conversion-for-UART-output*/
void itoa(long unsigned int value, volatile char* result, int base) {
      // Check that base is valid
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
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = result[0];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[1];

//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = result[2];
//
//    while (!(UCA0IFG & UCTXIFG));
//    UCA0TXBUF = result[3];
}

struct Queue {
    int front, rear, size;
    unsigned capacity;
    char* array;
};

// Create queue of given capacity with initial size 0
struct Queue* CreateQueue(unsigned capacity) {
    struct Queue* theQueue = (struct Queue*) malloc(sizeof(struct Queue));
    theQueue->capacity = capacity;
    theQueue->front = theQueue->size = 0;
    theQueue->rear = capacity - 1; // rear of queue
    theQueue->array = (char*)malloc(theQueue->capacity * sizeof(char));

    return theQueue;
}

int IsFull(struct Queue* theQueue) {
    return (theQueue->size == theQueue->capacity);
}

int IsEmpty(struct Queue* theQueue) {
    return(theQueue->size == 0);
}

void Enqueue(struct Queue* theQueue, char item) {
    if (IsFull(theQueue))
        return;
    theQueue->rear = (theQueue->rear + 1) % theQueue->capacity;
    theQueue->array[theQueue->rear] = item;
    theQueue->size = theQueue->size + 1;
}

uint16_t Dequeue(struct Queue* theQueue) {
    if (IsEmpty(theQueue))
        return INT_MIN; // most negative integer
    uint16_t item = theQueue->array[theQueue->front];
    theQueue->front = (theQueue->front + 1) % theQueue->capacity;
    theQueue->size = theQueue->size - 1;
    return item;
}

void newLine() {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\n';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\r';
}

char error[] = "Error";
// Sauce: https://forum.43oh.com/topic/1643-g2553-hardware-uart-hello-world-example/
void errorMessage(void) {
    unsigned int i = 0;
    for (i = 0; i < 5; i++){
            while (!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = error[i];
        }
}

struct Queue* queue;

void setClk(void);
void setUART(void);
void message(void);
void setLEDs(void);
void setTimer(void);
void setADC(void);
void setAccel(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    _EINT(); // enable global interrupts
    queue = CreateQueue(16); // size 16 bytes

    setLEDs();
    setClk();
//    setUART();
    setTimer();
    setADC();
    setAccel();

    // Set P3.4 to be Timer B output for verification (ds81)
    P3DIR |= BIT4;
    P3OUT &= ~BIT4;
    P3SEL0 |= BIT4;
    P3SEL1 &= ~BIT4;

//    while (1) {
//        int i;
//        // Send elements of queue to TX buffer to print
//        for(i = 0; i < queue->size; i++) {
//            while (!(UCA0IFG & UCTXIFG));
//            int idx = queue->front + i;
//            if (idx >= queue->capacity) {
//                idx -= queue->capacity;
//            }
//            UCA0TXBUF = queue->array[idx];
//        }
//        if(i) // single new line
//            newLine();
//
//        PJOUT ^= BIT0;
//        __delay_cycles(100000); // to avoid spamming serial reader
//    }
    return 0;
}

//#pragma vector = USCI_A0_VECTOR
//__interrupt void USCI_A0_ISR(void){
//    unsigned char RxByte = 0;
//    RxByte = UCA0RXBUF; // get new byte from the Rx buffer
//
//    // Check if receive byte is to pop
//    if (RxByte == ' ') {
//        if (IsEmpty(queue)) {
//            errorMessage();
//            newLine();
//            __delay_cycles(1000000); // to show popped item
//        }
//
//        char popByte = Dequeue(queue);
//        while (!(UCA0IFG & UCTXIFG));
//        UCA0TXBUF = popByte;
//        __delay_cycles(1000000); // to show popped item
//        newLine();
//    }
//    // Else enqueue
//    else {
//        if (IsFull(queue)) {
//            errorMessage();
//            newLine();
//            __delay_cycles(1000000); // to show popped item
//        }
//        Enqueue(queue, RxByte);
//    }
//}

// ADC10 interrupt routine
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
 if (ADC_counter == 0) // X-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_13;  // next channel is the Y-axis
   xAcc = ADC10MEM0; // already 8 bits set by ADC10CTL2
   ADC_counter++;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
 else if (ADC_counter == 1)  // Y-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_14;  // next channel is the Z-axis
   yAcc = ADC10MEM0;
   ADC_counter++;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
 else  // Z-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;  // next channel is the X-axis
   zAcc = ADC10MEM0;
   ADC_counter = 0;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
}

int i;

#pragma vector = TIMER1_B1_VECTOR
__interrupt void TIMER1_B1_ISR(void) {

    if (IsFull(queue)) {
        accTotal = 0;
        for (i = 0; i < 16; i++)
            accTotal = accTotal + Dequeue(queue);
        accTotal = accTotal / 16;
        if (accTotal > 130) { // flash 10Hz
            TB1CCR0 = 25000;
            TB1CCR1 = 12500;
        }
        else if (accTotal > 120) { // flash 15Hz
            TB1CCR0 = 16667;
            TB1CCR1 = 8333;
        }
        else if (accTotal > 110) { // flash 20Hz
            TB1CCR0 = 12500;
            TB1CCR1 = 6250;
        }
        else if (accTotal > 100){ // flash 100Hz
            TB1CCR0 = 2500;
            TB1CCR1 = 1250;
        }
    }
    else
        Enqueue(queue, xAcc);

//    uint16_t timerCap = (8000000 / 8) / data;
//    uint16_t timerSwitch = timerCap / 2;
//    TB1CCR0 = timerCap;
//    TB1CCR1 = timerSwitch;

//    TB1CCR0 = 10000 - 1; // = (CLK/divider)/target = (8E6/32)/100 aka 4x divisions; subtract one since it counts more
//    TB1CCR1 = 5000; // 50% duty cycle
    TB1CCTL1 &= ~CCIFG; // reset flag
}

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__32; // set SMCLK divider to /8
}

void setUART() {
    // Configure UART on P2.0 and 2.1
    P2SEL0 &= ~(BIT0 + BIT1); // set to 00 ds74
    P2SEL1 |= BIT0 + BIT1; // set to 11 ds74
    UCA0CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
//    UCA0MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF700; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
    UCA0BRW = 52; // ug490 and ug497, bit clock prescaler ***Why is this 52 for both 9600 and 57600 baud?
    UCA0IE |= UCRXIE; // enable UART RX interrupt
}

void setLEDs() {
    PJDIR |= 0x0F; // 00001111 // unsure LSB
    P3DIR |= 0xF0; // 11110000

    // Toggle to show ready
    PJOUT |= 0x0F;
    P3OUT |= 0xF0;
    __delay_cycles(500000);
    PJOUT &= ~0x0F; // turn off
    P3OUT &= ~0xF0; // turn off
}

void setTimer() {
    // Set timer B
    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
    TB1CCTL1 = OUTMOD_3 + CCIE; // set/reset and interrupt enable (ug375, ug366 diagrams)

    // Set X waves (draw up graph to show)
    TB1CCR0 = 10000 - 1; // = (CLK/divider)/target = (8E6/32)/100 aka 4x divisions; subtract one since it counts more
    TB1CCR1 = 5000; // 50% duty cycle
}

void setADC() {
    // Enable ADC_B (ug449)
    ADC10CTL0 &= ~ADC10ENC;                        // ensure ENC is clear
    ADC10CTL0 = ADC10ON + ADC10SHT_5;
    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
    ADC10CTL2 &= ~ADC10RES; // 8 bit ADC out
    ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;
    ADC10IV = 0x00;    // clear all ADC12 channel int flags
    ADC10IE |= ADC10IE0;  // enable ADC10 interrupts

    ADC10CTL0 |= ADC10ENC | ADC10SC; // start the first sample. If this is not done the ADC10 interrupt will not trigger.
}

void setAccel() {
    // Set P2.7 to output HIGH to power accel
    P2DIR |= BIT7;
    P2OUT |= BIT7;

    // Set P3.0, 3.1, 3.2 to output A12, A13, A14
    P3DIR &= ~(BIT0 + BIT1 + BIT2);
    P3SEL0 |= BIT0 + BIT1 + BIT2; // ds80
    P3SEL1 |= BIT0 + BIT1 + BIT2;
}
