#include <XBee.h>

#define XBEE_UART1

// Baussian / Jankowiak
#define BAUSSIAN_JANKOWIAK 0x406FB3D3
#define CAMPERGUE_CEBOLLADO 0x4086D834
#define BIGOT_PEDEBEARN 0x40762056

#define Q RREQ
#define P RREP
#define M DATA

// Request sign
char sign;
//Request id
char id;
// Request destinataire
char dst[3];
// Request source
char src[3];

uint8_t payload_to_send[100];

char msg_received[100];

char msg[100];

char received[20][6];

int nbReceived = 0;

bool trouve = 0;
bool ok = 1;

XBeeAddress64 addr;
char StringAddr;

XBee xbee = XBee();

uint8_t* payload;
uint8_t payload_len;

Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

bool addr_known(char * addr){
  bool known = false;
  if((addr[0] == '5')&&(addr[1] == '6')){
    //Serial.println("Stan and Aymeric send a Route Request.");
    known = true;
  }
  else if ((addr[0] == '3')&&(addr[1] == '4')){
    //Serial.println("Johan and Dylan send a Route Request.");
    known = true;
  }
  else if ((addr[0] == 'D')&&(addr[1] == '3')){
    //Serial.println("Florian and Anthony send a Route Request.");
    known = true;
  }
  return known;
}

void setup() {
  
  Serial.begin(38400);
  Serial.println("Arduino. Will receive packets.");
  
#ifdef XBEE_UART1
  xbee.setSerial(Serial1);
  // Inits the XBee 802.15.4 library
  Serial1.begin(38400);
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
      Serial.println();    
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
        //Serial.print((char)payload[i]);
        sprintf(msg_received, "%s%c", msg_received, (char)payload[i]);
      }
      //sign = (char)payload[0];
      //id = (char)payload[1];
      //dst[0] = (char)payload[2];
      //dst[1] = (char)payload[3];
      //src[0] = (char)payload[4];
      //src[1] = (char)payload[5];
      //Serial.println(addr.getLsb(), HEX);

      // Display message received
      Serial.println("msg_received:");
      Serial.println(msg_received);

      // Check if the message has already been received
      // if not: save it
      // else we shouldn't compute it
      int i = 0;
      while((!(trouve)) && (i < nbReceived%20)){
        int j = 0;
        while(ok && (j<6)){
          ok = (received[i][j] == msg_received[j]);
          j++;
        }
        for(int k = 0; k<6; k++){
          //Serial.print(received[i][k]);
        }
        //Serial.println();
        trouve = ok;
        i++;
        ok = 1;
      }
      if (!(trouve)){
        for(int i = 0; i<6; i++){
          received[nbReceived%20][i] = (char)payload[i];
        }
        Serial.println("saved");
        nbReceived++;
      }
      else{
        Serial.println("deja recu");
      }
      trouve=0;
      i=0;

      // if message sign is known
      // we compute it
      // else we don't understand

      // if it is a Route Request
      if(sign == 'Q'){
        Serial.println();
        sprintf(msg, "%s = %d","payload length", payload_len);
        Serial.println(msg);
        Serial.println("I've received a Route Request.");
        //sprintf(msg, "%s%c","Request type: ", sign);
        //Serial.println(msg);
        /*sprintf(msg, "%s%c","Request identification: ", id);
        Serial.println(msg);
        sprintf(msg, "%s%s","Request destinataire: ", dst);
        Serial.println(msg);
        sprintf(msg, "%s%s","Request source: ", src);
        Serial.println(msg);*/
        /*
        if((src[0] == '5')&&(src[1] == '6')){
          Serial.println("Stan and Aymeric send a Route Request.");
        }
        else if ((src[0] == '3')&&(src[1] == '4')){
          Serial.println("Johan and Dylan send a Route Request.");
        }
        else if ((src[0] == 'D')&&(src[1] == '3')){
          Serial.println("Florian and Anthony send a Route Request.");
        }*/

        // If I'm the recipient
        // I can respond with a Route Reply
        if((dst[0] == '5')&&(dst[1] == '3')){
            Serial.print("I'm must send a Route Reply to: ");
            Serial.println(src);
        }
        // else I spread the Route Request to address I known
        else {
            Serial.println("I must spread the Route Request.");
            XBeeAddress64 addr1 = XBeeAddress64(0x0013A200, BAUSSIAN_JANKOWIAK);
            XBeeAddress64 addr2 = XBeeAddress64(0x0013A200, CAMPERGUE_CEBOLLADO);
            XBeeAddress64 addr3 = XBeeAddress64(0x0013A200, BIGOT_PEDEBEARN);
  
            sprintf(msg_received, "%s%s", msg_received, "53");
            
            for (int i; i<sizeof(msg_received); i++)
              payload_to_send[i]=(uint8_t)msg_received[i];
  
            Tx64Request tx1 = Tx64Request(addr1, (uint8_t*)msg_received, sizeof(msg_received));
            Tx64Request tx2 = Tx64Request(addr2, (uint8_t*)msg_received, sizeof(msg_received));
            Tx64Request tx3 = Tx64Request(addr3, (uint8_t*)msg_received, sizeof(msg_received));
  
            // Print our request's sign
            Serial.println("I'm spreading: ");
            // Print our request
            Serial.println(msg_received);
            
            // Send the request
            xbee.send(tx1);
            xbee.send(tx2);
            xbee.send(tx3);
  
            TxStatusResponse txStatus = TxStatusResponse();
    
            // read incoming pkt, mainly to get the transmit status
            xbee.readPacket();
          
            if(xbee.getResponse().isAvailable()) {        
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
      }
      // else if it is a Route Reply
      else if(sign == 'P'){
        Serial.println();
        sprintf(msg, "%s = %d","payload length", payload_len);
        Serial.println(msg);
        Serial.println("I've received a Route Reply");
        //sprintf(msg, "%s%c","Request type: ", sign);
        //Serial.println(msg);
        /*sprintf(msg, "%s%c","Request identification: ", id);
        Serial.println(msg);
        sprintf(msg, "%s%s","Request destinataire: ", dst);
        Serial.println(msg);
        sprintf(msg, "%s%s","Request source: ", src);
        Serial.println(msg);*/
      }
      // else if it is a Data Request
      else if(sign == 'M'){
        Serial.println();
        sprintf(msg, "%s = %d","payload length", payload_len);
        Serial.println(msg);
        Serial.println("I've received a message");
        //sprintf(msg, "%s%c","Request type: ", sign);
        //Serial.println(msg);
        /*sprintf(msg, "%s%c","Request identification: ", id);
        Serial.println(msg);
        sprintf(msg, "%s%s","Request destinataire: ", dst);
        Serial.println(msg);
        sprintf(msg, "%s%s","Request source: ", src);
        Serial.println(msg);*/
      }
      // else the message is wrong formed
      else{
        Serial.println("I don't understand what I've received.");
      }

      sprintf(msg_received, "","","");
      
      //Serial.println(addr.getLsb(), HEX);
      Serial.println("\ndone");
      delay(1000);
    } 
  } 
}

 
