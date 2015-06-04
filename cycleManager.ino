#include <OneWire.h>

OneWire ds(A5); // Plug in all DS18S20 Temperature chips here
const int ledpin=13; // led on D13 will show blink on / off
const int btState = A4;
const unsigned long loopDelay = 1000; //ms between log intervals. Default 250 (4 samples/s)

const byte totalChanels = 3;
const byte chanel[3] = {2, 2, 1};
const byte chanelPins[3] = {11, 10, 9};

int BluetoothData; // the data given from Computer
unsigned long nextLoop = 0;
unsigned long sampleCount = 0;

byte totalTemp = 0; //Total # of temp sensors detected
float fahrenheit[4] = {0.0, 0.0, 0.0, 0.0}; //support for up to 4 temp sensors
byte addr[32];
byte data[12];

const byte bufferSize = 16;
char inputBuffer[bufferSize];
byte buffIndex = 0;
boolean waitForNewline = false;

#define UNSET 0
#define USB 1
#define BLUETOOTH 2
byte commMode = UNSET;

void setup() {
  // put your setup code here, to run once:
  
  pinMode(ledpin, OUTPUT);
  pinMode(btState, INPUT);
  for (byte i = 0; i < totalChanels; ++i)
  {
    pinMode(chanelPins[i], OUTPUT);
    digitalWrite(chanelPins[i], LOW);
  }
  
  Serial.begin(9600);
  delay(100);
  Serial1.begin(9600);
  delay(100);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for Leonardo only
  //}
  //while(true)
  //{
  //  digitalWrite(ledpin, !digitalRead(ledpin));
  //  Serial.println("Entering main loop.");
  //  delay(500);
  //}
  
  establishContact();
  
  initTemp();
  
  nextLoop = millis(); //Start counting loops from now
  
  println("Entering main loop.");
}

void loop()
{
  readTemp();
  
  printTemps();
  println("F");
  print("BT Status: ");
  println(digitalRead(btState));
  
  sampleCount++;
  print("Sample: ");
  println(sampleCount);
  
  delayUntil();
}

void delayUntil()
{
  nextLoop += loopDelay;
  
  print("Sleeping for ");
  unsigned long cTime = millis();
  if (nextLoop > cTime)
  {
    unsigned long d = nextLoop - cTime;
    println(d);
  }
  else
  {
    println("0ms!");
  }
  
  digitalWrite(ledpin,0);
  //Handle input while waiting
  while (millis() < nextLoop)
  {
    readInput();
    delay(50);
  }
  digitalWrite(ledpin,1);
}

void readInput()
{
  while (available() > 0)
  {
    char newData = read();
    
    if (newData == '\n' || newData == '\r')
    {
      //New line
      waitForNewline = false;
      inputBuffer[buffIndex] = '\0';
      if (buffIndex > 0)
      {
        processInput();
      }
      buffIndex = 0;
    }
    else
    {
      //Not new line. Check if character can be put into buffer
      if (buffIndex < bufferSize-1 && waitForNewline == false)
      {
        inputBuffer[buffIndex] = newData;
        buffIndex++;
      }
      else
      {
        //Command too long. Wait for a new line character before attempting to begin reading again.
        println("Command too long! Send new line to reset.");
        buffIndex=0;
        waitForNewline = true;
      }
      
    }
  }    
}

void processInput()
{
  println("This just in ... ");
  println(inputBuffer);
  
  byte length = strlen(inputBuffer);
  print("Command Length: ");
  println(length);
  
  //Set power - 0 to 5 leds
  if (strncmp("p", inputBuffer, 1) == 0)
  {
    setPower();
  }
  //Set output chanel
  else if (strncmp("c", inputBuffer, 1) == 0)
  {
    setChanel();
  }
  //Set led pin 13
  else if (strncmp("l", inputBuffer, 1) == 0)
  {
    setLED();
  }
}

void setLED()
{
  if(strcmp("1", &inputBuffer[1]) == 0)
  {
    digitalWrite(ledpin,1);
    println("LED On D13 ON ! ");
  }
  else if(strcmp("0", &inputBuffer[1]) == 0)
  {
    digitalWrite(ledpin,0);
    println("LED On D13 Off ! ");
  }
  else
  {
    println("setLED: Invalid value");
  }
}

void setPower()
{
  print("setPower: ");
  println(inputBuffer);
  
  //Turn second character of buffer into int
  char *end;  
  int value = (int)strtol(&inputBuffer[1], &end, 10); 
  if (end == &inputBuffer[1] || *end != '\0')
  {
    println("setPower: Invalid value");
    return;
  }
  if (value < 0)
  {
    println("setPower: Negative value is invalid");
    return;
  }
  
  print("Power goal: ");
  println(value);
  
  int power = 0;
  for(byte i = 0; i < totalChanels; i++)
  {
    if (power + chanel[i] <= value)
    {
      power += chanel[i];
      digitalWrite(chanelPins[i], HIGH);
    }
    else
    {
      digitalWrite(chanelPins[i], LOW);
    }
  }
  
  print("Power Achieved: ");
  println(power);
}

void setChanel()
{
  print("setChanel: ");
  println(inputBuffer);
  
  //Turn second character of buffer into int
  /*char *end;  
  int value = (int)strtol(&inputBuffer[1], &end, 10); 
  if (end == &inputBuffer[1] || *end != '\0')
  {
    println("setPower: Invalid value");
    return;
  }
  if (value < 0)
  {
    println("setPower: Negative value is invalid");
    return;
  }*/
}

void establishContact() {
  delay(5000);
  while (true)
  {   
    println("Press A to initialize");
    digitalWrite(ledpin, !digitalRead(ledpin));
    delay(1500);
    
    while (Serial.available())
    {
      int data = Serial.read();
      
      if (data == 'A' || data == 'a')
      {
        commMode = USB;
        clearBuffer();
        println("GottaChargeFast.com Initializing on USB...");
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
        println("GottaChargeFast.com Initializing on Bluetooth...");
        return;
      }
    }
  }
}

void clearBuffer()
{
  while (available())
  {
    read();
  }
}

void printTemps()
{
  print("Temps: ");
  for (byte i = 0; i < totalTemp; i++)
  {
    print(fahrenheit[i]);
    if (totalTemp - i != 1)
    {
      print(", ");
    }
  }
}

boolean readTemp()
{
  //Check if any sensor is still processing
  for (byte i = 0; i < totalTemp; i++)
  {
    ds.reset();
    ds.select(&addr[i*8]);
    //If any sensor is still busy, return.
    //They should all take the same amount of time so this simplifies things
    if (ds.read() == 0)
    {
      println("Duplicate reading");
      return false;
    }
  }
  
  //Sensor not processing. Get the new value and restart
  print("New Reading: ");
  
  for (byte i = 0; i < totalTemp; i++)
  {
    //Tell sensor we want to read scratchpad
    ds.reset();
    ds.select(&addr[i*8]);
    ds.write(0xBE);
    
    //Receive data from sensor
    for (byte j = 0; j < 9; j++) { // we need 9 bytes (but reserved 12 bytes???) -- from code example
      data[j] = ds.read();
      print(data[j], HEX);
      print(" ");
    }
    print(' ');
    convertTemp(i); //(i) tells the function where to save the result
  }
  println("");
  
  startNewConversion();
  
  return true;
}

void convertTemp(byte i)
{
  float celsius = ( (data[1] << 8) + data[0] )*0.0625;
  fahrenheit[i] = celsius * 1.8 + 32.0;
}

void startNewConversion()
{
  //Tell all sensors to start new conversion
  ds.reset();
  ds.write(0xCC); //Select all
  ds.write(0x44);
}

void initTemp()
{
  println("Initializing Temp Sensors...");
  
  byte currentSensor = 0;
  byte newAddr[8];
  
  while (ds.search(newAddr))
  {
    //Print unique number for temp sensor
    print("Sensor ");
    print(currentSensor);
    print(": ROM =");
    for(byte i = 0; i < 8; i++)
    {
      print(' ');
      print(newAddr[i], HEX);
    }
    //Verify crc for temp sensor
    if (OneWire::crc8(newAddr, 7) != newAddr[7]) {
      println("CRC is not valid!");
      delay(2000);
      continue;
    }
    // Print temp sensor type
    boolean valid = false;
    switch (newAddr[0])
    {
      case 0x10:
        println(" Unsupported Chip=DS18S20");  // or old DS1820
        break;
      case 0x28:
        println(" Chip = DS18B20");
        valid = true;
        break;
      case 0x22:
        println(" Unsupported Chip=DS1822");
        break;
      default:
        println("Device is not a DS18x20 family device.");
        break;
    }
    //Valid device found, save address
    if (valid)
    {
      for(byte i = 0; i < 8; i++)
      {
        addr[totalTemp * 8 + i] = newAddr[i];
      }
      totalTemp++;
    }
    currentSensor++;
  }
  
  print("Found ");
  print(totalTemp);
  println(" temp sensors.");
  
  for(byte i = 0; i < 32; i++)
  {
    print(' ');
    print(addr[i], HEX);
  }
  println("");
  
  /*while(true)
  {
    delay(2000);
    println("Wait");
  }*/
  
  startNewConversion();
  do
  {
    delay(200);
  }
  while (!readTemp());
  
  print("Initial ");
  printTemps();
  return;
}
