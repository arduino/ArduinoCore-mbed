#include "Wire.h"
#include "USB251xB.h"

struct usb251xb hub;

void configure_hub(struct usb251xb* hub) {
  hub->vendor_id = USB251XB_DEF_VENDOR_ID;
  hub->product_id = 0x2512;
  hub->device_id = USB251XB_DEF_DEVICE_ID;
  hub->conf_data1 = USB251XB_DEF_CONFIG_DATA_1;
#if 0
  if (of_get_property(np, "self-powered", NULL)) {
    hub->conf_data1 |= BIT(7);

    /* Configure Over-Current sens when self-powered */
    hub->conf_data1 &= ~BIT(2);
    if (of_get_property(np, "ganged-sensing", NULL))
      hub->conf_data1 &= ~BIT(1);
    else if (of_get_property(np, "individual-sensing", NULL))
      hub->conf_data1 |= BIT(1);
  } else if (of_get_property(np, "bus-powered", NULL)) {
    hub->conf_data1 &= ~BIT(7);

    /* Disable Over-Current sense when bus-powered */
    hub->conf_data1 |= BIT(2);
  }
  if (of_get_property(np, "disable-hi-speed", NULL))
    hub->conf_data1 |= BIT(5);

  if (of_get_property(np, "multi-tt", NULL))
    hub->conf_data1 |= BIT(4);
  else if (of_get_property(np, "single-tt", NULL))
    hub->conf_data1 &= ~BIT(4);

  if (of_get_property(np, "disable-eop", NULL))
    hub->conf_data1 |= BIT(3);

  if (of_get_property(np, "individual-port-switching", NULL))
    hub->conf_data1 |= BIT(0);
  else if (of_get_property(np, "ganged-port-switching", NULL))
    hub->conf_data1 &= ~BIT(0);
#else
  hub->conf_data1 |= (1 << 7);
  hub->conf_data1 |= (1 << 5);
  hub->conf_data1 &= ~(1 << 2);
#endif
  hub->conf_data2 = USB251XB_DEF_CONFIG_DATA_2;
  hub->conf_data3 = USB251XB_DEF_CONFIG_DATA_3;
  hub->non_rem_dev = USB251XB_DEF_NON_REMOVABLE_DEVICES;
  hub->port_disable_sp = USB251XB_DEF_PORT_DISABLE_SELF;
  hub->port_disable_bp = USB251XB_DEF_PORT_DISABLE_BUS;
  hub->max_power_sp = USB251XB_DEF_MAX_POWER_SELF;
  hub->max_power_bp = USB251XB_DEF_MAX_POWER_BUS;
  hub->max_current_sp = USB251XB_DEF_MAX_CURRENT_SELF;
  hub->max_current_bp = USB251XB_DEF_MAX_CURRENT_BUS;
  hub->power_on_time = USB251XB_DEF_POWER_ON_TIME;
  hub->lang_id = USB251XB_DEF_LANGUAGE_ID;
  hub->port_swap = USB251XB_DEF_PORT_SWAP;
  hub->bat_charge_en = USB251XB_DEF_BATTERY_CHARGING_ENABLE;
  hub->bat_charge_en |= (0xF << 1);
  hub->boost_up = USB251XB_DEF_BOOST_UP;
  hub->boost_57 = USB251XB_DEF_BOOST_57;
  hub->boost_14 = USB251XB_DEF_BOOST_14;
  hub->port_map12 = USB251XB_DEF_PORT_MAP_12;
  hub->port_map34 = USB251XB_DEF_PORT_MAP_34;
  hub->port_map56 = USB251XB_DEF_PORT_MAP_56;
  hub->port_map7  = USB251XB_DEF_PORT_MAP_7;
}


void write_hub_configuration(struct usb251xb* hub) {
  char i2c_wb[USB251XB_I2C_REG_SZ];
  memset(i2c_wb, 0, USB251XB_I2C_REG_SZ);

  i2c_wb[USB251XB_ADDR_VENDOR_ID_MSB]     = (hub->vendor_id >> 8) & 0xFF;
  i2c_wb[USB251XB_ADDR_VENDOR_ID_LSB]     = hub->vendor_id & 0xFF;
  i2c_wb[USB251XB_ADDR_PRODUCT_ID_MSB]    = (hub->product_id >> 8) & 0xFF;
  i2c_wb[USB251XB_ADDR_PRODUCT_ID_LSB]    = hub->product_id & 0xFF;
  i2c_wb[USB251XB_ADDR_DEVICE_ID_MSB]     = (hub->device_id >> 8) & 0xFF;
  i2c_wb[USB251XB_ADDR_DEVICE_ID_LSB]     = hub->device_id & 0xFF;
  i2c_wb[USB251XB_ADDR_CONFIG_DATA_1]     = hub->conf_data1;
  i2c_wb[USB251XB_ADDR_CONFIG_DATA_2]     = hub->conf_data2;
  i2c_wb[USB251XB_ADDR_CONFIG_DATA_3]     = hub->conf_data3;
  i2c_wb[USB251XB_ADDR_NON_REMOVABLE_DEVICES] = hub->non_rem_dev;
  i2c_wb[USB251XB_ADDR_PORT_DISABLE_SELF] = hub->port_disable_sp;
  i2c_wb[USB251XB_ADDR_PORT_DISABLE_BUS]  = hub->port_disable_bp;
  i2c_wb[USB251XB_ADDR_MAX_POWER_SELF]    = hub->max_power_sp;
  i2c_wb[USB251XB_ADDR_MAX_POWER_BUS]     = hub->max_power_bp;
  i2c_wb[USB251XB_ADDR_MAX_CURRENT_SELF]  = hub->max_current_sp;
  i2c_wb[USB251XB_ADDR_MAX_CURRENT_BUS]   = hub->max_current_bp;
  i2c_wb[USB251XB_ADDR_POWER_ON_TIME]     = hub->power_on_time;
  i2c_wb[USB251XB_ADDR_LANGUAGE_ID_HIGH]  = (hub->lang_id >> 8) & 0xFF;
  i2c_wb[USB251XB_ADDR_LANGUAGE_ID_LOW]   = hub->lang_id & 0xFF;
  i2c_wb[USB251XB_ADDR_MANUFACTURER_STRING_LEN] = hub->manufacturer_len;
  i2c_wb[USB251XB_ADDR_PRODUCT_STRING_LEN]      = hub->product_len;
  i2c_wb[USB251XB_ADDR_SERIAL_STRING_LEN]       = hub->serial_len;
  memcpy(&i2c_wb[USB251XB_ADDR_MANUFACTURER_STRING], hub->manufacturer,
         USB251XB_STRING_BUFSIZE);
  memcpy(&i2c_wb[USB251XB_ADDR_SERIAL_STRING], hub->serial,
         USB251XB_STRING_BUFSIZE);
  memcpy(&i2c_wb[USB251XB_ADDR_PRODUCT_STRING], hub->product,
         USB251XB_STRING_BUFSIZE);
  i2c_wb[USB251XB_ADDR_BATTERY_CHARGING_ENABLE] = hub->bat_charge_en;
  i2c_wb[USB251XB_ADDR_BOOST_UP]          = hub->boost_up;
  i2c_wb[USB251XB_ADDR_BOOST_57]          = hub->boost_57;
  i2c_wb[USB251XB_ADDR_BOOST_14]          = hub->boost_14;
  i2c_wb[USB251XB_ADDR_PORT_SWAP]         = hub->port_swap;
  i2c_wb[USB251XB_ADDR_PORT_MAP_12]       = hub->port_map12;
  i2c_wb[USB251XB_ADDR_PORT_MAP_34]       = hub->port_map34;
  i2c_wb[USB251XB_ADDR_PORT_MAP_56]       = hub->port_map56;
  i2c_wb[USB251XB_ADDR_PORT_MAP_7]        = hub->port_map7;
  i2c_wb[USB251XB_ADDR_STATUS_COMMAND] = USB251XB_STATUS_COMMAND_ATTACH;

  Wire.begin();
  Wire.setClock(100000);

  if (hub->skip_config) {
    Wire.beginTransmission(0x2C);
    Wire.write(USB251XB_ADDR_STATUS_COMMAND);
    Wire.write(1);
    Wire.write(USB251XB_STATUS_COMMAND_ATTACH);
    Wire.endTransmission();
    return;
  }

  for (int i = 0; i < (USB251XB_I2C_REG_SZ / USB251XB_I2C_WRITE_SZ); i++) {
    int offset = i * USB251XB_I2C_WRITE_SZ;
    uint8_t wbuf[USB251XB_I2C_WRITE_SZ + 1];

    /* The first data byte transferred tells the hub how many data
       bytes will follow (byte count).
    */
    wbuf[0] = USB251XB_I2C_WRITE_SZ;
    memcpy(&wbuf[1], &i2c_wb[offset], USB251XB_I2C_WRITE_SZ);

    //printf("writing %d byte block %d to 0x%02X\n",
    //        USB251XB_I2C_WRITE_SZ, i, offset);

    Wire.beginTransmission(0x2C);
    Wire.write(offset);
    Wire.write(wbuf, USB251XB_I2C_WRITE_SZ + 1);
    Wire.endTransmission();
  }
}

void start_hub() {
  configure_hub(&hub);
  write_hub_configuration(&hub);
}
