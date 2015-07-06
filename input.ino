void clearBuffer()
{
  while (available())
  {
    read();
  }
}

void readInput()
{
  //Only process input for 100ms just in case theres too much to do
  unsigned long endTime = millis() + 100;
  
  while (available() > 0 && millis() < endTime)
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
        println(F("Command too long! Send new line to reset."));
        buffIndex=0;
        waitForNewline = true;
      }
      
    }
  }
}

void processInput()
{
  print('>');
  println(inputBuffer);
  
  byte len = strlen(inputBuffer);
  print(F("Length: "));
  println(len);
  
  //TODO: use character comparisons here instead of strncmp
  //Set power - 0 to 5 leds
  if (strncmp("p", inputBuffer, 1) == 0)
  {
    setPower();
  }
  //Calibration
  else if (strncmp("c", inputBuffer, 1) == 0)
  {
    calibration();
  }
  else if (strncmp("a", inputBuffer, 1) == 0)
  {
    resetAVCheck();
  }
  //Set led pin 13
  else if (strncmp("l", inputBuffer, 1) == 0)
  {
    setLED();
  }
  //Toggle LCD backlight
  else if (strncmp("b", inputBuffer, 1) == 0)
  {
    setBacklight();
  }
  //Restart cycle
  else if (strncmp("r", inputBuffer, 1) == 0)
  {
    newCycle();
  }
  else
  {
    println(F("Unknown Command"));
  }
}

void setLED()
{
  if(strcmp("1", &inputBuffer[1]) == 0)
  {
    digitalWrite(ledpin,1);
    println(F("LED On D13 ON ! "));
  }
  else if(strcmp("0", &inputBuffer[1]) == 0)
  {
    digitalWrite(ledpin,0);
    println(F("LED On D13 Off ! "));
  }
  else
  {
    println(F("setLED: Invalid value"));
  }
}

void setPower()
{
  byte pos = 1;
  removeWhitespace(pos);
  
  //Turn first non-blank character of buffer into int
  char *end;  
  int value = (int)strtol(&inputBuffer[pos], &end, 10); 
  if (end == &inputBuffer[pos] || *end != '\0')
  {
    println(F("setPower: Invalid value"));
    return;
  }
  if (value < 0)
  {
    println(F("setPower: Negative value is invalid"));
    return;
  }
  
  print(F("Power goal: "));
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
  
  print(F("Power Achieved: "));
  println(power);
}

//READ CALIBRATION
//c <type> <num>
//type = m or b calibration, num = 0-5 channel num
//SET B TERM CALIBRATION: c set b <num>
//Set b term to zero out channel <num>'s readings against the current voltage
//SET M TERM CALIBRATION: c set m <num> = <F.FF>
//Set the m term so that the channel <num>'s reading = <F.FF>
void calibration()
{
  byte pos = 1;
  removeWhitespace(pos);
  print(F("Cal: "));
  
  boolean set = false;
  
  //Set calibration mode
  if (strncmp("set", &inputBuffer[pos], 3) == 0)
  {
    pos += 3; //Move past the 'set' in the input buffer
    removeWhitespace(pos);
    print(F("Set: "));
    set = true;
  }
  else
  {
    print(F("Read: "));
  }
    
  //TODO: Possibly just read the character manually?
  //Get mode
  if (strncmp("m", &inputBuffer[pos], 1) == 0)
  {
    print(F("M: "));
  }
  else if (strncmp("b", &inputBuffer[pos], 1) == 0)
  {
    print(F("B: "));
  }
  else
  {
    println(F("IM"));
    return;
  }
  
  char mode = inputBuffer[pos];
  pos++;
  removeWhitespace(pos);
  
  //Get what channel we will be working with
  char *end;  
  int channel = (int)strtol(&inputBuffer[pos], &end, 10); 
  if (end == &inputBuffer[pos])
  {
    println(F("IC"));
    return;
  }
  if (channel < 0 || channel >= 6)
  {
    println(F("IC"));
    return;
  }
  print(channel);
  print(F(" :"));
  pos = end - inputBuffer; //set position to where strtol() left off
  print(F(" pos:"));
  print(pos);
  print(F(" : "));
  
  if (set)
  {
    if (mode == 'm')
    {
      //Set the M to the value necessary to make current reading equal user-entered reading
      removeWhitespace(pos);
      //Look for '=' character
      if (strncmp("=", &inputBuffer[pos], 1) != 0)
      {
        print(F("IF"));
        return;
      }
      pos++;
      
      //Read new calibration value as double and set it for selected channel
      removeWhitespace(pos);
      char *end;  
      float calVal = strtod(&inputBuffer[pos], &end); 
      if (end == &inputBuffer[pos])
      {
        println(F("IV1"));
        return;
      }
      if (calVal < 0 || calVal >= 1)
      {
        println(F("IV2"));
        return;
      }
      pos = end - inputBuffer; //set position to where strtol() left off
      print(F(" pos:"));
      print(pos);
      print(F(" F: "));
      println(calVal, 6);
    }
    else
    {
      //Set the B to offset the current readings
      int sampleCount = 1024;
      unsigned long offset = 0;
      for (int s = 0; s < sampleCount; s++)
      {
        offset += read_adc(channel);
      }
      print(F(" Offset: "));
      println(offset);
      offset /= sampleCount;
      print(F(" Offset: "));
      println(offset);
    }
  }
  else
  {
    //Reads current calibration values, no modification or EEPROM access
    if (mode == 'm')
    {
      println(calM[channel], 6);
    }
    else
    {
      println(calB[channel]);
    }
  }
    
    /*
  }
  else
  {
    //Read calibration mode
    print(F("Read: "));
    boolean valid = false;
    
    //TODO: Possibly just read the character manually?
    if (strncmp("m", &inputBuffer[pos], 1) == 0)
    {
      print(F("M: "));
      valid = true;
      
    }
    else if (strncmp("b", &inputBuffer[pos], 1) == 0)
    {
      print(F("B: "));
      valid = true;
    }
    else
    {
      println(F("Invalid"));
    }
    
    if (valid)
    {
      char mode = inputBuffer[pos];
      pos++;
      removeWhitespace(pos);
      
      char *end;  
      int value = (int)strtol(&inputBuffer[pos], &end, 10); 
      if (end == &inputBuffer[pos] || *end != '\0')
      {
        println(F("Invalid value"));
        return;
      }
      if (value < 0 || value >= 6)
      {
        println(F("Invalid value"));
        return;
      }
      println(value);
      
      if (mode == 'm')
      {
        println(calM[value], 6);
      }
      else
      {
        println(calB[value], 6);
      }
    }
  }*/
}

void resetAVCheck()
{
  byte pos = 1;
  removeWhitespace(pos);
  
  //Always print out status of AV
  print(F("AV: "));
  print(analogRead(avPin));
  print(F(" AVC: "));
  println(avCheck());
  
  //Don't do anything if just checking status
  if (inputBuffer[pos] == '\0')
  {
    return;
  }
  
  //1: set avError = true;
  //0: set avError = false if anlog voltage is good
  if(strcmp("1", &inputBuffer[pos]) == 0)
  {
    avError = true;
    disableSPI();
    println(F("Set avError"));
  }
  else if(strcmp("0", &inputBuffer[pos]) == 0)
  {
    if (analogRead(avPin) < avCutoff)
    {
      println(F("Error: < avCutoff"));
    }
    else
    {
      avError = false;
      enableSPI();
      println(F("Reset avError"));
    }
  }
  else
  {
    println(F("Invalid Value"));
  }
}

void setBacklight()
{
  static boolean backlightOn = true;
  byte pos = 1;
  removeWhitespace(pos);
  
  if (inputBuffer[pos] == '\0')
  {
    if (backlightOn)
    {
      lcd.noBacklight();
    }
    else
    {
      lcd.backlight();
    }
    backlightOn = !backlightOn;
    println(F("LCD Backlight Toggle"));
    return;
  }
  
  if(strcmp("1", &inputBuffer[pos]) == 0)
  {
    backlightOn = true;
    lcd.backlight();
    println(F("LCD Backlight ON!"));
  }
  else if(strcmp("0", &inputBuffer[pos]) == 0)
  {
    backlightOn = false;
    lcd.noBacklight();
    println(F("LCD Backlight OFF!"));
  }
  else
  {
    println(F("setBacklight: Invalid value"));
  }
}

void newCycle()
{
  byte pos = 1;
  removeWhitespace(pos);
  
  if (inputBuffer[pos] == '\0')
  {
    cycleStart = millis();
    cycle++;
    println(F("newCycle: Started"));
  }
  else
  {
    println(F("newCycle: Invalid value"));
  }
}

//moves to next non-whitespace character in input buffer
void removeWhitespace(byte &pos)
{
  while (inputBuffer[pos] == ' ')
  {
    pos++;
  }
}
