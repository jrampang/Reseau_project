#include <XBee.h>

#define XBEE_UART1

// Notre adresse 

// Baussian / Jankowiak
#define BAUSSIAN_JANKOWIAK 0x406FB3D3
#define CAMPERGUE_CEBOLLADO 0x4086D834
#define BIGOT_PEDEBEARN 0x40762056

#define Q RREQ
#define P RREP
#define M DATA

XBee xbee = XBee();

uint8_t payload[100];

char sign[1];

// 
char RREQ[96]="Q036";

char data[96]="Test pour Johan";
 

void setup() {
  
  Serial.begin(38400);
  Serial.println("Arduino. Will send packets.");
  
  sprintf(RREQ, "%s%s", RREQ, "53");
  
#ifdef XBEE_UART1
  xbee.setSerial(Serial1);
  // Inits the XBee 802.15.4 library
  Serial1.begin(38400);
#else        
  xbee.begin(38400);
#endif  
}

void loop() {
  
  delay(5000);
    
  // 64-bit addressing: This is the SH + SL address of remote XBee knowned to us
  XBeeAddress64 addr1 = XBeeAddress64(0x0013A200, BAUSSIAN_JANKOWIAK);
  XBeeAddress64 addr2 = XBeeAddress64(0x0013A200, CAMPERGUE_CEBOLLADO);
  XBeeAddress64 addr3 = XBeeAddress64(0x0013A200, BIGOT_PEDEBEARN);

  // XBeeAddress64 broodcast = XBeeAddress64(0x00000000, 0x0000FFFF);
  
  // unless you have MY on the receiving radio set to FFFF, this will be received as a RX16 packet
  
  for (int i; i<sizeof(RREQ); i++)
    payload[i]=(uint8_t)RREQ[i];
  
  // in this way, we know the exact size of the payload

  // Preparing the 3 requests to the XBee which existence are knowned
  Tx64Request tx1 = Tx64Request(addr1, (uint8_t*)RREQ, sizeof(RREQ));
  Tx64Request tx2 = Tx64Request(addr2, (uint8_t*)RREQ, sizeof(RREQ));
  Tx64Request tx3 = Tx64Request(addr3, (uint8_t*)RREQ, sizeof(RREQ));

  // Tx64Request txBroodcast = Tx64Request(broodcast, (uint8_t*)RREQ, sizeof(RREQ));

  // Print our request's sign
  Serial.println(RREQ[0]);
  // Print our request
  Serial.println(RREQ);
  
  // Send your requests
  xbee.send(tx1);
  xbee.send(tx2);
  xbee.send(tx3);
  // xbee.send(txBroodcast);

  TxStatusResponse txStatus = TxStatusResponse();
  
  // read incoming pkt, mainly to get the transmit status
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {        
          // is it a response to the previously sent packet?            	
        if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
              
                
              xbee.getResponse().getTxStatusResponse(txStatus);	  
          
                if (txStatus.isSuccess()) {
                      Serial.println("Send successful");
                } else
                  Serial.println("Send failed");
                  
          }
  }              
}
