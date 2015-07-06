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
