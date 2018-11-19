## DEPRECATED

This repository is deprecated. The code has been integrated into Mbed OS, so no developent will happen here anymore.

`TLSSocket` is now in Mbed OS master, and will be published in Mbed OS 5.11



## TLSSocket

TLSSocket provides the ability to easily secure any stream based socket connection, for example TCP stream. This allows existing protocol libraries to be used through secure connections.

`TLSSocket` is inheriting the `Socket` class, which allows any application that uses `Socket` to use `TLSSocket` instead.
Secure socket both uses Socket interface as its transport layer and implements it. This makes it transport independent and there is no direct dependency to IP stack. For example, we can use HTTP library and give `TLSSocket` for it, to covert it to HTTPS.

### Usage example

TLSSocket API follows Socket API so it is easy to use after setting up:

```
TLSSocket *socket = new TLSSocket();
socket->open(network)
socket->set_root_ca_cert(certificate);

socket->connect(HOST_NAME, PORT)

socket->send(data, size);
```

Please note that internal TLS structures require over 1 kB of RAM, so each TLSSocket should be allocated from heap using `new` command, instead of using stack or statically allocating it.


### Design

Internally `TLSSocket` consist of two classes `TLSSocketWrapper` and `TLSSocket` as shown in the following diagram:

![TLSSocket UML](tlssocket.png)

The `TLSSocketWrapper` is able to use any `Socket` as its transport. `TLSSocket` is a helper that uses directly `TCPSocket` for its transport, making it easy to adopt existing TCP based applications to TLS.

One use case of `TLSSocketWrapper` is that existing TCP socket can be upgraded to TLS, by wrapping it like this:

```
TCPSocket connection(net);
connection.connect(SERVER, PORT);

// First talk with the server without encryption
connection.send("STARTTLS\r\n", 10);

// Wrap the TCP into TLS object
TLSSocketWrapper tls = new TLSSocketWrapper(connection, SERVER, TLSSocketWrapper::TRANSPORT_CLOSE);

// Initiate TLS handshake
tls.connect();

// Now the secure connection can be used like regular socket
tls.send("HELLO", 5);
```

### API

`TLSSocketWrapper` implements Mbed OS Socket API and extends it with functions that allow configuring security certificates. Please note that for most of the use cases, you are using these methods through `TLSSocket` class.

#### Configuring certificates

`TLSSocketWrapper` provides following API to set server certificate. You can use either BASE64 formatted PEM certificate, or binary DER certificates. Later form of these functions just assumes `root_ca_pem` or `client_cert_pem` to be standard C string and counts its lenght and passes to method which takes just `void*` and `len`.

```
/** Sets the certification of Root CA.
 *
 * @param root_ca Root CA Certificate in any mbed-TLS supported format.
 * @param len     Length of certificate (including terminating 0 for PEM).
 */
nsapi_error_t TLSSocketWrapper::set_root_ca_cert(const void *root_ca, size_t len);

/** Sets the certification of Root CA.
 *
 * @param root_ca_pem Root CA Certificate in PEM format
 */
nsapi_error_t TLSSocketWrapper::set_root_ca_cert(const char *root_ca_pem);
```

If client authentication is required, following API allows you to set the client certificate and private key:

```
/** Sets client certificate, and client private key.
 *
 * @param client_cert Client certification in any mbed-TLS supported format.
 * @param client_private_key Client private key in PEM format.
 */
nsapi_error_t TLSSocketWrapper::set_client_cert_key(const void *client_cert_pem, size_t client_cert_len,
                                                    const void *client_private_key_pem, size_t client_private_key_len);

/** Sets client certificate, and client private key.
 *
 * @param client_cert_pem Client certification in PEM format.
 * @param client_private_key Client private key in PEM format.
 */
nsapi_error_t TLSSocketWrapper::set_client_cert_key(const char *client_cert_pem, const char *client_private_key_pem);
```

#### Socket API

`TLSSocketWrapper` implements [Mbed OS Socket API](https://os.mbed.com/docs/v5.10/apis/network-socket.html) as follows.

```
virtual nsapi_error_t close();
```

Destroys the memory allocated by TLS library.
Also closes the transport socket, unless [transport mode](#transport-modes) is set to `TRANSPORT_KEEP` or `TRANSPORT_CONNECT`.


```
virtual nsapi_error_t connect(const SocketAddress &address);
```

Initiates the TCP connection and continues to TLS hanshake. If [transport mode](#transport-modes) is either `TRANSPORT_KEEP` or `TRANSPORT_CLOSE`, TCP is assumed to be open and state directly goes into TLS handshake.
This is currently forced to blocking mode. After succesfully connecting, you can set it to non-blockin mode.

```
virtual nsapi_size_or_error_t send(const void *data, nsapi_size_t size);
virtual nsapi_size_or_error_t recv(void *data, nsapi_size_t size);
virtual nsapi_size_or_error_t sendto(const SocketAddress &address, const void *data, nsapi_size_t size);
virtual nsapi_size_or_error_t recvfrom(SocketAddress *address, void *data, nsapi_size_t size);
```
These work as expected, but `SocketAddress` parameters are ignored. TLS connection cannot
change the peer. Also `recvfrom()` call does not set the peer address.

Mbed TLS error codes `MBEDTLS_ERR_SSL_WANT_READ` and `MBEDTLS_ERR_SSL_WANT_WRITE` are
translated to `NSAPI_ERROR_WOULD_BLOCK` before passing to user.

`MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY` is ignored and zero is returned to user (connection closed). Other error codes are passed through.

```
virtual nsapi_error_t bind(const SocketAddress &address);
virtual void set_blocking(bool blocking);
virtual void set_timeout(int timeout);
virtual void sigio(mbed::Callback<void()> func);
virtual nsapi_error_t setsockopt(int level, int optname, const void *optval, unsigned optlen);
virtual nsapi_error_t getsockopt(int level, int optname, void *optval, unsigned *optlen);
```
These are passed through to transport socket.


```
virtual Socket *accept(nsapi_error_t *error = NULL);
virtual nsapi_error_t listen(int backlog = 1);
```
These are returning `NSAPI_ERROR_UNSUPPORTED` as TLS socket cannot be set to listening mode.

#### Transport modes

`TLSSocketWrapper` has four modes that are given in the constructor and affect how the transport Socket is used in connection and closing phases.

|Mode|Behaviour on trasport socket|
|----|----------------------------|
|TRANSPORT_KEEP | Keep the transport as it is. Does not call `connect()` or `close()` methods. |
|TRANSPORT_CONNECT_AND_CLOSE | Both `connect()` and `close()` are called. (default) |
|TRANSPORT_CONNECT | Call `connect()` but do not close the connection when finished.  |
|TRANSPORT_CLOSE | Call `close()` when connection is finished. |

Default mode is `TRANSPORT_CONNECT_AND_CLOSE`.


#### Advanced usage: using internal Mbed TLS structures

User may choose to use internal Mbed TLS structures to configure the TLS instance. This is supported by exposing some Mbed TLS structures like this:

```
mbedtls_x509_crt *get_own_cert();
int set_own_cert(mbedtls_x509_crt *);
mbedtls_x509_crt *get_ca_chain();
void set_ca_chain(mbedtls_x509_crt *);
mbedtls_ssl_config *get_ssl_config();
void set_ssl_config(mbedtls_ssl_config *);
```

For guidance of how to use these, please refer to Mbed TLS documentation.
