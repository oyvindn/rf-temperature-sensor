#include <Arduino.h>
#include <LowPower.h>

#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>

#define USE_BME280 true
#if USE_BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#else
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#include "arduino_secrets.h"

#define DS18B20_TEMPERATURE_SENSOR_PIN 9

#define RF_INTERRUPT_PIN 2
#define RF_FREQUENCY 433.00
#define RF_GATEWAY_ADDRESS 1
#define RF_NODE_ADDRESS 2

#define SLEEP_CYCLES 38 // times 8 seconds power down

RH_RF69 rf69Driver(PIN_SPI_SS, RF_INTERRUPT_PIN);
RHReliableDatagram radioManager(rf69Driver, RF_NODE_ADDRESS);

#if USE_BME280
Adafruit_BME280 bme;
#else
OneWire oneWire(DS18B20_TEMPERATURE_SENSOR_PIN);
DallasTemperature temperatureSensors(&oneWire);
#endif

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  initRadio();

#if USE_BME280
  initBME280();
#else
  temperatureSensors.begin();
#endif
}

void loop()
{
  char message[50];

  long vcc = readVcc();

#if USE_BME280
  bme.takeForcedMeasurement();
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  char temperatureStr[10];
  dtostrf(temperature, 1, 2, temperatureStr);

  char humidityStr[10];
  dtostrf(humidity, 1, 0, humidityStr);

  char pressureStr[10];
  dtostrf(pressure, 1, 0, pressureStr);

  sprintf(message, "THPV;%s;%s;%s;%d", temperatureStr, humidityStr, pressureStr, vcc);
#else
  float temperature = readDS18B20Temperature();
  char temperatureStr[10];
  dtostrf(temperature, 1, 2, temperatureStr);
  sprintf(message, "TV;%s;%d", temperatureStr, vcc);
#endif

  radioManager.sendtoWait((uint8_t *)message, sizeof(message), RF_GATEWAY_ADDRESS);
  powerDown(SLEEP_CYCLES);
}

#if !USE_BME280
float readDS18B20Temperature()
{
  temperatureSensors.requestTemperatures();
  float temp = temperatureSensors.getTempCByIndex(0);
  return temp;
}
#endif

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

void powerDown(int times)
{
  rf69Driver.sleep();

  for (int i = 0; i < times; i++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void initRadio()
{
  if (radioManager.init())
  {
    Serial.println("[RFM69] Init success");
  }
  else
  {
    Serial.println("[RFM69] Init failed");
  }
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  if (rf69Driver.setFrequency(RF_FREQUENCY))
  {
    Serial.println("[RFM69] Frequency set to 433MHz");
  }
  else
  {
    Serial.println("[RFM69] Failed to set frequency");
  }

  uint8_t encryptionKey[] = SECRET_RF_ENCRYPTION_KEY;
  rf69Driver.setEncryptionKey(encryptionKey);
}

#if USE_BME280
void initBME280()
{
  if (!bme.begin(0x76))
  {
    Serial.println("[BME280] No BME280 device found!");
  }
  else
  {
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF);
  }
}
#endif
