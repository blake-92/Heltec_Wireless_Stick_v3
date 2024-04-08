/*
Placa: ESP32 Heltec Wireless Stick v3
Adafruit BME280 Library by Adafruit version 2.2.4
*/
/////////////////////////////////////////////////////////////////////////////////////
// BME280 Libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// OLED Libraries
#include "HT_SSD1306Wire.h"

/////////////////////////////////////////////////////////////////////////////////////
// BME280 Variables
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_DIR   0x76
#define I2C_SDA_2 7
#define I2C_SCL_2 6
#define I2C_BAUD  400000
// Data variables
float temperature_bme;
float humidity_bme;
float pressure_bme;
float altitude2_bme;

/////////////////////////////////////////////////////////////////////////////////////
// BME280 Objetcs
TwoWire I2CBME = TwoWire(1);
Adafruit_BME280 bme;
// OLED Object
SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);

/////////////////////////////////////////////////////////////////////////////////////
// OLED Functions
void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

/////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);

  // Initializaing OLED
  VextON();
  delay(100);
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_10);

  // Initializing BME280
  I2CBME.begin(I2C_SDA_2, I2C_SCL_2, I2C_BAUD);

  if (!bme.begin(BME_DIR, &I2CBME)) {
    Serial.println("BME280 Not found.");
    while (!bme.begin(BME_DIR, &I2CBME)) {
      Serial.println("Retrying...");
      delay(3000);
    }
  } else {
    Serial.println("BME280 OK.");
  }

}

/////////////////////////////////////////////////////////////////////////////////////
void loop() {
  
  // Data Sampling
  sampling();

  // Serial Output
  printData();

  // Display Output
  printDisplay();

  // Sampling time
  delay(10000);

}

/////////////////////////////////////////////////////////////////////////////////////
static void printData() {
  Serial.print("Temperature = ");
  Serial.print(temperature_bme);
  Serial.println(" *C");

  Serial.print("Humidity = ");
  Serial.print(humidity_bme);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(pressure_bme);
  Serial.println(" hPa");

  Serial.print("Altitude = ");
  Serial.print(altitude2_bme);
  Serial.println(" m");

  Serial.println();
}

/////////////////////////////////////////////////////////////////////////////////////
void printDisplay() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Tem:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 0, String(temperature_bme) + " C");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "Hum:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 10, String(humidity_bme) + " %");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 20, "hPa:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 20, String(pressure_bme));
  display.display();
}

/////////////////////////////////////////////////////////////////////////////////////
void sampling() {
  temperature_bme = bme.readTemperature();
  humidity_bme = bme.readHumidity();
  pressure_bme = bme.readPressure() / 100.0F;
  altitude2_bme = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

/////////////////////////////////////////////////////////////////////////////////////