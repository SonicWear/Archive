/*This code sits on the remote devices (what the dancers will wear).  
  Unit ID for each remote device needs to be set in 'byte[data]' using the keys below:
    Unit 0: 0x30
    Unit 1: 0x31
    Unit 2: 0x32
    Unit 3: 0x33
    Unit 4: 0x34
    Unit 5: 0x35
    Unit 6: 0x36
    Unit 7: 0x37
    Unit 8: 0x38
    Unit 9: 0x39
*/

#include <SoftwareSerial.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

SoftwareSerial mySerial(MOSI, 4); // RX, TX of the Xbee

byte data[] = {0x7E, //Start delimiter
               0x00, 0x21, //Character count
               0x10, //Transmit Request
               0x00,  //Frame ID (set for no ACK, 0x01 if ACK desired)
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //64-bit dest. address for network Coordinator
               0xFF, 0xFE,  //16-bit dest.address
               0x00,  //Broadcast Radius
               0x00,  //Options
               //****************
               0x31, //Unit ID 
               //****************
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //sensor data
               0x00 //Checksum
              };

byte pointer;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  mySerial.begin(9600);  //this serial corresponds to MAX PATCH
  //digVal=0;
 for (int i = 2; i < 14; i++) {
   digitalWrite(i, HIGH); //enable pullups
 }
  //while (establishContact()==0){delay(100);}  //wait for 99 byte
}

void loop() {
  // read raw accel/gyro measurements from device
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);

  // display tab-separated accel/gyro x/y/z values
//  Serial.print(ax); Serial.print("\t");
//  Serial.print(ay); Serial.print("\t");
//  Serial.print(az); Serial.print("\t");
//  Serial.print(gx); Serial.print("\t");
//  Serial.print(gy); Serial.print("\t");
//  Serial.print(gz); Serial.print("\t");
//  Serial.print(mx); Serial.print("\t");
//  Serial.print(my); Serial.print("\t");
//  Serial.println(mz);

  pointer = 18;

  packValueMPU9150(ax >> 2);
  packValueMPU9150(ay >> 2);
  packValueMPU9150(az >> 2);
  packValueMPU9150(mx);
  packValueMPU9150(my);
  packValueMPU9150(mz);
  packValueMPU9150(gx >> 2);
  packValueMPU9150(gy >> 2);
  packValueMPU9150(gz >> 2);

  sendMessage();
  Serial.println("Message Sent");
  
  delay(50);
}

void packValueMPU9150(int16_t value) {
  int16_t tempA = value;
  data[pointer] = tempA & 127;
  pointer++;
  data[pointer] = (tempA >> 7);
  pointer++;
}

//Calculates the checksum and sends the message
void sendMessage() {
  byte pointer = 0;
  byte sum = 0;
  for (pointer = 3; pointer != 36; pointer++) {
    sum += data[pointer];
  }
  data[36] = (-sum)-1;
  for (pointer = 0; pointer != 37; pointer++) {
   // Serial.write(data[pointer]);
    mySerial.write(data[pointer]);
  }
  
//Print the packet we are sending (debug purposes)  
//  Serial.print('\n');
//  for (pointer = 0; pointer != 37; pointer++) {
//    if (data[pointer] < 0x10) {
//      Serial.print("0");
//    }
//    Serial.print(data[pointer], HEX);
//  }
//   Serial.print('\n');
}
