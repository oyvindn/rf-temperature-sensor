#include <Arduino.h>
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LowPower.h>

#include "arduino_secrets.h"

#define DS18B20_TEMPERATURE_SENSOR_GPIO 9

#define RF_INTERRUPT_GPIO 2
#define RF_FREQUENCY 433.00
#define RF_GATEWAY_ADDRESS 1
#define RF_NODE_ADDRESS 2

#define SLEEP_CYCLES 38 // times 8 seconds power down

RH_RF69 rf69Driver(SS, RF_INTERRUPT_GPIO);
RHReliableDatagram radioManager(rf69Driver, RF_NODE_ADDRESS);

OneWire oneWire(DS18B20_TEMPERATURE_SENSOR_GPIO);
DallasTemperature temperatureSensors(&oneWire);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  initRadio();
  temperatureSensors.begin();
}

void loop()
{
  float temperature = readTemperature();
  long vcc = readVcc();
  transmit(temperature, vcc);
  powerDown(SLEEP_CYCLES);
}

float readTemperature()
{
  temperatureSensors.requestTemperatures();
  float temp = temperatureSensors.getTempCByIndex(0);
  return temp;
}

long readVcc()
{
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  delay(2);            // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC))
    ; // measuring

  uint8_t low = ADCL;  // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  // Vbat = 1100mV * 1024 / ADC value
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;              // Vcc in millivolts
}

void transmit(float temperature, long vcc)
{
  char tempStr[10];
  dtostrf(temperature, 1, 2, tempStr);
  char message[40];
  sprintf(message, "%s;%d", tempStr, vcc);
  Serial.print("Sending message: ");
  Serial.println(message);
  if (!radioManager.sendtoWait((uint8_t *)message, sizeof(message), RF_GATEWAY_ADDRESS))
  {
    Serial.println("Transmission failed");
  }
}

void powerDown(int times)
{
  rf69Driver.sleep();

  for (int i = 0; i < times; i++)
  {
    Serial.print("Powering down in 8s ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.println(times);
    Serial.flush();
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void initRadio()
{
  if (radioManager.init())
  {
    Serial.println("RFM69 - Init success");
  }
  else
  {
    Serial.println("RFM69 - Init failed");
  }
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  if (rf69Driver.setFrequency(RF_FREQUENCY))
  {
    Serial.println("RFM69 - Frequency set to 433MHz");
  }
  else
  {
    Serial.println("RFM69 - Failed to set frequency");
  }

  uint8_t encryptionKey[] = SECRET_RF_ENCRYPTION_KEY;
  rf69Driver.setEncryptionKey(encryptionKey);
}
