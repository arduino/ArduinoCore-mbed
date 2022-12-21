//go:build !darwin

package ymodem

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"path/filepath"
	"runtime"
	"time"
)

const SOH byte = 0x01
const STX byte = 0x02
const EOT byte = 0x04
const ACK byte = 0x06
const NAK byte = 0x15
const POLL byte = 0x43

const SHORT_PACKET_PAYLOAD_LEN = 128
const LONG_PACKET_PAYLOAD_LEN = 1024

var InvalidPacket = errors.New("invalid packet")

func CRC16(data []byte) uint16 {
	var u16CRC uint16 = 0

	for _, character := range data {
		part := uint16(character)

		u16CRC = u16CRC ^ (part << 8)
		for i := 0; i < 8; i++ {
			if u16CRC&0x8000 > 0 {
				u16CRC = u16CRC<<1 ^ 0x1021
			} else {
				u16CRC = u16CRC << 1
			}
		}
	}

	return u16CRC
}

func CRC16Constant(data []byte, length int) uint16 {
	var u16CRC uint16 = 0

	for _, character := range data {
		part := uint16(character)

		u16CRC = u16CRC ^ (part << 8)
		for i := 0; i < 8; i++ {
			if u16CRC&0x8000 > 0 {
				u16CRC = u16CRC<<1 ^ 0x1021
			} else {
				u16CRC = u16CRC << 1
			}
		}
	}

	for c := 0; c < length-len(data); c++ {
		u16CRC = u16CRC ^ (0x04 << 8)
		for i := 0; i < 8; i++ {
			if u16CRC&0x8000 > 0 {
				u16CRC = u16CRC<<1 ^ 0x1021
			} else {
				u16CRC = u16CRC << 1
			}
		}
	}

	return u16CRC
}

func sendBlock(c io.ReadWriter, block uint16, data []byte) error {
	//send STX
	if _, err := c.Write([]byte{STX}); err != nil {
		return err
	}
	if _, err := c.Write([]byte{uint8(block)}); err != nil {
		return err
	}
	if _, err := c.Write([]byte{255 - uint8(block)}); err != nil {
		return err
	}

	//send data
	var toSend bytes.Buffer
	toSend.Write(data)
	for toSend.Len() < LONG_PACKET_PAYLOAD_LEN {
		toSend.Write([]byte{EOT})
	}

	//calc CRC
	u16CRC := CRC16Constant(data, LONG_PACKET_PAYLOAD_LEN)
	toSend.Write([]byte{uint8(u16CRC >> 8)})
	toSend.Write([]byte{uint8(u16CRC & 0x0FF)})

	sent := 0
	for sent < toSend.Len() {
		if n, err := c.Write(toSend.Bytes()[sent:]); err != nil {
			return err
		} else {
			sent += n
		}
	}

	return nil
}

func ModemSend(c io.ReadWriter, data []byte, filename string) error {
	oBuffer := make([]byte, 1)

	// Wait for Poll
	if _, err := c.Read(oBuffer); err != nil {
		return err
	}

	// Send zero block with filename and size
	if oBuffer[0] == POLL {
		var send bytes.Buffer
		send.WriteString(filepath.Base(filename))
		send.WriteByte(0x0)
		send.WriteString(fmt.Sprintf("%d", len(data)))
		for send.Len() < LONG_PACKET_PAYLOAD_LEN {
			send.Write([]byte{0x0})
		}

		fmt.Printf("Sending %d bytes\n", len(data))

		sendBlock(c, 0, send.Bytes())

		// Wait for ACK
		if _, err := c.Read(oBuffer); err != nil {
			return err
		}

		if oBuffer[0] != ACK {
			fmt.Printf("oBuffer[0] : %x\n", oBuffer[0])
			return errors.New("Failed to send header block")
		}
	}

	// Wait for Poll
	if _, err := c.Read(oBuffer); err != nil {
		return err
	}

	// Send remaining data
	if oBuffer[0] == POLL {
		var blocks uint16 = uint16(len(data) / LONG_PACKET_PAYLOAD_LEN)
		if len(data) > int(int(blocks)*int(LONG_PACKET_PAYLOAD_LEN)) {
			blocks++
		}
		fmt.Printf("blocks : %d\n", blocks)

		failed := 0
		var currentBlock uint16 = 0
		for currentBlock < blocks && failed < 10 {
			if int(int(currentBlock+1)*int(LONG_PACKET_PAYLOAD_LEN)) > len(data) {
				sendBlock(c, currentBlock+1, data[int(currentBlock)*int(LONG_PACKET_PAYLOAD_LEN):])
			} else {
				sendBlock(c, currentBlock+1, data[int(currentBlock)*int(LONG_PACKET_PAYLOAD_LEN):(int(currentBlock)+1)*int(LONG_PACKET_PAYLOAD_LEN)])
			}

			if runtime.GOOS == "windows" {
				time.Sleep(50 * time.Millisecond)
			}
			if _, err := c.Read(oBuffer); err != nil {
				return err
			}

			if oBuffer[0] == ACK {
				currentBlock++
			} else {
				failed++
			}
		}
	}

	// Wait for NAK and send EOT
	if _, err := c.Write([]byte{EOT}); err != nil {
		return err
	}

	if _, err := c.Read(oBuffer); err != nil {
		return err
	}

	if oBuffer[0] != NAK {
		return errors.New("Didn't get a nak when expected")
	}

	// Send EOT again
	if _, err := c.Write([]byte{EOT}); err != nil {
		return err
	}

	if _, err := c.Read(oBuffer); err != nil {
		return err
	}

	if oBuffer[0] != ACK {
		return errors.New("Failed to send end block")
	}

	// Wait for POLL
	if _, err := c.Read(oBuffer); err != nil {
		return err
	}

	if oBuffer[0] != POLL {
		return errors.New("Failed to send end block")
	}

	// Send empty block to signify end
	var zero bytes.Buffer
	for zero.Len() < LONG_PACKET_PAYLOAD_LEN {
		zero.Write([]byte{0x0})
	}

	sendBlock(c, 0, zero.Bytes())

	// Wait for ACK
	if _, err := c.Read(oBuffer); err != nil {
		return err
	}

	if oBuffer[0] != ACK {
		return errors.New("Failed to send end block")
	}

	return nil
}

func receivePacket(c io.ReadWriter) ([]byte, error) {
	oBuffer := make([]byte, 1)
	//dBuffer := make([]byte, LONG_PACKET_PAYLOAD_LEN)

	if _, err := c.Read(oBuffer); err != nil {
		return nil, err
	}
	pType := oBuffer[0]

	if pType == EOT {
		return nil, nil
	}

	var packetSize int
	switch pType {
	case SOH:
		packetSize = SHORT_PACKET_PAYLOAD_LEN
		break
	case STX:
		packetSize = LONG_PACKET_PAYLOAD_LEN
		break
	}

	if _, err := c.Read(oBuffer); err != nil {
		return nil, err
	}
	packetCount := oBuffer[0]

	if _, err := c.Read(oBuffer); err != nil {
		return nil, err
	}
	inverseCount := oBuffer[0]

	if inverseCount+packetCount != 255 {
		if _, err := c.Write([]byte{NAK}); err != nil {
			return nil, err
		}
		return nil, InvalidPacket
	}

	received := 0
	var pData bytes.Buffer

	for received < packetSize {

		tempBuffer := make([]byte, packetSize-received)

		n, err := c.Read(tempBuffer)
		if err != nil {
			return nil, err
		}

		received += n
		pData.Write(tempBuffer[:n])
	}

	var crc uint16
	if _, err := c.Read(oBuffer); err != nil {
		return nil, err
	}
	crc = uint16(oBuffer[0])

	if _, err := c.Read(oBuffer); err != nil {
		return nil, err
	}
	crc <<= 8
	crc |= uint16(oBuffer[0])

	// Calculate CRC
	crcCalc := CRC16(pData.Bytes())

	if crcCalc != crc {
		if _, err := c.Write([]byte{NAK}); err != nil {
			return nil, err
		}
	}

	if _, err := c.Write([]byte{ACK}); err != nil {
		return nil, err
	}

	return pData.Bytes(), nil
}

func ModemReceive(c io.ReadWriter) (string, []byte, error) {
	var data bytes.Buffer

	// Start Connection
	if _, err := c.Write([]byte{POLL}); err != nil {
		return "", nil, err
	}

	// Read file information
	pktData, err := receivePacket(c)
	if err != nil {
		return "", nil, err
	}

	filenameEnd := bytes.IndexByte(pktData, 0x0)
	filename := string(pktData[0:filenameEnd])

	var filesize int
	fmt.Sscanf(string(pktData[filenameEnd+1:]), "%d", &filesize)

	if _, err := c.Write([]byte{POLL}); err != nil {
		return "", nil, err
	}

	// Read Packets
	for {
		pktData, err := receivePacket(c)
		if err == InvalidPacket {
			continue
		}

		if err != nil {
			return "", nil, err
		}

		// End of Transmission
		if pktData == nil {
			break
		}

		data.Write(pktData)
	}

	// Send NAK to respond to EOT
	if _, err := c.Write([]byte{NAK}); err != nil {
		return "", nil, err
	}

	oBuffer := make([]byte, 1)
	if _, err := c.Read(oBuffer); err != nil {
		return "", nil, err
	}

	// Send ACK to respond to second EOT
	if oBuffer[0] != EOT {
		return "", nil, err
	}

	if _, err := c.Write([]byte{ACK}); err != nil {
		return "", nil, err
	}

	// Second POLL to get remaining file or close
	if _, err := c.Write([]byte{POLL}); err != nil {
		return "", nil, err
	}

	// Get remaining data ( for now assume one file )
	if _, err := receivePacket(c); err != nil {
		return "", nil, err
	}

	return filename, data.Bytes()[0:filesize], nil
}
