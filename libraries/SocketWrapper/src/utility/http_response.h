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

#ifndef _MBED_HTTP_HTTP_RESPONSE
#define _MBED_HTTP_HTTP_RESPONSE
#include <string>
#include <vector>
#include "http_parser/http_parser.h"

using namespace std;

class HttpResponse {
public:
    HttpResponse() {
        status_code = 0;
        concat_header_field = false;
        concat_header_value = false;
        expected_content_length = 0;
        is_chunked = false;
        is_message_completed = false;
        body_length = 0;
        body_offset = 0;
        body = NULL;
    }

    ~HttpResponse() {
        if (body != NULL) {
            free(body);
        }

        for (uint32_t ix = 0; ix < header_fields.size(); ix++) {
            delete header_fields[ix];
            delete header_values[ix];
        }
    }

    void set_status(int a_status_code, string a_status_message) {
        status_code = a_status_code;
        status_message = a_status_message;
    }

    int get_status_code() {
        return status_code;
    }

    string get_status_message() {
        return status_message;
    }

    void set_url(string a_url) {
        url = a_url;
    }

    string get_url() {
        return url;
    }

    void set_method(http_method a_method) {
        method = a_method;
    }

    http_method get_method() {
        return method;
    }

    void set_header_field(string field) {
        concat_header_value = false;

        // headers can be chunked
        if (concat_header_field) {
            *header_fields[header_fields.size() - 1] = (*header_fields[header_fields.size() - 1]) + field;
        }
        else {
            header_fields.push_back(new string(field));
        }

        concat_header_field = true;
    }

    void set_header_value(string value) {
        concat_header_field = false;

        // headers can be chunked
        if (concat_header_value) {
            *header_values[header_values.size() - 1] = (*header_values[header_values.size() - 1]) + value;
        }
        else {
            header_values.push_back(new string(value));
        }

        concat_header_value = true;
    }

    void set_headers_complete() {
        for (uint32_t ix = 0; ix < header_fields.size(); ix++) {
            if (strcicmp(header_fields[ix]->c_str(), "content-length") == 0) {
                expected_content_length = (uint32_t)atoi(header_values[ix]->c_str());
                break;
            }
        }
    }

    uint32_t get_headers_length() {
        return header_fields.size();
    }

    vector<string*> get_headers_fields() {
        return header_fields;
    }

    vector<string*> get_headers_values() {
        return header_values;
    }

    void set_body(const char *at, uint32_t length) {
        // Connection: close, could not specify Content-Length, nor chunked... So do it like this:
        if (expected_content_length == 0 && length > 0) {
            is_chunked = true;
        }

        // only malloc when this fn is called, so we don't alloc when body callback's are enabled
        if (body == NULL && !is_chunked) {
            body = (char*)malloc(expected_content_length);
        }

        if (is_chunked) {
            if (body == NULL) {
                body = (char*)malloc(length);
            }
            else {
                char* original_body = body;
                body = (char*)realloc(body, body_offset + length);
                if (body == NULL) {
                    free(original_body);
                    return;
                }
            }
        }

        memcpy(body + body_offset, at, length);

        body_offset += length;
    }

    void* get_body() {
        return (void*)body;
    }

    string get_body_as_string() {
        string s(body, body_offset);
        return s;
    }

    void increase_body_length(uint32_t length) {
        body_length += length;
    }

    uint32_t get_body_length() {
        return body_offset;
    }

    bool is_message_complete() {
        return is_message_completed;
    }

    void set_chunked() {
        is_chunked = true;
    }

    void set_message_complete() {
        is_message_completed = true;
    }

private:
    // from http://stackoverflow.com/questions/5820810/case-insensitive-string-comp-in-c
    int strcicmp(char const *a, char const *b) {
        for (;; a++, b++) {
            int d = tolower(*a) - tolower(*b);
            if (d != 0 || !*a) {
                return d;
            }
        }
    }

    char tolower(char c) {
        if(('A' <= c) && (c <= 'Z')) {
            return 'a' + (c - 'A');
        }

        return c;
    }

    int status_code;
    string status_message;
    string url;
    http_method method;

    vector<string*> header_fields;
    vector<string*> header_values;

    bool concat_header_field;
    bool concat_header_value;

    uint32_t expected_content_length;

    bool is_chunked;

    bool is_message_completed;

    char * body;
    uint32_t body_length;
    uint32_t body_offset;
};

#endif
