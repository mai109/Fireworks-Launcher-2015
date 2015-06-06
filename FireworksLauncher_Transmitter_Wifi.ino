//Fireworks Launcher 2015
//
//By: Mark Ingram
//Date Created: 06.03.15
//
//Changelog:
//06.03.15 Created
//06.06.15 Complete

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <SPI.h>
//#include <SD.h>

#define ROWS 4
#define COLS 4
#define TFT_DC 9
#define TFT_CS 10
//TFT display will also use spi pins pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK
#define SD_CS A0
//#define DEBUG
#define TRUE 1
#define FALSE 0

#define BLACK 0x0000
#define BLUE 0x001f
#define RED 0xF800
#define GREEN 0x7E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define TOP_FRAME_SIZE 40
#define BOTTOM_FRAME_SIZE 22

byte rowPins[ROWS] = {6,5,4,3};
byte colPins[COLS] = {A2,A1,A0,8};
int debounceTime = 22;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
unsigned int color = WHITE;
byte currentLevel = 0;
char current[] = {'0','0'};
boolean captureChar = FALSE;
char myChar;
String launchSequence = "";
boolean newChar = FALSE;
String defaultDelay = "D999;";

Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS); //initiates matrixed keypad
SoftwareSerial wifi (A3,A4); //instantiates softwareserial for wifi module esp8266
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {
  #ifdef DEBUG
     // Serial.begin(9600);
  #endif
  wifi.begin(9600);
  wifi.setTimeout(100);
  startupSequence();
  keypad.addEventListener(keypadEvent);
  keypad.setDebounceTime(debounceTime);
  drawFrame();
  drawMenu();
}

void loop() {
  keypad.getKey();
}

/******************
** EXTRA METHODS **
******************/

void keypadEvent(KeypadEvent eKey){  // this is code copied straight from example
  switch (keypad.getState()){        // it handles what happens when a button is pushed
    case PRESSED:
      if(captureChar && (eKey != 'A') && (eKey != 'B'))
      {
        myChar = eKey;
        newChar = TRUE;
      }
      else
      {
	switch (eKey){
	  case 'A': //back button
            if(current[1] == '0'){
              current[0] = '0';
              currentLevel = 0;  
            }
            else{
              current[1] = '0'; 
            }
            captureChar = FALSE;
            drawMenu();
            break;
	  case 'B': //main menu button
            current[0] = '0';
            current[1] = '0';   
            currentLevel = 0; 
            captureChar = FALSE;
            drawMenu();
            break;      
	  default:
            current[currentLevel] = eKey;
            currentLevel = 1;
            drawMenu();
         }
     }
  }
}

void drawFrame()
{
  tft.drawRect(0,0,320,240,color);
  tft.drawFastHLine(0,TOP_FRAME_SIZE,320,color);
  tft.drawFastHLine(0,(240 - BOTTOM_FRAME_SIZE),320,color);
  tft.setTextColor(color);
  tft.setCursor(5,5);
  tft.setTextSize(4);
  tft.print("F-Launcher");
  tft.setTextSize(2);
  tft.setCursor(5,(240-BOTTOM_FRAME_SIZE+3));
  tft.print("A:back  B:main menu");
}

void drawMenu()
{
 // drawFrame();
  switch (current[0]){
    case '0':
      mainMenu();
      break;
    case '1':
      switch (current[1]){
        case '0':
          launchMenu();
          break;
        case '1':
          captureChar = TRUE;
          newFrame();
          captureChar = FALSE;
          drawMenu();
          break;
        case '2':
          captureChar = TRUE;
          customDelay();
          captureChar = FALSE;
          drawMenu();
          break;
        case '3':
          clearSequence();
          break;
        case '4':
          captureChar = TRUE;
          launchConfirm();
          captureChar = FALSE;
          drawMenu();
          break;
        case '6':
          launchGo(); 
          drawMenu();  
          break;
        case '5':
          captureChar = TRUE;
          manualMode();
          captureChar = FALSE;
          drawMenu();
          break;   
      }
      break;
    case '2':
      testWifi();
      break;
    case '3':
      switch (current[1]){
        case '0':
          settingsMenu();
          break;
        case '1':
          setColor();
          break;
        case '2':
          captureChar = TRUE;
          setDefDelay();
          captureChar = FALSE;
          drawMenu();
      }
  }
}

void mainMenu(){
  menuClear();
  writeLine(1, F("1) Start Launchin"));
  writeLine(2, F("2) Test Wifi"));
  writeLine(3, F("3) Settings"));
}

void testWifi(){
  menuClear();
  writeLine(1,F("Sending test "));
  writeLine(2,F("packet..."));
  wifi.print(F("AT+CIPSEND=1"));
  wifi.print('\r');
  wifi.print('\n');
  delay(800);
  wifi.print(F("T"));
  wifi.print('\r');
  wifi.print('\n');
  writeLine(3,F("Waiting..."));
  //flushSerial();
  delay(1000);
  byte loops = 0;
  while((!wifi.find("+")) && (loops<100)){
    delay(30);
    loops++;
  }
  wifi.find(":");
  if(wifi.read() == 'O'){
    writeLine(5,F("SUCCESS!!"));
  }
  else{
    writeLine(5,F("Failure :/"));
  }  
}

void launchMenu(){
  menuClear();
  writeLine(1,F("1) New Frame"));
  writeLine(2,F("2) Custom Delay"));
  writeLine(3,F("3) Clear All"));
  writeLine(4,F("4) LAUNCH!"));
  writeLine(5,F("5) Manual Mode"));
}

void newFrame(){
  menuClear();
  writeLine(1,F("Enter a list of "));
  writeLine(2,F("channels:"));
  writeLine(3,F("(Hit # when done)"));
  tft.drawFastHLine(10,179,300,color);
  boolean changed=FALSE;
  byte myOldChar = 0;
  String tempLaunchSequence = "";
  while(myChar != '#'){
    keypad.getKey();
    if((myChar != myOldChar) && (myChar > 48) && (myChar < 57)){
      tempLaunchSequence = tempLaunchSequence + myChar + ",";
      changed=TRUE;
    } 
    if(changed){
      writeLine(4,tempLaunchSequence);
    }
    myOldChar = myChar;
    changed = FALSE;
  }
  launchSequence = launchSequence + "F" + tempLaunchSequence + ";" + defaultDelay;
  myChar = 0;
  current[1] = '0';
}

void customDelay(){
  menuClear();
  writeLine(1,F("Enter a custom "));
  writeLine(2,F("delay value (ms):"));
  writeLine(3,F("(Hit # when done)"));
  tft.drawFastHLine(10,179,300,color);
  boolean changed=FALSE;
  String tempString = "";
  while(myChar != '#'){
    keypad.getKey();
    if((newChar) && (myChar > 47) && (myChar < 58)){
      tempString += myChar;
      changed=TRUE;
      newChar = FALSE;
    } 
    if(changed){
      writeLine(4,tempString);
    }
    changed = FALSE;
  }
  launchSequence = launchSequence + "D" + tempString + ";";
  myChar = 0;
  current[1] = '0';
}

void clearSequence(){
  menuClear();
  writeLine(1,F("Please Wait..."));
  delay(1000);
  launchSequence = "";
  menuClear();
  writeLine(1,F("Done!"));
  delay(700);
  current[1] = '0';
  drawMenu();
}

void launchConfirm(){
  menuClear();
  if(launchSequence.length() > 54){
    writeLine(1,F("Error!"));
    writeLine(2,F("String too long!"));
    writeLine(5,F("(# to go back)"));
    while(myChar != '#'){
      keypad.getKey();
    }
    current[1] = '0';
    myChar = 0;
  }
  else{
    writeLine(1,F("Send this?"));
    tft.setTextSize(2);
    tft.setCursor(10,(TOP_FRAME_SIZE + 44));
    tft.print(launchSequence);
    writeLine(5,F("(# to continue)"));
    while(myChar != '#'){
      keypad.getKey();
    }
    current[1]++;
    current[1]++;
    myChar = 0;
  }
}

void launchGo(){
  menuClear();
  writeLine(1,F("Sending Data..."));
  wifi.print(F("AT+CIPSEND="));
  wifi.print(launchSequence.length());
  wifi.print('\r');
  wifi.print('\n');
  delay(800);
  wifi.print(launchSequence);
  wifi.print('\r');
  wifi.print('\n');
  delay(800);
  wifi.print(F("AT+CIPSEND=1"));
  wifi.print('\r');
  wifi.print('\n');
  delay(1200);
  wifi.print("L");
  wifi.print('\r');
  wifi.print('\n');
  tft.setTextSize(4);
  tft.setCursor(10,(TOP_FRAME_SIZE + 78));
  tft.print("LAUNCH :D");
  delay(1200);
  launchSequence = "";
  current[1] = '0';  
}

void manualMode(){
  menuClear();
  writeLine(1,F("Enter the number"));
  writeLine(2,F("of a channel to"));
  writeLine(3,F("launch: (#=exit)"));
  tft.drawRect(140,156,37,52,color);
  boolean changed=FALSE;
  tft.setTextSize(5);
  while(myChar != '#'){
    keypad.getKey();
    if((newChar) && (myChar > 48) && (myChar < 57)){
      changed = TRUE;
      newChar = FALSE;
    } 
    if(changed){
      tft.setCursor(146,162);
      tft.print(myChar);
      launchSingle(myChar);
      tft.fillRect(146,162,25,40,BLACK);
    }
    changed = FALSE;
  }
  myChar = 0;
  current[1] = '0';
  
}

void launchSingle(char channel){
  String launching = "M";
  launching += channel;
  wifi.print(F("AT+CIPSEND=2"));
  wifi.print('\r');
  wifi.print('\n');
  delay(800);
  wifi.print(launching);
  wifi.print('\r');
  wifi.print('\n');
  tft.setCursor(182,162);
  tft.print("+");
  delay(800);
  tft.fillRect(182,162,25,40,BLACK);
}

void settingsMenu(){
  menuClear();
  writeLine(1,F("1) Change Theme"));
  writeLine(2,F("2) Default Delay"));
}

void setColor(){
  menuClear();
  captureChar = TRUE;
  tft.setTextSize(2);
  tft.setCursor(10,TOP_FRAME_SIZE + 10);
  tft.print(F("1) Blue"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21);
  tft.print(F("2) Green"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*2);
  tft.print(F("3) Red"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*3);
  tft.print(F("4) Cyan"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*4);
  tft.print(F("5) Magenta"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*5);
  tft.print(F("6) Yellow"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*6);
  tft.print(F("7) White"));
  myChar = 0;
  while(myChar == 0)
  {
    keypad.getKey();
  }
  switch (myChar){
    case '1':
      color = BLUE;
      break;
    case '2':
      color = GREEN;
      break;
    case '3':
      color = RED;
      break;
    case '4': 
      color = CYAN;
      break;
    case '5':
      color = MAGENTA;
      break;
    case '6':
      color = YELLOW;
      break;
    case '7':
      color = WHITE;
  }
  current[1] = '0';
  drawFrame();
  drawMenu();  
  captureChar = FALSE;
}

void setDefDelay(){
  menuClear();
  writeLine(1,F("Enter a default "));
  writeLine(2,F("delay value (ms):"));
  writeLine(3,F("(Hit # when done)"));
  tft.drawFastHLine(10,179,300,color);
  boolean changed=FALSE;
  String tempString = "";
  while(myChar != '#'){
    keypad.getKey();
    if((newChar) && (myChar > 47) && (myChar < 58)){
      tempString += myChar;
      changed=TRUE;
      newChar = FALSE;
    } 
    if(changed){
      writeLine(4,tempString);
    }
    changed = FALSE;
  }
  defaultDelay = "D" + tempString + ";";
  myChar = 0;
  current[1] = '0';
}

void menuClear(){
  tft.fillRect(1,(TOP_FRAME_SIZE + 1),318,(240 - BOTTOM_FRAME_SIZE - TOP_FRAME_SIZE - 2),BLACK);
}

void writeLine(byte line, String data){
  tft.setTextSize(3);
  switch (line){
    case 1:
      tft.setCursor(10,(TOP_FRAME_SIZE + 10));
      tft.print(data);
      break;
    case 2:
      tft.setCursor(10,(TOP_FRAME_SIZE + 44));
      tft.print(data);
      break;
    case 3:
      tft.setCursor(10,(TOP_FRAME_SIZE + 78));
      tft.print(data);
      break;
    case 4:
      tft.setCursor(10,(TOP_FRAME_SIZE + 112));
      tft.print(data);
      break;
    case 5:
      tft.setCursor(10,(TOP_FRAME_SIZE + 146));
      tft.print(data);
      break;
  }
}

void startupSequence(){
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(BLACK);  
  tft.setCursor(5,5);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print(F("Loading"));
  delay(1000);
  tft.print(".");
  delay(1000);
  tft.print(".");
  delay(1000);
  tft.print(F("\n\nStarting WIFI"));
  delay(1000);
  wifi.print(F("AT+CIPSTART=\"TCP\",\"192.168.4.1\",333"));
  wifi.print('\r');
  wifi.print('\n');
  tft.print(".");
  delay(1000);
  wifi.print(F("ATE0"));
  wifi.print('\r');
  wifi.print('\n');
  delay(500);
  tft.print(F("\n\nREADY"));
  delay(1000);
  tft.fillScreen(BLACK);  
}

void flushSerial()
{
 // Serial.println("Emptying...");
  wifi.find("Z");
}
