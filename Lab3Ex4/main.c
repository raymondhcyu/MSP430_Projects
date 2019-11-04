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

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	return 0;
}
