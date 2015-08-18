void setup()
{
  //IMPORTANT: Pullup on Serial1 RX ON!
  // Allows disconnection of Bluetooth module from Arduino without floating serial input pins causing garbage input
  pinMode(0, INPUT);
  digitalWrite(0, HIGH);
  
  pinMode(ledpin,   OUTPUT);
  pinMode(avLED,    OUTPUT);
  pinMode(btState,  INPUT);
  pinMode(avPin,    INPUT);
  pinMode(selPin,   OUTPUT);
  pinMode(enterPin, OUTPUT);
  pinMode(stopPin,  OUTPUT);
  pinMode(beepPin,  INPUT);
  
  digitalWrite(ledpin, LOW);
  digitalWrite(avLED, LOW);
  digitalWrite(selPin, HIGH);
  digitalWrite(enterPin, LOW);
  digitalWrite(beepPin, LOW);
  
  for (byte i = 0; i < totalChanels; ++i)
  {
    pinMode(chanelPins[i], OUTPUT);
    digitalWrite(chanelPins[i], LOW);
  }
  
  memset(sample, 0, sizeof(sample));
  memset(fastSample, 0, sizeof(fastSample));
  
  Serial.begin(9600);
  Serial1.begin(9600);
  
  lcd.begin(20,4);
  
  establishContact();
  
  enableSPI();
  avCheck();
  //barTest();
  
  lcd.clear();
  initTemp();
  initializeCalibration();
  attachInterrupt(4, beepInterrupt, CHANGE);
  
  nextLoop = millis(); //Start counting loops from now
  
  timerStart = millis(); //TODO: Make work better
  
  println(F("Entering main loop."));
  println();
}

void establishContact() {
  /*lcd.setCursor(1, 0);
  lcd.print("GottaChargeFast.com");
  lcd.setCursor(1, 1);
  lcd.print("Press 'A' on");
  lcd.setCursor(1, 2);
  lcd.print("USB or");
  lcd.setCursor(1, 3);
  lcd.print("Bluetooth");*/
  
  //Clear buffers of anything that may have gotten in there from startup
  while (Serial.available()){Serial.read();}
  while (Serial1.available()){Serial1.read();}
  
  //boolean flash = false;
  //setArrow();
  
  while (true)
  {   
    println(F("Press A to initialize"));
    digitalWrite(ledpin, !digitalRead(ledpin));
    
    /*for (byte i = 0; i < 2; i++)
    {
      char arrowChar = 126;
      if (flash)
      {
        arrowChar = 7;
      }
      if (digitalRead(btState))
      {
        lcd.setCursor(0, 2);
        lcd.print(' ');
        lcd.setCursor(0, 3);
        lcd.print(arrowChar);
      }
      else
      {
        lcd.setCursor(0, 3);
        lcd.print(' ');
        lcd.setCursor(0, 2);
        lcd.print(arrowChar);
      }
      flash = !flash;
      
      safeDelay(750);
    }*/
    delay(1500);
    
    while (Serial.available())
    {
      int data = Serial.read();
      
      if (data == 'A' || data == 'a')
      {
        commMode = USB;
        clearBuffer();
        println(F("GottaChargeFast.com Initializing on USB..."));
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
        println(F("GottaChargeFast.com Initializing on Bluetooth..."));
        return;
      }
    }
  }
}
