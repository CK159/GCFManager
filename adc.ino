//Reads calibration from eeprom on startup
//Writes calibration to eeprom if hard coded variables are newer
//Rewrites all eeprom if resetEEPROM == true
void initializeCalibration()
{
  print("Calibration:");
  //First int = eeprom version
  //Secont int = calibration version
  //Then float calM[6]
  //then int calB[6]
  int readPos = 0;
  int eeV = 0;
  //EEPROM Version
  readPos += EEPROM_readAnything(0, eeV);
  if (resetEEPROM || eeV < eepromVersion)
  {
    EEPROM_writeAnything(0, eepromVersion);
    print(" WE ");
    print(eepromVersion);
  }
  //Calibration Version
  int eeC = 0;
  readPos += EEPROM_readAnything(readPos, eeC);
  if (resetEEPROM || eeV < eepromVersion || eeC < calVersion)
  {
    //Write calibration to eeprom
    int writePos = sizeof(eepromVersion);
    writePos += EEPROM_writeAnything(writePos, calVersion);
    writePos += EEPROM_writeAnything(writePos, calM);
    EEPROM_writeAnything(writePos, calB);
    print(" WC ");
    println(calVersion);
  }
  else
  {
    //Read calibration from eeprom
    calVersion = eeC;
    readPos += EEPROM_readAnything(readPos, calM);
    EEPROM_readAnything(readPos, calB);
    print(" RC ");
    println(eeC);
  }
}

//Applies calM and calB calibration data for selected channel to get final reading
float rawConvert(int raw, int channel)
{
  return calM[channel] * (raw + calB[channel]);
}

int simpleSample(int sampleCount, byte chanel)
{
    long val = 0;
    for (int s = 0; s < sampleCount; s++)
    {
      val += read_adc(chanel);
    }
    val /= sampleCount;
    return val;
}

void updateCalM(float goal, byte chanel)
{
  //Get current chanel reading, add chanel offset
  int raw = simpleSample(1024, chanel) + calB[chanel];
  float m = goal / raw;
  calM[chanel] = m;
  
  updateEEPROM(false);
}

void updateCalB(int offset, byte chanel)
{
  calB[chanel] = offset;
  
  updateEEPROM(true);
}

//Updates the calM or calB in eeprom when it is changed by serial commands
void updateEEPROM(boolean updateCalB)
{
  int pos = sizeof(eepromVersion);
  //Update & save version
  calVersion++;
  pos += EEPROM_writeAnything(pos, calVersion);
  if (updateCalB)
  {
    //calB is after calM so move past it
    pos += sizeof(calM);
    EEPROM_writeAnything(pos, calB);
    return;
  }
  EEPROM_writeAnything(pos, calM);
}

//Delay while constantly checking the analog voltage
void safeDelay(unsigned long d)
{
  unsigned long endTime = millis() + d;
  while(millis() < endTime)
  {
    avCheck();
  }
}

//Set up SPI for ADC
void enableSPI()
{
  if (!spiEnabled)
  {
    pinMode(selPin, OUTPUT);
    SPI.setClockDivider( SPI_CLOCK_DIV16 ); 
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0); //SPI 0,0 as per MCP330x data sheet 
    SPI.begin();
    spiEnabled = true;
  }
}

//Disable SPI and cut power going out on any pins
void disableSPI()
{
  if (spiEnabled)
  {
    SPI.end();
    pinMode(selPin, INPUT);
    pinMode(MOSI, INPUT);
    pinMode(MISO, INPUT);
    pinMode(SCK, INPUT);
    spiEnabled = false;
  }
}

boolean avCheck()
{
  if (avError)
  {
    return false;
  }
  
  digitalWrite(avLED, HIGH);
  int voltage = analogRead(avPin);
  digitalWrite(avLED, LOW);
  
  if (voltage < avCutoff)
  {
    //Shutdown SPI pins to prevent backfeeding power through ADC
    disableSPI();
    
    avError = true;
    print(F("AV Error: "));
    print(voltage);
    print(F(" < "));
    println(avCutoff);
    return false;
  }
  return true;
}

void averages(int *buf, unsigned long *input, int count)
{
  if (count != 0)
  {
    for (byte i = 0; i < 6; i++)
    {
      unsigned long temp = input[i] / count;
      buf[i] = (int)temp;
    }
  }
  else
  {
    memset(buf, 0, sizeof(int)*6);
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

void adcSample()
{
  //ensure analog board has power before trying to do anything
  if (!avCheck())
  {
    return;
  }
  
  for (int s = 0; s < sampleSize; s++)
  {
    for (byte i = 0; i < 6; i++)
    {
      fastSample[i] += read_adc(i);
    }
  }
  fastCount += sampleSize;
}

//From http://playground.arduino.cc/Code/MCP3208 
//0 - 7 to select ADC channel to read in single ended mode
int read_adc(int channel){
  int adcvalue = 0;
  int b1 = 0, b2 = 0;
  int sign = 0;
  
  //Command bits #1 and #2 format:
  //0000SM12
  //30000000
  //S: Start bit of 1 begins data transfer
  //M: 1 = Single ended mode, 0 = differential mode
  //1,2,3: channelz bits (D2, D1, D0 in datasheet)
  //Everything after this: Wait for ADC to process sample

  digitalWrite (selPin, LOW); // Select adc
  
  byte commandbits = B00001100;  // first byte
  //Use this for differential mode
  //byte commandbits = B00001000;
  
  commandbits |= (channel >> 1);   // high bit of channel
  SPI.transfer(commandbits);       // send out first byte of command bits

  // second byte; Bx0000000; leftmost bit is D0 channel bit
  commandbits = B00000000;
  commandbits |= (channel << 7);        // if D0 is set it will be the leftmost bit now
  b1 = SPI.transfer(commandbits);       // send out second byte of command bits

  // hi byte will have XX0SBBBB
  // set the top 3 don't care bits so it will format nicely
  b1 |= B11100000;
  sign = b1 & B00010000;
  int hi = b1 & B00001111;

  // read low byte
  b2 = SPI.transfer(b2);              // don't care what we send
  int lo = b2;
  digitalWrite(selPin, HIGH); // turn off device

  int reading = hi * 256 + lo;

  if (sign) {
    reading = (4096 - reading) * -1;
  }

  return (reading);
}
