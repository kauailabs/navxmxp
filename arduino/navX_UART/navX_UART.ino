#include <SoftwareSerial.h>
#include "AHRSProtocol.h"
int rx = 8;
int tx = 9;
SoftwareSerial serport(rx,tx);

byte data[512];
char protocol_buffer[128];

#define ITERATION_DELAY_MS    10

void setup()
{
  Serial.begin(115200);
  serport.begin(57600);
  for ( int i = 0; i < sizeof(data); i++ ) {
      data[i] = 0;
  }
  int bytes_to_send = IMUProtocol::encodeStreamCommand( protocol_buffer, 
        MSGID_YPR_UPDATE, 60);  
  serport.write((const uint8_t *)protocol_buffer,bytes_to_send);
}

int loop_count = 0;
int first = 0;

bool packet_start_found = false;
int packet_byte_index = 0;
bool packet_end_found = false;
  
void loop()
{
  int uart_bytes_received = 0;
  
  while(serport.available()) {
    if ( first == 0 ) {
        first = 1;
    }
    byte b = (char)serport.read();
    uart_bytes_received++;

    if (b == PACKET_START_CHAR) {
      packet_start_found = true;
      packet_end_found = false;
      packet_byte_index = 0;
    } else if (b == 10 /* LineFeed */) {
      packet_end_found = true;
    }
    if (packet_start_found) {
      data[packet_byte_index++] = b;
      if (packet_end_found) {
        if (data[packet_byte_index-2] == 13 /* Carriage Return */) {
          if (data[1] == MSGID_YPR_UPDATE) {
            struct IMUProtocol::YPRUpdate ypr;
            if (IMUProtocol::decodeYPRUpdate((char *)data,packet_byte_index, ypr)){              
              /* Display orientation values */
              Serial.print("yaw:  ");
              Serial.print(ypr.yaw,2);
              Serial.print("  pitch:  ");
              Serial.print(ypr.pitch,2);
              Serial.print("  roll:  ");
              Serial.print(ypr.roll,2);
              Serial.print("  heading:  ");
              Serial.print(ypr.compass_heading,2);  
              Serial.println("");                
            }
          }
        }
        packet_start_found = false;
      }
    }
    
    /* If max chars received within the iteration period */
    /* take a break */
    if ( uart_bytes_received > 100 ) {
      Serial.println();
      break;
    }
  }
  delay(ITERATION_DELAY_MS);
}
