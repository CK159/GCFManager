void printTemps()
{
  print(F("t:"));
  for (byte i = 0; i < totalTemp; i++)
  {
    print(sensorMap[i]);
    print(':');
    print(fahrenheit[i]);
    if (totalTemp - i != 1)
    {
      print(' ');
    }
  }
  println();
}

boolean readTemp()
{
  //Check if any sensor is still processing
  //THIS DOESNT WORK QUITE RIGHT
  for (byte i = 0; i < totalTemp; i++)
  {
    ds.reset();
    ds.select(&addr[i*8]);
    
    ds.write_bit(1);
    delayMicroseconds(500);
    
    //If any sensor is still busy, return.
    //They should all take the same amount of time so this simplifies things
    if (ds.read_bit() == 0)
    {
      println(F("Duplicate temps"));
      return false;
    }
  }
  
  //Sensor not processing. Get the new value and restart
  //println(F("New Temps"));
  
  for (byte i = 0; i < totalTemp; i++)
  {
    byte data[12];
    //Tell sensor we want to read scratchpad
    ds.reset();
    ds.select(&addr[i*8]);
    ds.write(0xBE);
    
    //Receive data from sensor
    for (byte j = 0; j < 9; j++) { // we need 9 bytes (but reserved 12 bytes???) -- from code example
      data[j] = ds.read();
    }
    convertTemp(i, data); //(i) tells the function where to save the result
  }
  
  startNewConversion();
  
  return true;
}

void convertTemp(byte i, byte *data)
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
  println(F("Initializing Temp Sensors..."));
  
  byte currentSensor = 0;
  byte newAddr[8];
  
  while (ds.search(newAddr))
  {
    //Make sure maxTemp is not exceded
    if (totalTemp + 1 > maxTemp)
    {
      println(F("Maximum sensor count exceded. Ignoring all remaining devices"));
      continue;
    }
    //Print unique number for temp sensor
    print(F("Sensor "));
    print(currentSensor);
    print(F(": ROM ="));
    for(byte i = 0; i < 8; i++)
    {
      print(' ');
      print(newAddr[i], HEX);
    }
    //Verify crc for temp sensor
    if (OneWire::crc8(newAddr, 7) != newAddr[7]) {
      println(F("CRC is not valid!"));
      delay(2000);
      continue;
    }
    // Print temp sensor type
    boolean valid = false;
    switch (newAddr[0])
    {
      case 0x10:
        println(F(" Unsupported Chip=DS18S20"));  // or old DS1820
        break;
      case 0x28:
        println(F(" Chip = DS18B20"));
        valid = true;
        break;
      case 0x22:
        println(F(" Unsupported Chip=DS1822"));
        break;
      default:
        println(F("Device is not a DS18x20 family device."));
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
  
  print(F("Found "));
  print(totalTemp);
  println(F(" temp sensors."));
  
  for(byte i = 0; i < 8*maxTemp; i++)
  {
    print(addr[i], HEX);
    print(' ');
  }
  println();
  
  startNewConversion();
  //DOESNT WORK QUITE RIGHT - cant get the sensors to respond to status polling, wait fixed time instead
  /*do
  {
    println("t delay");
    delay(200);
  }
  while (!readTemp());*/
  delay(750);
  readTemp();
  
  print(F("Initial "));
  printTemps();
  println();
  return;
}
