/************************************
 * This program is functional code for the PIC32MKGPE100 development board to communicate with
 * 24LC1025 EEPROM by Microchip, program is able tor read and write pages to any of
 * the available addresses on the EEPROM.
 * Connect WP,A0 to GND, A2,A1 TO Vdd, and SDA4 to SDA,SCL4 to SCL
****************************************/
#include <SOLAR_ASSIST.h>                           // constants, funcs for startup and UART
#include <stdlib.h>
#include <stdio.h>

// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol 
// Master will use I2C4 SDA4 and SCL4 

/************************************//**
** Macros and Definitions
****************************************/
#define LEN(x)  (sizeof(x) / sizeof(x[0]))
#define read  0b10100101
#define write 0b10100100
void initI2C(void)
{
    // I2C iniT
    TRISEbits.TRISE13 = 0;                          //!  E13 digital output
    TRISEbits.TRISE12 = 0;                          //!  E12 digital output
    I2C4CONbits.ON=0;
    I2C4CON = 0;
    I2C4BRG = 95;                                   // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2
                                                    // SYSCLK=40MHz, PBCLK=20MHz
                                                    // Fsck is the freq (300 kHz here), PGD = 104 ns
    
    I2C4RCV = 0;                                    //! Receive Register initialization
    I2C4TRN = 0;                                    //! Transmit Register initialization
    I2C4ADD = 0;
    I2C4CONbits.ON = 1;                             //!< I2C ON 
}
/*********************************/
//! STEP 1
/*********************************/
//start a transmission on the I2C bus

// I2C_wait_for_idle() waits until the I2C peripheral is no longer doing anything  
void I2C_wait_for_idle(void)
{
    while(I2C4CON & 0x1F);                          // Acknowledge sequence not in progress
                                                    // Receive sequence not in progress
                                                    // Stop condition not in progress
                                                    // Repeated Start condition not in progress
                                                    // Start condition not in progress
    while(I2C4STATbits.TRSTAT); // Bit = 0 ? Master transmit is not in progress
}
void i2c_master_start(void){
    I2C_wait_for_idle();
    I2C4CONbits.SEN=1;                              //send the START Bit
    while(I2C4CONbits.SEN==1);                      //wait for the start bit to be sent
}

void i2c_master_restart(void){
    I2C_wait_for_idle();
    I2C4CONbits.RSEN=1;
    while(I2C4CONbits.RSEN==1);                     ///wait for the restart to clear
}
/*********************************/
//! STEP 2
/*********************************/
void i2c_master_send(unsigned char byte) { 
    // send a byte to slave
    //LSB is RW,0-WRITE,1-READ
    I2C4TRN = byte;                                 // if an address, bit 0 = 0 for write, 1 for read 
    while(I2C4STATbits.TBF==1) { ; }                // (while transmitting wait))
    I2C_wait_for_idle();
    /*********************************/
    //! STEP 3
    /*********************************/
    if(I2C4STATbits.ACKSTAT) {                      // if this is high, slave has not acknowledged
        serialTransmit("I2C4 Master: failed to receive ACK\r\n"); 
    }
    else{
        //serialTransmit("slave  receive correctly\n\r");
    }
}

unsigned char i2c_master_recv(void){
    I2C4CONbits.RCEN=1;                             //initiate receive request
    while (I2C1CONbits.RCEN);
    while(!I2C4STATbits.RBF);                       //wait to receive data
    return I2C4RCV;                                 //read and return data
}

void i2c_master_ack(int val){                       // sends ACK = 0 (slave should send another byte)
    I2C_wait_for_idle();                            // or NACK = 1 (no more bytes requested from slave)
    I2C4CONbits.ACKDT = val;                        // store ACK/NACK in ACKDT
    I2C4CONbits.ACKEN = 1;                          // send ACKDT
    while(I2C4CONbits.ACKEN);  
}                                            

void i2c_master_stop(void) { 
    I2C_wait_for_idle();
    I2C4CONbits.PEN = 1; 
    //while(I2C4CONbits.PEN);
}


void write_eeprom(unsigned short addr,const char data[],int len){
    int i=0;
    i2c_master_start();                             //send start bit
    i2c_master_send(write);                         //send 8 bit control bytes(A0,A1,B,)
    i2c_master_send((addr & 0xFF00) >> 8);          //send 8 bit address high byte(A0,A1,B,)
    i2c_master_send(addr & 0x00FF);                 //send 8 bit address low byte(A0,A1,B,)
    for(i=0;i<len;++i){                             //send each char in the initialised writeData
        i2c_master_send(data[i]);
    }
    i2c_master_stop();                              //stop
}
void poll(void){
    int ready=0;
    while(ready!=1){
        i2c_master_start();                         //send start bit
        I2C4TRN = write;                            // if an address, bit 0 = 0 for write, 1 for read 
        while(I2C4STATbits.TRSTAT) { ; }            // (while transmitting wait))
        
        if(I2C4STATbits.ACKSTAT) {                  // if this is high, slave has not acknowledged
            serialTransmit("Data still not written yet\r\n"); 
        }
        else{
            ready=1;
            serialTransmit("Data is now on chip\n\r");
        }
    }
}
void read_eeprom(unsigned short addr,char data[],int len){
    int i=0;
    char bufc[100];
    i2c_master_start();                             //send start bit
    i2c_master_send(write);                         //send 8 bit control bytes(A0,A1,B,)
    i2c_master_send((addr & 0xFF00) >> 8);          //send 8 bit address high byte(A0,A1,B,)
    i2c_master_send(addr & 0x00FF);                 //send 8 bit address low byte(A0,A1,B,)
    
    i2c_master_start();                             //send start bit
    i2c_master_send(read);                          //send read commmand
    for( i=0 ; i<len ; i++){
        if(i==len-1){                               //if receiving last bit
            data[i]=i2c_master_recv();              //just get the data don't acknowledge it, and then issue a stop
        }else{
            data[i] = i2c_master_recv();            // send dummy byte to read 1 byte
            //sprintf(bufc,"%d",data[i]);
            //serialTransmit(bufc);
            i2c_master_ack(0);                      // send and ACK=0, to let slave know she should send more data
        }
     
    }
    i2c_master_stop();                              //stop

}


/*********************************/
int main(void){
    char writeData[] = "im stuck";
    char readData[40];                              // Array to read 35 bytes of data
    unsigned short addr1 = 0x0000;                  //define address to read/write from
    char buf[100];                                  // create buffer  to display final result
    
    __builtin_enable_interrupts();                  // global interrupt enable  
    //INTCONbits.MVEC = 1;                          // Set the interrupt controller for multi-vector mode
    //! all digital
    ANSELA = 0; ANSELB = 0; ANSELC = 0; ANSELD = 0; ANSELE = 0; ANSELF = 0;
    ANSELG = 0; ANSELF = 0; 
    
    PMCONbits.ON = 0;                               //! no PMP
    picConfigure(9600);
    initI2C();
    CFGCONbits.IOLOCK = 0;                          //! Necessary to define IO pins
    
    write_eeprom(addr1,writeData,LEN(writeData));
    poll();
    read_eeprom(addr1,readData,LEN(readData));
    sprintf(buf,"Read %s from ram at address 0x%x\r\n", readData, addr1); 
    serialTransmit(buf);
    return 1;
}
