

const int tempPin = 4; //Plug in all DS18B20 Temperature chips here
const int btState = 5; //Pin for monitoring link state from bluetooth module
const int selPin = A1; //chip-select for ADC SPI Communication
const int avPin = 8;  //Monitor analog board 5V rail. Ensure it has power before attempting communication
const int avLED = A0;  //Lights up when checking analog board voltage
const int ledpin = 13; //Lights up when processing in main loop
const int enterPin = A5;
const int stopPin = A4;
const int beepPin = 7;
const byte totalChanels = 3; //Number of power chanels for discharging
const byte chanelPins[totalChanels] = {11, 10, 9}; //Pins of each chanel. Must match up with chanel array above
const byte chanelPower[totalChanels] = {2, 2, 1}; //Relative power of each chanel. MUST BE IN DESCENDING ORDER
const byte maxTempSensors = 4; //Maximum number of DS18B20 temp sensors that will be detected
const unsigned long loopDelay = 1000; //ms between log intervals. Default 1000 (1 samples/s)
const unsigned long fastLCDDelay = 250; //approx ms between partial LCD screen updates (V, A, cell V only)
const byte bufferSize = 20; //maximum length a valid serial comand can be
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
char channelCodes[6] = {'d', 'c', 'x', 'y', 'z', 't'}; //How do I make a non-null terminated string?

//EEPROM variables
//Format: eepromVersion, calVersion, calM[6], calB[6]
const boolean resetEEPROM = false; // Will rewrite the EEPROM on startup with values below (useful if EEPROM is not in known state)
const int eepromVersion = 1; //Rewrites EEPROM content if eeprom version < this
int calVersion = 0; // Loads calibration from eeprom if this < eeprom calVersion, otherwise write calibration to eeprom on startup

//CALIBRATION
//These are default calibration values. EEPROM stores current calibration and it can be adjusted through serial commands 
//y = M(x + B) (M = multiplication factor, B = offset voltage correction, x = raw (or averaged) ADC integer value)
float calM[6] = {0.0050f, 0.0050f, 0.0012207f, 0.0012207f, 0.0012207f, 0.0033f};
int   calB[6] = {0, 1, 2, 3, 4, 5};

//LCD bar graph scaling
const int lcdSteps = 41;
const float voltStart = 9.6f; //9.6-13 voltage range
const float voltEnd = 13.0f;
const float chargeMax = 21.0f; //0-21 amps
const float dischargeMax = 25.0f; //0-25 amps
const float tempScale = 35.0f; //0-35 degree above ambient

byte totalTemp = 0; //Total # of temp sensors detected, will not excede maxTemp. Excess sensors will be ignored
float fahrenheit[maxTempSensors]; //Most recent temperature for each sensor
byte addr[8*maxTempSensors]; //8 byte address for each detedted temp sensor
unsigned long nextLoop = 0;
unsigned long intervalCount = 0;
unsigned long timerStart = 0;
int cycle = 0;
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

//Charger commands and beep detection parameters
const unsigned long beepMin = 100; //Minimum length of a valid charger beep
const unsigned long beepMax = 500; //Maximum length of a valid beep
const unsigned long beepWindow = 3000; //Time window for beeps to be grouped together to detect end of charge
const byte beepCount = 4; //Number of beeps in the beepWindow to detect end of charge
const int pressTime = 200; //Time spent triggering a button on the charger
const byte longPressCount = 2; //full cycles spent triggering a long button press on the charger
byte longPress = 0; //full cycles spent triggering a long button press on the charger
volatile byte pendingBeep = 0; //Number of beeps detected in this beep sequence
volatile unsigned long beepStart = 0; //When a beep starts. Used to calculate beep length
volatile unsigned long beepWindowStart = 0; //When a beep sequence starts.
volatile boolean ignoreBeeps = false; //Used to not listen to the beeps that occur when triggering start and stop buttons

//Charger state variables. This controls when the charger will switch modes
int cycleLimit = 0; //Number of cycles to perform - must be set at startup
int dischargePower = 5; //Power used for discharging. TODO: Make roughly equivalent to watts and make it auto adjust
const unsigned long waitTime = 30000; //Milliseconds to spend in M_CHGW and M_DSCW
unsigned long waitStart = 0; //millis() of when M_CHGW or M_DSCW begin
const float targetCell = 3.7f; //Target resting voltage of lowest cell after M_DSCW
float cutoffCell = 3.4f; //Calculated loaded cell voltage used to cut off M_DSC. Recalculated after M_DSCW based on actual cell and targetCell

//Safety Constants + error detection for abnormal conditions, exceeded limits, etc.
const float maxTemp = 135.0f; //The f is for Fahrenheit - max absolute battery temp
const float maxDeltaTemp = 35.0f; //Effectively puts max ambient temp around 100f
const float maxImbalance = 0.25f; //Maximum voltage difference allowed between any cells
const float minCell = 3.1f; //Error triggered if any cell falls below this
const float maxCell = 4.25f; //Error triggered if triggered if any cell exceeds this
const unsigned long minChg = 420000; //7 minutes (7*60*1000) - error if charging time is less than this (except initial charge)
const unsigned long minDsc = 240000; //4 minutes - error if discharging time is less than this
const unsigned long maxChg = 1800000; //30 minutes  - error if charging time is more than this
const unsigned long maxDsc = 900000; //15 minutes - error if discharging time is more than this
int overshootCount = 0; //Number of times the auto discharge power adjust overshot or undershot targer output
int undershootCount = 0; //If these numbers are high, could indicate some channel of discharger has failed
const int undershootLimit = 10; //Error if more than this number of overshoots in one discharge cycle

//Charger modes
const char stateText[7][5] = {"WAIT", "CHG+", "CHGW", "DSC-", "DSCW", "DONE", "EROR"};
byte state = 0; //Current charger state
char error = '\0'; //Indicates type of error that ocurred.
//'Convenient' constants for all the modes
#define M_WAIT 0 //Not doing any active cycling, startup state
#define M_CHG 1 //The charger is active
#define M_CHGW 2 //Charge Wait: Monitor voltage falloff after charger indicates its done
#define M_DSC 3 //The discharge load is active
#define M_DSCW 4 //Discharge Wait: Discharger shut off, Monitor voltage bounce back
#define M_DONE 5 //Number of cycles or capacity cutoff reached
#define M_ERROR 6 //Error occurred, shut off charger and discharger.

//Communication mode
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


void loop()
{
  digitalWrite(ledpin, HIGH);
  
  readTemp();
  intervalCount++;
  processLongPress();
  unsigned long seconds = (millis() - timerStart) / 1000;

  //Update both lcd parts
  LCDUpdate(seconds);
  LCDPartialUpdate();
  transferFastSample(); //Redundant but for safety

  //ADC Readings - Get full averages and use them to update everything this cycle
  int avg[6];
  averages(avg, sample, sampleCount);
  float avgf[6];
  for (byte i = 0; i < 6; i++)
  {
    avgf[i] = rawConvert(avg[i], i);
  }

  //Determine what the program should be doing
  stateManager(avgf);
  
  //Begin entry
  print('@');
  print(intervalCount);
  print(F(" M:"));
  print(stateText[state]);
  print(F(" C:"));
  println(cycle);
  
  //Temperature
  printTemps();
  
  print(F(":V"));
  //Output all channels
  for (byte i = 0; i < 6; i++)
  {
    print(' ');
    print(channelCodes[i]);
    print(':');
    print(avgf[i]);
  }
  println();
  
  //Elapsed time this cycle
  print(F(":T "));
  println(seconds);

  //Any specialty satae-related outputs
  printState(avgf);
  
  //Closing entry for this update
  print('$');
  println(intervalCount);
  println();

  //Long beep check (now handled in stateManager()
  //processBeep();
  
  //Reset averages
  memset(sample, 0, sizeof(sample));
  sampleCount = 0;
  
  digitalWrite(ledpin, LOW);
  delayUntil();
}

void setPower(int goal)
{
  print(F("Power goal: "));
  println(goal);
  
  int power = 0;
  for(byte i = 0; i < totalChanels; i++)
  {
    if (power + chanelPower[i] <= goal)
    {
      power += chanelPower[i];
      digitalWrite(chanelPins[i], HIGH);
    }
    else
    {
      digitalWrite(chanelPins[i], LOW);
    }
  }
  
  print(F("Power Achieved: "));
  println(power);
}

void startBtn()
{
  ignoreBeeps = true;
  longPress = longPressCount;
  digitalWrite(enterPin, HIGH);
}

void stopBtn()
{
  ignoreBeeps = true;
  digitalWrite(stopPin, HIGH);
  delay(pressTime);
  digitalWrite(stopPin, LOW);
  ignoreBeeps = false;
}

//Long beep check - Stop pressing start button after set number of iterations of main loop
void processLongPress()
{
  if (longPress > 0)
  {
    longPress--;
    if (longPress == 0)
    {
      digitalWrite(enterPin, LOW);
      ignoreBeeps = false;
    }
  }
}

void beepInterrupt()
{
  boolean state = digitalRead(beepPin);
  if (state)
  {
    if (ignoreBeeps) {return;}
    
    beepStart = millis();
    //Beginning of beep sequence
    if (pendingBeep == 0)
    {
      beepWindowStart = beepStart;
    }
  }
  else
  {
    unsigned long len = millis() - beepStart;
    //Check if valid beep length
    if (len > beepMin && len < beepMax)
    {
      pendingBeep++;
      beepStart = 0;
    }
  }
}

void delayUntil()
{
  unsigned long cTime = millis();
  unsigned long nextLCD = cTime + fastLCDDelay;
  nextLoop += loopDelay;
  
  //print(F("Sleeping for "));
  if (nextLoop > cTime)
  {
    //println(nextLoop - cTime);
  }
  else
  {
    print('!');
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
