byte arrow[8] = { 
  B00000, 
  B00100, 
  B00010, 
  B11111, 
  B00010, 
  B00100, 
  B00000, 
  B11111 };
  
byte vChar[8] = { 
  B00000, 
  B01010, 
  B01010, 
  B01010, 
  B01010, 
  B00100, 
  B00000, 
  B00000, };
  
byte aChar[8] = { 
  B00000, 
  B00100, 
  B01010, 
  B01010, 
  B01110, 
  B01010, 
  B00000, 
  B00000 };
  
byte tChar[8] = { 
  B00000, 
  B01110, 
  B00100, 
  B00100, 
  B00100, 
  B00100, 
  B00000, 
  B00000 };
  
byte bChar[8] = { 
  B00000, 
  B01100, 
  B01010, 
  B01100, 
  B01010, 
  B01100, 
  B00000, 
  B00000 };

void LCDUpdate(unsigned long seconds)
{
  //Status
  lcd.setCursor(0, 0);
  lcd.print(stateText[state]);
  
  //MAH
  lcd.setCursor(5, 1);
  lcd.print("1234");
  
  //Time
  //unsigned long seconds = (millis() - cycleStart) / 1000;
  printTime(seconds, 9, 1);
  
  //Cycle count
  lcd.setCursor(0, 1);
  clearPrint((long)cycle, 3, 0, 1, '0');
  lcd.print('C');
  
  //Temps + Temp bar graph
  float maxBatt = max(fahrenheit[BATT1], fahrenheit[BATT2]);
  float delta = maxBatt - fahrenheit[AMBIENT];
  float scaledTemp = max(delta * (lcdSteps / tempScale), 0);
  
  drawBar((byte)scaledTemp, 3, 'T');
  clearPrint(maxBatt, 3, 1, 0, 3);
  lcd.setCursor(6, 3);
  lcd.print('(');
  clearPrint(delta, 2, 1, 7, 3);
  lcd.print(')');
  clearPrint(fahrenheit[AUX], 3, 1, 12, 3);
  
  //Everything else (now handled in the main loop for some fast-sample related timing benefits)
  //LCDPartialUpdate();
}

//Updates just the more frequently changing information (voltages,currents, etc)
void LCDPartialUpdate()
{
  int avg[6];
  averages(avg, fastSample, fastCount);

  float voltage = rawConvert(avg[TOT], TOT);
  float amperage = rawConvert(avg[CHG], CHG);
  
  clearPrint(amperage, 2, 2, 5, 0);
  lcd.print('A');
  
  clearPrint(voltage, 2, 2, 11, 0);
  lcd.print('V');
  
  //Cell Voltages: Cell 1
  clearPrint(max(rawConvert(avg[C1], C1), 0), 1, 3, 0, 2);  //Cell 1
  clearPrint(max(rawConvert(avg[C2], C2), 0), 1, 3, 6, 2);  //Cell 2
  clearPrint(max(rawConvert(avg[C3], C3), 0), 1, 3, 12, 2); //Cell 3
  //NOTE: Temporary negative values on startup can cause the cell lines to print funny due to negative sign.
  //Either clamp value to minimum 0 OR always blank the empty space character after each cell reading
  
  //Done - TODO: Make this based off of the voltage and amperage floats which have the calibration values applied to them
  float aBar = max(amperage / chargeMax * lcdSteps, 0);
  float vBar = max((voltage - voltStart) / (voltEnd - voltStart) * lcdSteps , 0);
  
  drawBar((byte)vBar, 1, 'V');
  drawBar((byte)aBar, 2, 'A');
  
  transferFastSample();
}

//Put a floating point number onto LCD in specified location with specified # decimal and whole digits
void clearPrint(float num, byte whole, byte decimal, byte col, byte line)
{
  char buf[12];
  byte width = min(8, 1 + whole + decimal);
  
  dtostrf(num, width, decimal, buf);
  
  lcd.setCursor(col, line);
  lcd.print(buf);
}

//Put a long int number onto LCD in specified location with specified # digits. Unused spaces filled with fillChar
void clearPrint(long num, byte whole, byte col, byte line, char fillChar)
{
  char buf[17];
  ltoa(num, buf, 10);
  byte shift = max(0, whole - strlen(buf));
  
  lcd.setCursor(col, line);
  for(byte i = 0; i < shift; i++)
  {
    lcd.print(fillChar);
  }
  lcd.print(buf);
}

//Put an unsigned long onto LCD in specified location with specified # digits. Unused spaces filled with fillChar
void clearPrint(unsigned long num, byte whole, byte col, byte line, char fillChar)
{
  char buf[17];
  ultoa(num, buf, 10);
  byte shift = max(0, whole - strlen(buf));
  
  lcd.setCursor(col, line);
  for(byte i = 0; i < shift; i++)
  {
    lcd.print(fillChar);
  }
  lcd.print(buf);
}

//Draws a vertical bar graph. Label = A B V T
void drawBar(byte value, byte col, char label)
{
  for(byte i = 0; i < 4; i++)
  {
    drawChar(value, col, i, label);
  }
}

//Draws a single character in a vertical bar graph
void drawChar(byte value, byte col, byte row, char label)
{
    byte height = value / 9;
    byte thisValue;
    byte *labelChar;
    
    //Drawing for a row entirely above the bar
    if (height < row)
    {
      lcd.setCursor(col+16, 3-row);
      if (row == 2)
      {
         lcd.print(label);
      }
      else
      {
        lcd.print(' ');
      }
      return;
    }
    
    thisValue = value - (9 * row);
    byte c[8];
    memset(c,0,sizeof(c));
     
    //Find what custom character array we may use on line 2
    if (label == 'V')
    {
      labelChar = vChar;
    }
    else if (label == 'A')
    {
      labelChar = aChar;
    }
    else if (label == 'B')
    {
      labelChar = bChar;
    }
    else
    {
      labelChar = tChar;
    }
    
    //Determine the custom character number (0-6)
    char charNum = 2 * (col - 1);
    //Determine if this the 'even' or 'odd' row
    if (row % 2)
    {
      charNum++;
    }
    
    //Drawing for a row entirely below the bar
    if (thisValue > 11)
    {
      if (row == 2)
      { 
        for (byte i = 0; i < 8; i++)
        {
          c[i] = ~labelChar[i];
        }
        lcd.createChar(charNum, c);
        lcd.setCursor(col+16, 3-row);
        lcd.print(charNum);
      }
      else
      {
        lcd.setCursor(col+16, 3-row);
        lcd.print((char)255);
      }
      return;
    }
    
    byte loops = min(thisValue, 8);
    
    for (byte i = 0; i < loops; i++)
    {
      byte width = min(thisValue-i, 5);
      byte shift = 5-width;
      byte line = ~(255 << width);
      c[7-i] = line << shift;
    }
    
    //invert character
    if (row == 2)
    {
      for (byte i = 0; i < 8; i++)
      {
        c[i] ^= labelChar[i];
      }
    }
    
    lcd.createChar(charNum, c);
    lcd.setCursor(col+16, 3-row);
    lcd.print(charNum);
}

void barTest()
{
  lcd.clear();
  while(true)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bounce b");
    for (byte i = 0; i < 45; i++)
    {
      for(byte j = 0; j < 4; j++)
      {
        drawChar(   i, 1, j, 'V');
        drawChar(45-i, 2, j, 'A');
        drawChar(45-i, 3, j, 'T');
      }
      lcd.setCursor(0, 1);
      lcd.print(i);
    }
  }
}

void setArrow()
{
  lcd.createChar(7, arrow);
}
