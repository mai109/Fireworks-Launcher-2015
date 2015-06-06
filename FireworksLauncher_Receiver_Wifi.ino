//Fireworks Launcher 2015
//
//By: Mark Ingram
//Date Created: 06.03.15
//
//Changelog:
//06.06.15 Created and quasi-finished

#include <ShiftRegister74HC595.h>
#include <SoftwareSerial.h>


#define TRUE 1
#define FALSE 0
#define MAX_FRAMES 15
#define NUM_OF_CHANNELS 8
#define IGNITE_TIME 1500

SoftwareSerial Serial1(5,6); // RX,TX
ShiftRegister74HC595 sr = ShiftRegister74HC595(1,2,3,4);
byte data[32];
byte currentFrame = 0;
unsigned int launchSequence[MAX_FRAMES][NUM_OF_CHANNELS+1] = {
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0},
};

void setup() 
{
  sr.setAllLow();
  Serial.begin(9600);
  //setupWifi(Serial1);
  delay(1500);
  Serial1.begin(9600);
  Serial1.setTimeout(200);
  Serial1.print(F("AT+CIPMUX=1"));
  Serial1.print('\r');
  Serial1.print('\n');
  delay(500);
  Serial1.print(F("AT+CIPSERVER=1"));
  Serial1.print('\r');
  Serial1.print('\n');
  delay(1000);
  Serial1.print(F("AT+CIPSTO=6000"));
  Serial1.print('\r');
  Serial1.print('\n');
  delay(500);
  Serial1.print(F("ATE0"));
  Serial1.print('\r');
  Serial1.print('\n');
  delay(500);
  pinMode(13,OUTPUT);
  sr.setAllHigh();
  delay(400);
  sr.setAllLow();
  flushSerial1();
}

void loop() 
{
  if(Serial1.available())
  {
    delay(20);
  //  Serial.println("Got Something");
    if(Serial1.peek()=='0')
    {
    //  Serial.println("Got a zero!");
      if(Serial1.find("0,C"))
      {
    //    Serial.println("Found 0,c");
      }
      if(Serial1.peek()=='O')
      {
        Serial.println("connect");
        digitalWrite(13,HIGH);
        flushSerial1();
      }
      else if(Serial1.peek()=='L')
      {
        Serial.println("disconnect");
        digitalWrite(13,LOW);
        flushSerial1();
      }
    }
    else if((Serial1.peek()== 13) || (Serial1.peek() == 10))
    {
      Serial1.find(":");
      Serial.write("Got a magic sequence!");
    }
      switch(Serial1.peek())
      {
        case 'F':
          Serial1.read();
          Serial.println("Adding Frame");
          addFrame(Serial1.readBytesUntil(';',data,32));
          clearData();
          debugLaunchSequence();
          break;
        case 'D':
          Serial1.read();
          Serial.println("Adding Delay");
          //addDelay(Serial1.parseInt());
          addDelay(Serial1.readStringUntil(';'));
          //Serial.find(";");
          debugLaunchSequence();
          break;
        case 'T':
          Serial1.find(";");
          Serial.println("Testing Wifi");
          testWifi();
          flushSerial1();
          debugLaunchSequence();
          break;
        case 'M':
          Serial1.read();
          Serial.println("Manual Launching: ");
          launch((Serial1.read()-49)); 
          Serial1.find(";");
          debugLaunchSequence();        
          break;
        case 'L':
          Serial1.read();
          Serial.println("Auto Launch Begin!!");
          debugLaunchSequence();
          beginLaunchSequence();  
          debugLaunchSequence();
      }
    //}
/*    else
    {
      flushSerial1();
    }*/
  }
}
void flushSerial1()
{
  Serial.println("Emptying...");
  Serial1.find("Z");
}

void addFrame(int length){
    for(byte i = 0; i<length; i++){
    Serial.print(data[i]);
    Serial.print(' ');
    if((data[i] > 48) && (data[i] < 57)){
      launchSequence[currentFrame][data[i]-48] = TRUE;
    }
  }
  currentFrame++;
}

void addDelay(String delay){
  unsigned int delayInt = delay.toInt();
  launchSequence[currentFrame][0] = 1;
  launchSequence[currentFrame][1] = delayInt;
  currentFrame++;
}

void clearData(){
  for(byte i=0; i<32; i++){
    data[i] = 0;
  }
}

void clearLaunchSequence(){
  for(byte i=0; i<MAX_FRAMES; i++){
    for(byte j=0; j<NUM_OF_CHANNELS+1; j++){
      launchSequence[i][j] = 0;
    }
  } 
  currentFrame = 0;
}

void testWifi(){
  Serial1.print(F("AT+CIPSEND=0,2"));
  Serial1.print('\r');
  Serial1.print('\n');
  delay(1000);
  Serial1.print(F("OK"));
  Serial1.print('\r');
  Serial1.print('\n');
}

void launch(byte num){
  Serial.println(num);
  byte shifty[1];
  shifty[0] = 0;
  bitSet(shifty[0],num);
  sr.setAll(shifty);
  delay(IGNITE_TIME);
  sr.setAllLow();
}

void beginLaunchSequence(){
  byte shifty[1];
  for(byte i=0; i<currentFrame; i++){
    if(launchSequence[i][0] == 0){
      shifty[0] = 0;
      for(byte j=1; j<=NUM_OF_CHANNELS; j++){
        if(launchSequence[i][j]){
          bitSet(shifty[0],(j-1));
        }
      }
      sr.setAll(shifty);
      delay(IGNITE_TIME);
      sr.setAllLow();
    }
    else{
      delay(launchSequence[i][1]);
    }
  }
  clearLaunchSequence();
}

void debugLaunchSequence(){
  for(byte i=0; i<MAX_FRAMES; i++){
     Serial.print('{');
    for(byte j=0; j<NUM_OF_CHANNELS+1; j++){
     Serial.print(launchSequence[i][j]);
     Serial.print(',');
    }
    Serial.println('}');
  } 
  Serial.print("CurrentFrame = ");
  Serial.println(currentFrame);
  Serial.println('\n');
  
}
