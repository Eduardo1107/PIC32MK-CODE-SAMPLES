/************************************
 * This program is base code for the PIC32MKGPE100 development board to communicate with
 * ADS8344 A/D converter by TI, code is still in development and is 
 * NOT FUNCTIONAL
****************************************/
#include <SOLAR_ASSIST.h>                           //configure UART, pins, and disable interrupts
#include <stdio.h>
#define CS LATFbits.LATF9                           // chip select pin28 for SS1
//#define CS1 LATFbits.LATF10                       // chip select pin29 for SS2

unsigned int spi_io(uint8_t o) {                    //simultaneously read data from slave and write data 
    unsigned int temp;
    temp = SPI2BUF;                                 // dummy read of the SPI2BUF register to clear the SPIRBF flag
    while (SPI2STATbits.SPITBF);
    SPI2BUF = o;                                    //send data to the slave
    while(!SPI2STATbits.SPIRBF);
    
    //every time you read from SPI2BUF you should also read from it to prevent overflow
    return SPI2BUF;                                 //read the data sent back by the slave
}

//initialize the spi2 and ADC module
void ad_init(){
    ANSELA = 0;                                     //turn off analog functionality since some pins default to analog mode
    ANSELB = 0; 
    ANSELF = 0;
    int i;
    
    PMCONbits.ON = 0;                               //! no Parallel master port
    PB2DIVbits.ON = 1;                              //! PBCLK2 clock  => ON 
    
    IEC1bits.SPI2EIE = IEC1bits.SPI2RXIE = IEC1bits.I2C2SIE = 0; // SPI2 clear Interrupt
    IPC13bits.SPI2EIP = IPC13bits.SPI2EIS = 0;      //! Clear Fault priority
    IPC13bits.SPI2RXIP = IPC13bits.SPI2RXIS = 0;    //! Clear Receive priority
    IPC13bits.SPI2TXIP = IPC13bits.SPI2TXIS = 0;    //! Clear Transmit priority
    IFS1bits.SPI2EIF = IFS1bits.SPI2RXIF = IFS1bits.SPI2TXIF = 0; //! Clear Flag 
    SPI2CON = 0;
    CFGCONbits.IOLOCK = 0;                          //! Necessary to define IO pins
    PMD5bits.SPI2MD = 0;                            //! For SPI2 clock enabled
    
    //Master SPI2 pins(Dev Board) are: SCK2(Pin70), SDI2(RPA14), SDO2(RPB7), SS/CS=RB9
    //Master SPI1 pins(Final Circuit) are: SCK1(72), SDI1(RPC7), SDO1(RPC7), SS
    TRISFbits.TRISF9=0;                             // set gpio RF9 as output SS1
    //TRISFbits.TRISF10=0;                          // set gpio RF10 as output SS2  
    
    // NO DATA being sent right now so set CS to high, when data is going to be sent drive CS TO LOW(0);
    CS=1;
    
    //PPS configuration and output/input decleration
    TRISAbits.TRISA14=1;                            //make SDI2 pin an input
    SDI2R=0b1101;                                   //configure pin rpa14 as sdi2
    TRISBbits.TRISB7=0;                             //make SDO2 pin an output
    RPB7R=0b00100;                                  //configure pin rpb7 as sdo2
    //RPB9R=0b00100;                                //configure pin rpb9 as ss2
    TRISBbits.TRISB6 = 0;                           //make SCK2 pin an Output
    CFGCONbits.IOLOCK = 1;                          //lock the bits so they can't change anymore
    
    //Initialize SPI Registers
    SPI2CON = 0;                                    //Stop and reset SPI2
    SPI2BUF;                                        //clear the rx buffer by reading from it
    SPI2BRG=0x4;                                    //baud rate to 2 MHz [SPI2BRG = (20000000/(2*desired))-1]
                                                    //SPI2 controlled by PBCLK2
    SPI2STATbits.SPIROV=0;                          //clear the overflow bit
    SPI2CONbits.MSTEN=1;                            //Pic is the Master
    SPI2CONbits.SSEN=0;                             //Pic is not a slave
    SPI2CONbits.MODE16 = 0;                         // use 8-bit transfer mode
    SPI2CONbits.MODE32 = 0;
    SPI2CONbits.SMP=0;                              //input data sampled in the middle
    SPI2CONbits.MCLKSEL = 0;                        //1 maser clock select => PBCLK2 (PBCLk2=SYSCLK/2) so 20MHz
    
    /* https://people.ece.cornell.edu/land/courses/ece4760/PIC32/index_SPI.html */
    SPI2CONbits.CKP=0;                              //clock is low(idle) when no data is being transmitted                                           
    SPI2CONbits.CKE=1;                              //Dout data changes on falling edge
    
    SPI2CONbits.ON=1;                               //turn SPI on
    
    //set desired clock mode for the A/D convertor according to ADS8344 Datasheet
    
    CS=0;                                           //enable the ADC chip
    spi_io(0b10000011);                             //When power is first applied user must set desired clock mode
    spi_io(0b00000000);                             //0b10000111 is sent:No power down between conversion
    spi_io(0b00000000);
    spi_io(0b00000000);
    CS=1;                                           //disable the ADC chip
}


unsigned int adc_rw(int channel){
    unsigned int adcvalue = 0;
    uint8_t commandbits;  
    char bufa[100];                                 // buffer for uart comm with the user
    char bufb[100];                                 // buffer for uart comm with the user
    char bufc[100];                                 // buffer for uart comm with the user
    uint8_t first;                                  // initialize data user will recieve from ADS8344
    uint8_t second;
    uint8_t third;
    int i;
    switch(channel)  // Switch case to select channel
    {
      case 1:  {
        commandbits = 0b10000111;                   // Select channel 0
        }
        break;
      case 2:  {
        commandbits = 0b11000111;                   // Select channel 1 
        }
        break;
      case 3:  {
        commandbits = 0b10010111;                   // Select channel 2 
        }
        break;
      case 4:  {
        commandbits = 0b11010111;                   // Select channel 3 
        }
        break;
      case 5:  {
        commandbits = 0b10100111;                   // Select channel 4 
        }
        break;
        //TODO: add command bits for channels 4-8
    }
    
    // One full readd will take 32 clock cycles
    // Sending 8 bits per transfer, requires 4 transfers total
    
    CS=0;                                           //Enable ADS8344
    spi_io(commandbits);                            //send command bits to ADS8344 chip, dont read data since data isnt ready yet
   
    first=spi_io(0b00000000);                       //read first byte, bits 16-9, first bit will be a dont care
      
    sprintf(bufb,"%u\r\n",first);                   //print result to screen
    serialTransmit(bufb);
    
    second=spi_io(0b00000000);                      //get 2nd byte, bits 9-1
    
    sprintf(bufa,"%u\r\n",second);                  //print result to screen
    serialTransmit(bufa);
    
    third=spi_io(0b00000000);                       //get 3rd byte, very last bit, the other 7 bits are zeros
    
    CS=1;                                           // disable ADS8344
    
    adcvalue=((first << 8) | second)<<1 | (third>>7); // left shift first, then OR with second, then left shift result by one
                                                      // or that result by third transmission right shifted by 7

    return adcvalue;                                //return adc value
}


int main(void){
    int i=0;
    char buf[100]={};                               // buffer for comm. with the user
    picConfigure(9600);                             // function defined in SOLARASSIST.c, set up pic for a 9600 baud rate
                                                    // communication with computer
    __builtin_disable_interrupts();                 // global interrupt disable
    ad_init();                                      //initialize the A/D convertor module
    float readValue=0;
    float readVoltage=0;
    int iChannel;
    while(1){
        for(iChannel=1;iChannel<=3;iChannel++){     //read each channel
            readValue=adc_rw(iChannel);
            readVoltage = (readValue*(3.3))/(65535);        //convert to analog reading
            sprintf(buf,"Channel %d Voltage: %6.4f V\r\n",iChannel,readVoltage); //send over UART
            serialTransmit(buf);
            for(i=0;i<100000;i++){;}                //small delay
        }
    }
    return 0;
}