#pragma once
#include <SoftwareSerial.h>

typedef struct {
  float temperature;
  float humidity;
  uint32_t gas;
  uint32_t pressure;
  uint16_t IAQ;
  int16_t altitude;
  uint8_t IAQ_accuracy;
} reading_t;

class GY_MCU680 {
public:
  void begin(int rx, int tx);
  reading_t read();
  String IAQToText(int iaq);

private:
  reading_t _read();
  SoftwareSerial *mySerial_ = nullptr;
  #define mySerial (*mySerial_)
  unsigned char Re_buf[30], counter = 0;
  unsigned char sign = 0;
  uint16_t temp1 = 0;
  int16_t temp2 = 0;
};

void GY_MCU680::begin(int rx, int tx) {
  Serial.printf("[GY_MCU680::begin] : Setup GY MCU680 sensor..\r\n");

  mySerial_ = new SoftwareSerial(rx, tx);

  mySerial.begin(9600);
  mySerial.listen();

  delay(5000);

  mySerial.write(0XA5);
  mySerial.write(0X55);
  mySerial.write(0X3F);
  mySerial.write(0X39);

  delay(100);

  mySerial.write(0XA5);
  mySerial.write(0X56);
  mySerial.write(0X02);
  mySerial.write(0XFD);

  Serial.printf("[GY_MCU680::begin] : Setup completed\r\n");
  delay(100);
}

reading_t GY_MCU680::_read() {
  float temperature = 0, humidity = 0;
  unsigned char i = 0, sum = 0;
  uint32_t gas = 0;
  uint32_t pressure = 0;
  uint16_t IAQ = 0;
  int16_t altitude = 0;
  uint8_t IAQ_accuracy = 0;

  while (mySerial.available()) {
    Re_buf[counter] = (unsigned char)mySerial.read();
    if (counter == 0 && Re_buf[0] != 0x5A) {
      Serial.printf("[GY_MCU680::_read] : Nothing received\r\n");      
    }

    if (counter == 1 && Re_buf[1] != 0x5A) {
      counter = 0;
      return { temperature, humidity, gas, pressure, IAQ, altitude, IAQ_accuracy };
    };

    counter++;

    if (counter == 20) {
      counter = 0;
      sign = 1;
    }
  }

  if (sign) {
    sign = 0;

    if (Re_buf[0] == 0x5A && Re_buf[1] == 0x5A) {      
      for (i = 0; i < 19; i++)
        sum += Re_buf[i];

      if (sum == Re_buf[i]) {
        temp2 = (Re_buf[4] << 8 | Re_buf[5]);
        temperature = (float)temp2 / 100;
        temp1 = (Re_buf[6] << 8 | Re_buf[7]);
        humidity = (float)temp1 / 100;
        uint32_t pressure = ((uint32_t)Re_buf[8] << 16) | ((uint16_t)Re_buf[9] << 8) | Re_buf[10];
        IAQ_accuracy = (Re_buf[11] & 0xf0) >> 4;
        IAQ = ((Re_buf[11] & 0x0F) << 8) | Re_buf[12];
        gas = ((uint32_t)Re_buf[13] << 24) | ((uint32_t)Re_buf[14] << 16) | ((uint16_t)Re_buf[15] << 8) | Re_buf[16];
        altitude = (Re_buf[17] << 8) | Re_buf[18];

        Serial.print("temperature:");
        Serial.print(temperature);
        Serial.print(", humidity:");
        Serial.print(humidity);
        Serial.print(", pressure:");
        Serial.print(pressure);
        Serial.print(", IAQ:");
        Serial.print(IAQ);
        Serial.print(", gas:");
        Serial.print(gas);
        Serial.print(", altitude:");
        Serial.print(altitude);
        Serial.print("  ,IAQ_accuracy:");
        Serial.println(IAQ_accuracy);
      }       
    }
  }

  return { temperature, humidity, gas, pressure, IAQ, altitude, IAQ_accuracy };
}

reading_t GY_MCU680::read() {
  long unsigned int max_wait_time = 5000;
  const auto t0 = millis();

  // because sometimes get bad readings.
  for (; (millis() - t0) < max_wait_time; delay(1000)) {
    reading_t r = _read();
    if (r.temperature > 0) {
      return r;
    }
  }

  return { 0, 0, 0, 0, 0, 0, 0 };
}

String GY_MCU680::IAQToText(int iaq) {
  String disp;

  if(iaq <= 50) disp="GOOD";
  else if(iaq > 50 && iaq <= 100) disp = "AVER";
  else if(iaq > 100 && iaq <= 150) disp = "L.BAD";
  else if(iaq > 150 && iaq <= 200) disp = "BAD";
  else if(iaq > 200 && iaq <= 300) disp = "WORSE";
  else disp = "V.BAD";   

  return disp;
}