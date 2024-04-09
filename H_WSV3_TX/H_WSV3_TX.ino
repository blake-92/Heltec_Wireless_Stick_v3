/*
Placa: ESP32 Heltec Wireless Stick v3
ArduinoJson by Benoit Blanchon version 7.0.4
Adafruit BME280 Library by Adafruit version 2.2.4
*/
/////////////////////////////////////////////////////////////////////////////////////
// LORA Library
#include "LoRaWan_APP.h"
#include "Arduino.h"
// JSON Library
#include <ArduinoJson.h>
// BME280 Libraaries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// OLED Libraries
#include "HT_SSD1306Wire.h"

/////////////////////////////////////////////////////////////////////////////////////
// Lora Variables
#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             5        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 128 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

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
// LORA Object
static RadioEvents_t RadioEvents;
// JSON Object
JsonDocument doc;
// BME280 Objects
TwoWire I2CBME = TwoWire(1);
Adafruit_BME280 bme;
// OLED Object
SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);

/////////////////////////////////////////////////////////////////////////////////////
// LORA Functions
void OnTxDone( void );
void OnTxTimeout( void );
// OLED Functions
void VextON(void) {
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

/////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Mcu.begin();
	
  txNumber=0;

  // LED
  pinMode(35, OUTPUT);
  digitalWrite(35, LOW);

  // Initializing OLED
  VextON();
  delay(100);
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_10);

  // Initializing LORA
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
    
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "LORA:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 0, "OK");
  display.display();

  // Initializing BME280
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "BME :");
  
  I2CBME.begin(I2C_SDA_2, I2C_SCL_2, I2C_BAUD);

  if (!bme.begin(BME_DIR, &I2CBME)) {
    Serial.println("BME280 Not found.");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(64, 10, "FAIL");
    display.display();
    while (1);
  } else {
    Serial.println("BME280 OK.");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(64, 10, "OK");
    display.display();
  }

  // Wait message
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 20, "Wait....");
  display.display();

}

/////////////////////////////////////////////////////////////////////////////////////
void loop() {
	if(lora_idle == true)
	{
    smartDelay(60000);

    txNumber += 0.01;

    // Data Sampling
    sampling();

    // Serial Output
    printData();

    // Display Output
    printDisplay();
    
    // Prepare txpacket
    publishData();
   
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

    // Transmit the packet
		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
    lora_idle = false;

    // LED Indicator
    digitalWrite(35, HIGH);
    smartDelay(500);
    digitalWrite(35, LOW);
    smartDelay(500);


	}
  Radio.IrqProcess( );
}

/////////////////////////////////////////////////////////////////////////////////////
void OnTxDone( void ) {
	Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void ) {
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}

/////////////////////////////////////////////////////////////////////////////////////
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms);
}

/////////////////////////////////////////////////////////////////////////////////////
static void publishData() {
  char txNumberStr[8];
  char temperatureStr[8];
  char humidityStr[8];
  char pressureStr[8];

  snprintf(txNumberStr, sizeof(txNumberStr), "%.2f", txNumber);
  snprintf(temperatureStr, sizeof(temperatureStr), "%.2f", temperature_bme);
  snprintf(humidityStr, sizeof(humidityStr), "%.2f", humidity_bme);
  snprintf(pressureStr, sizeof(pressureStr), "%.2f", pressure_bme);

  doc["i"] = "vn0000";        // ID
  doc["c"] = txNumberStr;     // Counter
  doc["t"] = temperatureStr;  // Temperature
  doc["h"] = humidityStr;     // Humidity
  doc["p"] = pressureStr;     // Pressure

  serializeJson(doc, txpacket);
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