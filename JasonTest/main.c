#include <msp430.h>
#include <msp430fr5739.h>

#define TXD BIT1
#define RXD BIT0
#define TEMP_CALIBRATION 170
#define ROOM_TEMP 20
#define TEMP_THRESHOLD 5

//PART 8

//GLOBAL VARS
volatile unsigned int rawTemp = 0;
volatile unsigned int correctedTemp = 0;
volatile char result[4];

//FUNCTIONS
void setupClock();
void setupUART();
void setupADC();
void setupTimer();
void setupLED();
void sendInt(int num);
void sendComma();
void newLine();
void itoa(long unsigned int value, volatile char* result, int base);


//MAIN ---------------------------------------------------------------------------------------------------------------------------------------------------
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    _EINT(); // enable global interrupts

    setupClock();

    setupUART();

    setupADC();

    setupTimer();

    setupLED();

    while(1) {

            PJOUT = BIT0; // turn on one LED for default

            if (correctedTemp > ROOM_TEMP + 10)
                PJOUT = BIT1;
            if (correctedTemp > ROOM_TEMP + 20)
                PJOUT = BIT2;
            if (correctedTemp > ROOM_TEMP + 30)
                PJOUT = BIT3;
            if (correctedTemp > ROOM_TEMP + 40)
                P3OUT = BIT4;
            if (correctedTemp > ROOM_TEMP + 50)
                P3OUT = BIT5;
            if (correctedTemp > ROOM_TEMP + 60)
                P3OUT = BIT6;
            if (correctedTemp > ROOM_TEMP + 70)
                P3OUT = BIT7;
            else {
                PJOUT &= ~0x01;
                P3OUT &= ~0xF0;
            }
        }

    return 0;

}


//FUNCTION DEFINITIONS -------------------------------------------------------------------------------------------------------------------

//INT TO STRING (Found online)
void itoa(long unsigned int value, volatile char* result, int base) {
      // check that the base if valid
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

//SEND INT to TX Buffer
void sendInt(int num) {

    itoa(num, result, 10);
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[0];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[1];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[2];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[3];

}

void sendUnits() {

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 'C';

}

void sendComma() {

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = ',';

}


//SEND '\n' to TX Buffer
void newLine() {

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\n';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\r';

}

//CLOCK SETUP
void setupClock(){

    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0
    CSCTL1 |= DCOFSEL_3; // enables 8MHz DCO (pg16 ds [ds = datasheet]
    CSCTL2 = SELA0 + SELA1 + SELS0 + SELS1 + SELM0 + SELM1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO
    //CSCTL2 |= SELS__DCOCLK; // sets SMCLK to run off DCO (pg82 ug), 011b = DCOCLK
    CSCTL3 |= DIVS__32; // set SMCLK divider to /32

}

//UART SETUP
void setupUART(){

    // Configure UART0
    UCA0CTLW0 = UCSSEL0;                    // Run the UART using ACLK
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900;   // Baud rate = 9600 from an 8 MHz clock
    UCA0BRW = 52;

    //FOR 57.6kBaud
    //UCA0MCTLW = UCOS16 + UCBRF3 + UCBFR1 + 0xF700;   // Baud rate = 57.6 from an 8 MHz clock
    //UCA0BRW = 8; //For 57.6kBaud --> Doesnt work? Must be 52 still?

    //UCA0IE |= UCRXIE;                       // Enable UART Rx interrupt

    //Configure Ports P2.0 and P2.1 for UART communications. pg 74/119 - says what to set pins to.
    P2SEL0 &= ~(RXD + TXD); // set to 00 ds74
    P2SEL1 |= RXD + TXD;    // set to 11 ds74

    //Set as output
    PJDIR |= BIT0; //PJ.0
    //PJOUT &= ~BIT0; //Turn off LED1
}

//ADC SETUP
void setupADC(){

    //SET P2.7 HIGH (1) to power Accelerometer and Thermistor????
    P2DIR |= BIT7; //Set P2.7 as output
    P2OUT |= BIT7; //Set P2.7 as high (1)

    //SET P1.4 to INPUT to A4 (Temp Sensor)
    P1DIR &= ~BIT4; //(0) = input //Not sure if matters, I think setting SEL1 and SEL0 to 1 disables IO for analog.
    P1SEL1 |= BIT4;
    P1SEL0 |= BIT4;

    ADC10CTL0 &= ~ADC10ENC; //ENC must = 0 to write to ADC10CTLx registers
    ADC10CTL0 = ADC10ON + ADC10SHT_3;
    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
    ADC10CTL2 = ADC10RES;
    ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_4; //Check Port A4
    ADC10IV = 0x00;    //Clear all ADC12 channel int flags
    ADC10IE |= ADC10IE0;  //Enable ADC10 interrupts

//    ADC10CTL0 &= ~ADC10ENC;                        // ensure ENC is clear
//    ADC10CTL0 = ADC10ON + ADC10SHT_3;
//    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
//    ADC10CTL2 = ADC10RES; // 8 or 10 bit ADC out
//    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_4; // ADC10INCH for temperature
//    ADC10IV = 0x00;    // clear all ADC12 channel int flags
//    ADC10IE |= ADC10IE0;  // enable ADC10 interrupts

    ADC10CTL0 |= ADC10ENC | ADC10SC; //Start the first sample. If this is not done the ADC10 interrupt will not trigger.

}

//TIMER SETUP (40ms interrupt)
void setupTimer(){

    TB0CTL = TBSSEL_2 + ID__1 + MC_1;   //SMCLK, /1, Up Mode
    TB0CCTL1 = CCIE;
    TB0CCR0 = 40000;
    TB0CCR1 = 20000;

}

void setupLED(){
    // Configure PJ.0, PJ.1, PJ.2, PJ.3, P3.4, P3.5, P3.6, and P3.7 as digital outputs.
    PJDIR |= 0x0F; // 00001111
    P3DIR |= 0xF0; // 11110000

    //Clear LEDs
    PJOUT = 0x00; //clear all LEDs
    P3OUT = 0x00; //clear all LEDs

}

void checkTemp(){
    if (correctedTemp > ROOM_TEMP + TEMP_THRESHOLD) {

        P3OUT &= ~0xF0; // 11110000 Turn OFF
        PJOUT |= 0x0F; // 00001111 Turn ON

    }
    else if (correctedTemp < ROOM_TEMP - TEMP_THRESHOLD) {

            PJOUT &= ~0x0F; // 00001111 Turn OFF
            P3OUT |= 0xF0; // 11110000 Turn ON
    }
    else {
        PJOUT &= ~0x0F; // 00001111 Turn OFF
        P3OUT &= ~0xF0; // 11110000 Turn OFF
    }
}


//VECTORS/INTERRUPTS ---------------------------------------------------------------------------------------------------------------------------------

//Send data over UART at interval specified by Timer
#pragma vector = TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void){

    sendInt(rawTemp);
    sendComma();
    sendInt(correctedTemp);
    sendUnits();
    newLine();

    /*
    //Troubleshooting interrupt
    PJOUT ^= BIT0;

    for (i=0;i<20000;i++){
        _NOP();
    }
    */

    TB0CCTL1 &= ~BIT0; // reset interrupt flag
}

//Retrieve NTC Temp raw data captured by the ADC
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{

   ADC10CTL0 &= ~ADC10ENC;
   rawTemp = ADC10MEM0;
   correctedTemp = rawTemp - TEMP_CALIBRATION;
   //checkTemp();
   ADC10CTL0 |= ADC10ENC | ADC10SC;



}
