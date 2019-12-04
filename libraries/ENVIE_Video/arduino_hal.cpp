#include "arduino_hal.h"

PinName gpio_request(char* name) {
    if (strcmp(name, "anx7625_reset_n") == 0) {
        return PJ_3;
    }
    if (strcmp(name, "anx7625_p_on_ctl") == 0) {
        return PK_2;
    }
    if (strcmp(name, "anx7625_cbl_det") == 0) {
        return PK_3;
    }
    if (strcmp(name, "anx7625_intr_comm") == 0) {
        return PK_4;
    }
}

int gpio_direction_output(PinName gpio, int value) {
    mbed::DigitalOut(gpio).write((int)value);
    return 0;
}

int gpio_direction_input(PinName gpio) {
    mbed::DigitalIn(gpio).read();
    return 0;
}

PinName desc_to_gpio(struct gpio_desc* gpio) {
    return gpio->name;
}

void gpio_set_value(PinName gpio, int value) {
    digitalWrite(gpio, value);
}

int gpio_get_value(PinName gpio) {
    return digitalRead(gpio);
}

void usleep_range(int low, int high) {
    delayMicroseconds(low);
}

mbed::I2C i2c(I2C_SDA , I2C_SCL); 

int i2c_smbus_write_byte_data(struct i2c_client * client, uint8_t command, uint8_t value) {

    char cmd[2];
    cmd[0] = command;
    cmd[1] = value;
    return i2c.write(client->addr << 1, cmd, 2);

    Wire.beginTransmission(client->addr);
    Wire.write(command);
    Wire.write(value);
    return Wire.endTransmission(true);
}

int i2c_smbus_write_i2c_block_data(struct i2c_client * client, uint8_t command, size_t len, uint8_t* buf) {

    char cmd[len +1];
    cmd[0] = command;
    memcpy(&cmd[1], buf, len);
    return i2c.write(client->addr << 1, cmd, len + 1);

    Wire.beginTransmission(client->addr);
    Wire.write(command);
    Wire.write(buf, len);
    return Wire.endTransmission(true);
}

int i2c_smbus_read_i2c_block_data(struct i2c_client * client, uint8_t reg_addr, size_t len, uint8_t* buf) {

    char cmd[len];
    cmd[0] = reg_addr;
    i2c.write(client->addr << 1, cmd, 1);
    return i2c.read(client->addr << 1, cmd, len);

    Wire.beginTransmission(client->addr);
    Wire.write(reg_addr);
    int ret = Wire.endTransmission(false);
    if (ret != 0) {
        return -1;
    }

    Wire.requestFrom(client->addr, len);
    size_t i = 0;
    while (Wire.available() && i < len) {
        buf[i++] = Wire.read();
    }
    return i;
}

int i2c_smbus_read_byte_data(struct i2c_client * client, uint8_t reg_addr) {

    char cmd[1];
    cmd[0] = reg_addr;
    i2c.write(client->addr << 1, cmd, 1);
    i2c.read(client->addr << 1, cmd, 1);
    return cmd[0];

    Wire.beginTransmission(client->addr);
    Wire.write(reg_addr);
    int ret = Wire.endTransmission(false);
    if (ret != 0) {
        return -1;
    }

    Wire.requestFrom(client->addr, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return -1;
}

static rtos::Mutex _mut;

void mutex_lock(struct mutex* mut) {
    _mut.lock();
}

void mutex_init(struct mutex* mut) {
}

void mutex_unlock(struct mutex* mut) {
    _mut.unlock();
}
