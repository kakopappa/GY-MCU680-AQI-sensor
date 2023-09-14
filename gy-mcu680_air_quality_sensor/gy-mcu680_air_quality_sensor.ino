//GY_MCU680 air quality sensor for esp8266
 
#include "gy_mcu680_uart.h"

// Optional - you can define air pressure correction for your altitude
#define ALT_CORRECTION 3144

GY_MCU680 gymcu;

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  
  gymcu.begin(D6, D5); // RX, TX
}

void loop(){
  reading_t r = gymcu.read();
  delay(1000); 

  Serial.print("Temperature:");
  Serial.print(r.temperature); 
  Serial.print(", humidity:"); 
  Serial.print(r.humidity); 
  Serial.print(", pressure:"); 
  Serial.print(r.pressure);
  Serial.print("|");

  float altPressure = (float(r.pressure) + ALT_CORRECTION) / 100;
  Serial.print(altPressure);      
  Serial.print(", IAQ:");
  Serial.print(r.IAQ);  //  (0-500);
  Serial.print(", gas:");  // gas resistance [Ω]
  Serial.print(r.gas); 
  Serial.print(", altitude:"); 
  Serial.print(r.altitude);                       
  Serial.print(", IAQ_accuracy:"); 
  Serial.println(r.IAQ_accuracy); 

  /*   
    IAQ_accuracy =0 could either mean:
    BSEC was just started, and the sensor is stabilizing (this lasts normally 5min in LP mode or 20min in ULP mode),
    there was a timing violation (i.e. BSEC was called too early or too late), which should be indicated by a warning/error flag by BSEC,
    IAQ_accuracy =1 means the background history of BSEC is uncertain. This typically means the gas sensor data was too stable for BSEC to clearly define its references,
    IAQ_accuracy =2 means BSEC found a new calibration data and is currently calibrating,
    IAQ_accuracy =3 means BSEC calibrated successfully.
  */ 

  if(r.IAQ_accuracy == 3) {
    String airquality = gymcu.IAQToText(r.IAQ);
    Serial.print("Air quality:"); 
    Serial.println(airquality); 
  }

  if(r.IAQ > 150) {
    badAirQLeds();
  }
} 

void badAirQLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
