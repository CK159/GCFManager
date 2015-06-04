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
    print("Available: commMode not set");
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
    print("Read: commMode not set");
    return 0;
  }
}

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

void print(const byte str, const byte format)
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

void println(const byte str, const byte format)
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
