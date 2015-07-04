#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "AHRSProtocol.h"
int ss = 10;
int rx = 8;
int tx = 9;
SoftwareSerial serport(rx,tx);

byte x = 0;
byte y = 0;
byte data[512];
char protocol_buffer[128];

#define TEST_I2C
#define TEST_SPI
#define TEST_UART
#define TEST_STREAM_UPDATE_RATE_CHANGE
#define TUNING_VAR_TEST 
//#define I2C_UPDATE_RATE_TEST
#define SPI_UPDATE_RATE_TEST
#define ITERATION_DELAY_MS 10
void tuning_var_test();
void periodic_update_rate_modify_i2c();

void setup()
{
  Serial.begin(115200);
  Wire.begin(); // join i2c bus (address optional for master)
  pinMode(ss,OUTPUT);
  digitalWrite(SS, HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(8); /* 16Mhz/32 = 500kHz; /16=1Mhz; /8=2Mhz */ 
  serport.begin(57600);
  for ( int i = 0; i < sizeof(data); i++ ) {
      data[i] = 0;
  }
  int bytes_to_send = IMUProtocol::encodeStreamCommand( protocol_buffer, 
        MSGID_YPR_UPDATE, 60);  
  serport.write((const uint8_t *)protocol_buffer,bytes_to_send);
}

int loop_count = 0;
int next = 0;
void loop()
{
  int bytes_to_send = 0;
  int uart_bytes_received = 0;
  int i = 0;
  int first = 0;
  uint8_t spi_crc;
  uint8_t spi_data[3];
  //next++;
  if ( next > 1 ) {
      next = 0;  
  }
#ifdef TUNING_VAR_TEST  
  tuning_var_test();
#endif
#ifdef I2C_UPDATE_RATE_TEST
  periodic_update_rate_modify_i2c();
#endif
#ifdef SPI_UPDATE_RATE_TEST
  periodic_update_rate_modify_spi();
#endif  
#ifdef TEST_I2C
  /* Transmit I2C data */
  Wire.beginTransmission(0x32); // transmit to device #0x32 (50)
  Wire.write(next); // Sends the starting register address
  Wire.write(32);   // Send number of bytes to read
  Wire.endTransmission();    // stop transmitting
  /* Receive the echoed value back */
  Wire.beginTransmission(0x32);
  Wire.requestFrom(0x32,32);
  while(Wire.available()) { // slave may send less than requested
     data[i++] = Wire.read();
  }
  Wire.endTransmission();
  Serial.print("I2C:  ");
  for ( int x = 0; x < i; x++ ) {
      Serial.print(data[x],HEX);
      data[x] = 0;
      Serial.print(" ");
  }
  Serial.println("");
#endif  
#ifdef TEST_SPI
  /* Transmit SPI data */
  spi_data[0] = 0;   // Start register address (high bit clear == read)
  spi_data[1] = 32; // Number of bytes to read
  spi_data[2] = IMURegisters::getCRC(spi_data,2);
  Serial.print("SPI:  ");
  digitalWrite(SS, LOW);
  for ( int spi_data_index = 0; spi_data_index < 3; spi_data_index++ ) {
      SPI.transfer(spi_data[spi_data_index]);
  }
  digitalWrite(SS, HIGH);
  delayMicroseconds(200); // Read 0xFFs until ready
  digitalWrite(SS, LOW);
  for ( int x = 0; x < 33; x++ ) {
      data[x] = SPI.transfer((byte)0xFF);
  }
  digitalWrite(SS, HIGH);  
  spi_crc = IMURegisters::getCRC(data,32);
  /* NOTE:  One more byte than requested is */
  /* sent.  This is a Checksum. */
  for ( int x = 0; x < 33; x++ ) {
      Serial.print(data[x],HEX);
      Serial.print(" ");
  }
  Serial.println("");
  if ( spi_crc != data[32] ) {
      Serial.print("SPI CRC ERROR!  ");
      Serial.println(spi_crc);
  }
#endif
#ifdef TEST_UART
  while(serport.available()) {
    if ( first == 0 ) {
        Serial.print("UART:  ");
        first = 1;
    }
    byte b = (char)serport.read();
    if ( isascii(b) || b == 10 || b == 13 ) {
      Serial.print(b);
    }
    uart_bytes_received++;
    /* If max chars received within the iteration period */
    /* take a break */
    if ( uart_bytes_received > 100 ) {
      Serial.println();
      break;
    }
  }
#ifdef TEST_STREAM_UPDATE_RATE_CHANGE  
  if ( loop_count == 100 ) {
      Serial.println();
      Serial.print("***YPR UPDATE!***  ");
      Serial.println();
      bytes_to_send = IMUProtocol::encodeStreamCommand( protocol_buffer, 
        MSGID_YPR_UPDATE, 60);  
      Serial.write((const uint8_t *)protocol_buffer,bytes_to_send);
      Serial.println("");
  } else if ( loop_count == 200 ) {
      Serial.println();
      Serial.print("***QUATERNION UPDATE!***  ");
      Serial.println();
      bytes_to_send = IMUProtocol::encodeStreamCommand( protocol_buffer, 
        MSGID_QUATERNION_UPDATE, 60);  
      Serial.write((const uint8_t *)protocol_buffer,bytes_to_send);
      Serial.println("");
  }  else if ( loop_count == 300 ) {
      Serial.println();
      Serial.print("***GYRO UPDATE!***  ");
      Serial.println();
      bytes_to_send = IMUProtocol::encodeStreamCommand( protocol_buffer, 
        MSGID_GYRO_UPDATE, 60);  
      Serial.write((const uint8_t *)protocol_buffer,bytes_to_send);
      Serial.println("");
  /*
  }  else if ( loop_count == 400 ) {
      Serial.println();
      Serial.print("***AHRS UPDATE!***  ");
      Serial.println();
      bytes_to_send = IMUProtocol::encodeStreamCommand( protocol_buffer, 
        MSGID_AHRS_UPDATE, 60);  
      Serial.write((const uint8_t *)protocol_buffer,bytes_to_send);
      Serial.println("");
  */
  }
  if ( bytes_to_send > 0 ) 
  {
    serport.write((unsigned char *)protocol_buffer, bytes_to_send);           
  }
  if ( loop_count == 400 ) {
      loop_count = 0;
  } else {
      loop_count++;
  }
#endif // TEST_STREAM_UPDATE_RATE_CHANGE  
#endif // TEST_UART
  delay(ITERATION_DELAY_MS);
}

int var = 1;
void tuning_var_test()
{
    int bytes_to_send = 0;
    int first = 0;
    if ( var > 4 ) {
        var = 1;
    }
    bytes_to_send = AHRSProtocol::encodeDataGetRequest( protocol_buffer, 
        TUNING_VARIABLE, (AHRS_TUNING_VAR_ID)var++);  
  if ( bytes_to_send > 0 ) 
  {
    serport.write((unsigned char *)protocol_buffer, bytes_to_send);           
  }
  delay(1);
  while(serport.available()) {
    if ( first == 0 ) {
        Serial.print("UART:  ");
        first = 1;
    }
    Serial.print((char)serport.read());
  }
}

uint8_t min_update_rate = 1;
uint8_t max_update_rate = 100;
uint8_t curr_update_rate = min_update_rate;

uint8_t periodic_i2c_update_iteration_count = 0;
#define PERIODIC_I2C_UPDATE_ITERATIONS 50

void periodic_update_rate_modify_i2c()
{
    periodic_i2c_update_iteration_count++;
    if ( periodic_i2c_update_iteration_count >= PERIODIC_I2C_UPDATE_ITERATIONS ) {
        periodic_i2c_update_iteration_count = 0;
    } else {
        return;
    }
    if ( curr_update_rate > max_update_rate ) {
        curr_update_rate = min_update_rate;
    }
  /* Transmit I2C data */
  Wire.beginTransmission(0x32); // transmit to device #0x32 (50)
  Wire.write(0x80 | NAVX_REG_UPDATE_RATE_HZ); // Sends the starting register address
  Wire.write(curr_update_rate++);   // Send number of bytes to read
  Wire.endTransmission();    // stop transmitting
}

uint8_t periodic_spi_update_iteration_count = 0;
#define PERIODIC_SPI_UPDATE_ITERATIONS 50

void periodic_update_rate_modify_spi()
{
    uint8_t spi_data[3];
    periodic_spi_update_iteration_count++;
    if ( periodic_spi_update_iteration_count >= PERIODIC_SPI_UPDATE_ITERATIONS ) {
        periodic_spi_update_iteration_count = 0;
    } else {
        return;
    }
    if ( curr_update_rate > max_update_rate ) {
        curr_update_rate = min_update_rate;
    }
  /* Transmit SPI data */
  spi_data[0] = 0x80 | NAVX_REG_UPDATE_RATE_HZ; // Starting Register Address + Write Request
  spi_data[1] = curr_update_rate++;
  spi_data[2] = IMURegisters::getCRC(spi_data,2);
  digitalWrite(SS, LOW);
  for ( int i = 0; i < 3; i++ ) {
      SPI.transfer(spi_data[i]);
  }
  digitalWrite(SS, HIGH);
}



