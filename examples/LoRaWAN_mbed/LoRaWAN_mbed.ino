/**
   Copyright (c) 2017, Arm Limited and affiliates.
   SPDX-License-Identifier: Apache-2.0

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <stdio.h>

#include "lorawan/LoRaWANInterface.h"
//#include "lorawan/phy/LoRaPHYEU868.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
#include "USBSerial.h"

// Application helpers
#include "mbed-lora-radio-drv/SX1276/SX1276_LoRaRadio.h"
#include "lorawan/lorastack/phy/LoRaPHYEU868.h"
#include "lorawan/lorastack/phy/LoRaPHYAS923.h"
#include "lorawan/lorastack/phy/LoRaPHYAU915.h"
#include "lorawan/lorastack/phy/LoRaPHYCN470.h"
#include "lorawan/lorastack/phy/LoRaPHYCN779.h"
#include "lorawan/lorastack/phy/LoRaPHYEU433.h"
#include "lorawan/lorastack/phy/LoRaPHYIN865.h"
#include "lorawan/lorastack/phy/LoRaPHYKR920.h"
#include "lorawan/lorastack/phy/LoRaPHYUS915.h"

SX1276_LoRaRadio radio(MBED_CONF_APP_LORA_SPI_MOSI,
                       MBED_CONF_APP_LORA_SPI_MISO,
                       MBED_CONF_APP_LORA_SPI_SCLK,
                       MBED_CONF_APP_LORA_CS,
                       MBED_CONF_APP_LORA_RESET,
                       MBED_CONF_APP_LORA_DIO0,
                       MBED_CONF_APP_LORA_DIO1,
                       MBED_CONF_APP_LORA_DIO2,
                       MBED_CONF_APP_LORA_DIO3,
                       MBED_CONF_APP_LORA_DIO4,
                       MBED_CONF_APP_LORA_DIO5,
                       MBED_CONF_APP_LORA_RF_SWITCH_CTL1,
                       MBED_CONF_APP_LORA_RF_SWITCH_CTL2,
                       MBED_CONF_APP_LORA_TXCTL,
                       MBED_CONF_APP_LORA_RXCTL,
                       MBED_CONF_APP_LORA_ANT_SWITCH,
                       MBED_CONF_APP_LORA_PWR_AMP_CTL,
                       MBED_CONF_APP_LORA_TCXO);

using namespace events;

#define MBED_CONF_LORA_DUTY_CYCLE_ON  1

//#undef Serial
//mbed::Serial serial(PA_2, PA_3, 115200);

//REDIRECT_STDOUT_TO(SerialUSB);

USBSerial ser;
#define printf ser.printf

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

/*
   Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
*/
#define TX_TIMER                        10000

/**
   Maximum number of events for the event queue.
   10 is the safe number for the stack events, however, if application
   also uses the queue for whatever purposes, this number should be increased.
*/
#define MAX_NUMBER_OF_EVENTS            10

/**
   Maximum number of retries for CONFIRMED messages before giving up
*/
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
   Dummy pin for dummy sensor
*/
#define PC_9                            0

#define DEV_EUI   "9784564454666555"
#define APP_EUI   "70B3D57EF0005EBC"
#define APP_KEY   "321B0E2D4D4A1371146D37D658129DF0"

/**
  This event queue is the global event queue for both the
  application and stack. To conserve memory, the stack is designed to run
  in the same thread as the application and the application is responsible for
  providing an event queue to the stack that will be used for ISR deferment as
  well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
   Event handler.

   This will be passed to the LoRaWAN stack to queue events for the
   application which in turn drive the application.
*/
static void lora_event_handler(lorawan_event_t event);

/**
   Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
*/

static LoRaPHYEU868 phy;

static LoRaWANInterface lorawan(radio, phy);

/**
   Application specific callbacks
*/
static lorawan_app_callbacks_t callbacks;

/**
   Entry point for application
*/
void loop() {
  delay(1000);
}

static uint8_t* string_to_uint8_buf(const char* str) {
  uint8_t* buf = (uint8_t*)malloc(strlen(str) / 2);
  for (int i = 0; i < strlen(str); i += 2) {
    char temp[2];
    temp[0] = str[i];
    temp[1] = str[i + 1];
    buf[i / 2] = strtoul(temp, NULL, 16);
  }
  return buf;
}

void setup(void)
{
  // SerialUSB.begin(115200);
  // setup tracing
  //delay(5000);
  printf("\r\n Mbed LoRaWANStack initializing \r\n");

  lorawan_connect_t info;
  info.connect_type = LORAWAN_CONNECTION_OTAA;
  info.connection_u.otaa.dev_eui = string_to_uint8_buf(DEV_EUI);
  info.connection_u.otaa.app_eui = string_to_uint8_buf(APP_EUI);
  info.connection_u.otaa.app_key = string_to_uint8_buf(APP_KEY);
  info.connection_u.otaa.nb_trials = 5;

  // stores the status of a call to LoRaWAN protocol
  lorawan_status_t retcode;

  // Initialize LoRaWAN stack
  if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
    printf("\r\n LoRa initialization failed! \r\n");
  }

  printf("\r\n Mbed LoRaWANStack initialized \r\n");

  // prepare application callbacks
  callbacks.events = mbed::callback(lora_event_handler);
  lorawan.add_app_callbacks(&callbacks);

  // Set number of retries in case of CONFIRMED messages
  if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
      != LORAWAN_STATUS_OK) {
    printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
  }

  printf("\r\n CONFIRMED message retries : %d \r\n",
         CONFIRMED_MSG_RETRY_COUNTER);

  // Enable adaptive data rate
  if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
    printf("\r\n enable_adaptive_datarate failed! \r\n");
  }

  //lorawan.set_datarate(DR_0);

  printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

  retcode = lorawan.connect(info);

  if (retcode == LORAWAN_STATUS_OK ||
      retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
  } else {
    printf("\r\n Connection error, code = %d \r\n", retcode);
  }

  printf("\r\n Connection - In Progress ...\r\n");

  // make your event queue dispatching events forever
  ev_queue.dispatch_forever();
}

/**
   Sends a message to the Network Server
*/
static void send_message()
{
  uint16_t packet_len;
  int16_t retcode;
  int sensor_value = analogRead(A0);

  packet_len = sprintf((char*) tx_buffer, "Dummy Sensor Value is %d",
                       sensor_value);


  retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                         MSG_UNCONFIRMED_FLAG);

  if (retcode < 0) {
    retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
    : printf("\r\n send() - Error code %d \r\n", retcode);

    if (retcode == LORAWAN_STATUS_WOULD_BLOCK) {
      //retry in 3 seconds
      if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
        ev_queue.call_in(3000, send_message);
      }
    }
    return;
  }

  printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
  memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
   Receive a message from the Network Server
*/
static void receive_message()
{
  uint8_t port;
  int flags;
  int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

  if (retcode < 0) {
    printf("\r\n receive() - Error code %d \r\n", retcode);
    return;
  }

  printf(" RX Data on port %u (%d bytes): ", port, retcode);
  for (uint8_t i = 0; i < retcode; i++) {
    printf("%02x ", rx_buffer[i]);
  }
  printf("\r\n");

  memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
   Event handler
*/
static void lora_event_handler(lorawan_event_t event)
{
  switch (event) {
    case CONNECTED:
      printf("\r\n Connection - Successful \r\n");
      if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
        send_message();
      } else {
        ev_queue.call_every(TX_TIMER, send_message);
      }

      break;
    case DISCONNECTED:
      ev_queue.break_dispatch();
      printf("\r\n Disconnected Successfully \r\n");
      break;
    case TX_DONE:
      printf("\r\n Message Sent to Network Server \r\n");
      if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
        send_message();
      }
      break;
    case TX_TIMEOUT:
    case TX_ERROR:
    case TX_CRYPTO_ERROR:
    case TX_SCHEDULING_ERROR:
      printf("\r\n Transmission Error - EventCode = %d \r\n", event);
      // try again
      if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
        send_message();
      }
      break;
    case RX_DONE:
      printf("\r\n Received message from Network Server \r\n");
      receive_message();
      break;
    case RX_TIMEOUT:
    case RX_ERROR:
      printf("\r\n Error in reception - Code = %d \r\n", event);
      break;
    case JOIN_FAILURE:
      printf("\r\n OTAA Failed - Check Keys \r\n");
      break;
    case UPLINK_REQUIRED:
      printf("\r\n Uplink required by NS \r\n");
      if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
        send_message();
      }
      break;
    default:
      MBED_ASSERT("Unknown Event");
  }
}
