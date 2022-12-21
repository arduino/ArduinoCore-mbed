/*
   ESP32 YModem driver

   Copyright (C) LoBo 2017

   Author: Boris Lovosevic (loboris@gmail.com)

   All rights reserved.

   Permission to use, copy, modify, and distribute this software
   and its documentation for any purpose and without fee is hereby
   granted, provided that the above copyright notice appear in all
   copies and that both that the copyright notice and this
   permission notice and warranty disclaimer appear in supporting
   documentation, and that the name of the author not be used in
   advertising or publicity pertaining to distribution of the
   software without specific, written prior permission.

   The author disclaim all warranties with regard to this
   software, including all implied warranties of merchantability
   and fitness.  In no event shall the author be liable for any
   special, indirect or consequential damages or any damages
   whatsoever resulting from loss of use, data or profits, whether
   in an action of contract, negligence or other tortious action,
   arising out of or in connection with the use or performance of
   this software.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "Arduino.h"
#include "ymodem.h"

//----------------------------------
static void LED_toggle() {
#if YMODEM_LED_ACT
  if (GPIO.out & (1 << YMODEM_LED_ACT)) {
    GPIO.out_w1tc = (1 << YMODEM_LED_ACT);
  } else {
    GPIO.out_w1ts = (1 << YMODEM_LED_ACT);
  }
#endif
}

//------------------------------------------------------------------------
static unsigned short crc16(const unsigned char *buf, unsigned long count)
{
  unsigned short crc = 0;
  int i;

  while (count--) {
    crc = crc ^ *buf++ << 8;

    for (i = 0; i < 8; i++) {
      if (crc & 0x8000) crc = crc << 1 ^ 0x1021;
      else crc = crc << 1;
    }
  }
  return crc;
}

//--------------------------------------------------------------
static int32_t Receive_Byte (unsigned char *c, uint32_t timeout)
{
  long start = millis();

  while (Serial.available() <= 0 && millis() - start < timeout) {
    delay(1);
  }

  if (!Serial.available()) {
    return -1;
  }

  *c = Serial.read();
  return 0;
}

//------------------------
static void uart_consume()
{
  //while (Serial.available()) {
  //  Serial.read();
  //}
}

//--------------------------------
static uint32_t Send_Byte (char c)
{
  Serial.write(&c, 1);
  return 0;
}

//----------------------------
static void send_CA ( void ) {
  Send_Byte(CA);
  Send_Byte(CA);
}

//-----------------------------
static void send_ACK ( void ) {
  Send_Byte(ACK);
}

//----------------------------------
static void send_ACKCRC16 ( void ) {
  Send_Byte(ACK);
  Send_Byte(CRC16);
}

//-----------------------------
static void send_NAK ( void ) {
  Send_Byte(NAK);
}

//-------------------------------
static void send_CRC16 ( void ) {
  Send_Byte(CRC16);
}


/**
    @brief  Receive a packet from sender
    @param  data
    @param  timeout
    @param  length
       >0: packet length
        0: end of transmission
       -1: abort by sender
       -2: error or crc error
    @retval 0: normally return
           -1: timeout
           -2: abort by user
*/
//--------------------------------------------------------------------------
static int32_t Receive_Packet (uint8_t *data, int *length, uint32_t timeout)
{
  int count, packet_size, i;
  unsigned char ch;
  *length = 0;

  // receive 1st byte
  if (Receive_Byte(&ch, timeout) < 0) {
    return -1;
  }

  switch (ch) {
    case SOH:
      packet_size = PACKET_SIZE;
      break;
    case STX:
      packet_size = PACKET_1K_SIZE;
      break;
    case EOT:
      *length = 0;
      return 0;
    case CA:
      if (Receive_Byte(&ch, timeout) < 0) {
        return -2;
      }
      if (ch == CA) {
        *length = -1;
        return 0;
      }
      else return -1;
    case ABORT1:
    case ABORT2:
      return -2;
    default:
      delay(100);
      uart_consume();
      return -1;
  }

  *data = (uint8_t)ch;
  uint8_t *dptr = data + 1;
  count = packet_size + PACKET_OVERHEAD - 1;

  for (i = 0; i < count; i++) {
    if (Receive_Byte(&ch, timeout) < 0) {
      return -1;
    }
    *dptr++ = (uint8_t)ch;;
  }

  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) {
    *length = -2;
    return 0;
  }
  if (crc16(&data[PACKET_HEADER], packet_size + PACKET_TRAILER) != 0) {
    *length = -2;
    return 0;
  }

  *length = packet_size;
  return 0;
}

// Receive a file using the ymodem protocol.
//-----------------------------------------------------------------
int Ymodem_Receive (FILE *ffd, unsigned int maxsize, char* getname)
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  uint8_t *file_ptr;
  char file_size[128];
  unsigned int i, file_len, write_len, session_done, file_done, packets_received, errors, size = 0;
  int packet_length = 0;
  file_len = 0;
  int eof_cnt = 0;

  for (session_done = 0, errors = 0; ;) {
    for (packets_received = 0, file_done = 0; ;) {
      LED_toggle();
      switch (Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT)) {
        case 0:  // normal return
          switch (packet_length) {
            case -1:
              // Abort by sender
              send_ACK();
              size = -1;
              goto exit;
            case -2:
              // error
              errors ++;
              if (errors > 5) {
                send_CA();
                size = -2;
                goto exit;
              }
              send_NAK();
              break;
            case 0:
              // End of transmission
              eof_cnt++;
              if (eof_cnt == 1) {
                send_NAK();
              }
              else {
                send_ACKCRC16();
              }
              break;
            default:
              // ** Normal packet **
              if (eof_cnt > 1) {
                send_ACK();
              }
              else if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0x000000ff)) {
                errors ++;
                if (errors > 5) {
                  send_CA();
                  size = -3;
                  goto exit;
                }
                send_NAK();
              }
              else {
                if (packets_received == 0) {
                  // ** First packet, Filename packet **
                  if (packet_data[PACKET_HEADER] != 0) {
                    errors = 0;
                    // ** Filename packet has valid data
                    if (getname) {
                      for (i = 0, file_ptr = packet_data + PACKET_HEADER; ((*file_ptr != 0) && (i < 64));) {
                        *getname = *file_ptr++;
                        getname++;
                      }
                      *getname = '\0';
                    }
                    for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < packet_length);) {
                      file_ptr++;
                    }
                    for (i = 0, file_ptr ++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);) {
                      file_size[i++] = *file_ptr++;
                    }
                    file_size[i++] = '\0';
                    if (strlen(file_size) > 0) size = strtol(file_size, NULL, 10);
                    else size = 0;

                    // Test the size of the file
                    if ((size < 1) || (size > maxsize)) {
                      // End session
                      send_CA();
                      if (size > maxsize) size = -9;
                      else size = -4;
                      goto exit;
                    }

                    file_len = 0;
                    send_ACKCRC16();
                  }
                  // Filename packet is empty, end session
                  else {
                    errors ++;
                    if (errors > 5) {
                      send_CA();
                      size = -5;
                      goto exit;
                    }
                    send_NAK();
                  }
                }
                else {
                  // ** Data packet **
                  // Write received data to file
                  if (file_len < size) {
                    file_len += packet_length;  // total bytes received
                    if (file_len > size) {
                      write_len = packet_length - (file_len - size);
                      file_len = size;
                    }
                    else write_len = packet_length;

                    int written_bytes = fwrite((char*)(packet_data + PACKET_HEADER), 1, write_len, ffd);
                    if (written_bytes != write_len) { //failed
                      /* End session */
                      send_CA();
                      size = -6;
                      goto exit;
                    }
                    LED_toggle();
                  }
                  //success
                  errors = 0;
                  send_ACK();
                }
                packets_received++;
              }
          }
          break;
        case -2:  // user abort
          send_CA();
          size = -7;
          goto exit;
        default: // timeout
          if (eof_cnt > 1) {
            file_done = 1;
          }
          else {
            errors ++;
            if (errors > MAX_ERRORS) {
              send_CA();
              size = -8;
              goto exit;
            }
            send_CRC16();
          }
      }
      if (file_done != 0) {
        session_done = 1;
        break;
      }
    }
    if (session_done != 0) break;
  }
exit:
#if YMODEM_LED_ACT
  gpio_set_level(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
#endif
  return size;
}

//------------------------------------------------------------------------------------
static void Ymodem_PrepareIntialPacket(uint8_t *data, char *fileName, uint32_t length)
{
  uint16_t tempCRC;

  memset(data, 0, PACKET_SIZE + PACKET_HEADER);
  // Make first three packet
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;

  // add filename
  sprintf((char *)(data + PACKET_HEADER), "%s", fileName);

  //add file site
  sprintf((char *)(data + PACKET_HEADER + strlen((char *)(data + PACKET_HEADER)) + 1), "%d", length);
  data[PACKET_HEADER + strlen((char *)(data + PACKET_HEADER)) +
       1 + strlen((char *)(data + PACKET_HEADER + strlen((char *)(data + PACKET_HEADER)) + 1))] = ' ';

  // add crc
  tempCRC = crc16(&data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER] = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-------------------------------------------------
static void Ymodem_PrepareLastPacket(uint8_t *data)
{
  uint16_t tempCRC;

  memset(data, 0, PACKET_SIZE + PACKET_HEADER);
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;
  tempCRC = crc16(&data[PACKET_HEADER], PACKET_SIZE);
  //tempCRC = crc16_le(0, &data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER] = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-----------------------------------------------------------------------------------------
static void Ymodem_PreparePacket(uint8_t *data, uint8_t pktNo, uint32_t sizeBlk, FILE *ffd)
{
  uint16_t i, size;
  uint16_t tempCRC;

  data[0] = STX;
  data[1] = (pktNo & 0x000000ff);
  data[2] = (~(pktNo & 0x000000ff));

  size = sizeBlk < PACKET_1K_SIZE ? sizeBlk : PACKET_1K_SIZE;
  // Read block from file
  if (size > 0) {
    size = fread(data + PACKET_HEADER, 1, size, ffd);
  }

  if ( size  < PACKET_1K_SIZE) {
    for (i = size + PACKET_HEADER; i < PACKET_1K_SIZE + PACKET_HEADER; i++) {
      data[i] = 0x00; // EOF (0x1A) or 0x00
    }
  }
  tempCRC = crc16(&data[PACKET_HEADER], PACKET_1K_SIZE);
  //tempCRC = crc16_le(0, &data[PACKET_HEADER], PACKET_1K_SIZE);
  data[PACKET_1K_SIZE + PACKET_HEADER] = tempCRC >> 8;
  data[PACKET_1K_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-------------------------------------------------------------
static uint8_t Ymodem_WaitResponse(uint8_t ackchr, uint8_t tmo)
{
  unsigned char receivedC;
  uint32_t errors = 0;

  do {
    if (Receive_Byte(&receivedC, NAK_TIMEOUT) == 0) {
      if (receivedC == ackchr) {
        return 1;
      }
      else if (receivedC == CA) {
        send_CA();
        return 2; // CA received, Sender abort
      }
      else if (receivedC == NAK) {
        return 3;
      }
      else {
        return 4;
      }
    }
    else {
      errors++;
    }
  } while (errors < tmo);
  return 0;
}


//------------------------------------------------------------------------
int Ymodem_Transmit (char* sendFileName, unsigned int sizeFile, FILE *ffd)
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  uint16_t blkNumber;
  unsigned char receivedC;
  int i, err;
  uint32_t size = 0;

  // Wait for response from receiver
  err = 0;
  do {
    Send_Byte(CRC16);
    LED_toggle();
  } while (Receive_Byte(&receivedC, NAK_TIMEOUT) < 0 && err++ < 45);

  if (err >= 45 || receivedC != CRC16) {
    send_CA();
    return -1;
  }

  // === Prepare first block and send it =======================================
  /* When the receiving program receives this block and successfully
     opened the output file, it shall acknowledge this block with an ACK
     character and then proceed with a normal YMODEM file transfer
     beginning with a "C" or NAK tranmsitted by the receiver.
  */
  Ymodem_PrepareIntialPacket(packet_data, sendFileName, sizeFile);
  do
  {
    // Send Packet
    Serial.write((char *)packet_data, PACKET_SIZE + PACKET_OVERHEAD);

    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return -2;                  // timeout or wrong response
    }
    else if (err == 2) return 98; // abort
    LED_toggle();
  } while (err != 1);

  // After initial block the receiver sends 'C' after ACK
  if (Ymodem_WaitResponse(CRC16, 10) != 1) {
    send_CA();
    return -3;
  }

  // === Send file blocks ======================================================
  size = sizeFile;
  blkNumber = 0x01;

  // Resend packet if NAK  for a count of 10 else end of communication
  while (size)
  {
    // Prepare and send next packet
    Ymodem_PreparePacket(packet_data, blkNumber, size, ffd);
    do
    {
      Serial.write((char *)packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD);

      // Wait for Ack
      err = Ymodem_WaitResponse(ACK, 10);
      if (err == 1) {
        blkNumber++;
        if (size > PACKET_1K_SIZE) size -= PACKET_1K_SIZE; // Next packet
        else size = 0; // Last packet sent
      }
      else if (err == 0 || err == 4) {
        send_CA();
        return -4;                  // timeout or wrong response
      }
      else if (err == 2) return -5; // abort
    } while (err != 1);
    LED_toggle();
  }

  // === Send EOT ==============================================================
  Send_Byte(EOT); // Send (EOT)
  // Wait for Ack
  do
  {
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 3) {   // NAK
      Send_Byte(EOT); // Send (EOT)
    }
    else if (err == 0 || err == 4) {
      send_CA();
      return -6;                  // timeout or wrong response
    }
    else if (err == 2) return -7; // abort
  } while (err != 1);

  // === Receiver requests next file, prepare and send last packet =============
  if (Ymodem_WaitResponse(CRC16, 10) != 1) {
    send_CA();
    return -8;
  }

  LED_toggle();
  Ymodem_PrepareLastPacket(packet_data);
  do
  {
    // Send Packet
    Serial.write((char *)packet_data, PACKET_SIZE + PACKET_OVERHEAD);

    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return -9;                  // timeout or wrong response
    }
    else if (err == 2) return -10; // abort
  } while (err != 1);

#if YMODEM_LED_ACT
  gpio_set_level(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
#endif
  return 0; // file transmitted successfully
}
