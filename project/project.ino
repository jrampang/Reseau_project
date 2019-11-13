#include <XBee.h> 

#define XBEE_UART1

// Route Request Signature
#define Q RREQ
// Route Reply
#define P RREP
// Data Request
#define M DATA

// Notre adresse
#define MY_ADDRESS 0x40762053

// Baussian / Jankowiak
#define BAUSSIAN_JANKOWIAK 0x406FB3D3
#define CAMPERGUE_CEBOLLADO 0x4086D834
#define BIGOT_PEDEBEARN 0x40762056

// Request sign
char sign;
// Request number
char id;
// Request destinataire
char dst[3];
// Request source
char src[3];

char msg[100];

XBeeAddress64 addr;
char StringAddr;

XBee xbee = XBee();

uint8_t* payload;
uint8_t payload_len;

Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

void setup() {
  
  Serial.begin(38400);
  Serial.println("Arduino. Will receive packets.");
  
#ifdef XBEE_UART1
  //xbee.setSerial(Serial1);
  // Inits the XBee 802.15.4 library
  //Serial1.begin(38400);
#else        
  xbee.begin(38400);
#endif  
}

void loop() {

  // read incoming pkt
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    
    Serial.println("receive");
    
    // is it a response to the previously sent packet?              
    if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
    
    }    
    
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {
    
      if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
        xbee.getResponse().getRx16Response(rx16);
        
        payload = rx16.getData();
        payload_len=rx64.getDataLength();     
      } 
      else 
      {
        xbee.getResponse().getRx64Response(rx64);
        
        payload = rx64.getData();
        payload_len=rx64.getDataLength();
        addr = rx64.getRemoteAddress64();
      }
          
      for (int i=0; i<payload_len; i++){
        if(i == 0)
          sign = (char)payload[i];
        if(i == 1)
          id = (char)payload[i];
        if(i == 2){
          dst[0] = (char)payload[i];
          dst[1] = (char)payload[i+1];
        }
        if(i == 4){
          src[0] = (char)payload[i];
          src[1] = (char)payload[i+1];
        }
        Serial.print((char)payload[i]);
      }
      
      //sign = (char)payload[0];
      //dst[0] = (char)payload[1];
      //dst[1] = (char)payload[2];
      //src[0] = (char)payload[3];
      //src[1] = (char)payload[4];
      
      Serial.println();
      Serial.println();
      sprintf(msg, "%s%c","Request type: ", sign);
      Serial.println(msg);
      sprintf(msg, "%s%c","Request id: ", id);
      Serial.println(msg);
      sprintf(msg, "%s%s","Request destinataire: ", dst);
      Serial.println(msg);
      sprintf(msg, "%s%s","Request source: ", src);
      Serial.println(msg);
      if(sign == 'Q'){
        Serial.println("I've received a Route Request.");
        if(dst == "53")
          Serial.println("The RREQ is for me.");
        else{
          Serial.println("I must propaged the RREQ");
        }  
      }
      else if(sign == 'R'){
        Serial.println("I've received a Route Reply.");
        if(src == "53")
          Serial.println("I can send a Data Request.");
        else
          Serial.println("I must transfert the Route Reply.");
      }
      else if(sign == 'M'){
        Serial.println("I've received a Data Request.");
        if(dest == "53"){
          Serial.println("I display the data.");
        }
        else
          Serial.println("I transfert the data request .");
      }
      else{
        Serial.println("I don't understand what I've received a Route Request.");
      }
      
      //Serial.println(addr.getLsb(), HEX);
      Serial.println("\ndone");  
    } 
  } 
}

 
