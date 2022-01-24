////////////////////////////////////////
//   Serial Packet - MbedJsonValue    //
//  Arkadiraf@gmail.com - 15/06/2017  //
////////////////////////////////////////
/*
    Construct packet from uart and parse with json format aand put it on CAN BUS
    json:<PACKET>\r\n
    test with
    json:{"my_array": ["demo_string", 10], "my_boolean": true}\r\n
*/




/*
   Board : Nucleo STM32F446RE
*/

/*
    Pinout:
    PC - Serial 2
    PA_2 (Tx) --> STLINK
    PA_3 (Rx) --> STLINK
*/


///////////////
// Libraries //
///////////////
#include "mbed.h"
#include "MbedJSONValue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#include "BufferedSerial.h" // uart handler library
///////////////
// #defines  //
///////////////

#define DEBUG_MOD1
#define MSG_BUFFER_SIZE 50
#define HEADER_SIZE 5
#define FOOTER_SIZE 2

/////////////
// Objects //
/////////////

// uart
//Serial pc(USBTX, USBRX);
Serial pc(D1,D0);
// digital output
//DigitalOut led(PA_5);
DigitalOut led(LED1);
///////////////
// variables //
///////////////
const uint16_t  WHEEL_ID = 0x202;    // ID of Wheel Message
const uint16_t  THROTLE_ID = 0x203;    // ID of Throtle Message
const uint16_t  BREAK_ID = 0x201;    // ID of Break Message
CAN can1(D10, D2); 
// json buffer
char json[MSG_BUFFER_SIZE];

// packet variables
struct packetMSG_struct {
    // recieve message variables
    uint8_t header[HEADER_SIZE];
    uint8_t footer[FOOTER_SIZE];
    uint8_t syncIndex; // sync index for header / footer
    uint8_t syncFlag; // 0 - waiting for header, 1 -  waiting for footer, 2 - verify footer, 3 - finish footer send to parser, flash buffer
    // buffer
    uint16_t bufferIndex; // buffer index
    char buffer[MSG_BUFFER_SIZE];
} ;

packetMSG_struct packetMSG;
///////////////
// Functions //
///////////////

// Serial Event function
void rxCallback(void);

// initialize packet struct
void initPacket(void);

// Packet Parser
void parsePacket(void);
////////////////////////
//  Main Code Setup : //
////////////////////////
int main()
{
    // init packet:
    initPacket();
    // init uart
    pc.baud(57600);
    // attach serial event interrupt
    pc.attach(&rxCallback, Serial::RxIrq);

#ifdef DEBUG_MOD1
    pc.printf("UART Test \r\n");
#endif
    ///////////////////////
    //  Main Code Loop : //
    ///////////////////////
    while(1) {
        rxCallback();
        //sleep();
    }// end main loop
}// end main



///////////////
// Functions //
///////////////

// Serial Event function
void rxCallback(void)
{
    float send;
    while (pc.readable()) {
        // read icoming
    
        uint8_t in_byte = pc.getc();
#ifdef DEBUG_MOD1
        pc.putc(in_byte);
#endif
        // detect start message , end message
        switch (packetMSG.syncFlag) {
                // waiting for header
            case 0: {
                if (packetMSG.header[packetMSG.syncIndex] == in_byte) {
                    packetMSG.syncIndex++;
                    if (packetMSG.syncIndex == HEADER_SIZE) { // finish header SYNC
                        packetMSG.syncFlag = 1; // start collecting data, wait for footer
                        packetMSG.bufferIndex = 0;
                        packetMSG.syncIndex=0;
                    }
                } else { // reinit sync
                    packetMSG.syncIndex=0;
                   

                }
                //pc.printf("case 0 , %d  \r\n",packetMSG.syncIndex);
                break;
            }
            // waiting for footer
            case 1: {
               
                
                // add byte to buffer
                packetMSG.buffer[packetMSG.bufferIndex] = in_byte;
                packetMSG.bufferIndex++;
                if (packetMSG.bufferIndex >= MSG_BUFFER_SIZE) { // buffer overflow
                    // reset buffer
                    packetMSG.bufferIndex = 0;
                    packetMSG.syncIndex = 0;
                    packetMSG.syncFlag = 0;
                } else if (packetMSG.footer[packetMSG.syncIndex] == in_byte) { // footer char recieved
                    packetMSG.syncIndex++;
                    packetMSG.syncFlag=2; // move to verify footer
                }
                //pc.printf("case 2 , %d  \r\n",packetMSG.syncIndex);
                break;
            }
            // verify footer
            case 2: {
               
                // add byte to buffer
                packetMSG.buffer[packetMSG.bufferIndex] = in_byte;
                packetMSG.bufferIndex++;
                if (packetMSG.bufferIndex >= MSG_BUFFER_SIZE) { // buffer overflow
                    // reset buffer
                    packetMSG.bufferIndex = 0;
                    packetMSG.syncIndex = 0;
                    packetMSG.syncFlag = 0;
                } else if (packetMSG.footer[packetMSG.syncIndex] == in_byte) { // footer char recieved
                    packetMSG.syncIndex++;
                    if (packetMSG.syncIndex == FOOTER_SIZE) { // finish footer SYNC
                       
                      
                        
                        send= atof(packetMSG.buffer+1);
                        if(packetMSG.buffer[0]=='B'){
                             
                             if(can1.write(CANMessage(BREAK_ID, reinterpret_cast<char*>(&(send)), sizeof(send))))
                             {
                                led = !led;
                                }
                        }
                        if(packetMSG.buffer[0]=='T'){
                             if(can1.write(CANMessage(THROTLE_ID, reinterpret_cast<char*>(&(send)), sizeof(send))))
                             {
                                led = !led;
                                }
                        }
                        if(packetMSG.buffer[0]=='S'){
                             if(can1.write(CANMessage(WHEEL_ID, reinterpret_cast<char*>(&(send)), sizeof(send))))
                             {
                              led = !led;
                              }
                        }
                        packetMSG.syncFlag = 3;
                        // copy packet to json buffer
                        memcpy (&json, &packetMSG.buffer, packetMSG.bufferIndex);
                        json[packetMSG.bufferIndex]=NULL; // end with NULL to indicate end of string
                        // copy packet to json buffer with sprintf
                        //sprintf(json, "%.*s", packetMSG.bufferIndex, packetMSG.buffer );
                        // send msg to parse.
                        parsePacket();
                        // reset buffer
                        packetMSG.bufferIndex = 0;
                        packetMSG.syncIndex = 0;
                        packetMSG.syncFlag = 0;
                        
                    }
                } else { // footer broke restart wait for footer
                    packetMSG.syncFlag=1;
                    // verify that it didnt broke on first footer char
                    if (packetMSG.footer[0] == in_byte) {
                        packetMSG.syncIndex=1;
                    } else {
                        packetMSG.syncIndex=0;
                    }
                }
                break;
            }
            default: {
                pc.printf("Sonmething went wrong \r\n");
                break;
               
                
                
            }
        } // end switch
       
         
    }// end uart readable
} // end rxCallback


// initialize packet struct
void initPacket(void)
{
    // init variables to default:
    packetMSG.header[0] = 'j';
    packetMSG.header[1] = 's';
    packetMSG.header[2] = 'o';
    packetMSG.header[3] = 'n';
    packetMSG.header[4] = ':';

    packetMSG.footer[0]= 13; // /r
    packetMSG.footer[1]= 10; // /n

    packetMSG.syncIndex=0; // sync index for header / footer
    packetMSG.syncFlag=0; // 0 - waiting for header, 1 -  waiting for footer, 2 - verify footer, 3 - finish footer send to parser, flash buffer
    packetMSG.bufferIndex=0; // buffer index
}

// Packet Parser
void parsePacket(void)
{
#ifdef DEBUG_MOD1
    // write buffer to screen
    //pc.printf("%d, %.*s", packetMSG.bufferIndex ,packetMSG.bufferIndex, packetMSG.buffer );
    //pc.printf("%s", json);
#endif

    MbedJSONValue demo;

    //const  char * json = "{\"my_array\": [\"demo_string\", 10], \"my_boolean\": true}";
    // json:{"my_array": ["demo_string", 10], "my_boolean": true}
    //parse the previous string and fill the object demo
    parse(demo, json);

//    string my_str;
//    int my_int;
//    bool my_bool;
//
//    my_str = demo["my_array"][0].get<std::string>();
//    my_int = demo["my_array"][1].get<int>();
//    my_bool = demo["my_boolean"].get<bool>();
//
//    printf("my_str: %s\r\n", my_str.c_str());
//    printf("my_int: %d\r\n", my_int);
//    printf("my_bool: %s\r\n", my_bool ? "true" : "false");

}