# Managing CA Certs for TLS connections

## Generating `certificates.h` from PEM file:

> [!NOTE]
> Pre-requisites: `xxd` from `vim` packages or standalone
```
xxd -i cacert.pem -n cacert_pem | sed 's/^unsigned/const unsigned/g' > certificates.h
```

## Getting PEM file from `certificates.h`

> [!NOTE]
> Pre-requisites: `xxd`, GNU Tools (Use g-tools on MacOS: e.g., `gtail`, `ghead`)
```
cat certificates.h | tail -n +2 | head -n -2 | xxd -r -p > cacert.pem
```
## Listing certifcates in `certificates.h`

> [!NOTE]
> Pre-requisites: `openssl`

```
cat certificates.h | tail -n +2 | head -n -2 | xxd -r -p > cacert.pem
openssl crl2pkcs7 -nocrl -certfile cacert.pem | openssl pkcs7 -print_certs | grep '^subject'
```

## Adding a new root certificate to `certificates.h`

> [!Note]
> The PEM file for the root CA to add, e.g., `new_root.pem` 

```
cat certificates.h | tail -n +2 | head -n -2 | xxd -r -p | cat - new_root.pem | xxd -n cacert_pem -i | sed 's/^unsigned/const unsigned/g' > certificates.h
```
