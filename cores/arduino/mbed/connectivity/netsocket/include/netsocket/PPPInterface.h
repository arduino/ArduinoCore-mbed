/*
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
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

#ifndef PPP_INTERFACE_H
#define PPP_INTERFACE_H

#include "nsapi.h"
#include "OnboardNetworkStack.h"
#include "NetworkInterface.h"
#include "PPP.h"


/** PPPInterface class
 *  Implementation of the NetworkInterface for an PPP-service
 *
 * This class provides the necessary glue logic to create a NetworkInterface
 * based on an PPP and an OnboardNetworkStack.
 *
 */
class PPPInterface : public virtual NetworkInterface {
public:
    /** Create an PPP-based network interface.
     *
     * The default arguments obtain the default PPP, which will be target-
     * dependent (and the target may have some JSON option to choose which
     * is the default, if there are multiple). The default stack is configured
     * by JSON option nsapi.default-stack.
     *
     * Due to inability to return errors from the constructor, no real
     * work is done until the first call to connect().
     *
     * @param ppp  Reference to PPP to use
     * @param stack Reference to onboard-network stack to use
     */
    PPPInterface(PPP &ppp = PPP::get_default_instance(),
                 OnboardNetworkStack &stack = OnboardNetworkStack::get_default_instance());
    ~PPPInterface() override;

    /** @copydoc NetworkInterface::set_network */
    nsapi_error_t set_network(const SocketAddress &ip_address, const SocketAddress &netmask, const SocketAddress &gateway) override;

    /** @copydoc NetworkInterface::connect */
    nsapi_error_t connect() override;

    /** @copydoc NetworkInterface::disconnect */
    nsapi_error_t disconnect() override;

    /** @copydoc NetworkInterface::get_ip_address */
    nsapi_error_t get_ip_address(SocketAddress *address) override;

    /** @copydoc NetworkInterface::get_netmask */
    nsapi_error_t get_netmask(SocketAddress *address) override;

    /** @copydoc NetworkInterface::get_gateway */
    nsapi_error_t get_gateway(SocketAddress *address) override;

    /** @copydoc NetworkInterface::get_interface_name */
    char *get_interface_name(char *interface_name) override;

    /** @copydoc NetworkInterface::set_as_default */
    void set_as_default() override;

    /** @copydoc NetworkInterface::attach */
    void attach(mbed::Callback<void(nsapi_event_t, intptr_t)> status_cb) override;

    /** @copydoc NetworkInterface::get_connection_status */
    nsapi_connection_status_t get_connection_status() const override;

    /** @copydoc NetworkInterface::set_blocking */
    nsapi_error_t set_blocking(bool blocking) override;

    /** Sets file stream used to communicate with modem
     *
     * @param stream Pointer to file handle
     */
    void set_stream(mbed::FileHandle *stream);

    /** Sets IP protocol versions of IP stack
     *
     * @param ip_stack IP protocol version
     */
    void set_ip_stack(nsapi_ip_stack_t ip_stack);

    /** Sets user name and password for PPP protocol
     *
     * @param uname    User name
     * @param password Password
     */
    void set_credentials(const char *uname, const char *password);

    /** Provide access to the PPP
     *
     * This should be used with care - normally the network stack would
     * control the PPP, so manipulating the PPP while the stack
     * is also using it (ie after connect) will likely cause problems.
     *
     * @return          Reference to the PPP in use
     */
    PPP &getppp() const
    {
        return _ppp;
    }

#if 0
    /* NetworkInterface does not currently have pppInterface, so this
     * "dynamic cast" is non-functional.
     */
    PPPInterface *pppInterface() final
    {
        return this;
    }
#endif

protected:
    /** Provide access to the underlying stack
     *
     *  @return The underlying network stack
     */
    NetworkStack *get_stack() final;

    PPP &_ppp;
    OnboardNetworkStack &_stack;
    OnboardNetworkStack::Interface *_interface = nullptr;
    bool _blocking = true;
    char _ip_address[NSAPI_IPv6_SIZE] {};
    char _netmask[NSAPI_IPv4_SIZE] {};
    char _gateway[NSAPI_IPv4_SIZE] {};
    mbed::Callback<void(nsapi_event_t, intptr_t)> _connection_status_cb;

    mbed::FileHandle *_stream = nullptr;
    nsapi_ip_stack_t _ip_stack = DEFAULT_STACK;
    const char *_uname = nullptr;
    const char *_password = nullptr;

};

#endif
