#define OTP_QSPI_MAGIC    0xB5

typedef struct __attribute__((packed)) {
    uint8_t magic;
    uint8_t version;
    union {
        uint16_t board_functionalities;
        struct {
            uint8_t wifi  :1;
            uint8_t rs485 :1;
            uint8_t ethernet :1;
        } _board_functionalities;
    };
    uint16_t revision;
    uint8_t external_flash_size;
    uint16_t vid;
    uint16_t pid;
    uint8_t mac_address[6];
    uint8_t mac_address_2[6];
    uint8_t plc_license[16];
} OptaBoardInfo;
