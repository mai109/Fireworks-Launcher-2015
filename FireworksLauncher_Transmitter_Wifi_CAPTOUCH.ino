//Fireworks Launcher 2015
//
//By: Mark Ingram
//Date Created: 06.03.15
//
//Changelog:
//06.03.15 Created
//06.06.15 Complete
//06.18.15 Changes:
//                  1)Fixing instances where captureChar is not properly turned off
//                    (example: times when back and home buttons are hit)
//                  2)Fixing testWifi connection where it is capturing the OK from its
//                    own command ack instead of the OK received from the other arduino
//                  3)Fixing a bug where the myChar is not properly emptied after setting
//                    a custom delay / theme color
//                  4)Fixing a bug where myChar was not properly emptied when "back" or 
//                    "home" was hit
//06.19.15 Adapted Code to be controlled by captouch
//06.22.15 -Completed captouch adaption
//         -Removed Unnecessary serial debug code

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
#define UP 0
#define DOWN 1

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
byte whereAt = 1;
byte maxAt = 1;

//Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS); //initiates matrixed keypad
SoftwareSerial wifi (A3,A4); //instantiates softwareserial for wifi module esp8266
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//SoftwareSerial cap (A5,7); //instantiates softwareserial for captouch com with MSP430

void setup() {
   Serial.begin(9600);
  //cap.begin(9600);
  wifi.begin(9600);
  wifi.setTimeout(100);
  startupSequence();
//  keypad.addEventListener(keypadEvent);
//  keypad.setDebounceTime(debounceTime);
  drawFrame();
  drawMenu();
}

void loop() {
  capTouchGetKey();
}

void capTouchGetKey(){
  if(Serial.available() != 0){
    #ifdef DEBUG
    Serial.println("got somethin!");
    #endif
    input(Serial.read());
  }
}
/******************
** EXTRA METHODS **
******************/

/*void keypadEvent(KeypadEvent eKey){  // this is code copied straight from example
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
            myChar = 0;
            drawMenu();
            break;
	  case 'B': //main menu button
            current[0] = '0';
            current[1] = '0';   
            currentLevel = 0; 
            captureChar = FALSE;
            myChar = 0;
            drawMenu();
            break;      
	  default:
            current[currentLevel] = eKey;
            currentLevel = 1;
            captureChar = FALSE;
            drawMenu();
         }
     }
  }
}*/

void input(byte data){
 // Serial.println(data);
  if(captureChar == FALSE){
    switch(data){
      case 3:
      case 13:
        current[currentLevel] = whereAt + 48;
        currentLevel = 1;
        captureChar = FALSE;
        drawMenu(); 
        break;     
      case 6:
        pointer(DOWN);
        break;
      case 9:
        if(current[1] == '0'){
          current[0] = '0';
          currentLevel = 0;  
        }
        else{
          current[1] = '0'; 
        }
        captureChar = FALSE;
        myChar = 0;
        drawMenu();
        break;
      case 12:
        pointer(UP);
        break;
      case 22:
        pointer(UP);
        break;
      case 33:
        pointer(DOWN);
    }
  }
  else{
    switch(data){
    case 3:
      myChar = 'R';
      break;
    case 13:
      myChar = 'E';
      break; 
    case 6:
      myChar = 'D';
      break;
    case 9:
      /*if(current[1] == '0'){
        current[0] = '0';
        currentLevel = 0;  
      }
      else{
        current[1] = '0'; 
      }
      captureChar = FALSE;
      myChar = 0;
      drawMenu();
      break;*/
      myChar = 'L';
      break;
    case 12:
      myChar = 'U';
      break;
    case 22:
      myChar = '-';
      break;
    case 33:
      myChar = '+';
      break;
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
  clearPointer(whereAt);
  whereAt = 1;
  switch (current[0]){
    case '0':
      maxAt = 3;
      mainMenu();
      drawPointer(whereAt);
      break;
    case '1':
      switch (current[1]){
        case '0':
          launchMenu();
          drawPointer(whereAt);
          maxAt = 5;
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
      captureChar = TRUE;
      testWifi();
      captureChar = FALSE;
      break;
    case '3':
      switch (current[1]){
        case '0':
          settingsMenu();
          drawPointer(whereAt);
          maxAt = 2;
          break;
        case '1':
          maxAt = 7;
          captureChar = TRUE;
          setColor();
          captureChar = FALSE;
          break;
        case '2':
          captureChar = TRUE;
          setDefDelay();
          captureChar = FALSE;
          drawMenu();
      }
  }
  whereAt = 1;
  drawPointer(whereAt);
}

void mainMenu(){
  menuClear();
  writeLine(1, F(" Start Launching"));
  writeLine(2, F(" Test Wifi"));
  writeLine(3, F(" Settings"));
}

void testWifi(){
  menuClear();
  writeLine(1,F("Sending test "));
  writeLine(2,F("packet..."));
  wifi.print(F("AT+CIPSEND=1"));
  wifi.print('\r');
  wifi.print('\n');
  flushSerial();
  delay(700);
  wifi.print(F("T"));
  wifi.print('\r');
  wifi.print('\n');
  writeLine(3,F("Waiting..."));
  delay(600);
  Serial.find("OK");
  delay(20);
  byte loops = 0;
  while((!wifi.find("+")) && (loops<40)){
    delay(30);
    loops++;
  }
  delay(10); //make sure entire serial message has come in
  wifi.find(":");
  if(wifi.read() == 'O'){
    writeLine(5,F("SUCCESS!!"));
  }
  else{
    writeLine(5,F("Failure :/"));
  }  
  flushSerial();
}

void launchMenu(){
  menuClear();
  writeLine(1,F(" New Frame"));
  writeLine(2,F(" Custom Delay"));
  writeLine(3,F(" Clear All"));
  writeLine(4,F(" LAUNCH!"));
  writeLine(5,F(" Manual Mode"));
}

void newFrame(){
  menuClear();
  writeLine(1,F("Enter a list of "));
  writeLine(2,F("channels:"));
  writeLine(3,F("(Mid when done)"));
  tft.drawFastHLine(10,179,300,color);
  myChar = 0;
  byte update = 0;
  String tempLaunchSequence = "";
  char oldNum = '1';
  char newNum = '1';
  while((myChar != 'E') && (myChar != 'L')){
    myChar = 0;
    capTouchGetKey();
    update = numGen(newNum,1);
    #ifdef DEBUG
    Serial.print("update = ");
    Serial.println((byte)update);
    #endif
    if((update > 80) && (update < 160)){
      newNum = update - 80;
      clearLine(4,(tempLaunchSequence + oldNum));
      writeLine(4,(tempLaunchSequence + newNum));
      oldNum = newNum;
    }
    else if (update > 160){
      newNum = update - 160;
      tempLaunchSequence = tempLaunchSequence + (char)newNum + ",";
      newNum = '1';
      oldNum = '1';
    }
    else{
      writeLine(4,(tempLaunchSequence + newNum));
    }
    update = 0;
  }
  if(myChar == 'E'){
    tempLaunchSequence += newNum;
    launchSequence = launchSequence + "F" + tempLaunchSequence + ";" + defaultDelay;
  }
  myChar = 0;
  current[1] = '0';
}

void customDelay(){
  menuClear();
  writeLine(1,F("Enter a custom "));
  writeLine(2,F("delay value (ms):"));
  writeLine(3,F("(mid when done)"));
  tft.drawFastHLine(10,179,300,color);
  byte update = 0;
  String tempLaunchSequence = "";
  char oldNum = '0';
  char newNum = '0';  
  while((myChar != 'E') && (myChar != 'L')){
    myChar = 0;
    capTouchGetKey();
    update = numGen(newNum,FALSE);
    #ifdef DEBUG
    Serial.print("update = ");
    Serial.println((byte)update);
    #endif
    if((update > 80) && (update < 160)){
      newNum = update - 80;
      clearLine(4,(tempLaunchSequence + oldNum));
      writeLine(4,(tempLaunchSequence + newNum));
      oldNum = newNum;
    }
    else if (update > 160){
      newNum = update - 160;
      tempLaunchSequence = tempLaunchSequence + (char)newNum;
      newNum = '0';
      oldNum = '0';
      writeLine(4,(tempLaunchSequence + newNum));
    }
    update = 0;
  }
  if(myChar == 'E'){
    tempLaunchSequence += newNum;
    launchSequence = launchSequence + "D" + tempLaunchSequence + ";"; 
  }
  myChar = 0;
  current[1] = '0';
}

void clearSequence(){
  menuClear();
  writeLine(1,F("Please Wait..."));
  delay(600);
  launchSequence = "";
  menuClear();
  writeLine(1,F("Done!"));
  delay(400);
  current[1] = '0';
  drawMenu();
}

void launchConfirm(){
  menuClear();
  if(launchSequence.length() > 54){
    writeLine(1,F("Error!"));
    writeLine(2,F("String too long!"));
    writeLine(5,F("(left to go back)"));
    while((myChar != 'E') && (myChar != 'L')){
     // keypad.getKey();
     capTouchGetKey();
    }
    current[1] = '0';
    myChar = 0;
  }
  else{
    writeLine(1,F("Send this?"));
    tft.setTextSize(2);
    tft.setCursor(10,(TOP_FRAME_SIZE + 44));
    tft.print(launchSequence);
    writeLine(5,F("(middle if ok)"));
    while((myChar != 'E') && (myChar != 'L')){
     // keypad.getKey();
     capTouchGetKey();
    }
    if(myChar == 'E'){
      current[1]++;
      current[1]++;
    }
    else{
      current[1] = '0';
    }
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
  writeLine(3,F("launch: "));
  tft.drawRect(140,156,37,52,color);
  tft.setTextSize(5);
  /*while(myChar != 'L'){
   // keypad.getKey();
    capTouchGetKey();
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
  }*/
  byte update = 0;
  char oldNum = '1';
  char newNum = '1';
  tft.setCursor(146,162);
  tft.print(newNum); 
  while(myChar != 'L'){
    myChar = 0;
    capTouchGetKey();
    update = numGen(newNum,TRUE);
    #ifdef DEBUG
    Serial.print("update = ");
    Serial.println((byte)update);
    #endif
    if((update > 80) && (update < 160)){
      newNum = update - 80;
     // clearLine(4,(tempLaunchSequence + oldNum));
     // writeLine(4,(tempLaunchSequence + newNum));
      tft.setCursor(146,162);
      tft.setTextColor(BLACK);
      tft.print(oldNum);
      tft.setCursor(146,162);
      tft.setTextColor(color);
      tft.print(newNum);
      //tft.fillRect(146,162,25,40,BLACK);
      oldNum = newNum;
    }
    else if (update > 160){
      newNum = update - 160;
      launchSingle(newNum);
      newNum = '1';
      tft.setCursor(146,162);
      tft.setTextColor(BLACK);
      tft.print(oldNum);
      tft.setCursor(146,162);
      tft.setTextColor(color);
      tft.print(newNum);
      oldNum = '1';
    }
    else{
      if(myChar == 'E'){
        newNum = update;
        launchSingle(newNum);
        newNum = '1';
        tft.setCursor(146,162);
        tft.setTextColor(BLACK);
        tft.print(oldNum);
        tft.setCursor(146,162);
        tft.setTextColor(color);
        tft.print(newNum);
        oldNum = '1';
      }
    }
    update = 0;
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
  writeLine(1,F(" Change Theme"));
  writeLine(2,F(" Default Delay"));
}

void setColor(){
  menuClear();
  captureChar = TRUE;
  tft.setTextSize(2);
  tft.setCursor(10,TOP_FRAME_SIZE + 10);
  tft.print(F(" Blue"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21);
  tft.print(F(" Green"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*2);
  tft.print(F(" Red"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*3);
  tft.print(F(" Cyan"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*4);
  tft.print(F(" Magenta"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*5);
  tft.print(F(" Yellow"));
  tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*6);
  tft.print(F(" White"));
  myChar = 0;
  drawPointerSmall(1);
  while((myChar != 'E') && (myChar != 'R') && (myChar != 'L'))
  {
   // keypad.getKey();
   myChar = 0;
   capTouchGetKey();
   switch(myChar){
     case 'U':
     case '-':
       pointerSmall(UP);
       break;
     case 'D':
     case '+':
       pointerSmall(DOWN);
       break;
   }
   delay(5);
  }
  if(myChar != 'L'){
    switch(whereAt){
      case 1:
        color = BLUE;
        break;
      case 2:
        color = GREEN;
        break;
      case 3:
        color = RED;
        break;
      case 4: 
        color = CYAN;
        break;
      case 5:
        color = MAGENTA;
        break;
      case 6:
        color = YELLOW;
        break;
      case 7:
        color = WHITE;
    }
  }
  whereAt = 1;
  current[1] = '0';
  myChar = 0;
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
  byte update = 0;
  String tempLaunchSequence = "";
  char oldNum = '0';
  char newNum = '0';  
  while((myChar != 'E') && (myChar != 'L')){
    myChar = 0;
    capTouchGetKey();
    update = numGen(newNum,FALSE);
    #ifdef DEBUG
    Serial.print("update = ");
    Serial.println((byte)update);
    #endif
    if((update > 80) && (update < 160)){
      newNum = update - 80;
      clearLine(4,(tempLaunchSequence + oldNum));
      writeLine(4,(tempLaunchSequence + newNum));
      oldNum = newNum;
    }
    else if (update > 160){
      newNum = update - 160;
      tempLaunchSequence = tempLaunchSequence + (char)newNum;
      newNum = '0';
      oldNum = '0';
      writeLine(4,(tempLaunchSequence + newNum));
    }
    else{
      //writeLine(4,(tempLaunchSequence + newNum));
    }
    update = 0;
  }
  if(myChar == 'E'){
    tempLaunchSequence += newNum;
    defaultDelay = "D" + tempLaunchSequence + ";";
  }
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

void clearLine(byte line, String data){
  tft.setTextSize(3);
  tft.setTextColor(BLACK);
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
  tft.setTextColor(color);
}

void clearPointer(byte line){
  switch (line){
    case 1:
      tft.fillRect(10,(TOP_FRAME_SIZE + 10),15,24,BLACK);
      break;
    case 2:
      tft.fillRect(10,(TOP_FRAME_SIZE + 44),15,24,BLACK);
      break;
    case 3:
      tft.fillRect(10,(TOP_FRAME_SIZE + 78),15,24,BLACK);
      break;
    case 4:
      tft.fillRect(10,(TOP_FRAME_SIZE + 112),15,24,BLACK);
      break;
    case 5:
      tft.fillRect(10,(TOP_FRAME_SIZE + 146),15,24,BLACK);
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
  #ifdef DEBUG
  Serial.println("Emptying...");
  #endif
  wifi.find("Z");
}

//*******************************
//*** BEGIN CAP TOUCH METHODS ***
//*******************************
void pointer(boolean isDown){
  clearPointer(whereAt);
  if(isDown){
    if(whereAt < maxAt){
      whereAt++;
    }
    else{
      whereAt = 1;
    }
  }
  else{
    if(whereAt > 1){
      whereAt--;
    }
    else{
      whereAt = maxAt;
    }
  }
  drawPointer(whereAt);
}

void drawPointer(byte whereAtNow){
  writeLine(whereAtNow,">");
}

void pointerSmall(boolean isDown){
  clearPointerSmall(whereAt);
  if(isDown){
    if(whereAt < maxAt){
      whereAt++;
    }
    else{
      whereAt = 1;
    }
  }
  else{
    if(whereAt > 1){
      whereAt--;
    }
    else{
      whereAt = maxAt;
    }
  }
  drawPointerSmall(whereAt);
}


void drawPointerSmall(byte whereAtNow){
  tft.setTextSize(2);
  switch(whereAtNow){
    case 1:
      tft.setCursor(10,TOP_FRAME_SIZE + 10);
      tft.print(F(">"));
      break;
    case 2:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21);
      tft.print(F(">"));
      break;
    case 3:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*2);
      tft.print(F(">"));
      break;
    case 4:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*3);
      tft.print(F(">"));
      break;
    case 5:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*4);
      tft.print(F(">"));
      break;
    case 6:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*5);
      tft.print(F(">"));
      break;
    case 7:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*6);
      tft.print(F(">"));
  }
  tft.setTextSize(3);
}

void clearPointerSmall(byte whereAtNow){
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  switch(whereAtNow){
    case 1:
      tft.setCursor(10,TOP_FRAME_SIZE + 10);
      tft.print(F(">"));
    case 2:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21);
      tft.print(F(">"));
    case 3:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*2);
      tft.print(F(">"));
    case 4:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*3);
      tft.print(F(">"));
    case 5:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*4);
      tft.print(F(">"));
    case 6:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*5);
      tft.print(F(">"));
    case 7:
      tft.setCursor(10,TOP_FRAME_SIZE + 10 + 21*6);
      tft.print(F(">"));
  }
  tft.setTextSize(3);
  tft.setTextColor(color);
}

char numGen(byte current, boolean isaChannel){
  byte response = current;
  if(isaChannel){
    switch(myChar){
      case 'U':
      case '+':
        if(current<56){
          response++;
        }
        else{
          response = 49;
        }
        response += 80;
        break;
      case 'D':
      case '-':
        if(current>49){
          response--;
        }
        else{
          response = 56;
        }
        response += 80;
        break;
      case 'R':
        response +=160;
        break;
    }
  }
  else{
    switch(myChar){
      case 'U':
      case '+':
        if(current<57){
          response++;
        }
        else{
          response = 48;
        }
        response += 80;
        break;
      case 'D':
      case '-':
        if(current>48){
          response--;
        }
        else{
          response = 57;
        }
        response += 80;
        break;
      case 'R':
        response +=160;
        break;
    }
  }
  #ifdef DEBUG
  Serial.print("myChar currently is: ");
  Serial.println((byte)myChar);
  Serial.print("at end of numgen: ");
  Serial.println(response);
  #endif
  return response;
}
