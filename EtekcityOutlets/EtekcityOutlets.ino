#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>

// Sniffer to obtain codes available at: https://codebender.cc/sketch:80286
#include <RCSwitch.h>

/*************************** Network Setup ***********************************/

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

/************************ RF Transmitter Setup ********************************/

// The physical Arduino PIN (this will be called with pinMode()). Change this according to your board layout
#define RC_PIN_TX 2
#define RC_PROTOCOL 1
#define RC_PULSE_LENGTH 176 // 'Delay', if you got the right codes and this isn't working, check that the delay/pulse length from the sniffer matches this
#define RC_BIT_LENGTH 24

// Array of ON/OFF codes. You must replace these with your codes obtained using a sniffer. This matches the 5 outlet remote.
unsigned long rc_codes[3][2] = {
  // ON     // OFF 
  {4478259, 4478268}, /* Outlet 1 */
  {4478403, 4478412}, /* Outlet 2 */
  {4478723, 4478732}, /* Outlet 3 */
};

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "RPG405"
#define AIO_KEY         "4491c6cfb1974fd6b0ee54c515187f05"

/************************* Adafruit.io Feeds *********************************/

EthernetClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//Adafruit_MQTT_Subscribe outlet1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/f/outlet-1");
//Adafruit_MQTT_Subscribe outlet2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/f/outlet-2");
Adafruit_MQTT_Subscribe outlet3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/f/outlet-3");

/*************************** Sketch Code ************************************/
RCSwitch sendSwitch = RCSwitch();

// What does this do??
#define halt(s) { Serial.println(F( s )); while(1);  }

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting EtekcityOutlets sketch");
  
  setupRfTransmitter();
  setupEthernet();
}

void setupEthernet() {
  Serial.println("Initializing Ethernet...");

  Ethernet.begin(mac);
  delay(1000); //give the ethernet a second to initialize
  //mqtt.subscribe(&outlet1);
  //mqtt.subscribe(&outlet2);
  mqtt.subscribe(&outlet3);

  Serial.println("Ethernet initialized!");
}

void setupRfTransmitter() {
  Serial.println("Initializing RF...");
  
  sendSwitch.enableTransmit(RC_PIN_TX);
  sendSwitch.setProtocol(RC_PROTOCOL); // defaults to 1 anyway
  sendSwitch.setPulseLength(RC_PULSE_LENGTH); // this is critical

  Serial.println("RF initialized!");
}

void enableOutlet(int outletNumber, bool onOrOff)
{
  if (outletNumber < 1 || outletNumber > 3)
  {
    Serial.println("Invalid outlet number");
    return;
  }
  
  unsigned long *onOffCodes = rc_codes[outletNumber - 1];
  unsigned long codeToSend = onOffCodes[onOrOff ? 0 : 1];
  sendSwitch.send(codeToSend, RC_BIT_LENGTH);
  
  char outletNumberString[1];
  int retVal = snprintf(outletNumberString, 1, "%d", outletNumber);
  if (retVal < 0)
  {
    Serial.println("Log encoding error");
    return;
  }
  
  if (onOrOff)
  {
    Serial.print("Enabling");
  }
  else
  {
    Serial.print("Disabling");
  }
  
  Serial.print(" outlet ");
  Serial.println(outletNumberString);
}

uint32_t x=0;

void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &outlet3) {
      if (strcmp((char *)outlet3.lastread, "ON") == 0) {
        enableOutlet(3, true);
      }
      if (strcmp((char *)outlet3.lastread, "OFF") == 0) {
        enableOutlet(3, false);
      }
    }
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
