#include <BleConnectionStatus.h>
#include <BleKeyboard.h>
#include <BluetoothSerial.h>
#include <KeyboardOutputCallbacks.h>
#include "driver/rtc_io.h"
BleKeyboard bleKeyboard("Justin's Numbers");

#include <Keypad.h>
#include <Key.h>

const byte rows=5; // Rows in keyboard matrix
const byte cols=5; // Columns in keyboard matrix
const float deepSleepMin = 5; // Minutes before automatic deep sleep
long deepSleepMS = deepSleepMin*60*1000; // Converts minutes to milliseconds
long kptime = millis(); // stores time since last keypress
long btime = 0; // stores time since sleep button press
const int bwait = 1500; // milliseconds sleep buttom must be held before sleeping
int oldButton; // stores old button state
int newButton; // stores new button state

char keys[rows][cols]={ // Keymap, standard numpad plus one left row
  {KEY_INSERT,219,220,221,222},
  {KEY_DELETE,231,232,233,223},
  {178,228,229,230,223},
  {KEY_BACKSPACE,225,226,227,224},
  {KEY_TAB,234,234,235,224},
};

byte rowPins[rows]={27,26,25,33,32}; // pins for each row
byte colPins[cols]={18,17,16,15,13}; // pins for each column
int sleepPin = 0; // pin for sleep button

Keypad kp = Keypad(makeKeymap(keys),rowPins,colPins,rows,cols); // makes keyboard matrix


void setup() {
  // put your setup code here, to run once:
  pinMode(sleepPin, INPUT_PULLUP); // runs sleep button as pullup 
  bleKeyboard.begin(); // begin bluetooth keyboard
  Serial.begin(115200); // start serial port
  Serial.println("Booting"); // post status
}

void loop() {
  // put your main code here, to run repeatedly:
  getKeys(); // get status of matrix
  checkSleep(); // get status of sleep timer
  checkButtons(); // get status of buttons
}

void getKeys(){
   if (kp.getKeys()) // if keys present
   {
        for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        {
            if ( kp.key[i].stateChanged )   // Only find keys that have changed state.
            {
                switch (kp.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                    case PRESSED: // if key pressed
                    //msg = " PRESSED.";
                    bleKeyboard.press(kp.key[i].kchar); // write key to bluetooth HID
                    //Serial.println(kp.key[i].kchar);
                break;
                    case HOLD: // if key held
                    //msg = " HOLD.";
                    bleKeyboard.press(kp.key[i].kchar); // hold key
                break;
                    case RELEASED: // if key released
                    //msg = " RELEASED.";
                    bleKeyboard.release(kp.key[i].kchar); // release key
                break;
                    case IDLE: // if key idle
                    //msg = " IDLE.";
                    bleKeyboard.release(kp.key[i].kchar); // release key
                }
                kptime = millis(); // stores time of last key press
            }
        }
    }
}

void checkSleep(){
  //Serial.println(millis()-kptime);
  if (millis()-kptime>deepSleepMS) // if current time has passed deep sleep time
  {
    deepSleep(); // deep sleep
  }
}

void checkButtons(){
  newButton = digitalRead(sleepPin); // gets button state
  if (newButton == 0 && oldButton != 0){ // if current button state is pressed, and previous state is unpressed
    btime = millis() + bwait; // store time of first press
  }
  else if (newButton == 0 && btime <= millis()){ // if pressed and exceeded timer
    while(digitalRead(sleepPin) == 0){}; // wait until button is released, prevents immediate waking
    delay(1000); // wait 1 second
    Serial.println(newButton); // prints button status
    Serial.println(btime); // prints button hold time
    deepSleep(); // deep sleep
  }
  oldButton = newButton; // store new button state as old button state
}

void deepSleep(){
  Serial.println("Sleep"); // tell serial sleep
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // creates wake even when pin 0 is pulled to ground
  rtc_gpio_pulldown_en(GPIO_NUM_0); // enable pullup on pin 0
  esp_deep_sleep_start(); // start sleep
}
