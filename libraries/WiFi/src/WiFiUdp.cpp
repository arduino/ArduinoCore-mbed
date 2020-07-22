#include "WiFiUdp.h"

extern WiFiClass WiFi;

#ifndef WIFI_UDP_BUFFER_SIZE
#define WIFI_UDP_BUFFER_SIZE        508
#endif

arduino::WiFiUDP::WiFiUDP() {
    _packet_buffer = new uint8_t[WIFI_UDP_BUFFER_SIZE];
    _current_packet = NULL;
    _current_packet_size = 0;
    // if this allocation fails then ::begin will fail
}

arduino::WiFiUDP::~WiFiUDP() {
    delete[] _packet_buffer;
}

uint8_t arduino::WiFiUDP::begin(uint16_t port) {
    // success = 1, fail = 0

    nsapi_error_t rt = _socket.open(WiFi.getNetwork());
    if (rt != NSAPI_ERROR_OK) {
        return 0;
    }

    if (_socket.bind(port) < 0) {
        return 0; //Failed to bind UDP Socket to port
    }

    if (!_packet_buffer) {
        return 0;
    }

    // do not block when trying to read from socket
    _socket.set_blocking(false);

    return 1;
}

uint8_t arduino::WiFiUDP::beginMulticast(IPAddress ip, uint16_t port) {
    // success = 1, fail = 0
    if(begin(port) != 1){
        return 0;
    }

    nsapi_addr_t multicastGroup = {NSAPI_IPv4, {ip[0], ip[1], ip[2], ip[3]}};       

    if (_socket.join_multicast_group(SocketAddress(multicastGroup)) != NSAPI_ERROR_OK) {
        printf("Error joining the multicast group\n");
        return 0;
    }

    return 1;
}

void arduino::WiFiUDP::stop() {
    _socket.close();    
}

int arduino::WiFiUDP::beginPacket(IPAddress ip, uint16_t port) {
    nsapi_addr_t convertedIP = {NSAPI_IPv4, {ip[0], ip[1], ip[2], ip[3]}};   
    _host = SocketAddress(convertedIP, port);
    //If IP is null and port is 0 the initialization failed
    return (_host.get_ip_address() == nullptr && _host.get_port() == 0) ? 0 : 1;
}

int arduino::WiFiUDP::beginPacket(const char *host, uint16_t port) {     
    _host = SocketAddress(host, port);
    //If IP is null and port is 0 the initialization failed
    return (_host.get_ip_address() == nullptr && _host.get_port() == 0) ? 0 : 1;
}

int arduino::WiFiUDP::endPacket() {
    return 1;
}

// Write a single byte into the packet
size_t arduino::WiFiUDP::write(uint8_t byte) {
    uint8_t buffer[1] = { byte };    
    return _socket.sendto(_host, buffer, 1);
}

// Write size bytes from buffer into the packet
size_t arduino::WiFiUDP::write(const uint8_t *buffer, size_t size) {    
    return _socket.sendto(_host, buffer, size);
}

int arduino::WiFiUDP::parsePacket() {
    nsapi_size_or_error_t ret = _socket.recvfrom(&_remoteHost, _packet_buffer, WIFI_UDP_BUFFER_SIZE);

    if (ret == NSAPI_ERROR_WOULD_BLOCK) {
        // no data
        return 0;
    } else if(ret == NSAPI_ERROR_NO_SOCKET){
        // socket was not created correctly.
        return -1;
    }
    // error codes below zero are errors
    else if (ret <= 0) {
        // something else went wrong, need some tracing info...
        return -1;
    }

    // set current packet states
    _current_packet = _packet_buffer;
    _current_packet_size = ret;

    return _current_packet_size;
}

int arduino::WiFiUDP::available() {
    return _current_packet_size;
}

// Read a single byte from the current packet
int arduino::WiFiUDP::read() {
    // no current packet...
    if (_current_packet == NULL) {
        // try reading the next frame, if there is no data return
        if (parsePacket() == 0) return -1;
    }

    _current_packet++;

    // check for overflow
    if (_current_packet > _packet_buffer + _current_packet_size) {
        // try reading the next packet...
        if (parsePacket() > 0) {
            // if so, read first byte of next packet;
            return read();
        }
        else {
            // no new data... not sure what to return here now
            return -1;
        }
    }

    return _current_packet[0];
}

// Read up to len bytes from the current packet and place them into buffer
// Returns the number of bytes read, or 0 if none are available
int arduino::WiFiUDP::read(unsigned char* buffer, size_t len) {
    // Q: does Arduino read() function handle fragmentation? I won't for now...
    if (_current_packet == NULL) {
        if (parsePacket() == 0) return 0;
    }

    // how much data do we have in the current packet?
    int offset = _current_packet - _packet_buffer;
    if (offset < 0) {
        return 0;
    }

    int max_bytes = _current_packet_size - offset;
    if (max_bytes < 0) {
        return 0;
    }

    // at the end of the packet?
    if (max_bytes == 0) {
        // try read next packet...
        if (parsePacket() > 0) {
            return read(buffer, len);
        }
        else {
            return 0;
        }
    }

    if (len > max_bytes) len = max_bytes;

    // copy to target buffer
    memcpy(buffer, _current_packet, len);

    _current_packet += len;

    return len;
}

IPAddress arduino::WiFiUDP::remoteIP() {
    nsapi_addr_t address = _remoteHost.get_addr();
    return IPAddress(address.bytes[0], address.bytes[1], address.bytes[2], address.bytes[3]);
}

uint16_t arduino::WiFiUDP::remotePort() {    
    return _remoteHost.get_port();
}

void arduino::WiFiUDP::flush(){
    // TODO: a real check to ensure transmission has been completed
}

int arduino::WiFiUDP::peek(){
  if (_current_packet_size < 1){
    return -1;
  }

  return _current_packet[0];
}