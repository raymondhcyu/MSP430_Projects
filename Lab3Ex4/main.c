#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#define RXD BIT5
#define TXD BIT6

struct Queue {
    int front, rear, size;
    unsigned capacity;
    char* array;
};

// Create queue of given capacity with initial size 0
struct Queue* createQueue(unsigned capacity) {
    struct Queue* theQueue = (struct Queue*) malloc(sizeof(struct Queue));
    theQueue->capacity = capacity;
    theQueue->front = theQueue->size = 0;
    theQueue->rear = capacity - 1; // rear of queue
    theQueue->array = (char*)malloc(theQueue->capacity * sizeof(char));

    return theQueue;
}

int isFull(struct Queue* theQueue) {
    return (theQueue->size == theQueue->capacity);
}

int isEmpty(struct Queue* theQueue) {
    return(theQueue->size == 0);
}

void enqueue(struct Queue* theQueue, char item) {
    if (isFull(theQueue))
        return;
    theQueue->rear = (theQueue->rear + 1) % theQueue->capacity;
    theQueue->array[theQueue->rear] = item;
    theQueue->size = theQueue->size + 1;
}

char dequeue(struct Queue* theQueue) {
    if (isEmpty(theQueue))
        return INT_MIN; // most negative integer
    char item = theQueue->array[theQueue->front];
    theQueue->front = (theQueue->front + 1) % theQueue->capacity;
    theQueue->size = theQueue->size - 1;
    return item;
}

struct Queue* queue;

void setClk(void);
void setTimer(void);
void setUART(void);

volatile unsigned int data1 = 0; // encoder input 1
volatile unsigned int data2 = 0; // encoder input 2
volatile unsigned int escape = 0; // data escape byte

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
    queue = createQueue(50);

	// Receive inputs from P1.1 and P1.2
	P1DIR &= ~(BIT1 + BIT2);
	P1SEL1 |= BIT1 + BIT2;
	P1SEL0 &= ~(BIT1 + BIT2);

	setTimer();
	setUART();
	_EINT();

	while (1) {

	}

	return 0;
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    unsigned char RxByte = 0;
    RxByte = UCA1RXBUF; // get val from RX buffer
    enqueue(queue, RxByte);
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void) {
    // Message format: [Start][CW][CCW][Escape]
    while (!(UCTXIFG & UCA1IFG)); //
    UCA1TXBUF = 255;    // start byte

    while (!(UCTXIFG & UCA1IFG)); //
    UCA1TXBUF = 10;    // start byte

    while (!(UCTXIFG & UCA1IFG)); //
    UCA1TXBUF = 20;    // start byte

    while (!(UCTXIFG & UCA1IFG)); //
    UCA1TXBUF = 30;    // start byte

//    while(!(UCTXIFG & UCA1IFG));
//    data1 = TA0R; // get data from TA0
//    if (data1 == 255) { // check if 0
//        escape = 1;
//        UCA1TXBUF = 0;
//    }
//    else
//        UCA1TXBUF = data1;
//
//    while (!(UCTXIFG & UCA1IFG)); // Ensure that we are ready to transmit
//    data2 = TA1R;
//    if (data2 == 255) { // check if 0
//        escape = 2;
//        UCA1TXBUF = 0;
//    }
//    else
//        UCA1TXBUF = data2;
//
//    while(!(UCTXIFG & UCA1IFG));
//    UCA1TXBUF = escape;    // Finally send the escape byte
//
//    // Reset values
//    escape = 0;
//    TA0R = 0;
//    TA1R = 0;
////    TB0CCTL0 &= ~CCIFG; // reset flag
}

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__8; // set SMCLK divider to /8
}

void setTimer() {
    // Set up timer A to receive input as clock
    TA0CTL |= TASSEL_0 + MC_2; // continuous mode
    TA1CTL |= TASSEL_0 + MC_2; // continuous mode

    // Set up timer B to transmit data through UART
    TB0CTL |= TBSSEL_2 + MC_2; // continuous mode
    TB0CCTL0 |= OUTMOD_3 + CCIE; // enable interrupt

    // Set P1.4 and P1.5 as outputs
    P1DIR |= BIT4 + BIT5;
    P1OUT &= ~(BIT4 + BIT5);
    P1SEL0 |= BIT4 + BIT5;
    P1SEL1 &= ~(BIT4 + BIT5);
}

void setUART() {
    P2SEL0 &= ~(RXD + TXD); // set to 00 ds74
    P2SEL1 |= RXD + TXD; // set to 11 ds74
    UCA1CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
    UCA1MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
//    UCA1MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF700; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
    UCA1BRW = 52; // ug490 and ug497, bit clock prescaler ***Why is this 52 for both 9600 and 57600 baud?
    UCA1IE |= UCRXIE; // enable UART RX interrupt
}
