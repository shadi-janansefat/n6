#include <Timer.h>
#include "AM.h"
#include "Serial.h"
#include <I2C.h>
#include "iRobotRadio.h"
#include "math.h"

module iRobotMobileC @safe() {
  uses {
    interface Boot;
	
	interface Timer<TMilli> as dataTimer; //capturing/creating data
	interface Timer<TMilli> as sendTimer; //sending data
	interface Timer<TMilli> as lossTimer; //sending data
	interface Timer<TMilli> as bufferTimer; //sending data

	interface SplitControl as RadioControl;
    interface AMSend as RadioSend;
    interface Receive as RadioReceive;
    interface Packet as RadioPacket;
    interface AMPacket as RadioAMPacket;
	
	interface StdControl as UartControl;
	interface UartByte;
	interface UartStream;
    interface Leds;
	
	interface BusyWait<TMicro,uint16_t>;

	interface PacketAcknowledgements as packetAck ;
  }
}

implmentation
{
 uint8_t tosID;
 uint16_t bufferLen = 50;
 message_t *radioBuf[50];
 bool flagBuf[50];
 uint16_t sent=0;
 uint16_t empty=0;
 uint8_t dataNumber=0;
 uint8_t noAck = 0;
 bool noConnection=FALSE;
 bool bufferFull = FALSE ;
 bool sentEmpty = TRUE ;
   bool busy = FALSE;
 message_t pkt;
 message_t pkt2;
 message_t pkt4;
 message_t pkt5;
 bool flag = FALSE;

 
 bool store(uint8_t id,uint8_t data);
 void sendCmnd(uint8_t n);
/////////////////
event void Boot.booted() {
	uint8_t i;
	atomic tosID = TOS_NODE_ID;
	for(i=0;i<bufferLen;i++)
		flagBuf[i]=FALSE; // all empty at the begining
	call UartControl.start();
	call RadioControl.start();
}
///////////////////
event message_t *RadioReceive.receive(message_t *msg,void *payload,uint8_t len){


}
//////////////////
event void dataTimer.fired() {
	if (dataNumber ==250)
		dataNumber=0;
	if ( store(tosID,dataNumber) )
		dataNumber = dataNumber +2;
	call Leds.led0Toggle();
}
//////////////////
bool store(uint8_t id,uint8_t data) {
	iRobotRadioMsg* btrpkt2 = (iRobotRadioMsg*)(call RadioPacket.getPayload(&pkt2, sizeof(iRobotRadioMsg)));
	btrpkt2->nodeid = id;
	btrpkt2->data = data;
	call packetAck.requestAck( &pkt2);
	atomic if(!bufferFull){
		radioBuf[empty] = &pkt2 ;
		flagBuf[empty] = TRUE ;
		if(sentEmpty)
			sentEmpty = FALSE; 
		empty++;
		if  (empty == bufferLen)
			empty=0;
		if(flagBuf[empty] == TRUE )
			bufferFull = TRUE ;
		return TRUE ;
		}
	else
		return FALSE;
}
//////////////////




} // end of implementation
