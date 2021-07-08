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

#ifndef _MBED_HTTP_REQUEST_BUILDER_H_
#define _MBED_HTTP_REQUEST_BUILDER_H_

#include <string>
#include <map>
#include "http_parser/http_parser.h"
#include "http_parsed_url.h"

class HttpRequestBuilder {
public:
    HttpRequestBuilder(http_method a_method, ParsedUrl* a_parsed_url)
        : method(a_method), parsed_url(a_parsed_url)
    {
        std::string host(parsed_url->host());

        char port_str[10];
        sprintf(port_str, ":%d", parsed_url->port());

        if (strcmp(parsed_url->schema(), "http") == 0 && parsed_url->port() != 80) {
            host += std::string(port_str);
        }
        else if (strcmp(parsed_url->schema(), "https") == 0 && parsed_url->port() != 443) {
            host += std::string(port_str);
        }
        else if (strcmp(parsed_url->schema(), "ws") == 0 && parsed_url->port() != 80) {
            host += std::string(port_str);
        }
        else if (strcmp(parsed_url->schema(), "wss") == 0 && parsed_url->port() != 443) {
            host += std::string(port_str);
        }

        set_header("Host", host);
    }

    /**
     * Set a header for the request
     * If the key already exists, it will be overwritten...
     */
    void set_header(std::string key, std::string value) {
        std::map<std::string, std::string>::iterator it = headers.find(key);

        if (it != headers.end()) {
            it->second = value;
        }
        else {
            headers.insert(headers.end(), std::pair<std::string, std::string>(key, value));
        }
    }

    char* build(const void* body, uint32_t body_size, uint32_t &size, bool skip_content_length = false) {
        const char* method_str = http_method_str(method);

        bool is_chunked = has_header("Transfer-Encoding", "chunked");

        if (!is_chunked && (method == HTTP_POST || method == HTTP_PUT || method == HTTP_DELETE || body_size > 0)) {
            char buffer[10];
            snprintf(buffer, 10, "%lu", body_size);
            set_header("Content-Length", std::string(buffer));
        }

        size = 0;

        // first line is METHOD PATH+QUERY HTTP/1.1\r\n
        size += strlen(method_str) + 1 + strlen(parsed_url->path()) + (strlen(parsed_url->query()) ? strlen(parsed_url->query()) + 1 : 0) + 1 + 8 + 2;

        // after that we'll do the headers
        typedef std::map<std::string, std::string>::iterator it_type;
        for(it_type it = headers.begin(); it != headers.end(); it++) {
            // line is KEY: VALUE\r\n
            size += it->first.length() + 1 + 1 + it->second.length() + 2;
        }

        // then the body, first an extra newline
        size += 2;

        if (!is_chunked) {
            // body
            size += body_size;
        }

        // Now let's print it
        char* req = (char*)calloc(size + 1, 1);
        char* originalReq = req;

        if (strlen(parsed_url->query())) {
            sprintf(req, "%s %s?%s HTTP/1.1\r\n", method_str, parsed_url->path(), parsed_url->query());
        } else {
            sprintf(req, "%s %s%s HTTP/1.1\r\n", method_str, parsed_url->path(), parsed_url->query());
        }
        req += strlen(method_str) + 1 + strlen(parsed_url->path()) + (strlen(parsed_url->query()) ? strlen(parsed_url->query()) + 1 : 0) + 1 + 8 + 2;

        typedef std::map<std::string, std::string>::iterator it_type;
        for(it_type it = headers.begin(); it != headers.end(); it++) {
            // line is KEY: VALUE\r\n
            sprintf(req, "%s: %s\r\n", it->first.c_str(), it->second.c_str());
            req += it->first.length() + 1 + 1 + it->second.length() + 2;
        }

        sprintf(req, "\r\n");
        req += 2;

        if (body_size > 0) {
            memcpy(req, body, body_size);
        }
        req += body_size;

        // Uncomment to debug...
        // printf("----- BEGIN REQUEST -----\n");
        // printf("%s", originalReq);
        // printf("----- END REQUEST -----\n");

        return originalReq;
    }

private:
    bool has_header(const char* key, const char* value = NULL) {
        typedef std::map<std::string, std::string>::iterator it_type;
        for(it_type it = headers.begin(); it != headers.end(); it++) {
            if (strcmp(it->first.c_str(), key) == 0) { // key matches
                if (value == NULL || (strcmp(it->second.c_str(), value) == 0)) { // value is NULL or matches
                    return true;
                }
            }
        }

        return false;
    }

    http_method method;
    ParsedUrl* parsed_url;
    std::map<std::string, std::string> headers;
};

#endif // _MBED_HTTP_REQUEST_BUILDER_H_
