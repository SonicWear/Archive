//latest update!
//This code is for the HOST XBEE (with Arduino) attached to laptop.  Sends sensor data through to MaxMSP.

#include <SoftwareSerial.h>

byte buffer[100];
int pointer = 0;
SoftwareSerial mySerial(MOSI, 4); // RX, TX of the Xbee

void setup() {
  Serial.begin(9600);  //UART to Serial Monitor
  mySerial.begin(9600);  //Xbee serial to board
  Serial1.begin(9600);  //Out to Max
}

void loop() {
  int byteCount;
  byte inChar;
  byte checkSum;

    //Read incoming packet starting with 0x7E Start Delimiter
    if (getByte() ==  0x7E) {
      byteCount = int (getByte());
      byteCount <<= 8;
      byteCount += int (getByte());

      if (byteCount == 31) {

        Serial.println("\n\nBytecount OK ");

        checkSum = 0;
        pointer = 0;

        while (byteCount > -1){
          inChar = getByte();
          checkSum += inChar;
          buffer[pointer++] = inChar;
          byteCount--;
        }

        // get the checksum character
        inChar = getByte();
        checkSum +- inChar;
        Serial.print("\nChecksum: ");
        Serial.println(checkSum, HEX);

        Serial.print ("\nUnit ID: ");
        Serial.println(buffer[13],HEX);

        //data bytes
        int ptr = 14;
        for (int count=0;count < 9;count++) {
          if (buffer[ptr] < 0x10) {
            Serial.print("0");
          }
          Serial.print (buffer[ptr++],HEX);

          if (buffer[ptr] < 0x10) {
            Serial.print("0");
          }
          Serial.print (buffer[ptr++],HEX);
          Serial.print(" ");
        }
        Serial.println("");      
      }
    }

    // SEND THE BINARY DATA TO MAX //
    Serial1.write(0x81);  //prefix the data with 0x81 (begins message to Max)
    for (pointer = 13; pointer < 32; pointer++) {
      Serial1.write(buffer[pointer]);  //send bytes 13-31
    }
    Serial1.write(0xff);  //suffix the data with 0xFF (ends message to Max)
  }
}

byte getByte(){
  byte inByte; 
  if (mySerial.available() > 0) {
    inByte = mySerial.read();
    /*
    if (inByte < 0x10) {
     Serial.print("0");
     }
     Serial.print (inByte,HEX);
     Serial.print(" ");
     */
    return inByte;
  }
}

////Looks for connection to MaxMSP
//char establishContact(void){
//  if (Serial1.available() > 0) {
//    char checkup = Serial1.read();
//    if (checkup==99) return 1;  //looking for you to click 99 in Max to run the data
//    else return 0;
//  }
//  else return 0;
// }





