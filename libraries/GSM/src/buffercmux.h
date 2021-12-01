#ifndef _GSM0710_BUFFER_H_
#define _GSM0710_BUFFER_H_
/*
 * buffer.h -- buffer for the GSM 0710 protocol device driver
 *             and protocol frame definition
 *
 * Copyright (C) 2003 Tuukka Karvonen <tkarvone@iki.fi>
 *
 * Version 1.0 October 2003
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */


// basic mode flag for frame start and end
 #define F_FLAG 0xF9

// bits: Poll/final, Command/Response, Extension
#define PF 16
#define CR_CMD 2
#define EA 1
// the types of the frames
#define SABM 47
#define UA 99
#define DM 15
#define DISC 67
#define UIH 239
#define UI 3
// the types of the control channel commands
#define C_CLD 193
#define C_TEST 33
#define C_MSC 225
#define C_NSC 17
// V.24 signals: flow control, ready to communicate, ring indicator, data valid
// three last ones are not supported by Siemens TC_3x
#define S_FC 2
#define S_RTC 4
#define S_RTR 8
#define S_IC 64
#define S_DV 128

#define FRAME_IS(type, frame) ((frame->control & ~PF) == type)

typedef struct GSM0710_Frame {
  unsigned char channel;
  unsigned char control;
  int data_length;
  char *data;
} GSM0710_Frame;

#define GSM0710_BUFFER_SIZE 2048

typedef struct GSM0710_Buffer {
  unsigned char data[GSM0710_BUFFER_SIZE];
  unsigned char *readp;
  unsigned char *writep;
  unsigned char *endp;
  int flag_found; // set if last character read was flag
  unsigned long received_count;
  unsigned long dropped_count;
} GSM0710_Buffer;

// increases buffer pointer by one and wraps around if necessary
#define INC_BUF_POINTER(buf,p) p++; if (p == buf->endp) p = buf->data;

/* Allocates memory for a new buffer and initializes it.
 *
 * RETURNS:
 * the pointer to a new buufer
 */
GSM0710_Buffer *gsm0710_buffer_init();

/* Destroys the buffer (i.e. frees up the memory
 *
 * PARAMS:
 * buf - buffer to be destroyed
 */
void gsm0710_buffer_destroy(GSM0710_Buffer *buf);

/* Tells, how many chars are saved into the buffer.
 *
 */
//int gsm0710_buffer_length(GSM0710_Buffer *buf);
#define gsm0710_buffer_length(buf) ((buf->readp > buf->writep) ? (GSM0710_BUFFER_SIZE - (buf->readp - buf->writep)) : (buf->writep-buf->readp))

/* Tells, how much free space there is in the buffer
 */
//int gsm0710_buffer_free(GSM0710_Buffer *buf);
#define gsm0710_buffer_free(buf) ((buf->readp > buf->writep) ? (buf->readp - buf->writep) : (GSM0710_BUFFER_SIZE - (buf->writep-buf->readp)))

/* Tries to read count number of chars from the buffer
 *
 * PARAMS:
 * buf    - pointer to the buffer
 * output - where to read the characters (in user memory)
 * count  - number of characters to be read
 * RETURNS:
 * number of characters read
 */
int gsm0710_buffer_read(GSM0710_Buffer *buf, char *output, int count);

/* Encapsulates data to a frame and writes it to the buffer
 *
 * PARAMS
 * buf     - pointer to the buffer
 * channel - logical channel where the frame is sent to
 * input   - input data (in user memory)
 * count   - how many characters should be written
 * RETURNS
 * number of characters written
 */
int gsm0710_buffer_write2ch(GSM0710_Buffer *buf, int channel, 
			    const char *input, int count);

/* Writes data to the buffer
 *
 * PARAMS
 * buf     - pointer to the buffer
 * input   - input data (in user memory)
 * count   - how many characters should be written
 * RETURNS
 * number of characters written
 */
int gsm0710_buffer_write(GSM0710_Buffer *buf,const char *input, int count);

/* Gets a frame from buffer. You have to remember to free this frame
 * when it's not needed anymore
 *
 * PARAMS:
 * buf   - the buffer, where the frame is extracted
 * RETURNS:
 * frame or null, if there isn't ready frame with given index
 */
GSM0710_Frame *gsm0710_buffer_get_frame(GSM0710_Buffer *buf);

// destroys a frame
void destroy_frame(GSM0710_Frame *frame);

/* Calculates frame check sequence from given characters.
 *
 * PARAMS:
 * input - character array
 * count - number of characters in array (that are included)
 * RETURNS:
 * frame check sequence
 */
unsigned char make_fcs(const unsigned char *input, int count);

#endif /* _GSM0710_BUFFER_H_ */
