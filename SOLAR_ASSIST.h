/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    SOLAR_ASSIST.h

  @Summary
 HEADER file used to initialize functions for UART to pic COMM with computer

 */
/* ************************************************************************** */

#ifndef SOLAR_ASSIST_H    /* Guard against multiple inclusion */
#define SOLAR_ASSIST_H


#include <stdlib.h>
#include <xc.h>
#include <sys/attribs.h>
#include <string.h>

// Defines
#define SYSCLK 40000000L
#define LED1 LATGbits.LATG12
#define LED2 LATGbits.LATG13
#define LED3 LATGbits.LATG14

// Macros
// Equation to set baud rate from UART reference manual equation 21-1
#define Baud2BRG(desired_baud)      ( (SYSCLK / (16*desired_baud))-1)
// Function Prototypes
void serialTransmit(const char *string);
void SerialReceive(char * message, int maxLength);
void picConfigure(int baud);


#endif /* SOLAR_ASSIST_H */

/* *****************************************************************************
 End of File
 */
