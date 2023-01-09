#include "LedControl.h"

// CODE BY InterLinkKnight, VISIT HIS YOUTUBE FOR THE ORIGINAL CODE.

LedControl lc=LedControl(12,11,10,1);

const byte PulsesPerRevolution = 4; 
const unsigned long ZeroTimeout = 500000;  
const byte numReadings = 20;  
volatile unsigned long LastTimeWeMeasured; 
volatile unsigned long PeriodBetweenPulses = ZeroTimeout+1000;  
volatile unsigned long PeriodAverage = ZeroTimeout+1000;
unsigned long FrequencyRaw;  
unsigned long FrequencyReal; 
unsigned long RPM;  
unsigned long KPH; 
unsigned int PulseCounter = 1; 
unsigned long PeriodSum; 
unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;  
unsigned long CurrentMicros = micros(); 
unsigned int AmountOfReadings = 1;
unsigned int ZeroDebouncingExtra;  
unsigned long readings[numReadings];  
unsigned long readIndex;  
unsigned long total;  
unsigned long average; 

void setup()  // Start of setup:
{
  Serial.begin(9600);  
  attachInterrupt(digitalPinToInterrupt(2), Pulse_Event, RISING); 
  delay(1000); 
  lc.shutdown(0,false);
  lc.setIntensity(0,4);
  lc.clearDisplay(0);
} 

void loop()  
{

  LastTimeCycleMeasure = LastTimeWeMeasured;  // Store the LastTimeWeMeasured in a variable.
  CurrentMicros = micros();  // Store the micros() in a variable.
  if(CurrentMicros < LastTimeCycleMeasure)
  {
    LastTimeCycleMeasure = CurrentMicros;
  }


  FrequencyRaw = 10000000000 / PeriodAverage;  // Calculate the frequency using the period between pulses.
  
  if(PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra)
  {  
    FrequencyRaw = 0;  // Set frequency as 0.
    ZeroDebouncingExtra = 2000;  // Change the threshold a little so it doesn't bounce.
  }
  else
  {
    ZeroDebouncingExtra = 0;  // Reset the threshold to the normal value so it doesn't bounce.
  }

  FrequencyReal = FrequencyRaw / 10000;
  RPM = FrequencyRaw / PulsesPerRevolution * 60;                                           
  RPM = (RPM / 10000); 
  KPH = ((RPM*1.550*60)/1000); // calculates rpm into kph. fine tune the speed by adjusting the second variable (1.550)





  // Smoothing RPM:
  total = total - readings[readIndex];  // Advance to the next position in the array.
  readings[readIndex] = RPM;  // Takes the value that we are going to smooth.
  total = total + readings[readIndex];  // Add the reading to the total.
  readIndex = readIndex + 1;  // Advance to the next position in the array.

  if (readIndex >= numReadings)  // If we're at the end of the array:
  {
    readIndex = 0;  // Reset array index.
  }
  average = total / numReadings;  // The average value it's the smoothed result.
  Serial.print("Period: ");
  Serial.print(PeriodBetweenPulses);
  Serial.print("\tReadings: ");
  Serial.print(AmountOfReadings);
  Serial.print("\tKPH: ");
  Serial.print(KPH);
  Serial.print("\tRPM: ");
  Serial.print(RPM);
  Serial.print("\tTachometer: ");
  Serial.println(average);

  if (KPH < 100) lc.setChar(0,0,0x13,false);
  else lc.setChar(0,0,(((KPH/100))%10),false);

  if (KPH < 10) lc.setChar(0,1,0x13,false);
  else lc.setChar(0,1,(((KPH/10))%10),false);
  
  lc.setChar(0,2,(((KPH/1))%10),false);
  
 
  

}  // End of loop.l




 
void Pulse_Event()  // The interrupt runs this to calculate the period between pulses:
{

  PeriodBetweenPulses = micros() - LastTimeWeMeasured;
  LastTimeWeMeasured = micros(); 
  if(PulseCounter >= AmountOfReadings)  // If counter for amount of readings reach the set limit:
  {
    PeriodAverage = PeriodSum / AmountOfReadings;  // Calculate the final period dividing the sum of all readings by the
                                                   // amount of readings to get the average.
    PulseCounter = 1;  // Reset the counter to start over. The reset value is 1 because its the minimum setting allowed (1 reading).
    PeriodSum = PeriodBetweenPulses;
    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10); 
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);  // Constrain the value so it doesn't go below or above the limits.
    AmountOfReadings = RemapedAmountOfReadings;  // Set amount of readings as the remaped value.
  }
  else
  {
    PulseCounter++;  // Increase the counter for amount of readings by 1.
    PeriodSum = PeriodSum + PeriodBetweenPulses;  // Add the periods so later we can average.
  }

}  // End of Pulse_Event.
