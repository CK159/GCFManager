//Errors: X: Max charge time exceeded, x: Min charge time not met
//Y: Max discharge time exceeded, y: Min discharge time not met
//C: Max cell voltage exceeded, c: Min cell voltage below limit
//V: Max total voltage exceeded, v: Min total voltage below limit
//T: Battery delta ambient temp exceeded, t: Battery absolute temp exceeded
//A: Analog board voltage error, ADC Shutdown
//B: An unexpected beep sequence was detected (detected while not charging)
//L: Cycle limit not set before beginning a charge


//What to do: 2 types of events/situations:
//Constant task run continuously
//Beginning and ending events when transitioning between states
void stateManager(float avgf[6])
{
  boolean sequenceDetected = processBeep();
  if (state != M_WAIT)
  {
    if (state != M_ERROR && state != M_DONE)
    {
      tempMon();
      voltMon(avgf);
      if (sequenceDetected && state != M_CHG)
      {
        setError('B');
      }
    }
  }

  if (state == M_CHG)
  {
    if (sequenceDetected)
    {
      changeState(M_CHGW);
    }
    //Charge timeout
    if (millis() > timerStart + maxChg)
    {
      setError('X');
    }
  }
  else if (state == M_CHGW)
  {
    chargeWait(M_CHGW);
  }
  else if (state == M_DSC)
  {
    checkEndDischarge(avgf);
    //Discharge timeout
    if (millis() > timerStart + maxDsc)
    {
      setError('Y');
    }
  }
  else if (state == M_DSCW)
  {
    chargeWait(M_DSCW);
  }
}

//Runs beginning and ending tasks when changing states
void changeState(byte newState)
{
  if (newState == M_CHG)
  {
    if (cycleLimit <= 0)
    {
      setError('L');
    }
    else
    {
      if (cycle >= cycleLimit)
      {
        //Completed all cycles
        //Do this to avoid recursive call, normally wouldnt return early in this function
        state = M_DONE;
        timerStart = millis();
        return;
      }
      
      //New Cycle
      cycle++;
      startBtn();
    }
  }
  else if (newState == M_CHGW || newState == M_DSCW)
  {
    waitStart = millis();
  }
  else if (M_DSC)
  {
    //Set power
  }
  
  timerStart = millis();
  state = newState;
}

void checkEndCharge(float avgf[6])
{
  /*if (avgf[C1] < cutoffCell || avgf[C2] < cutoffCell || avgf[C3] < cutoffCell)
  {
    changeState(M_DSCW);
  }*/
}

void checkEndDischarge(float avgf[6])
{
  if (avgf[C1] < cutoffCell || avgf[C2] < cutoffCell || avgf[C3] < cutoffCell)
  {
    changeState(M_DSCW);
  }
}

void chargeWait(byte newState)
{
  //Wait time is over
  if (millis() > waitStart + waitTime)
  {
    waitStart = 0;
    if (newState == M_CHGW)
    {
      changeState(M_DSC);
    }
    else
    {
      changeState(M_CHG);
    }
  }
}

//Print state-related stuff to screen (mostly voltage bounce back)
void printState(float avgf[6])
{
  
}

boolean processBeep()
{
  boolean sequenceDetected = false;
  //Detect if a valid beep sequence has been detected
  if (pendingBeep >= beepCount)
  {
    println("BEEP:");
    println(pendingBeep);
    
    pendingBeep = 0;
    beepWindowStart = 0;
    sequenceDetected = true;
  }

  //Reset beep sequence after timing window expires
  if (beepWindowStart + beepWindow < millis())
  {
    beepWindowStart = 0;
    pendingBeep = 0;
    println("BRS:");
  }
  return sequenceDetected;
}

void setError(char code)
{
  state = M_ERROR;
  error = code;
  stopBtn();
  //TODO: Shutdown discharger and charger
  setPower(0);
}

//Ensure safe battery temperature
void tempMon()
{
  float maxBatt = max(fahrenheit[BATT1], fahrenheit[BATT2]);
  float delta = maxBatt - fahrenheit[AMBIENT];

  if (delta > maxDeltaTemp)
  {
    setError('T');
  }
  if (maxBatt > maxTemp)
  {
    setError('t');
  }
}

void voltMon(float avgf[6])
{
  //Cell min voltages
  if (avgf[C1] < minCell || avgf[C2] < minCell || avgf[C3] < minCell)
  {
    setError('c');
  }
  //Cell max voltages
  else if (avgf[C1] > maxCell || avgf[C2] > maxCell || avgf[C3] > maxCell)
  {
    setError('C');
  }
  //Min total voltage 
  else if (avgf[TOT] < minCell * 3)
  {
    setError('v');
  }
  //Max total voltage 
  else if (avgf[TOT] > maxCell * 3)
  {
    setError('V');
  }
}

