#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "H_WSV3_RX"                         //change this
 
const char WIFI_SSID[] = "SRTC";               //change this
const char WIFI_PASSWORD[] = "Caza 12071992#";           //change this
const char AWS_IOT_ENDPOINT[] = "a4c4xb0ujqz9m-ats.iot.us-east-1.amazonaws.com";       //change this
 
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                               //change this
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
 
 
)KEY";
 
// Device Private Key                                               //change this
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
 
 
)KEY";
