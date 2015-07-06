

const int tempPin = 4; //Plug in all DS18B20 Temperature chips here
const int btState = 5; //Pin for monitoring link state from bluetooth module
const int selPin = A1; //chip-select for ADC SPI Communication
const int avPin = 8;  //Monitor analog board 5V rail. Ensure it has power before attempting communication
const int avLED = A0;  //Lights up when checking analog board voltage
const int ledpin = 13; //Lights up when processing in main loop
const int enterPin = A5;
const int stopPin = A4;
const int beepPin = 7;
const byte chanelPins[3] = {11, 10, 9}; //Pins of each chanel. Must match up with chanel array above
const byte chanel[3] = {2, 2, 1}; //Relative power of each chanel. MUST BE IN DESCENDING ORDER
const byte totalChanels = 3; //Number of power chanels for discharging
const byte maxTemp = 4; //Maximum number of DS18B20 temp sensors that will be detected
const unsigned long loopDelay = 1000; //ms between log intervals. Default 1000 (1 samples/s)
const unsigned long fastLCDDelay = 250; //approx ms between partial LCD screen updates (V, A, cell V only)
const byte bufferSize = 16; //maximum length a valid serial comand can be
const int avCutoff = 920; //Cutoff of avPin ADC reading. Below this will be considered analog board voltage error
const int sampleSize = 32; //Number of samples per chanel to read each time the readADC() function is called

//Map each temp sensor to a unique value. Useful for assigning a number to a particular sensor purpose
//Sensors are listed in order based on their unique ID thus the sensor list order can change when any sensor is changed
//This number can stay the same even if the physical temp sensor is replaced and ends up in a different place in the sensor list
#define AMBIENT 0
#define BATT1 1
#define BATT2 2
#define AUX 3
const byte sensorMap[] = {AMBIENT, BATT1, BATT2, AUX}; //Should have number of entries = maxTemp

#define DSC 0
#define CHG 1
#define C1 2
#define C2 3
#define C3 4
#define TOT 5

//CALIBRATION
const int calVersion = 0; //The default calibration specified here will be used if this number is larger than the EEPROM cal version
//These are default calibration values. EEPROM stores current calibration and it can be adjusted through serial commands 
//y = Mx + B (M = multiplication factor, B = offset voltage correction, x = raw ADC value)
/*const float voltM = .0033f;
const float voltB = 0f;
const float ampM = .0050f;
const float ampB = 0f;*/
const float calM[6] = {0.0050f, 0.0050f, 0.0012207f, 0.0012207f, 0.0012207f, 0.0033f};
const int   calB[6] = {0, 1, 2, 3, 4, 5};

//LCD bar graph scaling
const float voltStart = 9.6f; //9.6-12.6 voltage range
const float voltScale = 13.66f;
const float chargeScale = 0.01025f; //0-21 amps
const float dischargeScale = 0.006098f; //0-25 amps
const float tempScale = 1.1714f; //0-35 degree above ambient

byte totalTemp = 0; //Total # of temp sensors detected, will not excede maxTemp. Excess sensors will be ignored
float fahrenheit[maxTemp]; //Most recent temperature for each sensor
byte addr[8*maxTemp]; //8 byte address for each detedted temp sensor
unsigned long nextLoop = 0;
unsigned long intervalCount = 0;
unsigned long cycleStart = 0;
unsigned int cycle = 0;
char inputBuffer[bufferSize];
byte buffIndex = 0;
boolean waitForNewline = false;
boolean avError = false;
boolean spiEnabled = false; //Controlled by enableSPI() and disableSPI()
//Holds totals and counts of ADC readings for the 6 chanels measured.
//fastSample is used to calculate values on the LCD at fast intervals ONLY. Values added to sample[] after each fast LCD update
unsigned long fastSample[6];
unsigned long sample[6]; //Holds one whole cycle worth of data
unsigned int fastCount = 0;
unsigned int sampleCount = 0;

#define UNSET 0
#define USB 1
#define BLUETOOTH 2
byte commMode = UNSET;

#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <EEPROM.h>
#include "eepromAccess.h"

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
OneWire ds(tempPin);

void setup()
{
  //IMPORTANT: Pullup on Serial1 RX ON!
  // Allows disconnection of Bluetooth module from Arduino without floating serial input pins causing garbage input
  pinMode(0, INPUT);
  digitalWrite(0, HIGH);
  
  pinMode(ledpin,   OUTPUT);
  pinMode(avLED,    OUTPUT);
  pinMode(btState,  INPUT);
  pinMode(avPin,    INPUT);
  pinMode(selPin,   OUTPUT);
  pinMode(enterPin, OUTPUT);
  pinMode(stopPin,  OUTPUT);
  pinMode(beepPin,  INPUT);
  
  digitalWrite(ledpin, LOW);
  digitalWrite(avLED, LOW);
  digitalWrite(selPin, HIGH);
  digitalWrite(enterPin, LOW);
  digitalWrite(beepPin, LOW);
  
  for (byte i = 0; i < totalChanels; ++i)
  {
    pinMode(chanelPins[i], OUTPUT);
    digitalWrite(chanelPins[i], LOW);
  }
  
  memset(sample, 0, sizeof(sample));
  memset(fastSample, 0, sizeof(fastSample));
  
  Serial.begin(9600);
  Serial1.begin(9600);
  
  lcd.begin(20,4);
  
  enableSPI();
  avCheck();
  
  establishContact();
  
  //barTest();
  
  lcd.clear();
  initTemp();
  
  //This number is incremented any time the calibration values are changed
  print(F("Calibration: "));
  int calNum = -1;
  EEPROM_readAnything(0, calNum);
  println(calNum);
  
  nextLoop = millis(); //Start counting loops from now
  
  cycleStart = millis(); //TODO: Make work better
  
  println(F("Entering main loop."));
  println();
}

void loop()
{
  digitalWrite(ledpin, HIGH);
  
  readTemp();
  intervalCount++;
  
  print(F("BT Status: "));
  println(digitalRead(btState));
  
  //Use 'a' command for this info
  /*digitalWrite(avLED, HIGH);
  print(F("AV: "));
  println(analogRead(avPin));
  digitalWrite(avLED, LOW);
  print(F("AVC: "));
  println(avCheck());*/
  
  print(F("Interval: "));
  println(intervalCount);

  unsigned int fsTemp = fastCount;
  LCDUpdate();
  transferFastSample(); //Redundant but for safety
  
  //Value processing
  printTemps();
  println(F("Vals: "));
  print(F("S"));
  println(sampleCount);
  print(F("F"));
  println(fsTemp);
  
  //Reset averages
  memset(sample, 0, sizeof(sample));
  sampleCount = 0;
  
  digitalWrite(ledpin, LOW);
  delayUntil();
}



void LCDUpdate()
{
  //Status
  lcd.setCursor(0, 0);
  lcd.print("STAT");
  
  //MAH
  lcd.setCursor(5, 1);
  lcd.print("1234");
  
  //Time
  unsigned long seconds = (millis() - cycleStart) / 1000;
  printTime(seconds, 9, 1);
  
  //Cycle count
  lcd.setCursor(0, 1);
  clearPrint((long)cycle, 3, 0, 1, '0');
  lcd.print('C');
  
  //Temps + Temp bar graph
  float maxBatt = max(fahrenheit[BATT1], fahrenheit[BATT2]);
  float delta = maxBatt - fahrenheit[AMBIENT];
  float scaledTemp = max(delta * tempScale, 0);
  
  drawBar((byte)scaledTemp, 3, 'T');
  clearPrint(maxBatt, 3, 1, 0, 3);
  lcd.setCursor(6, 3);
  lcd.print('(');
  clearPrint(delta, 2, 1, 7, 3);
  lcd.print(')');
  clearPrint(fahrenheit[AUX], 3, 1, 12, 3);
  
  //Everything else
  LCDPartialUpdate();
}

//Updates just the more frequently changing information (voltages,currents, etc)
void LCDPartialUpdate()
{
  unsigned long avg[6];
  averages(avg, fastSample, fastCount);
  
  lcd.setCursor(0, 2);
  lcd.print('A');
  clearPrint(avg[0], 7, 1, 2, '0');
  lcd.setCursor(8, 2);
  lcd.print('B');
  clearPrint(avg[1], 7, 9, 2, '0');
  
  //float amperage = ampM * (float)avg[1] + ampB;
  float amperage = calM[CHG] * (float)avg[CHG] + calB[CHG];
  
  clearPrint(amperage, 2, 1, 6, 0);
  lcd.print('A');
  
  //float voltage = voltM * (float)avg[0] + voltB;
  float voltage = calM[TOT] * (float)avg[TOT] + calB[TOT];
  
  clearPrint(voltage, 2, 2, 11, 0);
  lcd.print('V');
  
  //lcd.setCursor(0, 2);
  //lcd.print("4.00  4.00  4.00");
  
  
  //TODO: Make this based off of the voltage and amperage floats which have the calibration values applied to them
  float aBar = max(avg[1] * chargeScale , 0);
  float vBar = max((voltage - voltStart) * voltScale , 0);
  
  unsigned long a1 = (millis() - cycleStart) % 45;
  unsigned long b1 = millis() % 45;
  
  drawBar((byte)vBar, 1, 'V');
  drawBar((byte)aBar, 2, 'A');
  
  transferFastSample();
}

void averages(unsigned long *buf, unsigned long *input, int count)
{
  if (count != 0)
  {
    for (byte i = 0; i < 6; i++)
    {
      buf[i] = input[i] / count;
    }
  }
  else
  {
    memset(buf, 0, sizeof(unsigned long)*6);
  }
}

//Takes any samples in the fastSample and puts them into the main sample variables
void transferFastSample()
{
  for (byte i = 0; i < 6; i++)
  {
    sample[i] += fastSample[i];
    fastSample[i] = 0;
  }
  sampleCount += fastCount;
  fastCount = 0;
}

void delayUntil()
{
  unsigned long cTime = millis();
  unsigned long nextLCD = cTime + fastLCDDelay;
  nextLoop += loopDelay;
  
  print(F("Sleeping for "));
  if (nextLoop > cTime)
  {
    println(nextLoop - cTime);
  }
  else
  {
    print(F("0ms! "));
    print(cTime - nextLoop);
    println(F("ms lost!"));
    //Prevent falling behind and building up a bunch of queued loops
    nextLoop = cTime;
  }
  
  //Handle input while waiting
  while (millis() < nextLoop)
  {
    if (millis() > nextLCD)
    {
      nextLCD = millis() + fastLCDDelay;
      LCDPartialUpdate();
    }
    readInput();
    adcSample();
  }
}

void establishContact() {
  lcd.setCursor(1, 0);
  lcd.print("GottaChargeFast.com");
  lcd.setCursor(1, 1);
  lcd.print("Press 'A' on");
  lcd.setCursor(1, 2);
  lcd.print("USB or");
  lcd.setCursor(1, 3);
  lcd.print("Bluetooth");
  
  //Clear buffers of anything that may have gotten in there from startup
  while (Serial.available()){Serial.read();}
  while (Serial1.available()){Serial1.read();}
  
  boolean flash = false;
  setArrow();
  
  while (true)
  {   
    println(F("Press A to initialize"));
    digitalWrite(ledpin, !digitalRead(ledpin));
    
    for (byte i = 0; i < 2; i++)
    {
      char arrowChar = 126;
      if (flash)
      {
        arrowChar = 7;
      }
      if (digitalRead(btState))
      {
        lcd.setCursor(0, 2);
        lcd.print(' ');
        lcd.setCursor(0, 3);
        lcd.print(arrowChar);
      }
      else
      {
        lcd.setCursor(0, 3);
        lcd.print(' ');
        lcd.setCursor(0, 2);
        lcd.print(arrowChar);
      }
      flash = !flash;
      
      safeDelay(750);
    }
    
    while (Serial.available())
    {
      int data = Serial.read();
      
      if (data == 'A' || data == 'a')
      {
        commMode = USB;
        clearBuffer();
        println(F("GottaChargeFast.com Initializing on USB..."));
        return;
      }
    }
    while (Serial1.available())
    {
      int data = Serial1.read();
      
      if (data == 'A' || data == 'a')
      {
        commMode = BLUETOOTH;
        clearBuffer();
        println(F("GottaChargeFast.com Initializing on Bluetooth..."));
        return;
      }
    }
  }
}
