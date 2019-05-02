#include "SOLAR_ASSIST.h"
// DEVCFG3
#pragma config USERID = 0xFFFF // Enter Hexadecimal value (Enter Hexadecimal value)

// DEVCFG2
#pragma config FPLLICLK=PLL_FRC // Primary oscillator is input to PLL
#pragma config FPLLMULT = MUL_80 // PLL Multiplier: 8MHz Multiply by 8*80=640MHz
//Output to FPLLODIV needs to be within a range OF 350 MHZ and 700 MHz check EC frequency settings in datasheet
#pragma config FPLLODIV = DIV_16 // PLL Output Divider: 640MHz/16=40Mhz=SYSCLK

// DEVCFG1
#pragma config FPLLRNG=RANGE_5_10_MHZ //SYSTEM PLL INPUT RANGE 5-10 MHZ
#pragma config FNOSC = SPLL  // Oscillator Selection Bits (8Mhz)(Select which final osc source you want)
#pragma config FSOSCEN = OFF // Secondary Oscillator Disabled (Enable Secondary Oscillator)
#pragma config POSCMOD = OFF // Use oscillator between osc1 and osc2
#pragma config FWDTEN = OFF // Watchdog Timer Enable (WDT Disabled)
#pragma config FDMTEN = OFF // Deadman Timer Enable (Deadman Timer is disabled)


// DEVCFG0
#pragma config DEBUG = OFF // Background Debugger Enable (Debugger is disabled)
#pragma config JTAGEN = OFF // JTAG Enable (JTAG Disabled)

// DEVCP
#pragma config CP = OFF // Code Protect (Protection Disabled)
// #pragma config statements should be included in .c file
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
void picConfigure( int desired_baud){
    // CONFIGURE PPS
    U4RXR=0b1101;                   //configure pin rpd3 as U4RX(Input)
    RPA12R=0b00010;                 //configure pin rpa12 as U4TX(Output)
    // disable interrupts
    __builtin_disable_interrupts();
    INTCONbits.MVEC = 1;            // Set the interrupt controller for multi-vector mode 
    //Turn on LED1
    TRISGbits.TRISG12=0;            //configure LED1 as output

    //Initialize UART
    U4MODE=0;                       // disable autobaud, TX and RX enabled only, 8N1, idle=HIGH
    U4MODEbits.CLKSEL=0b01;         //select SYSCLK as the clock
    U4BRG = Baud2BRG(desired_baud); // U2BRG = (FPb / (16*baud)) - 1
    U4STA = 0x1400;                 // enable TX and RX, bits 10 and 12

    // enable the uart
    U4MODEbits.ON = 1;
         //Baud=Bits/second
}// END UART4Configure()

/* SerialTransmit() transmits a string to the UART4 TX pin MSB first
 *
 * Inputs: *buffer = string to transmit */
void serialTransmit(const char *string){
    while (*string != '\0') {
        while (U4STAbits.UTXBF) {
            ;
        }
        U4TXREG = *string;
        ++string;
    }
}

void SerialReceive(char *message, int maxLength){
    char data = 0;
    int complete = 0, num_bytes = 0;
    // loop until you get a \r or \n
    while (!complete) {
        if (U4STAbits.URXDA) { // if data is available
            data = U4RXREG; // read the data
            if ((data == '\n') || (data == '\r')) {
                complete = 1;
            }else {
                 message[num_bytes] = data;
                 ++num_bytes;
                 if (num_bytes >= maxLength) {
                     num_bytes = 0;
                }
            }
        }
    }
        // end the string
    message[num_bytes] = '\0';
}
