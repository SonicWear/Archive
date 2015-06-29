//This code is for the "Host" Arduino (with XBee Series 2 radio in API mode) attached to laptop.
//Forwards sensor data through to MaxMSP with command bytes and terminated with 0xFF.

#include <SoftwareSerial.h>
SoftwareSerial xbSerial(MOSI, 4);      // RX, TX of the XBee.  This configuration is used when a SoMo V4 board is used as the Host.

//HardwareSerial & xbSerial = Serial1;     // Direct link to the XBee.  This configuration is used when an Arduino Leonardo with Wireless Proto Shield is used as the Host.

#define LED_UNIT1 2   // Red
#define LED_UNIT2 2   // Blue
#define LED_UNIT3 2   // Yellow  (Purple)
#define LED_UNIT4 2   // Green

int timer[5];
#define LED_TIMEOUT 5000 // Adjust this for timeout duration

void setup() 
{
  setupLEDs();
  Serial.begin(9600);    //   Out to Max
  xbSerial.begin(9600);   // XBee serial to board 
  
  for (int i=1; i<=4; i++)
  {
     lightLED(i);
     delay(200);
     clearLEDs();
  }
  clearLEDs();
}

void loop() 
{
  byte buffer[100];
  byte byteCountMSB=0;
  byte byteCountLSB=0;
  int  byteCount=0;
  byte packetType=0;
  byte unitID=0;
  
  byte inChar=0;
  byte checkSum=0;
  
  doTimer();  
  
  if (xbSerial.available())     // Check for incoming data, loop again if none
  { 
    if (getByte() ==  0x7E)     // Read incoming packet starting with 0x7E Start Delimiter
    {
      byteCountMSB = getByte();
      byteCountLSB = getByte();   
      
      byteCount = (byteCountMSB <<= 8) + byteCountLSB;
  
      if (byteCount == 31) 
      {
        checkSum = 0;
  
        for (int i=0; i<byteCount; i++)
        {
          inChar = getByte();
          checkSum += inChar;
          buffer[i] = inChar;
        }
  
        // get the checksum character
        checkSum = getByte();

/*  
       Serial.print("\nChecksum: ");
       Serial.println(checkSum, HEX);
*/      
        // Check the packet type - we only respond to "ZigBee Receive Packet"
        packetType = buffer[0];
        
        if (packetType != 0x90)
        {
          return;
        }
        
        unitID = buffer[12];   // Start of the user data portion
        
        // New on Leonardo board and enclosure - LED control    
        lightLED(unitID);
        resetTimer(unitID);
       
        // SEND THE BINARY DATA TO MAX
        Serial.write(0x81);  //prefix the data with 0x81 (begins message to Max)
        for (int p = 12; p < 31; p++)
        {
           Serial.write(buffer[p]);  //send sensor bytes
        }
        Serial.write(0xFF);  //suffix the data with 0xFF (ends message to Max)
        
        // TODO: Digital values? 0x82        
      }
    }
  }
}

byte getByte()
{  
  // Wait for a byte
  while(!xbSerial.available()) 
  {
     ; 
  }
 
  byte inByte = xbSerial.read();

/*
    if (inByte < 0x10) 
    {
      Serial.print("0");
    }  
    Serial.print(inByte, HEX);
    Serial.print(" ");
*/

  return inByte;
}


// New functions for controlling LEDs

void setupLEDs()
{
  pinMode(LED_UNIT1, OUTPUT);
  pinMode(LED_UNIT2, OUTPUT);
  pinMode(LED_UNIT3, OUTPUT);
  pinMode(LED_UNIT4, OUTPUT);
  clearLEDs();
}

void clearLEDs()
{
  digitalWrite(LED_UNIT1, LOW);
  digitalWrite(LED_UNIT2, LOW);
  digitalWrite(LED_UNIT3, LOW);
  digitalWrite(LED_UNIT4, LOW);
}

void lightLED(byte unitID)
{
  switch (unitID)
  {
    case 1: digitalWrite(LED_UNIT1, HIGH);  break;
    case 2: digitalWrite(LED_UNIT2, HIGH);  break;
    case 3: digitalWrite(LED_UNIT3, HIGH);  break;
    case 4: digitalWrite(LED_UNIT4, HIGH);  break;      
    default: break;
  }
}

void unlightLED(byte unitID)
{
  switch (unitID)
  {
    case 1: digitalWrite(LED_UNIT1, LOW);  break;
    case 2: digitalWrite(LED_UNIT2, LOW);  break;
    case 3: digitalWrite(LED_UNIT3, LOW);  break;
    case 4: digitalWrite(LED_UNIT4, LOW);  break;      
    default: break;
  }
}

void resetTimer(byte unitID)
{
  switch (unitID)
  {
    case 1: 
    case 2: 
    case 3: 
    case 4:  timer[unitID]=0;  break;
    default: break;
  }
}

void doTimer()
{  
  for (int i=1; i<=4; i++)
  {
    timer[i]++;
    
    if (timer[i] > LED_TIMEOUT)  
    {
      unlightLED(i);
      timer[i]=LED_TIMEOUT; // Prevent overflow
    }
  } 
}
