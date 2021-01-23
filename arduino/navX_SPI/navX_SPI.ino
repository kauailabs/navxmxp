#include <SPI.h>
#include "AHRSProtocol.h"             // navX-Sensor Register Definition header file

int ss = 10;                          // Arduino "Chip select" pin

byte data[512];

#define ITERATION_DELAY_MS                   10
#define NAVX_SENSOR_DEVICE_I2C_ADDRESS_7BIT  0x32
#define NUM_BYTES_TO_READ                    8

void setup()
{
  Serial.begin(115200);
  pinMode(ss,OUTPUT);
  digitalWrite(SS, HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(8); /* 16Mhz/32 = 500kHz; /16=1Mhz; /8=2Mhz */ 

  for ( int i = 0; i < sizeof(data); i++ ) {
      data[i] = 0;
  }
}

int register_address = NAVX_REG_YAW_L;

void loop()
{
  uint8_t spi_crc;
  uint8_t spi_data[3];
    
  /* Transmit SPI data */
  spi_data[0] = NAVX_REG_YAW_L;                     // Start register address (high bit clear == read)
  spi_data[1] = NUM_BYTES_TO_READ;                  // Number of bytes to read (not including the final CRC)
  spi_data[2] = IMURegisters::getCRC(spi_data,2);   // Calculate CRC 
  Serial.print("SPI:  ");
  digitalWrite(SS, LOW);
  for ( int spi_data_index = 0; spi_data_index < 3; spi_data_index++ ) {
      SPI.transfer(spi_data[spi_data_index]);
  }
  digitalWrite(SS, HIGH);

  /* Wait 200 microseconds, ensuring sensor is ready to reply */
  delayMicroseconds(200); // Read 0xFFs until ready

  /* Read requested data */
  /* NOTE:  One more byte than requested is */
  /* sent.  This is a Checksum. */  
  digitalWrite(SS, LOW);
  for ( int x = 0; x < (NUM_BYTES_TO_READ + 1); x++ ) {
      data[x] = SPI.transfer((byte)0xFF);
  }
  digitalWrite(SS, HIGH);  

  /* Verify CRC and display data */
  spi_crc = IMURegisters::getCRC(data,NUM_BYTES_TO_READ);
  if ( spi_crc != data[NUM_BYTES_TO_READ] ) {
      Serial.print("SPI CRC ERROR!  ");
      Serial.println(spi_crc);
  } else {
    /* Decode received data to floating-point orientation values */
    float yaw =     IMURegisters::decodeProtocolSignedHundredthsFloat((char *)&data[0]);   // The cast is needed on arduino
    float pitch =   IMURegisters::decodeProtocolSignedHundredthsFloat((char *)&data[2]);   // The cast is needed on arduino
    float roll =    IMURegisters::decodeProtocolSignedHundredthsFloat((char *)&data[4]);   // The cast is needed on arduino
    float heading = IMURegisters::decodeProtocolUnsignedHundredthsFloat((char *)&data[6]); // The cast is needed on arduino  
  
    /* Display orientation values */
    Serial.print("yaw:  ");
    Serial.print(yaw,2);
    Serial.print("  pitch:  ");
    Serial.print(pitch,2);
    Serial.print("  roll:  ");
    Serial.print(roll,2);
    Serial.print("  heading:  ");
    Serial.print(heading,2);  
    Serial.println("");
  }

  delay(ITERATION_DELAY_MS);
}
