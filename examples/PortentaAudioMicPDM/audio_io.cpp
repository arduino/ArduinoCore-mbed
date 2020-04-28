/*******************************************************************************
                            LINK OPERATIONS
*******************************************************************************/

/********************************* LINK AUDIO *********************************/

#include "audio.h"
#include "Wire.h"

/**
    @brief  Initializes Audio low level.
    @retval None
*/
void AUDIO_IO_Init(void)
{
  Wire1.begin();
}

/**
    @brief  De-Initializes Audio low level.
    @retval None
*/
void AUDIO_IO_DeInit(void)
{
}

/**
    @brief  Writes a single data.
    @param  Addr: I2C address
    @param  Reg: Reg address
    @param  Value: Data to be written
    @retval None
*/
void AUDIO_IO_Write(uint8_t Addr, uint16_t Reg, uint16_t Value)
{
  Wire1.beginTransmission(Addr);
  Wire1.write((uint8_t)Reg);
  Wire1.write((uint8_t)Value >> 8);
  Wire1.write((uint8_t)(Value & 0xFF));
  Wire1.endTransmission();
}

/**
    @brief  Reads a single data.
    @param  Addr: I2C address
    @param  Reg: Reg address
    @retval Data to be read
*/
uint16_t AUDIO_IO_Read(uint8_t Addr, uint16_t Reg)
{
  uint8_t read_value[2];

  Wire1.beginTransmission(Addr);
  Wire1.write((uint8_t)Reg);
  Wire1.endTransmission(false);

  Wire.requestFrom(Addr, 2);
  int i = 0;
  while (Wire.available()) {
    read_value[i++] = Wire.read();
  }

  return ((read_value[0] << 8) | read_value[1]);
}

/**
    @brief  AUDIO Codec delay
    @param  Delay: Delay in ms
    @retval None
*/
void AUDIO_IO_Delay(uint32_t Delay)
{
  delay(Delay);
}

/********************************* LINK CAMERA ********************************/
