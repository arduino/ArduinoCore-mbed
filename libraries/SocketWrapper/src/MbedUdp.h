/*
  MbedUdp.h - UDP implementation using mbed Sockets
  Copyright (c) 2021 Arduino SA.  All right reserved.

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

#ifndef MBEDUDP_H
#define MBEDUDP_H

#include "Arduino.h"
#include "SocketHelpers.h"
#include "api/Udp.h"

#include "netsocket/SocketAddress.h"
#include "netsocket/UDPSocket.h"

#ifndef WIFI_UDP_BUFFER_SIZE
#define WIFI_UDP_BUFFER_SIZE 508
#endif

namespace arduino {

class MbedUDP : public UDP {
private:
  UDPSocket _socket;          // Mbed OS socket
  SocketAddress _host;        // Host to be used to send data
  SocketAddress _remoteHost;  // Remote host that sent incoming packets

  uint8_t* _packet_buffer;  // Raw packet buffer (contains data we got from the UDPSocket)

  // The Arduino APIs allow you to iterate through this buffer, so we need to be able to iterate over the current packet
  // these two variables are used to cache the state of the current packet
  uint8_t* _current_packet;
  size_t _current_packet_size;

  RingBufferN<WIFI_UDP_BUFFER_SIZE> txBuffer;

protected:
  virtual NetworkInterface* getNetwork() = 0;

public:
  MbedUDP();  // Constructor
  ~MbedUDP();
  virtual uint8_t begin(uint16_t);                      // initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
  virtual uint8_t beginMulticast(IPAddress, uint16_t);  // initialize, start listening on specified multicast IP address and port. Returns 1 if successful, 0 if there are no sockets available to use
  virtual void stop();                                  // Finish with the UDP socket

  // Sending UDP packets

  // Start building up a packet to send to the remote host specific in ip and port
  // Returns 1 if successful, 0 if there was a problem with the supplied IP address or port
  virtual int beginPacket(IPAddress ip, uint16_t port);
  // Start building up a packet to send to the remote host specific in host and port
  // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
  virtual int beginPacket(const char* host, uint16_t port);
  // Finish off this packet and send it
  // Returns 1 if the packet was sent successfully, 0 if there was an error
  virtual int endPacket();
  // Write a single byte into the packet
  virtual size_t write(uint8_t);
  // Write size bytes from buffer into the packet
  virtual size_t write(const uint8_t* buffer, size_t size);

  using Print::write;

  // Start processing the next available incoming packet
  // Returns the size of the packet in bytes, or 0 if no packets are available
  virtual int parsePacket();
  // Number of bytes remaining in the current packet
  virtual int available();
  // Read a single byte from the current packet
  virtual int read();
  // Read up to len bytes from the current packet and place them into buffer
  // Returns the number of bytes read, or 0 if none are available
  virtual int read(unsigned char* buffer, size_t len);
  // Read up to len characters from the current packet and place them into buffer
  // Returns the number of characters read, or 0 if none are available
  virtual int read(char* buffer, size_t len) {
    return read((unsigned char*)buffer, len);
  };
  // Return the next byte from the current packet without moving on to the next byte
  virtual int peek();
  virtual void flush();  // Finish reading the current packet

  // Return the IP address of the host who sent the current incoming packet
  virtual IPAddress remoteIP();
  // // Return the port of the host who sent the current incoming packet
  virtual uint16_t remotePort();

  friend class MbedSocketClass;
};

}

#endif