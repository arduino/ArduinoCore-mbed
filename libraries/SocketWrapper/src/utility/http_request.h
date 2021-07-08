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

#ifndef _HTTP_REQUEST_
#define _HTTP_REQUEST_

#include <string>
#include <vector>
#include <map>
#include "http_request_base.h"
#include "http_parsed_url.h"
#include "TCPSocket.h"

/**
 * @todo:
 *      - Userinfo parameter is not handled
 */

#ifndef HTTP_RECEIVE_BUFFER_SIZE
#define HTTP_RECEIVE_BUFFER_SIZE 8 * 1024
#endif

/**
 * \brief HttpRequest implements the logic for interacting with HTTP servers.
 */
class HttpRequest : public HttpRequestBase {
public:
    friend class HttpRequestBase;

    /**
     * HttpRequest Constructor
     *
     * @param[in] network The network interface
     * @param[in] method HTTP method to use
     * @param[in] url URL to the resource
     * @param[in] bodyCallback Callback on which to retrieve chunks of the response body.
                               If not set, the complete body will be allocated on the HttpResponse object,
                               which might use lots of memory.
    */
    HttpRequest(NetworkInterface* network, http_method method, const char* url, mbed::Callback<void(const char *at, uint32_t length)> bodyCallback = 0)
        : HttpRequestBase(NULL, bodyCallback)
    {
        _error = 0;
        _response = NULL;
        _network = network;

        _parsed_url = new ParsedUrl(url);
        _request_builder = new HttpRequestBuilder(method, _parsed_url);

        _socket = new TCPSocket();
        ((TCPSocket*)_socket)->open(network);
        _we_created_socket = true;
    }

    /**
     * HttpRequest Constructor
     *
     * @param[in] socket An open TCPSocket
     * @param[in] method HTTP method to use
     * @param[in] url URL to the resource
     * @param[in] bodyCallback Callback on which to retrieve chunks of the response body.
                                If not set, the complete body will be allocated on the HttpResponse object,
                                which might use lots of memory.
    */
    HttpRequest(TCPSocket* socket, http_method method, const char* url, mbed::Callback<void(const char *at, uint32_t length)> bodyCallback = 0)
        : HttpRequestBase(socket, bodyCallback)
    {
        _error = 0;
        _response = NULL;

        _parsed_url = new ParsedUrl(url);
        _request_builder = new HttpRequestBuilder(method, _parsed_url);

        _we_created_socket = false;
    }

    virtual ~HttpRequest() {
    }

protected:

    virtual nsapi_error_t connect_socket(char *host, uint16_t port) {
        SocketAddress socketAddress = SocketAddress();
        socketAddress.set_port(port);
        _network->gethostbyname(host, &socketAddress);
        return ((TCPSocket*)_socket)->connect(socketAddress);
    }
};

#endif // _HTTP_REQUEST_
