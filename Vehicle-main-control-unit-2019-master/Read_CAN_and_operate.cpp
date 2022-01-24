///////////////////////////
//rajwanraz@gmail.com    //         
//sagicohen8@gmail.com   //
///////////////////////////
/*
construct packet from CAN_BUS and operate the suittable servo motor.
*/
#include "mbed.h"
#include "BufferedSerial.h"
#include <string>
#include "Servo.h"
//constants definition
const uint16_t  WHEEL_ID = 0x202;    // ID of Wheel Message
const uint16_t  THROTLE_ID = 0x203;    // ID of Throtle Message
const uint16_t  BREAK_ID = 0x201;    // ID of Break Message
//global variables definition
BufferedSerial pc(USBTX, USBRX);
DigitalOut led1(LED1);
CAN can1(D10, D2);
float recieved_float; // 
Servo myservo(D6);// pwm pin

int main() {
    pc.baud(57600);
    CANMessage msg;
    while(1) {
        if(can1.read(msg)) {
            if(msg.id == BREAK_ID)
            {
                
                recieved_float = *reinterpret_cast<float*>(msg.data);
                pc.printf("B=%f\r\n", recieved_float);
                led1 = !led1;
               // myservo=recieved_float;
                /*
                use servo motor for example: (myservo= recieved_float);
                */
               
            }
           
            if(msg.id == WHEEL_ID)
            {
                recieved_float = *reinterpret_cast<float*>(msg.data);
                pc.printf("W=%f\r\n", recieved_float);
                led1 = !led1;
                 /*
                use servo motor for example: (myservo=recieved_float );
                */
            }
            if(msg.id == THROTLE_ID)
            {
                recieved_float = *reinterpret_cast<float*>(msg.data);
                pc.printf("T=%f\r\n", recieved_float);
                led1 = !led1;
                myservo=recieved_float;
                
                
            }
            
        }
    }
}
 
