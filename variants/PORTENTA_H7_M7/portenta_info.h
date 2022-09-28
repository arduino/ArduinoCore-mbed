#define OTP_QSPI_MAGIC    0xB5

typedef struct {
    uint8_t magic;
    uint8_t version;
    union {
        uint16_t board_functionalities;
        struct {
            uint8_t usb_high_speed :1;
            uint8_t ethernet :1;
            uint8_t wifi :1;
            uint8_t video :1;
            uint8_t nxp_crypto :1;
            uint8_t mchp_crypto :1;
        } _board_functionalities;
    };
    uint16_t revision;
    uint16_t carrier;
    uint8_t external_ram_size;
    uint8_t external_flash_size;
    uint16_t vid;
    uint16_t pid;
    uint8_t mac_address[6];
    uint8_t mac_address_2[6];
} PortentaBoardInfo;

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t clock_source;
    uint8_t usb_speed;
    uint8_t ethernet;
    uint8_t wifi;
    uint8_t ram_size;
    uint8_t qspi_size;
    uint8_t video;
    uint8_t crypto;
    uint8_t extclock;
} PortentaBootloaderInfo;