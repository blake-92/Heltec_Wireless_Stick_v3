/*
Placa: ESP32 Heltec Wireless Stick v3
ArduinoJson version 7.0.4
*/
/////////////////////////////////////////////////////////////////////////////////////
// Lora Library
#include "LoRaWan_APP.h"
#include "Arduino.h"
// JSON Library
#include <ArduinoJson.h>

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
#define BUFFER_SIZE                                 64 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

/////////////////////////////////////////////////////////////////////////////////////
// LORA Object
static RadioEvents_t RadioEvents;
// JSON Object
JsonDocument doc;

/////////////////////////////////////////////////////////////////////////////////////
// LORA Functions
void OnTxDone( void );
void OnTxTimeout( void );

/////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Mcu.begin();
	
  txNumber=0;

  // Initializing Lora
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
    
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
}

/////////////////////////////////////////////////////////////////////////////////////
void loop()
{
	if(lora_idle == true)
	{
    delay(10000);

		txNumber += 0.01;

    // Prepare txpacket
    publishData();
   
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

    // Transmit the packet
		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    lora_idle = false;

	}
  Radio.IrqProcess( );
}

/////////////////////////////////////////////////////////////////////////////////////
void OnTxDone( void )
{
	Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}

/////////////////////////////////////////////////////////////////////////////////////
static void publishData() {
  doc["id"] = "vn0001";
  doc["data"] = txNumber;

  serializeJson(doc, txpacket);

  size_t jsonSize = measureJson(doc);
  Serial.println("JSON size: ");
  Serial.println(jsonSize);
  Serial.println(txpacket);
 
}

/////////////////////////////////////////////////////////////////////////////////////