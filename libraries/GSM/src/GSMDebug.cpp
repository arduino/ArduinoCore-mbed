/*
  GSMDebug.cpp - Library for GSM on mbed platforms.
  Copyright (c) 2011-2023 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <GSM.h>
#include <GSMDebug.h>

#if GSM_DEBUG_ENABLE

constexpr const char * const arduino::GSMClass::sim_state_str[] = {
  "Ready",
  "PIN Needed",
  "PUK Needed",
  "Unknown"
};

constexpr const char * const arduino::GSMClass::reg_type_str[] = {
  "Not Registered",
  "Registered (Home Network)",
  "Searching Network",
  "Registration Denied",
  "Registration Unknown",
  "Registered (Roaming)",
  "Registered (SMS Only Home)",
  "Registered (SMS Only Roaming)",
  "Attached (Emergency Only)",
  "Registered (CSFB Not Preferred Home)",
  "Registered (CSFB Not Preferred Roaming)",
  "Already Registered"
};

constexpr const char * const arduino::GSMClass::rat_str[] = {
  "GSM",
  "GSM_COMPACT",
  "UTRAN",
  "EGPRS",
  "HSDPA",
  "HSUPA",
  "HSDPA_HSUPA",
  "E_UTRAN",
  "CATM1",
  "NB1",
  "RAT unknown",
};

constexpr const char * const arduino::GSMClass::state_str[] = {
  "Init",
  "Power On",
  "Device ready",
  "SIM PIN",
  "Signal quality",
  "Registering network",
  "Attaching network",
  "Unknown"
};

constexpr const char * const arduino::GSMClass::event_str[] = {
  "Device ready",
  "SIM status",
  "Registration status",
  "Registration type",
  "Cell ID",
  "RAT",
  "Attach network",
  "Activate PDP context",
  "Signal quality",
  "Retry",
  "Timeout",
};

const char * arduino::GSMClass::getRATString(const mbed::CellularNetwork::RadioAccessTechnology rat) {
  switch (rat) {
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_GSM:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_GSM_COMPACT:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_UTRAN:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_EGPRS:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_HSDPA:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_HSUPA:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_HSDPA_HSUPA:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_E_UTRAN:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_CATM1:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_NB1:
      return rat_str[rat];
    break;

    case mbed::CellularNetwork::RadioAccessTechnology::RAT_UNKNOWN:
    case mbed::CellularNetwork::RadioAccessTechnology::RAT_MAX:
    default:
      return rat_str[mbed::CellularNetwork::RadioAccessTechnology::RAT_UNKNOWN];
    break;
  }
}

const char * arduino::GSMClass::getStateString(const mbed::CellularStateMachine::CellularState state) {
  switch (state) {
    case mbed::CellularStateMachine::CellularState::STATE_INIT:
    case mbed::CellularStateMachine::CellularState::STATE_POWER_ON:
    case mbed::CellularStateMachine::CellularState::STATE_DEVICE_READY:
    case mbed::CellularStateMachine::CellularState::STATE_SIM_PIN:
    case mbed::CellularStateMachine::CellularState::STATE_SIGNAL_QUALITY:
    case mbed::CellularStateMachine::CellularState::STATE_REGISTERING_NETWORK:
    case mbed::CellularStateMachine::CellularState::STATE_ATTACHING_NETWORK:
      return state_str[state];
    break;

    case mbed::CellularStateMachine::CellularState::STATE_MAX_FSM_STATE:
    default:
      return state_str[mbed::CellularStateMachine::CellularState::STATE_MAX_FSM_STATE];
    break;
  }
}

const char * arduino::GSMClass::getEventString(const cellular_event_status event) {
  switch (event) {
    case cellular_event_status::CellularDeviceReady:
    case cellular_event_status::CellularSIMStatusChanged:
    case cellular_event_status::CellularRegistrationStatusChanged:
    case cellular_event_status::CellularRegistrationTypeChanged:
    case cellular_event_status::CellularCellIDChanged:
    case cellular_event_status::CellularRadioAccessTechnologyChanged:
    case cellular_event_status::CellularAttachNetwork:
    case cellular_event_status::CellularActivatePDPContext:
    case cellular_event_status::CellularSignalQuality:
    case cellular_event_status::CellularStateRetryEvent:
    case cellular_event_status::CellularDeviceTimeout:
      return event_str[event - NSAPI_EVENT_CELLULAR_STATUS_BASE];
    break;

    default:
      return "Unknown";
    break;
  }
}

const char * arduino::GSMClass::getSIMStateString(const mbed::CellularDevice::SimState state) {
  switch (state) {
    case mbed::CellularDevice::SimStateReady:
    case mbed::CellularDevice::SimStatePinNeeded:
    case mbed::CellularDevice::SimStatePukNeeded:
    case mbed::CellularDevice::SimStateUnknown:
      return sim_state_str[state];
    break;

    default:
      return sim_state_str[mbed::CellularDevice::SimStateUnknown];
  }
}

const char * arduino::GSMClass::getRegistrationStateString(const mbed::CellularNetwork::RegistrationStatus state) {
  switch (state) {
    case mbed::CellularNetwork::StatusNotAvailable:
    case mbed::CellularNetwork::NotRegistered:
    case mbed::CellularNetwork::RegisteredHomeNetwork:
    case mbed::CellularNetwork::SearchingNetwork:
    case mbed::CellularNetwork::RegistrationDenied:
    case mbed::CellularNetwork::Unknown:
    case mbed::CellularNetwork::RegisteredRoaming:
    case mbed::CellularNetwork::RegisteredSMSOnlyHome:
    case mbed::CellularNetwork::RegisteredSMSOnlyRoaming:
    case mbed::CellularNetwork::AttachedEmergencyOnly:
    case mbed::CellularNetwork::RegisteredCSFBNotPreferredHome:
    case mbed::CellularNetwork::RegisteredCSFBNotPreferredRoaming:
    case mbed::CellularNetwork::AlreadyRegistered:
      return reg_type_str[state];
    break;

    default:
      return reg_type_str[mbed::CellularNetwork::Unknown];
  }
}

#endif

void arduino::GSMClass::onStatusChange(nsapi_event_t ev, intptr_t in) {

  cellular_event_status event = (cellular_event_status)ev;

  if(event == CellularStateRetryEvent) {
    feedWatchdog();
  }

#if GSM_DEBUG_ENABLE
  const cell_callback_data_t *data = (const cell_callback_data_t *)in;

  switch(event)
  {
    case CellularDeviceReady:
    {
      DEBUG_INFO("Modem is powered and ready to receive commands");
    }
    break;

    case CellularSIMStatusChanged:
    {
      const mbed::CellularDevice::SimState state = static_cast<mbed::CellularDevice::SimState>(data->status_data);
      DEBUG_INFO("SIM status: %s", getSIMStateString(state));
    }
    break;

    case CellularRegistrationStatusChanged:
    {
      const mbed::CellularNetwork::RegistrationStatus state = static_cast<mbed::CellularNetwork::RegistrationStatus>(data->status_data);
      DEBUG_INFO("Registration status: %s", getRegistrationStateString(state));
    }
    break;

    case CellularRegistrationTypeChanged:
    {
      /* Never called from mbed driver */
    }
    break;

    case CellularCellIDChanged:
    {
      DEBUG_INFO("Cellular ID changed: %d", data->status_data);
    }
    break;

    case CellularRadioAccessTechnologyChanged:
    {
      const mbed::CellularNetwork::RadioAccessTechnology rat = static_cast <mbed::CellularNetwork::RadioAccessTechnology>(data->status_data);
      DEBUG_INFO("RAT changed: %s", getRATString(rat));
    }
    break;

    case CellularAttachNetwork:
    {
      DEBUG_INFO("Network status: %s", data->status_data ? "Attached" : "Detached");
    }
    break;

    case CellularActivatePDPContext:
    {
      DEBUG_INFO("Activate PDP context %s", (data->error != NSAPI_ERROR_OK) ? "Failure" : "Success");
    }
    break;

    case CellularSignalQuality:
    {
      const cell_signal_quality_t * sig = (const cell_signal_quality_t *)data->data;
      if((data->error != NSAPI_ERROR_OK) || (sig->rssi == RSSI_UNKNOWN)) {
        DEBUG_INFO("RSSI: Unknown");
      } else {
        DEBUG_INFO("RSSI: %d", sig->rssi);
      }
    }
    break;

    case CellularStateRetryEvent:
    {
      const cell_retry_cb_t * retry_cb_data = (const cell_retry_cb_t *)data->data;
      const cellular_event_status event = static_cast<cellular_event_status>(data->status_data);
      const mbed::CellularStateMachine::CellularState state = static_cast<mbed::CellularStateMachine::CellularState>(retry_cb_data->state);
      DEBUG_WARNING("Cellular event %s timed out. Cellular state %s, retry count %d", getEventString(event), getStateString(state), retry_cb_data->retry_count);
    }
    break;

    case CellularDeviceTimeout:
    {
      const cell_timeout_cb_t * timeout_cb_data = (const cell_timeout_cb_t *)data->data;
      const cellular_event_status event = static_cast<cellular_event_status>(data->status_data);
      const mbed::CellularStateMachine::CellularState state = static_cast<mbed::CellularStateMachine::CellularState>(timeout_cb_data->state);
      DEBUG_DEBUG("Cellular state: %s, waiting for event %s. Timeout %d", getStateString(state), getEventString(event), timeout_cb_data->timeout);
    }
    break;
  }
#endif
}
