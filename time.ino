//http://forum.arduino.cc/index.php?topic=42211.0
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)

#define numberOfMinutesNoHours(_time_) ((_time_ / SECS_PER_MIN))
#define numberOfHoursNoDays(_time_) (( _time_) / SECS_PER_HOUR)

void printTime(unsigned long time, byte col, byte line)
{
  long minutes = numberOfMinutesNoHours(time);
  long seconds = numberOfSeconds(time);
  
  clearPrint(minutes, 4, col, line, ' ');
  lcd.setCursor(col+4, line);
  lcd.print(':');
  clearPrint(seconds, 2, col+5, line, '0');
}
