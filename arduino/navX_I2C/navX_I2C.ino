#include <Wire.h>
#include "AHRSProtocol.h"             // navX-Sensor Register Definition header file

byte data[512];

#define ITERATION_DELAY_MS                   10
#define NAVX_SENSOR_DEVICE_I2C_ADDRESS_7BIT  0x32
#define NUM_BYTES_TO_READ                    8

void setup()
{
  Serial.begin(115200);
  Wire.begin(); // join i2c bus (address optional for master)

  for ( int i = 0; i < sizeof(data); i++ ) {
      data[i] = 0;
  }
}

int register_address = NAVX_REG_YAW_L;

void loop()
{
  int i = 0;
  /* Transmit I2C data request */
  Wire.beginTransmission(NAVX_SENSOR_DEVICE_I2C_ADDRESS_7BIT); // Begin transmitting to navX-Sensor
  Wire.write(register_address);                                // Sends starting register address
  Wire.write(NUM_BYTES_TO_READ);                               // Send number of bytes to read
  Wire.endTransmission();                                      // Stop transmitting
  
  /* Receive the echoed value back */
  Wire.beginTransmission(NAVX_SENSOR_DEVICE_I2C_ADDRESS_7BIT); // Begin transmitting to navX-Sensor
  Wire.requestFrom(NAVX_SENSOR_DEVICE_I2C_ADDRESS_7BIT, NUM_BYTES_TO_READ);    // Send number of bytes to read
  delay(1);
  while(Wire.available()) {                                    // Read data (slave may send less than requested)
     data[i++] = Wire.read();
  }
  Wire.endTransmission();                                      // Stop transmitting

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

  delay(ITERATION_DELAY_MS);
}
