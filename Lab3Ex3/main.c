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

volatile unsigned int usePot = 0; // 0 for PWM, 1 for potentiometer
volatile unsigned int potVolt = 0; // store potentiometer voltage in here
volatile unsigned int dutyCycle = 0;
volatile unsigned int messageSize = 6; // for testing
volatile unsigned int sizeOfQueue;
volatile int stateVariable = 0; // range 0 - 7 starting at 0
volatile unsigned long sequence[8][4] = {
                                     {{1, 0, 0, 0}},
                                     {{1, 0, 1, 0}},
                                     {{0, 0, 1, 0}},
                                     {{0, 1, 1, 0}},
                                     {{0, 1, 0, 0}},
                                     {{0, 1, 0, 1}},
                                     {{0, 0, 0, 1}},
                                     {{1, 0, 0, 1}}};

int orange[] = {1, 1, 0, 0, 0, 0, 0, 1};
int yellow[] = {0, 0, 0, 1, 1, 1, 0, 0};
int black[]  = {0, 1, 1, 1, 0, 0, 0, 0};
int brown[]  = {0, 0, 0, 0, 0, 1, 1, 1};

int delay = 0; // delay between stepper pulses

void increaseStateVariable() {
    stateVariable++;
    if (stateVariable == 8) // reached maximum
        stateVariable = 0;
}

void decreaseStateVariable() {
    --stateVariable;
    if (stateVariable < 0) // reached minimum
        stateVariable = 7;
}

void setClk(void);
void setTimer(void);
void setUART(void);
void setADC(void);
void setPotInput(void);

int main(void)
{
    int i = 0;
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    setClk();
    setUART();

    _EINT(); // enable global interrupts

    queue = createQueue(messageSize); // message size

    // Verification output pins (connect to LEDs)
    PJDIR |= BIT0 + BIT2;
    PJOUT &= ~(BIT0 + BIT2);

    // Set outputs
    P1DIR |= BIT4 + BIT5;
    P3DIR |= BIT4 + BIT5;

    while (1) {

        if (queue->size >= messageSize) {
            char startByte = dequeue(queue);
            if (startByte == 255) { // data format: [start][motor][mode][direction][data1][data2][end]
                char motorByte = dequeue(queue); // select DC or stepper
                char modeByte = dequeue(queue); // select single step or continuous
                char directionByte = dequeue(queue);
                char dataByte1 = dequeue(queue);
                char dataByte2 = dequeue(queue);

                dutyCycle = dataByte1 << 8 | dataByte2;

                if (motorByte == 1) { // stepper motor
                    if (modeByte == 0) { // single step
                        if (directionByte == 0) { // CW
                            for (stateVariable = 0; stateVariable < 8; stateVariable++) {
                                if (orange[stateVariable] == 1)
                                    P3OUT |= BIT5;
                                else
                                    P3OUT &= ~BIT5;
                                if (yellow[stateVariable] == 1)
                                    P3OUT |= BIT4;
                                else
                                    P3OUT &= ~BIT4;
                                if (black[stateVariable] == 1)
                                    P1OUT |= BIT5;
                                else
                                    P1OUT &= ~BIT5;
                                if (brown[stateVariable] == 1)
                                    P1OUT |= BIT4;
                                else
                                    P1OUT &= ~BIT4;
                                __delay_cycles(1000);
                                increaseStateVariable();
                            }
                        }
                        else if (directionByte == 1) { // CCW
                            for (stateVariable = 7; stateVariable >= 0; stateVariable--) {
                                if (orange[stateVariable] == 1)
                                    P3OUT |= BIT5;
                                else
                                    P3OUT &= ~BIT5;
                                if (yellow[stateVariable] == 1)
                                    P3OUT |= BIT4;
                                else
                                    P3OUT &= ~BIT4;
                                if (black[stateVariable] == 1)
                                    P1OUT |= BIT5;
                                else
                                    P1OUT &= ~BIT5;
                                if (brown[stateVariable] == 1)
                                    P1OUT |= BIT4;
                                else
                                    P1OUT &= ~BIT4;
                                __delay_cycles(1000);
                                decreaseStateVariable();
                            }
                        }

                    }
                    else if (modeByte == 1) { // continuous
                        PJOUT ^= BIT0;
                        for (stateVariable = 7; stateVariable >= 0; stateVariable--) {
                            if (orange[stateVariable] == 1)
                                P3OUT |= BIT5;
                            else
                                P3OUT &= ~BIT5;
                            if (yellow[stateVariable] == 1)
                                P3OUT |= BIT4;
                            else
                                P3OUT &= ~BIT4;
                            if (black[stateVariable] == 1)
                                P1OUT |= BIT5;
                            else
                                P1OUT &= ~BIT5;
                            if (brown[stateVariable] == 1)
                                P1OUT |= BIT4;
                            else
                                P1OUT &= ~BIT4;
                            __delay_cycles(1000);
                        }
                    }
                }
            }
        }


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

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__8; // set SMCLK divider to /8
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
