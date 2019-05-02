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
#include <xc.h>
#define LED LATGbits.LATG12                      // initialize LED1 on PIC32MKGPE100

void delay(){                                    // create a delay
    int j;
    for(j=0;j<10000;j++){
        ;                                        // wait/do nothing
    }
}
int main(void){
    
    TRISGbits.TRISG12=0;                         // set LED1 as output
    LED=0;                                       // turn off
    while(1){
        LED=1;                                   // turn on
        delay();                                 // delay
        LED=0;                                   // turn off
        delay();                                 // delay 
    }     
    return 0;
}