/*
Placa: ESP32 Heltec Wireless Stick v3
*/
/////////////////////////////////////////////////////////////////////////////////////
// AWS Credentials
#include "secrets.h"
// WIFI Library
#include <WiFiClientSecure.h>
#include "WiFi.h"
// MQTT Library
#include <PubSubClient.h>
// JSON Library
#include <ArduinoJson.h>
// LORA Library
#include "LoRaWan_APP.h"
#include "Arduino.h"
// OLED Library
#include "HT_SSD1306Wire.h"

/////////////////////////////////////////////////////////////////////////////////////
// Lora Variables
#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

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

static RadioEvents_t RadioEvents;

int16_t rssiVal,rxSize,snrVal;

bool lora_idle = true;

// AWS Variables
#define AWS_IOT_PUBLISH_TOPIC   "hwsv3/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "hwsv3/sub"

/////////////////////////////////////////////////////////////////////////////////////
// JSON Object
JsonDocument doc;
// WIFI Object
WiFiClientSecure net = WiFiClientSecure();
// MQTT Object
PubSubClient client(net);
// OLED Objects
SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);

/////////////////////////////////////////////////////////////////////////////////////
// OLED Functions
void VextON(void) {
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

// WIFI-MQTT-AWS Function
void connectAWS() {
  // Initializing WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "WIFI:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 0, "OK");
  display.display();

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);

  // Initializing AWS IoT Core
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 10, "AWS :");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(64, 10, "FAIL");
    display.display();
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "AWS :");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 10, "OK");
  display.display();
}

// MQTT Callback
void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

/////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Mcu.begin();

  // Initializing OLED
  VextON();
  delay(100);
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_10);

  // Initializing AWS
  connectAWS();

  // Initializing Lora  
  RadioEvents.RxDone = OnRxDone;
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                             LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                             LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                             0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 20, "LORA:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 20, "OK");
  display.display();
  delay(1000);
}

/////////////////////////////////////////////////////////////////////////////////////
void loop() {
  client.loop();
  if(lora_idle)
  {
    lora_idle = false;
    Serial.println("into RX mode");
    Radio.Rx(0);
  }
  Radio.IrqProcess( );
}

/////////////////////////////////////////////////////////////////////////////////////s
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ) {
  rssiVal=rssi;
  rxSize=size;
  snrVal=snr;
  memcpy(rxpacket, payload, size );
  rxpacket[size]='\0';
  Radio.Sleep( );

  Serial.printf("\r\nreceived packet \"%s\" with rssi %d, length %d, snr %d\r\n", rxpacket, rssiVal, rxSize, snrVal);

  // Display Output
  printDisplay();

  client.publish(AWS_IOT_PUBLISH_TOPIC, rxpacket);
    
  lora_idle = true;
}

/////////////////////////////////////////////////////////////////////////////////////
void printDisplay() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "RSSI:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 0, String(rssiVal));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "SIZE:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 10, String(rxSize));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 20, "SNR :");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 20, String(snrVal));
  display.display();
}

/////////////////////////////////////////////////////////////////////////////////////