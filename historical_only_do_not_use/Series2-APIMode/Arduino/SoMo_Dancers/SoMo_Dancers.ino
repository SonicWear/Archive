/*This code sits on the remote devices (what the dancers will wear) 
  in a multiple dancer setup with XBee Series 2 Radios in API Mode. 

  Unit ID for each remote device needs to be set using the keys below:
  
    Unit 0: 0x00 // Used for single dancer only (XBee S1)
    Unit 1: 0x01
    Unit 2: 0x02
    Unit 3: 0x03
    Unit 4: 0x04
    Unit 5: 0x05
    Unit 6: 0x06
    Unit 7: 0x07
    Unit 8: 0x08
    Unit 9: 0x09
*/

byte unitID = 0x01; //Unit ID 

#include <SoftwareSerial.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

SoftwareSerial xbSerial(MOSI, 4); // RX, TX of the XBee

byte data[] = {0x7E,           //Start delimiter
               0x00, 0x21,     //Character count
               0x10,           //Transmit Request
               0x00,           //Frame ID (set for no ACK, 0x01 if ACK desired)
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //64-bit dest. address for network Coordinator
               0xFF, 0xFE,     //16-bit dest.address
               0x00,           //Broadcast Radius
               0x00,           //Options               
               unitID,         //****************
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //sensor data
               0x00            //Checksum
              };  

int digVal=0;     //digital pins bits are packed into a single variable
int pointer=0;    //global variable used by packValue__()

void setup() 
{
  // configure serial ports
  Serial.begin(38400);
  xbSerial.begin(38400); 

  // initialize device
  Serial.println("Initializing I2C devices...");
  Wire.begin();
  accelgyro.initialize();
  
  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
 
  // setup digital inputs
  for (int i = 2; i < 14; i++) 
  {
     digitalWrite(i, HIGH); //enable pullups
  }
}

void loop() 
{
  // read raw accel/gyro measurements from device
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);

/*
  // display tab-separated accel/gyro x/y/z values
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t");
  Serial.print(gx); Serial.print("\t");
  Serial.print(gy); Serial.print("\t");
  Serial.print(gz); Serial.print("\t");
  Serial.print(mx); Serial.print("\t");
  Serial.print(my); Serial.print("\t");
  Serial.println(mz);
*/

  // scale inputs to 0-1024 for backwards compatibility with old SensorBox code using pure analog inputs
  int scaled_ax = ScaleMAX(ax);
  int scaled_ay = ScaleMAX(ay);
  int scaled_az = ScaleMAX(az);
  int scaled_gx = ScaleMAX(gx);
  int scaled_gy = ScaleMAX(gy);
  int scaled_gz = ScaleMAX(gz);
  int scaled_mx = ScaleMAX_mag(mx);
  int scaled_my = ScaleMAX_mag(my);
  int scaled_mz = ScaleMAX_mag(mz); 


  // display SCALED tab-separated accel/gyro x/y/z values
  Serial.print("Scaled:\t");
  Serial.print(scaled_ax); 
  Serial.print("\t");
  Serial.print(scaled_ay); 
  Serial.print("\t");
  Serial.print(scaled_az); 
  Serial.print("\t");
  Serial.print(scaled_gx); 
  Serial.print("\t");
  Serial.print(scaled_gy); 
  Serial.print("\t");
  Serial.print(scaled_gz); 
  Serial.print("\t");
  Serial.print(scaled_mx); 
  Serial.print("\t");
  Serial.print(scaled_my); 
  Serial.print("\t");
  Serial.println(scaled_mz);


  // pack the data from the MPU9150 into the SensorBox numbers to transmit
  pointer=18;
  
  packValueMPU9150(scaled_ax); 
  packValueMPU9150(scaled_ay);
  packValueMPU9150(scaled_az);
  packValueMPU9150(scaled_mx);
  packValueMPU9150(scaled_my);
  packValueMPU9150(scaled_mz);
  packValueMPU9150(scaled_gx);
  packValueMPU9150(scaled_gy);
  packValueMPU9150(scaled_gz);
  
  // TODO: Digital values?  Need to adjust data[] length.

  sendMessage();  
  delay(1);
}

void packValueMPU9150(int16_t value)
{
  int16_t tempA = value; 
  data[pointer] = tempA & 127;
  pointer++;
  data[pointer] = (tempA >> 7);
  pointer++;
}

// Scale the inputs
int ScaleMAX(int raw)
{
   float temp = Scale(raw, -16000, 16000);  
   return temp*1024;
}

// Scale the inputs for magnetometer only
int ScaleMAX_mag(int raw)
{
   float temp = Scale(raw, -32000, 32000);   
   return temp*1024;
}

// Scale one of the inputs to between 0.0 and 1.0
float Scale(long in, long smin, long smax)
{
  // Bound
  if (in > smax) in=smax;
  if (in < smin) in=smin;
  
  // Change zero-offset
  in = in-smin;
  
  // Scale between 0.0 and 1.0 (0.5 would be halfway)
  float temp = (float)in/((float)smax-(float)smin);
  return temp;
}

//Calculates the checksum and sends the message
void sendMessage() 
{
  data[sizeof(data)-1] = checkSum(data, sizeof(data)-1);
  xbSerial.write(data, sizeof(data));
  
/*
  //Print the packet we are sending (debug purposes)  
  Serial.print('\n');
  for (pointer = 0; pointer != 37; pointer++) {
    if (data[pointer] < 0x10) {
      Serial.print("0");
    }
    Serial.print(data[pointer], HEX);
  }
   Serial.print('\n');
*/  
   
}

// XBee S2 API checksum
byte checkSum(byte * data, int size) 
{
  byte sum = 0;  
  for (int i=3; i<size; i++) 
  {
    sum += data[i];
  }  
  sum = (-sum)-1;  
  return sum;
}
