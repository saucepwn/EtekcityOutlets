// Arduino/MicroView code to control Etekcity power outlets with the RCSwitch library
// Outlets: http://www.amazon.com/gp/product/B00DQELHBS/
// Generic 433 Module: http://www.amazon.com/gp/product/B00M2CUALS (if you search for the code on the board you can find it in many other retailers)
// MicroView Pinout Guide: http://kit.microview.io/pinout/
// RCSwitch: https://code.google.com/p/rc-switch/
// Raspberry PI solution:  http://timleland.com/wireless-power-outlets/

// Sniffer to obtain codes available at: https://codebender.cc/sketch:80286
#include <RCSwitch.h>

// Array of ON/OFF codes. You must replace these with your codes obtained using a sniffer. This matches the 5 outlet remote.
unsigned long rc_codes[3][2] = {
  // ON     //OFF 
  {4478259, 4478268}, /* Outlet 1 */
  {4478403, 4478412}, /* Outlet 2 */
  {4478723, 4478732}, /* Outlet 3 */
};

// The physical Arduino PIN (this will be called with pinMode()). Change this according to your board layout
#define RC_PIN_TX 2

//––//

#define RC_PROTOCOL 1
#define RC_PULSE_LENGTH 176 // 'Delay', if you got the right codes and this isn't working, check that the delay/pulse length from the sniffer matches this
#define RC_BIT_LENGTH 24

RCSwitch sendSwitch = RCSwitch();

void setup()
{
  Serial.begin(9600);
  sendSwitch.enableTransmit(RC_PIN_TX);
  sendSwitch.setProtocol(RC_PROTOCOL); // defaults to 1 anyway
  sendSwitch.setPulseLength(RC_PULSE_LENGTH); // this is critical
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

void loop()
{
  enableOutlet(3, true);
  delay(3000);
  enableOutlet(3, false);
  delay(3000);
}
