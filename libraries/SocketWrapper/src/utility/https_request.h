/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MBED_HTTPS_REQUEST_H_
#define _MBED_HTTPS_REQUEST_H_

#include <string>
#include <vector>
#include <map>
#include "http_request_base.h"
#include "TLSSocket.h"

#ifndef HTTP_RECEIVE_BUFFER_SIZE
#define HTTP_RECEIVE_BUFFER_SIZE 8 * 1024
#endif

/**
 * \brief HttpsRequest implements the logic for interacting with HTTPS servers.
 */
class HttpsRequest : public HttpRequestBase {
public:
    /**
     * HttpsRequest Constructor
     * Initializes the TCP socket, sets up event handlers and flags.
     *
     * @param[in] network The network interface
     * @param[in] ssl_ca_pem String containing the trusted CAs
     * @param[in] method HTTP method to use
     * @param[in] url URL to the resource
     * @param[in] body_callback Callback on which to retrieve chunks of the response body.
                                If not set, the complete body will be allocated on the HttpResponse object,
                                which might use lots of memory.
     */
    HttpsRequest(NetworkInterface* network,
                 const char* ssl_ca_pem,
                 http_method method,
                 const char* url,
                 mbed::Callback<void(const char *at, uint32_t length)> body_callback = 0)
        : HttpRequestBase(NULL, body_callback)
    {
        _error = 0;
        _network = network;

        _parsed_url = new ParsedUrl(url);
        _request_builder = new HttpRequestBuilder(method, _parsed_url);
        _response = NULL;

        _socket = new TLSSocket();
        ((TLSSocket*)_socket)->open(network);
        if (ssl_ca_pem)
          ((TLSSocket*)_socket)->set_root_ca_cert(ssl_ca_pem);
        else
          ((TLSSocket*)_socket)->set_root_ca_cert_path("/wlan/");
        _we_created_socket = true;
    }

    /**
     * HttpsRequest Constructor
     * Sets up event handlers and flags.
     *
     * @param[in] socket A connected TLSSocket
     * @param[in] method HTTP method to use
     * @param[in] url URL to the resource
     * @param[in] body_callback Callback on which to retrieve chunks of the response body.
                                If not set, the complete body will be allocated on the HttpResponse object,
                                which might use lots of memory.
     */
    HttpsRequest(TLSSocket* socket,
                 http_method method,
                 const char* url,
                 mbed::Callback<void(const char *at, uint32_t length)> body_callback = 0)
        : HttpRequestBase(socket, body_callback)
    {
        _parsed_url = new ParsedUrl(url);
        _body_callback = body_callback;
        _request_builder = new HttpRequestBuilder(method, _parsed_url);
        _response = NULL;

        _we_created_socket = false;
    }

    virtual ~HttpsRequest() {}

protected:
    virtual nsapi_error_t connect_socket(char *host, uint16_t port) {
        SocketAddress socketAddress = SocketAddress();
        socketAddress.set_port(port);
        _network->gethostbyname(host, &socketAddress);
        return ((TLSSocket*)_socket)->connect(socketAddress);
    }
};

#endif // _MBED_HTTPS_REQUEST_H_
