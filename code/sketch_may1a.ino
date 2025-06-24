/*
The normal write example passed the test in ST3215 Servo, 
and if testing other models of ST series servos
please change the appropriate position, speed and delay parameters.
*/

#include <SCServo.h>
#include <Bluepad32.h>


SMS_STS st;
ControllerPtr myControllers[BP32_MAX_CONTROLLERS];


// the UART used to control servos.
// GPIO 18 - S_RXD, GPIO 19 - S_TXD, as default.
#define S_RXD 18
#define S_TXD 19

#define st1_max [450, 2800]
#define st2_max [1000, 3100]
#define st3_max [900, 3000]
#define st4_max [950, 3150]
#define st5_max [200, 4095]
#define st6_max [1560, 2750]

#define st1_90 [495, 1530, 2580] //POV from behind the arm : Left to Right
#define st2_90 [3130, 2070, 1080] //POV from the Left of the arm : forward to backward
#define st3_90 [3000, 1990, 980] //POV from the Left of the arm : forward to backward
#define st4_90 [2950, 2075, 1080] //POV from the Left of the arm : down to up (limited down to match still position)
#define st5_90 [175, 1215, 2225, 3265] //POV from front : clockwise rotation
#define st4_90 [1560, 1650, 1850, 2750] //Totally closed, parallel closed, Johnny Cash plectre closed, 90° open

//---------------------------- SERVO -------------------------
void end()
{
  st.EnableTorque(6,0);
  st.EnableTorque(5,0);
  st.EnableTorque(4,0);

  st.RegWritePosEx(3, 3000, 200, 50);
  st.RegWritePosEx(2, 1000, 400, 30);
  st.RegWritePosEx(6, 2500, 400, 50);

  st.RegWriteAction();
  delay(3000);

  st.RegWritePosEx(1, 1500, 200, 50);
  st.RegWritePosEx(5, 2225, 400, 50);
  st.RegWriteAction();
  delay(3000);

  st.RegWritePosEx(4, 2900, 400, 50);
  st.RegWritePosEx(6, 1600, 400, 50);
  st.RegWriteAction();
  delay(3000);
}

void readAll()
{
  for(int i = 1; i < 7; i++)
  {
    Serial.printf("%d : ", i);
    Serial.println(st.ReadPos(i));
  }
  Serial.println("----------------------");
}


void releaseAll()
{
  for(int i = 1; i < 7; i++)
  {
    st.EnableTorque(i, 0);
  }
}
//#####################################################
//-------------------- XBOX ---------------------------
//#####################################################

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.print("CALLBACK: Controller is connected, index=");
      Serial.println(i);
      myControllers[i] = ctl;
      foundEmptySlot = true;

      // Optional, once the gamepad is connected, request further info about the
      // gamepad.
      ControllerProperties properties = ctl->getProperties();
      char buf[80];
      sprintf(buf,
              "BTAddr: %02x:%02x:%02x:%02x:%02x:%02x, VID/PID: %04x:%04x, "
              "flags: 0x%02x",
              properties.btaddr[0], properties.btaddr[1], properties.btaddr[2],
              properties.btaddr[3], properties.btaddr[4], properties.btaddr[5],
              properties.vendor_id, properties.product_id, properties.flags);
      Serial.println(buf);
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
        "CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundGamepad = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.print("CALLBACK: Controller is disconnected from index=");
      Serial.println(i);
      myControllers[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }

  if (!foundGamepad) {
    Serial.println(
        "CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void processGamepad(ControllerPtr gamepad) {
  // Another way to query the buttons, is by calling buttons(), or
  // miscButtons() which return a bitmask.
  // Some gamepads also have DPAD, axis and more.
  char buf[256];
  snprintf(buf, sizeof(buf) - 1,
           "idx=%d, dpad: 0x%02x, buttons: 0x%04x, "
           "axis L: %4li, %4li, axis R: %4li, %4li, "
           "brake: %4ld, throttle: %4li, misc: 0x%02x, "
           "gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d, "
           "battery: %d",
           gamepad->index(),        // Gamepad Index
           gamepad->dpad(),         // DPad
           gamepad->buttons(),      // bitmask of pressed buttons
           gamepad->axisX(),        // (-511 - 512) left X Axis
           gamepad->axisY(),        // (-511 - 512) left Y axis
           gamepad->axisRX(),       // (-511 - 512) right X axis
           gamepad->axisRY(),       // (-511 - 512) right Y axis
           gamepad->brake(),        // (0 - 1023): brake button
           gamepad->throttle(),     // (0 - 1023): throttle (AKA gas) button
           gamepad->miscButtons(),  // bitmask of pressed "misc" buttons
           gamepad->gyroX(),        // Gyro X
           gamepad->gyroY(),        // Gyro Y
           gamepad->gyroZ(),        // Gyro Z
           gamepad->accelX(),       // Accelerometer X
           gamepad->accelY(),       // Accelerometer Y
           gamepad->accelZ(),       // Accelerometer Z
           gamepad->battery()       // 0=Unknown, 1=empty, 255=full
  );
  Serial.println(buf);
}


void setup()
{
  Serial.begin(115200);
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  st.pSerial = &Serial1;
  delay(1000);

  String fv = BP32.firmwareVersion();
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  // To get the BD Address (MAC address) call:
  const uint8_t* addr = BP32.localBdAddress();
  Serial.print("BD Address: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(addr[i], HEX);
    if (i < 5)
      Serial.print(":");
    else
      Serial.println();
  }

  // This call is mandatory. It sets up Bluepad32 and creates the callbacks.
  BP32.setup(&onConnectedController, &onDisconnectedController);
}

unsigned long previousMillis = 0;
const long interval = 3000;  // Time between movements
bool positionState = false;  // false = position A, true = position B

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (positionState) {
      st.WritePosEx(1, 1000, 500);  // Move Servo 1 to position A
    } else {
      st.WritePosEx(1, 2000, 500);  // Move Servo 1 to position B
    }

    positionState = !positionState;  // Toggle position
  }

  // Keep Bluepad32 alive, optional
  BP32.update();

  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    ControllerPtr myController = myControllers[i];

    if (myController && myController->isConnected()) {
      processGamepad(myController);
    }
  }
}
