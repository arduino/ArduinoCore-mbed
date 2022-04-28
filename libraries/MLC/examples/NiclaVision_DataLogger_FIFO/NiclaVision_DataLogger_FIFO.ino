/*
   This example exposes the second MB of Nicla Vision flash as a USB disk.
   The user can interact with this disk as a bidirectional communication with the board
   For example, the board could save data in a file to be retrieved later with a drag and drop.
   If the user does a double tap, the firmware goes to datalogger mode (green led on).
   Now the user can do another double tap to start a recording of the IMU data 
   (green led blinking). With another double tap the recording will be stopped (green led on).
   Now the user can start/stop other recordings of the IMU data using again the double tap.
   The log files are saved in flash with an increasing number suffix data_0.txt, data_1.txt, etc.
   If you want to transfer the log files to the PC, you can reset the board and
   wait for 10 seconds (blue led blinking).
*/

#include "PluggableUSBMSD.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "LSM6DSOXSensor.h"

#define INT_1 LSM6DS_INT
#define SENSOR_ODR 104.0f // In Hertz
#define ACC_FS 2 // In g
#define GYR_FS 2000 // In dps
#define MEASUREMENT_TIME_INTERVAL (1000.0f/SENSOR_ODR) // In ms
#define FIFO_SAMPLE_THRESHOLD 199
#define FLASH_BUFF_LEN 8192

typedef enum {
  DATA_STORAGE_STATE,
  DATA_LOGGER_IDLE_STATE,
  DATA_LOGGER_RUNNING_STATE
} demo_state_e;

volatile demo_state_e demo_state = DATA_STORAGE_STATE;
volatile int mems_event = 0;
uint32_t file_count = 0;
unsigned long timestamp_count = 0;
bool acc_available = false;
bool gyr_available = false;
int32_t acc_value[3];
int32_t gyr_value[3];
char buff[FLASH_BUFF_LEN];
uint32_t pos = 0;

QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3,  QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);
// Partition 1 is allocated to WiFi
mbed::MBRBlockDevice lsm_data(&root, 2);
static mbed::FATFileSystem lsm_fs("lsm");

LSM6DSOXSensor AccGyr(&SPI1, PIN_SPI_SS1);

USBMSD MassStorage(&root);

rtos::Thread acquisition_th;

FILE *f = nullptr;

void INT1Event_cb()
{
  mems_event = 1;
}

void USBMSD::begin()
{
  int err = lsm_fs.mount(&lsm_data);
  if (err) {
    Serial.println("mount failed");
    err = lsm_fs.reformat(&lsm_data);
    if (err) {
      Serial.println("Reformat failed");
      return;
    }
  }
}

mbed::FATFileSystem &USBMSD::getFileSystem()
{
  static mbed::FATFileSystem fs("lsm");
  return fs;
}

void led_green_thd()
{
  while (1) {
    if (demo_state == DATA_LOGGER_RUNNING_STATE) {
      digitalWrite(LEDG, LOW);
      delay(100);
      digitalWrite(LEDG, HIGH);
      delay(100);
    }
  }
}

void Read_FIFO_Data(uint16_t samples_to_read)
{
  uint16_t i;

  for (i = 0; i < samples_to_read; i++) {
    uint8_t tag;
    // Check the FIFO tag
    AccGyr.Get_FIFO_Tag(&tag);
    switch (tag) {
      // If we have a gyro tag, read the gyro data
      case LSM6DSOX_GYRO_NC_TAG: {
          AccGyr.Get_FIFO_G_Axes(gyr_value);
          gyr_available = true;
          break;
        }
      // If we have an acc tag, read the acc data
      case LSM6DSOX_XL_NC_TAG: {
          AccGyr.Get_FIFO_X_Axes(acc_value);
          acc_available = true;
          break;
        }
      // We can discard other tags
      default: {
          break;
        }
    }
    // If we have the measurements of both acc and gyro, we can store them with timestamp
    if (acc_available && gyr_available) {
      int num_bytes;
      num_bytes = snprintf(&buff[pos], (FLASH_BUFF_LEN - pos), "%lu %d %d %d %d %d %d\n", (unsigned long)((float)timestamp_count * MEASUREMENT_TIME_INTERVAL), (int)acc_value[0], (int)acc_value[1], (int)acc_value[2], (int)gyr_value[0], (int)gyr_value[1], (int)gyr_value[2]);
      pos += num_bytes;
      timestamp_count++;
      acc_available = false;
      gyr_available = false;
    }
  }
  // We can add the termination character to the string, so we are ready to save it in flash
  buff[pos] = '\0';
  pos = 0;
}

void setup()
{
  Serial.begin(115200);
  MassStorage.begin();
  pinMode(LEDB, OUTPUT);
  pinMode(LEDG, OUTPUT);
  digitalWrite(LEDB, HIGH);
  digitalWrite(LEDG, HIGH);

  // Initialize SPI1 bus.
  SPI1.begin();

  //Interrupts.
  attachInterrupt(INT_1, INT1Event_cb, RISING);

  // Initialize IMU.
  AccGyr.begin();
  AccGyr.Enable_X();
  AccGyr.Enable_G();
  // Configure ODR and FS of the acc and gyro
  AccGyr.Set_X_ODR(SENSOR_ODR);
  AccGyr.Set_X_FS(ACC_FS);
  AccGyr.Set_G_ODR(SENSOR_ODR);
  AccGyr.Set_G_FS(GYR_FS);
  // Enable the Double Tap event
  AccGyr.Enable_Double_Tap_Detection(LSM6DSOX_INT1_PIN);
  // Configure FIFO BDR for acc and gyro
  AccGyr.Set_FIFO_X_BDR(SENSOR_ODR);
  AccGyr.Set_FIFO_G_BDR(SENSOR_ODR);
  // Start Led blinking thread
  acquisition_th.start(led_green_thd);
}

void loop()
{

  if (mems_event) {
    mems_event = 0;
    LSM6DSOX_Event_Status_t status;
    AccGyr.Get_X_Event_Status(&status);
    if (status.DoubleTapStatus) {
      switch (demo_state) {
        case DATA_STORAGE_STATE: {
            // Go to DATA_LOGGER_IDLE_STATE state
            demo_state = DATA_LOGGER_IDLE_STATE;
            digitalWrite(LEDG, LOW);
            Serial.println("From DATA_STORAGE_STATE To DATA_LOGGER_IDLE_STATE");
            break;
          }
        case DATA_LOGGER_IDLE_STATE: {
            char filename[32];
            // Go to DATA_LOGGER_RUNNING_STATE state
            snprintf(filename, 32, "/lsm/data_%lu.txt", file_count);
            Serial.print("Start writing file ");
            Serial.println(filename);
            // open a file to write some data
            // w+ means overwrite, so every time the board is rebooted the file will be overwritten
            f = fopen(filename, "w+");
            if (f != nullptr) {
              // write header
              fprintf(f, "Timestamp[ms] A_X [mg] A_Y [mg] A_Z [mg] G_X [mdps] G_Y [mdps] G_Z [mdps]\n");
              fflush(f);
              Serial.println("From DATA_LOGGER_IDLE_STATE To DATA_LOGGER_RUNNING_STATE");
              demo_state = DATA_LOGGER_RUNNING_STATE;
              digitalWrite(LEDG, HIGH);
              timestamp_count = 0;
              pos = 0;
              acc_available = false;
              gyr_available = false;
              // Set FIFO in Continuous mode
              AccGyr.Set_FIFO_Mode(LSM6DSOX_STREAM_MODE);
            }
            break;
          }
        case DATA_LOGGER_RUNNING_STATE: {
            // Empty the FIFO
            uint16_t fifo_samples;
            AccGyr.Get_FIFO_Num_Samples(&fifo_samples);
            Read_FIFO_Data(fifo_samples);
            // Store the string in flash
            fprintf(f, "%s", buff);
            fflush(f);

            // Close the log file and increase the counter
            fclose(f);
            file_count++;
            // Set FIFO in Bypass mode
            AccGyr.Set_FIFO_Mode(LSM6DSOX_BYPASS_MODE);
            // Go to DATA_LOGGER_IDLE_STATE state
            demo_state = DATA_LOGGER_IDLE_STATE;
            // Wait for the led thread ends the blinking
            delay(250);
            digitalWrite(LEDG, LOW);
            Serial.println("From DATA_LOGGER_RUNNING_STATE To DATA_LOGGER_IDLE_STATE");
            break;
          }
        default:
          Serial.println("Error! Invalid state");
      }
    }
  }

  if (demo_state == DATA_LOGGER_RUNNING_STATE) {
    uint16_t fifo_samples;

    // Check the number of samples inside FIFO
    AccGyr.Get_FIFO_Num_Samples(&fifo_samples);

    // If we reach the threshold we can empty the FIFO
    if (fifo_samples > FIFO_SAMPLE_THRESHOLD) {
      // Empty the FIFO
      Read_FIFO_Data(fifo_samples);
      // Store the string in flash
      fprintf(f, "%s", buff);
      fflush(f);
    }
  }

  if (demo_state == DATA_STORAGE_STATE && millis() > 10000) {
    // Disable the sensor and go to Mass Storage mode
    AccGyr.Disable_Double_Tap_Detection();
    AccGyr.Disable_X();
    AccGyr.Disable_G();
    while (1) {
      digitalWrite(LEDB, LOW);
      delay(100);
      digitalWrite(LEDB, HIGH);
      delay(100);
    }
  }
}
