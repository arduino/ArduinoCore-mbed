/*
   This example shows how to load the MLC bytecode for Motion Intensity on LSM6DSOX
   of the Arduino Nano RP2040 Connect.
   You can find the video tutorial on LSM6DSOX MLC at: https://docs.arduino.cc/tutorials/nano-rp2040-connect/rp2040-imu-advanced
*/

// Includes
#include "WiFiNINA.h"
#include "LSM6DSOXSensor.h"
#include "lsm6dsox_motion_intensity.h"

#define INT_1 INT_IMU

//Interrupts.
volatile int mems_event = 0;

// Components
LSM6DSOXSensor AccGyr(&Wire, LSM6DSOX_I2C_ADD_L);

// MLC
ucf_line_t *ProgramPointer;
int32_t LineCounter;
int32_t TotalNumberOfLine;

void INT1Event_cb();
void printMLCStatus(uint8_t status);

void setup()
{
  // Led.
  pinMode(LEDB, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  digitalWrite(LEDB, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDR, LOW);

  // Initialize serial for output.
  Serial.begin(115200);

  // Initialize I2C bus.
  Wire.begin();

  AccGyr.begin();

  /* Feed the program to Machine Learning Core */
  /* Motion Intensity Default program */
  ProgramPointer = (ucf_line_t *)lsm6dsox_motion_intensity;
  TotalNumberOfLine = sizeof(lsm6dsox_motion_intensity) / sizeof(ucf_line_t);
  Serial.println("Motion Intensity for LSM6DSOX MLC");
  Serial.print("UCF Number Line=");
  Serial.println(TotalNumberOfLine);

  for (LineCounter = 0; LineCounter < TotalNumberOfLine; LineCounter++) {
    if (AccGyr.Write_Reg(ProgramPointer[LineCounter].address, ProgramPointer[LineCounter].data)) {
      Serial.print("Error loading the Program to LSM6DSOX at line: ");
      Serial.println(LineCounter);
      while (1) {
        // Led blinking.
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
      }
    }
  }

  Serial.println("Program loaded inside the LSM6DSOX MLC");

  AccGyr.Enable_X();
  AccGyr.Set_X_ODR(104.0f);
  AccGyr.Set_X_FS(2);

  //Interrupts.
  pinMode(INT_1, INPUT);
  attachInterrupt(INT_1, INT1Event_cb, RISING);
}

void loop()
{
  if (mems_event) {
    mems_event = 0;
    LSM6DSOX_MLC_Status_t status;
    AccGyr.Get_MLC_Status(&status);
    if (status.is_mlc1) {
      uint8_t mlc_out[8];
      AccGyr.Get_MLC_Output(mlc_out);
      printMLCStatus(mlc_out[0]);
    }
  }
}

void INT1Event_cb()
{
  mems_event = 1;
}

void printMLCStatus(uint8_t status)
{
  switch (status) {
    case 1:
      // Reset leds status
      digitalWrite(LEDB, LOW);
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDR, LOW);
      // LEDB On
      digitalWrite(LEDB, HIGH);
      Serial.println("Stationary");
      break;
    case 4:
      // Reset leds status
      digitalWrite(LEDB, LOW);
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDR, LOW);
      // LEDG On
      digitalWrite(LEDG, HIGH);
      Serial.println("Medium Intensity");
      break;
    case 8:
      // Reset leds status
      digitalWrite(LEDB, LOW);
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDR, LOW);
      // LEDR On
      digitalWrite(LEDR, HIGH);
      Serial.println("High Intensity");
      break;
    default:
      // Reset leds status
      digitalWrite(LEDB, LOW);
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDR, LOW);
      Serial.println("Unknown");
      break;
  }
}
