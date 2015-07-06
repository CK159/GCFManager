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
