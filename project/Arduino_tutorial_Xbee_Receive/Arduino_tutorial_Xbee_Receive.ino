#include <XBee.h>

#define XBEE_UART1

// Our address
#define MY_ADDRESS 0x40762053
// Our id (53 officially, any other value is for the tests)
#define ME "53"

// Our neighbours address
#define BAUSSIAN_JANKOWIAK 0x406FB3D3
#define CAMPERGUE_CEBOLLADO 0x4086D834
#define BIGOT_PEDEBEARN 0x40762056

// DSR protocol requests
#define ROUTE_REQUEST 'Q' 
#define ROUTE_REPLY 'P' 
#define DATA_DELIVERY 'M'

// for debugging
#define DEBUG true

// 64-bit addressing: This is the SH + SL address of remote XBee knowned to us
XBeeAddress64 addr1 = XBeeAddress64(0x0013A200, BAUSSIAN_JANKOWIAK);
XBeeAddress64 addr2 = XBeeAddress64(0x0013A200, CAMPERGUE_CEBOLLADO);
XBeeAddress64 addr3 = XBeeAddress64(0x0013A200, BIGOT_PEDEBEARN);

Tx64Request tx;

/*
 * Receive phase
 * 
 */

// Request sign
char sign;
//Request id
char id;
// Request destinataire
char dst[3];
// Request source
char src[3];
// Route Reply id
int msg_id = 0;
// message / request received
char msg_received[100];
// past message / request received
char received[20][6];

// current abscisse for the array
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

int randRslt = 0;

/*
 * Send phase
 * 
 * 
 */

// payload send
uint8_t payload_to_send[100];
// the others nodes in the network
char networks_nodes[11][3];
// the data we want to send
char data[] = "Our DSR protocol implementation is a success";
/*
 * display_msg_information(char msg_received[], int payload_len)
 * 
 * char msg_received[]: char array to display
 * int payload_len:     char array length to display
 * 
 * return nothing
 * 
 * This method purpose is for display the message(s)/request(s) data received
 * and debugging
 * 
 */
void display_msg_information(char msg_received[], int payload_len){
    // char array to display
    char msg[100];
    // request/message id
    char id;
    // request recipient/target
    char dst[3];
    // request source/sender
    char src[3];

    // we unstack the message/request 
    // to ease ("faciliter" pour les anglophobes) the displays
    id = (char)msg_received[1];
    dst[0] = (char)msg_received[2];
    dst[1] = (char)msg_received[3];
    dst[2] = '\0';
    src[0] = (char)msg_received[4];
    src[1] = (char)msg_received[5];
    src[2] = '\0';
    
    Serial.println();
    sprintf(msg, "%s = %d","payload length", payload_len);
    Serial.println(msg);
    sprintf(msg, "%s%c","Request identification: ", id);
    Serial.println(msg);
    sprintf(msg, "%s%s","Request destinataire: ", dst);
    Serial.println(msg);
    sprintf(msg, "%s%s","Request source: ", src);
    Serial.println(msg);
    Serial.println();
}

/*
 * check_transmit_status()
 * 
 * return nothing
 * 
 * This method read incoming pkt to get the transmit status
 * 
 */ 
void check_transmit_status(){
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

/*
 * send_route_request(XBeeAddress64 addr, char req[])
 * 
 * XBeeAddress64 addr:  the address to which we send the request
 * char req[]:          the request to send
 * 
 * return nothing
 * 
 * This method send a route request to a specified address
 * and listen to confirm or not if the send was successful or not
 * 
 */ 
void send_route_request(XBeeAddress64 addr, char req[]){
    
    char request[strlen(req)] = "";
    
    sprintf(request, "%s%s", req, ME);
    
    if(DEBUG){
      Serial.print("I spread: ");
      Serial.println(request);
    }
    
    for (int i = 0; i < sizeof(request); i++)
      payload_to_send[i]=(uint8_t)request[i];
      
    tx = Tx64Request(addr, (uint8_t*)request, sizeof(request));
    
    // Send the request
    xbee.send(tx);

    check_transmit_status();
}

/*
 * spread_route_request(char req[])
 * 
 * char req[]:  the request to spread
 * 
 * return nothing
 * 
 * This method spread the Route Request received
 * by calling 3 times the send_route_request
 * to send the request to the nodes known to us
 * 
 */ 
void spread_route_request(char req[]){
    if(DEBUG){
        Serial.print("Route request to spread: ");
        Serial.println(req);
        Serial.println();
    }
    
    send_route_request(addr1, req);
    send_route_request(addr2, req);
    send_route_request(addr3, req);
}

/*
 * send_route_reply(XBeeAddress64 addr, char req[])
 * 
 * XBeeAddress64 addr:  the address to which we send the request
 * char req[]:          the request to send
 * 
 * return nothing
 * 
 * This method send a route reply to a specified address
 * and listen to confirm or not if the send was successful or not
 * 
 */ 
void send_route_reply(XBeeAddress64 addr, char req[]){
    
    char reply[strlen(req)] = "";
    
    sprintf(reply, "%s%s", req, "");
    reply[0] = ROUTE_REPLY;

    char temp[3] = "";
    temp[0] = reply[2];
    temp[1] = reply[3];
    
    reply[2] = reply[4];
    reply[3] = reply[5];

    reply[4] = temp[0];
    reply[5] = temp[1];
    
    if(DEBUG){
      Serial.print("I reply: ");
      Serial.println(reply);
    }
    
    for (int i = 0; i < sizeof(reply); i++)
      payload_to_send[i]=(uint8_t)reply[i];
      
    tx = Tx64Request(addr, (uint8_t*)reply, sizeof(reply));
    
    // Send the request
    xbee.send(tx);

    check_transmit_status();
}

/*
 * transfer_to(XBeeAddress64 addr, char req[])
 * 
 * XBeeAddress64 addr:  the address to which we send the request
 * char req[]:          the request to send
 * 
 * return nothing
 * 
 * This method transfer a route reply or a data delivery to a specified address
 * and listen to confirm or not if the send was successful or not
 * 
 */ 
void transfer_to(XBeeAddress64 addr, char req[]){
    char reply[strlen(req)] = "";
    
    sprintf(reply, "%s%s", req, "");
    
    if(DEBUG){
      Serial.print("I transfer: ");
      Serial.println(reply);
    }
    
    for (int i = 0; i < sizeof(reply); i++)
      payload_to_send[i]=(uint8_t)reply[i];
      
    tx = Tx64Request(addr, (uint8_t*)reply, sizeof(reply));
    
    // Send the request
    xbee.send(tx);

    check_transmit_status();
}

/*
 * send_data_delivery(XBeeAddress64 addr, char req[])
 * 
 * XBeeAddress64 addr:  the address to which we send the request
 * char req[]:          the request to send
 * 
 * return nothing
 * 
 * This method send a route reply to a specified address
 * and listen to confirm or not if the send was successful or not
 * 
 */ 
void send_data_delivery(XBeeAddress64 addr, char req[], char dataParam[]){

    char data_to_send[strlen(req) + strlen(dataParam) + 1] = "";
    
    sprintf(data_to_send, "%s%s%s%c", req, dataParam, '\0');
    data_to_send[0] = DATA_DELIVERY;

    char temp[3] = "";
    temp[0] = data_to_send[2];
    temp[1] = data_to_send[3];
    
    data_to_send[2] = data_to_send[4];
    data_to_send[3] = data_to_send[5];

    data_to_send[4] = temp[0];
    data_to_send[5] = temp[1];
    
    if(DEBUG){
      Serial.print("I send a data delivery: ");
      Serial.println(data_to_send);
    }
    
    for (int i = 0; i < sizeof(data_to_send); i++)
      payload_to_send[i]=(uint8_t)data_to_send[i];
      
    tx = Tx64Request(addr, (uint8_t*)data_to_send, sizeof(data_to_send));
    
    // Send the request
    xbee.send(tx);

    check_transmit_status();
}

void receive(){
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
              sprintf(msg_received, "%s%c", msg_received, (char)payload[i]);
          }
  
  
          sign = (char)payload[0];
          id = (char)payload[1];
          dst[0] = (char)payload[2];
          dst[1] = (char)payload[3];
          dst[2] = '\0';
          src[0] = (char)payload[4];
          src[1] = (char)payload[5];
          src[2] = '\0';
          //Serial.println(addr.getLsb(), HEX);
  
          if(DEBUG){
              // Display message received
              Serial.print("msg received: ");
              Serial.println(msg_received);
          }  
          
          // Check if the message has already been received
          // if not: save it
          // else we shouldn't compute it
          // but instead dump it
          
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
          // if the message haven't been found
          // we save it
          // else we do nothing
          if (!(trouve)){
              for(int i = 0; i<6; i++){
                received[nbReceived%20][i] = (char)payload[i];
              }
              
              if(DEBUG)
                  Serial.println("saved");
                  
              nbReceived++;
              trouve=0;
              i=0;
          }
          else{
              if(DEBUG)
                  Serial.println("deja recu");
                  
              trouve=0;
              i=0;
          }
    
          // if the message sign is known
          // we compute it
          // else we don't understand
    
          if(sign == ROUTE_REQUEST){
              // if it is a Route Request
              // we check if we are the recipient (destinataire)
              // if we are
              // we send a Route Reply to the source
              // else we spread the request after having added to the message (header) our id
              
              if(DEBUG){
                  Serial.println("I've received a Route Request.");
                  display_msg_information(msg_received, payload_len);
                  if(strcmp(dst, ME) == 0){
                    Serial.println("I'm the recipient.");
                    send_route_reply(addr3, msg_received);
                  }
              }
      
              // If I'm the recipient
              // I can respond with a Route Reply
              // else I must spread the Route Request 
              
              if(strcmp(dst, ME) == 0){
                  // if we are the recipient
                  // we send a Route Reply to the source

                  char last_node[3] = "";
                  last_node[0] = msg_received[payload_len-2];
                  last_node[1] = msg_received[payload_len-1];
                  last_node[2] = '\0';
                  
                  if(DEBUG){
                      Serial.print("I'm must send a Route Reply to: ");
                      Serial.println(src);
                      Serial.print("The node which send me the Route Request: ");
                      Serial.println(last_node);
                  }
                  
                  if(strcmp(last_node, "56") == 0){
                      if(DEBUG)
                          Serial.println("I send the Route Reply to BIGOT_PEDEBEARN.");

                      send_route_reply(addr3, msg_received);
                  }
                  else if (strcmp(last_node, "34") == 0){
                      if(DEBUG)
                          Serial.println("I send the Route Reply to CAMPERGUE_CEBOLLADO.");

                      send_route_reply(addr2, msg_received);
                  }
                  else if (strcmp(last_node, "D3") == 0){
                      if(DEBUG)
                          Serial.println("I send the Route Reply to BAUSSIAN_JANKOWIAK.");

                      send_route_reply(addr1, msg_received);
                  }
              }
              else {
                  // I am not the recipient then
                  // I spread the Route Request
                  // to node(s) known to us
                  
                  if(DEBUG)
                      Serial.println("I must spread the Route Request.");
                          
                  spread_route_request(msg_received);
              }
          }
          else if(sign == ROUTE_REPLY){
              // else if it is a Route Reply
              // we check if we are the recipient (destinataire)
              // if we are
              // it means our Route Request reached our target
              // and so we send a Data Delivery to it
              // else we transfer the Route Reply according to the ids include in the header
              
              if(DEBUG){
                  Serial.println("I've received a Route Reply.");
                  display_msg_information(msg_received, payload_len);
              }

              // If I'm the recipient
              // I can send a Data Delivery
  
              if(strcmp(dst, ME) == 0){
                  // if we are the recipient
                  // we send a Data Delivery to the source

                  char last_node[3] = "";
                  last_node[0] = msg_received[payload_len-2];
                  last_node[1] = msg_received[payload_len-1];
                  last_node[2] = '\0';
                  
                  if(DEBUG){
                      Serial.print("I can send a Data Delivery to: ");
                      Serial.println(src);
                      Serial.print("The node which send me the Route Reply: ");
                      Serial.println(last_node);
                  }
                  
                  if(strcmp(last_node, "56") == 0){
                      if(DEBUG)
                          Serial.println("I send the Data Delivery to BIGOT_PEDEBEARN.");

                      send_data_delivery(addr3, msg_received, data);
                  }
                  else if (strcmp(last_node, "34") == 0){
                      if(DEBUG)
                          Serial.println("I send the Data Delivery to CAMPERGUE_CEBOLLADO.");

                      send_data_delivery(addr2, msg_received, data);
                  }
                  else if (strcmp(last_node, "D3") == 0){
                      if(DEBUG)
                          Serial.println("I send the Data Delivery to BAUSSIAN_JANKOWIAK.");

                      send_data_delivery(addr1, msg_received, data);
                  }
              }
              else {
                  // if I am not the recipient then
                  // I transfer the Route Reply
                  // to the recipient directly if I know him
                  // else 
                  // I transfert the Route Reply
                  // to the node which id is ahead of mine in the header
                  
                  if(DEBUG){
                      Serial.println("I must transfer a Route Reply.");
                  }
                  
                  // if I known the recipient
                  // I transfer the Route Reply directly to him
                  
                  if(strcmp(dst, "56") == 0){
                      if(DEBUG){
                          Serial.println("I known the recipient.");
                          Serial.println("I transfer the Route Reply directly to BIGOT_PEDEBEARN.");
                      }
                      transfer_to(addr3, msg_received);
                  }
                  else if (strcmp(dst, "34") == 0){
                      if(DEBUG){
                          Serial.println("I known the recipient.");
                          Serial.println("I transfer the Route Reply directly to CAMPERGUE_CEBOLLADO.");
                      }
                      transfer_to(addr2, msg_received);
                  }
                  else if (strcmp(dst, "D3") == 0){
                      if(DEBUG){
                          Serial.println("I known the recipient.");
                          Serial.println("I transfer the Route Reply directly to BAUSSIAN_JANKOWIAK.");
                      }
                      transfer_to(addr1, msg_received);
                  }
                  else{
                    // I don't known the recipient
                    // I transfert the Route Reply
                    // to the node which id is ahead of mine in the header
                    
                    char target[3] = "";
                    if(DEBUG){
                        Serial.println("I don't known the recipient.");
                    }
                    
                    for(int i = 6; i < strlen(msg_received); i++){
                        if((msg_received[i] == ME[0])&&(msg_received[i+1] == ME[1])){
                            target[0] = msg_received[i-2];
                            target[1] = msg_received[i-1];
                            target[2] = '\0';
                            if(DEBUG){
                                Serial.print("I transfer the Route Reply to: ");
                                Serial.println(target);
                            }
                        }
                    }
                    if(strcmp(target, "56") == 0){
                        if(DEBUG){
                            Serial.println("I transfer the Route Reply to BIGOT_PEDEBEARN.");
                        }
                        transfer_to(addr3, msg_received);
                    }
                    else if(strcmp(target, "34") == 0){
                        if(DEBUG){
                            Serial.println("I transfer the Route Reply to CAMPERGUE_CEBOLLADO.");
                        }
                        transfer_to(addr2, msg_received);
                    }
                    else if(strcmp(target, "D3") == 0){
                        if(DEBUG){
                            Serial.println("I transfer the Route Reply to BAUSSIAN_JANKOWIAK.");
                        }
                        transfer_to(addr1, msg_received);
                    }
                 }
              }
          }
          // else if it is a Data Request
          else if(sign == DATA_DELIVERY){
              // else if it is a Data Delivery
              // we check if we are the recipient (destinataire)
              // if we are
              // it means our Route Reply reached our target
              // which first sent a Route Request to us 
              // and so we can just display the data
              
              if(DEBUG){
                  Serial.println("I've received a data delivery.");
                  display_msg_information(msg_received, payload_len);
              }

              // If I'm the recipient
              // I can display the data

              if(strcmp(dst, ME) == 0){
                  if(DEBUG){
                      Serial.println("I am the recipient.");
                      Serial.println("I display the data.");
                  }
                  Serial.print("The data: ");
                  for(int i = 1; i < strlen(msg_received); i++){
                       if(!(isDigit(msg_received[i]))){
                            Serial.print(msg_received[i]);
                       }
                  }
              }
              else{
                  // I am not the recipient then
                  // I transfer the Data Delivery
                  // to the recipient directly if I know him
                  // else 
                  // I transfert Data Delivery
                  // to the node which id is ahead of mine in the header
                  
                  if(DEBUG){
                      Serial.println("I transfer the data.");
                  }

                  // if I known the recipient
                  // I transfer the Data Delivery directly to him
                  
                  if(strcmp(dst, "56") == 0){
                      if(DEBUG){
                          Serial.println("I known the recipient.");
                          Serial.println("I transfer the Data Delivery directly to BIGOT_PEDEBEARN.");
                      }
                  }
                  else if (strcmp(dst, "34") == 0){
                      if(DEBUG){
                          Serial.println("I known the recipient.");
                          Serial.println("I transfer the Data Delivery directly to CAMPERGUE_CEBOLLADO.");
                      }
                  }
                  else if (strcmp(dst, "D3") == 0){
                      if(DEBUG){
                          Serial.println("I known the recipient.");
                          Serial.println("I transfer the Data Delivery directly to BAUSSIAN_JANKOWIAK.");
                      }
                  }
                  else{
                    // I don't known the recipient
                    // I transfert the Data Delivery
                    // to the node which id is ahead of mine in the header
                    
                    char target[3] = "";
                    if(DEBUG){
                        Serial.println("I don't known the recipient.");
                    }
                    
                    for(int i = 6; i < strlen(msg_received); i++){
                        if((msg_received[i] == ME[0])&&(msg_received[i+1] == ME[1])){
                            target[0] = msg_received[i-2];
                            target[1] = msg_received[i-1];
                            target[2] = '\0';
                            if(DEBUG){
                                Serial.print("I transfer the Data Delivery to: ");
                                Serial.println(target);
                            }
                        }
                    }
                    if(strcmp(target, "56") == 0){
                        if(DEBUG){
                            Serial.println("I transfer the Data Delivery to BIGOT_PEDEBEARN.");
                        }
                        transfer_to(addr3, msg_received);
                    }
                    else if(strcmp(target, "34") == 0){
                        if(DEBUG){
                            Serial.println("I transfer the Data Delivery to CAMPERGUE_CEBOLLADO.");
                        }
                        transfer_to(addr2, msg_received);
                    }
                    else if(strcmp(target, "D3") == 0){
                        if(DEBUG){
                            Serial.println("I transfer the Data Delivery to BAUSSIAN_JANKOWIAK.");
                        }
                        transfer_to(addr1, msg_received);
                    }
                 }
              }
          }
          else{
              // the message is wrong formed
              Serial.write("ERROR: I don't understand what I've received.");
              if(DEBUG){
                  //display_msg_information(msg_received, payload_len);
              }
          }
    
          sprintf(msg_received, "","","");
  
          msg_id = (msg_id == 9) ? 0 : (msg_id++);
          
          //Serial.println(addr.getLsb(), HEX);
          Serial.println("\nReceive() done");
      } 
    } 
}

void setup() {
  
  Serial.begin(38400);
  Serial.println("Arduino. Will receive packets.");

  // Campergue / Cebollado
  networks_nodes[0][0] = '3';
  networks_nodes[0][1] = '4';
  // Baussian / Jankowiak
  networks_nodes[1][0] = 'D';
  networks_nodes[1][1] = '3';
  // Bigot / Pedebearn
  networks_nodes[2][0] = '5';
  networks_nodes[2][1] = '6';
  // Gnebehi / Delbouys
  networks_nodes[3][0] = '2';
  networks_nodes[3][1] = '2';
  // Kouami-kodia
  networks_nodes[4][0] = 'E';
  networks_nodes[4][1] = '5';
  // Boljonilla / Pham
  networks_nodes[5][0] = '3';
  networks_nodes[5][1] = '6';
  // Raf / Sofiane
  networks_nodes[6][0] = '2';
  networks_nodes[6][1] = '8';
  // Hincelin
  networks_nodes[7][0] = 'E';
  networks_nodes[7][1] = '9';
  // Decla / Guillerme
  networks_nodes[8][0] = '2';
  networks_nodes[8][1] = 'E';
  // Guicharousse / Bleu
  networks_nodes[9][0] = '5';
  networks_nodes[9][1] = 'E';
  // Franceze / Le Franc
  networks_nodes[10][0] = '7';
  networks_nodes[10][1] = '0';
  
#ifdef XBEE_UART1
  xbee.setSerial(Serial1);
  // Inits the XBee 802.15.4 library
  Serial1.begin(38400);
#else        
  xbee.begin(38400);
#endif  
}

void loop() {
    // in this infinite loop
    // we try to receive the request
    // send by the other nodes belonging to the network

    // we listen for a request/message send to us
    // the message will be compute if valid
    // else dump
    receive();
    
    // to help the debug process
    delay(1000);
}
