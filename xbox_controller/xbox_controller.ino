/*
The normal write example passed the test in ST3215 Servo, 
and if testing other models of ST series servos
please change the appropriate position, speed and delay parameters.
*/

#include <SCServo.h>
#include <Bluepad32.h>


SMS_STS st;
ControllerPtr myControllers[BP32_MAX_CONTROLLERS];

u8 servoIDs[] = {1, 2, 3, 4, 5, 6};
s16 targetPos[] = {1530, 2070, 1990, 2075, 1215, 1600};
u16 speeds[]    = {300, 300, 300, 300, 300, 300};
u8 accels[]    = {50, 50, 50, 50, 50, 50};
bool active[]   = {false, false, false, false, false, false};


// the UART used to control servos.
// GPIO 18 - S_RXD, GPIO 19 - S_TXD, as default.
#define S_RXD 18
#define S_TXD 19

// Maximum steps for each servo
const int st1_max[] = {450, 2800};
const int st2_max[] = {1000, 3100};
const int st3_max[] = {900, 3000};
const int st4_max[] = {950, 3150};
const int st5_max[] = {200, 4095};
const int st6_max[] = {1560, 2750};

// Step positions for 90° positions
// Comments preserved for clarity
const int st1_90[] = {495, 1530, 2580}; // POV from behind the arm: Left to Right
const int st2_90[] = {3130, 2070, 1080}; // POV from the Left of the arm: forward to backward
const int st3_90[] = {3000, 1990, 980}; // POV from the Left of the arm: forward to backward
const int st4_90[] = {2950, 2075, 1080}; // POV from the Left of the arm: down to up (limited down to match still position)
const int st5_90[] = {175, 1215, 2225, 3265}; // POV from front: clockwise rotation
const int st6_90[] = {1560, 1650, 1850, 2750}; // Totally closed, parallel closed, Johnny Cash plectre closed, 90° open


enum states{
  INIT = 0,
  CONNECTED = 1,
  DISCONNECTED = 2,
  END = 3,
};

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

void start()
{
  st.RegWritePosEx(1, 1530, 200, 50);
  st.RegWritePosEx(2, 2070, 400, 30);
  st.RegWritePosEx(3, 1990, 400, 50);

  st.RegWriteAction();
  delay(3000);

  st.RegWritePosEx(4, 2075, 200, 50);
  st.RegWritePosEx(5, 1215, 400, 50);
  st.RegWritePosEx(6, 1600, 400, 50);

  st.RegWriteAction();
  delay(3000);
}

void setPositionAsIs()
{
  for(int i = 0; i < 6; i++)
  {
    targetPos[i] = st.ReadPos(servoIDs[i]);
  }
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

void holdAll()
{
    for(int i = 1; i < 7; i++)
  {
    st.EnableTorque(i, 1);
  }
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

//ROTATION ---------------------------------

void rotate1(ControllerPtr gamepad)
{
    int idx = 0;  // Index for servo 1 in arrays
    int currentPos = st.ReadPos(servoIDs[idx]);

    if (gamepad->axisRX() < -100) {
        targetPos[idx] = max(currentPos - 10, st1_max[0]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else if (gamepad->axisRX() > 100) {
        targetPos[idx] = min(currentPos + 10, st1_max[1]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else {
        active[idx] = false;
    }
}

void rotate2(ControllerPtr gamepad)
{
    int idx = 1;  // Servo 2
    int currentPos = st.ReadPos(servoIDs[idx]);

    if (gamepad->throttle() > 50) {
        targetPos[idx] = max(currentPos - 50, st2_max[0]);
        speeds[idx] = 300;
        accels[idx] = 255;
        active[idx] = true;
    }
    else if (gamepad->brake() > 50) {
        targetPos[idx] = min(currentPos + 50, st2_max[1]);
        speeds[idx] = 300;
        accels[idx] = 255;
        active[idx] = true;
    }
    else {
        active[idx] = false;
    }
}

void rotate3(ControllerPtr gamepad)
{
    int idx = 2;
    int currentPos = st.ReadPos(servoIDs[idx]);

    if (gamepad->axisRY() > 100) {
        targetPos[idx] = max(currentPos - 50, st3_max[0]);
        speeds[idx] = 300;
        accels[idx] = 255;
        active[idx] = true;
    }
    else if (gamepad->axisRY() < -100) {
        targetPos[idx] = min(currentPos + 50, st3_max[1]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else {
        active[idx] = false;
    }
}

void rotate4(ControllerPtr gamepad)
{
    int idx = 3;
    int currentPos = st.ReadPos(servoIDs[idx]);

    if (gamepad->axisY() > 100) {
        targetPos[idx] = max(currentPos - 50, st4_max[0]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else if (gamepad->axisY() < -100) {
        targetPos[idx] = min(currentPos + 50, st4_max[1]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else {
        active[idx] = false;
    }
}

void rotate5(ControllerPtr gamepad)
{
    int idx = 4;
    uint16_t buttons = gamepad->buttons();
    uint8_t group = (buttons >> 5) & 0x07;
    int currentPos = st.ReadPos(servoIDs[idx]);


    if (group == 1) {
        targetPos[idx] = max(currentPos + 25, st5_max[0]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else if (buttons & (1 << 4)) {
        targetPos[idx] = min(currentPos - 25, st5_max[1]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else {
        active[idx] = false;
    }
}

void rotate6(ControllerPtr gamepad)
{
    int idx = 5;
    uint16_t buttons = gamepad->buttons();
    int currentPos = st.ReadPos(servoIDs[idx]);

    if (buttons & 0x0001) {
        targetPos[idx] = max(currentPos - 25, st6_max[0]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else if (buttons & 0x0002) {
        targetPos[idx] = min(currentPos + 25, st6_max[1]);
        speeds[idx] = 300;
        accels[idx] = 50;
        active[idx] = true;
    }
    else {
        active[idx] = false;
    }
}


void setup()
{
  Serial.begin(115200);
  Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
  st.pSerial = &Serial1;
  delay(1000);

  setPositionAsIs();

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

void loop() {
  // Keep Bluepad32 alive, optional
  BP32.update();

  ControllerPtr myController = myControllers[0];

  if (myController && myController->isConnected()) {
    rotate1(myController);
    rotate2(myController);
    rotate3(myController);
    rotate4(myController);
    rotate5(myController);
    rotate6(myController);

    st.SyncWritePosEx(servoIDs, sizeof(active), targetPos, speeds, accels);

    st.RegWriteAction();
  }
  else{
    holdAll();
  }
}
