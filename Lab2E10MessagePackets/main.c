#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

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

char Dequeue(struct Queue* theQueue) {
    if (IsEmpty(theQueue))
        return INT_MIN; // most negative integer
    char item = theQueue->array[theQueue->front];
    theQueue->front = (theQueue->front + 1) % theQueue->capacity;
    theQueue->size = theQueue->size - 1;
    return item;
}

void sendInt(int num) {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = num;
}

struct Queue* queue;

void setClk(void);
void setTimer(void);
void setUART(void);
void setLEDs();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    queue = CreateQueue(50);
    setLEDs();
    setClk();
    setTimer();
    setUART();

    _EINT(); // enable global interrupts

    while (1) {
        if (queue->size >= 5) { // 5 byte message
            char startByte = Dequeue(queue);
            if (startByte == 255) { // check start byte
                char commandByte = Dequeue(queue); // next byte
                char dataByte1 = Dequeue(queue);
                char dataByte2 = Dequeue(queue);
                char escapeByte = Dequeue(queue);

                if (escapeByte == 1) {
                    dataByte2 = 255;
                    escapeByte = 0;
                } else if (escapeByte == 2) {
                    dataByte1 = 255;
                    escapeByte = 0;
                } else if (escapeByte == 3) {
                    dataByte2 = 255;
                    dataByte1 = 255;
                    escapeByte = 0;
                }
                int data = dataByte1 << 8 | dataByte2; // data = dataByte1 + dataByte2*256;

                // Commands 0 to 3 toggle the LEDs
                if (commandByte == 0) {
                    PJOUT ^= BIT0;
                } else if (commandByte == 1) {
                    PJOUT ^= BIT1;
                } else if (commandByte == 2) {
                    PJOUT ^= BIT2;
                } else if (commandByte == 3) {
                    PJOUT ^= BIT3;
                } else if (commandByte == 4) {
                 // If we receive any other command, change the frequency of the timer
                 uint16_t timerCap = (8000000 / 8) / data;
                 uint16_t timerSwitch = timerCap / 2;
                 TB1CCR0 = timerCap;
                 TB1CCR1 = timerSwitch;
                }

                // Send data back
                sendInt(startByte);
                sendInt(commandByte);
                sendInt(dataByte1);
                sendInt(dataByte2);
                sendInt(escapeByte);
            }
        }
        __delay_cycles(100000); // to avoid spamming serial reader
    }
    return 0;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    unsigned char RxByte = 0;
    RxByte = UCA0RXBUF; // get new byte from the Rx buffer
    Enqueue(queue, RxByte);
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

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__8; // set SMCLK divider to /8
}

void setTimer() {
    // Set P3.4 and P3.5 to be Timer B output and LED output; um14, 17
    P3DIR |= BIT4 + BIT5;
    P3OUT &= ~(BIT4 + BIT5);
    P3SEL0 |= BIT4 + BIT5;
    P3SEL1 &= ~(BIT4 + BIT5);

    // Set timer B
    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
    TB1CCTL1 = OUTMOD_3; // set/reset and interrupt enable (ug375, ug366 diagrams)

    // Set 1000Hz waves (draw up graph to show)
    TB1CCR0 = 1000 - 1; // = (CLK/divider)/target = (8E6/8)/1000; subtract one since it counts more
    TB1CCR1 = 500; // 50% duty cycle
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
