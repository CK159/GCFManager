int available()
{
  if (commMode == USB)
  {
    return Serial.available();
  }
  else if (commMode == BLUETOOTH)
  {
    return Serial1.available();
  }
  else
  {
    //print(F("Available: commMode not set"));
    return 0;
  }
}

int read()
{
  if (commMode == USB)
  {
    return Serial.read();
  }
  else if (commMode == BLUETOOTH)
  {
    return Serial1.read();
  }
  else
  {
    //print(F("Read: commMode not set"));
    return 0;
  }
}

//Print F() macro
void print(const __FlashStringHelper* str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const __FlashStringHelper* str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}

//Print just blank line
void println()
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println();
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println();
  }
}

//Print char string
void print(const char *str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const char *str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}

//Print byte, default format
void print(const byte str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const byte str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}

//Print formatted byte
void print(const byte str, const int format)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str, format);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str, format);
  }
}

void println(const byte str, const int format)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str, format);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str, format);
  }
}

//Print integer
void print(const int str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const int str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}

//Print unsigned integer
void print(const unsigned int str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const unsigned int str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}

//Print one character
void print(const char str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const char str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}


//Print float
void print(const float str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const float str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}

//Print float with precision specification
void print(const float str, const int precision)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str, precision);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str, precision);
  }
}

void println(const float str, const int precision)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str, precision);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str, precision);
  }
}

//Print long
void print(const unsigned long str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.print(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.print(str);
  }
}

void println(const unsigned long str)
{
  if (commMode == USB || commMode == UNSET)
  {
    Serial.println(str);
  }
  if (commMode == BLUETOOTH || commMode == UNSET)
  {
    Serial1.println(str);
  }
}
