/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V0.1

Filename : 

Project  : ANX7625 

Created  : 20 Sept. 2016

Devices  : ANX7625

Toolchain: Keil
 
Description:

Revision History:

******************************************************************************/

// TODO: This header file was copied from ANX7688 project, which supports USB PD 2.0.
// TODO: However, ANX7625 supports USB PD 3.0. It's assumed that this header file needs some update/review.

#ifndef __PD_H__
#define __PD_H__

/**
  * @brief message header
  */
union tagMessageHeader
{
    struct tagAllMessageHeader
    {// SOP*
        unsigned char MessageType           :4; // Message Type
        unsigned char Rsvd1                 :1; // Reserved
        unsigned char                       :1; // 
        unsigned char SpecificationRevision :2; // Specification Revision
        unsigned char                       :1; // 
        unsigned char MessageID             :3; // MessageID
        unsigned char NumberOfDataObjects   :3; // Number of Data Objects
        unsigned char Rsvd2                 :1; // Reserved
    }all;
    struct tagSOPMessageHeader
    {// SOP only
        unsigned char MessageType           :4; // Message Type
        unsigned char Rsvd1                 :1; // Reserved
        unsigned char PortDataRole          :1; // Port Data Role
        unsigned char SpecificationRevision :2; // Specification Revision
        unsigned char PortPowerRole         :1; // Port Power Role
        unsigned char MessageID             :3; // MessageID
        unsigned char NumberOfDataObjects   :3; // Number of Data Objects
        unsigned char Rsvd2                 :1; // Reserved
    }SOP;
    struct tagSOP1MessageHeader
    {// SOP'
        unsigned char MessageType           :4; // Message Type
        unsigned char Rsvd1                 :1; // Reserved
        unsigned char Rsvd2                 :1; // Reserved
        unsigned char SpecificationRevision :2; // Specification Revision
        unsigned char CablePlug             :1; // Cable Plug
        unsigned char MessageID             :3; // MessageID
        unsigned char NumberOfDataObjects   :3; // Number of Data Objects
        unsigned char Rsvd3                 :1; // Reserved
    }SOP1;
    struct tagSOP2MessageHeader
    {// SOP"
        unsigned char MessageType           :4; // Message Type
        unsigned char Rsvd1                 :1; // Reserved
        unsigned char Rsvd2                 :1; // Reserved
        unsigned char SpecificationRevision :2; // Specification Revision
        unsigned char CablePlug             :1; // Cable Plug
        unsigned char MessageID             :3; // MessageID
        unsigned char NumberOfDataObjects   :3; // Number of Data Objects
        unsigned char Rsvd3                 :1; // Reserved
    }SOP2;
};

/**
  * @brief message header bit field
  */
#define NUMBER_OF_DATA_OBJECTS  12
#define MESSAGE_ID              9
#define PORT_POWER_ROLE         8
#define CABLE_PLUG              8
#define SPECIFICATION_REVISION  6
#define PORT_DATA_ROLE          5
#define MESSAGE_TYPE            0

#define PD_HEADER_SIZE              2
#define PD_DO_SIZE                  4
#define PD_VDM_HEADER_SIZE          PD_DO_SIZE
#define PD_DO_MAX                   7

/**
  * @brief Protocol revision
  */
#define PD_REV10    0x00
#define PD_REV20    0x01

/**
  * @brief Port Power Role
  */
#define PD_POWER_ROLE_SINK      0
#define PD_POWER_ROLE_SOURCE    1

/**
  * @brief Port Data Role
  */
#define PD_DATA_ROLE_UFP        0
#define PD_DATA_ROLE_DFP        1

/**
  * @brief Control Message type
  */
/* 0 Reserved */
#define PD_CTRL_GOOD_CRC        1
#define PD_CTRL_GOTO_MIN        2
#define PD_CTRL_ACCEPT          3
#define PD_CTRL_REJECT          4
#define PD_CTRL_PING            5
#define PD_CTRL_PS_RDY          6
#define PD_CTRL_GET_SOURCE_CAP  7
#define PD_CTRL_GET_SINK_CAP    8
#define PD_CTRL_DR_SWAP         9
#define PD_CTRL_PR_SWAP         10
#define PD_CTRL_VCONN_SWAP      11
#define PD_CTRL_WAIT            12
#define PD_CTRL_SOFT_RESET      13
/* 14-15 Reserved */

/**
  * @brief Data Message type
  */
/* 0 Reserved */
#define PD_DATA_SOURCE_CAP  1
#define PD_DATA_REQUEST     2
#define PD_DATA_BIST        3
#define PD_DATA_SINK_CAP    4
/* 5-14 Reserved */
#define PD_DATA_VENDOR_DEF  15

/**
  * @brief PDO : Power Data Object
  * 1. The vSafe5V Fixed Supply Object shall always be the first object.
  * 2. The remaining Fixed Supply Objects,
  *    if present, shall be sent in voltage order; lowest to highest.
  * 3. The Battery Supply Objects,
  *    if present shall be sent in Minimum Voltage order; lowest to highest.
  * 4. The Variable Supply (non battery) Objects,
  *    if present, shall be sent in Minimum Voltage order; lowest to highest.
  */
#define PDO_TYPE    30
    
    
#define PDO_FIXED_DUAL_ROLE     29 /* Dual role device */
#define PDO_FIXED_SUSPEND       28 /* USB Suspend supported */
#define PDO_FIXED_EXTERNAL      27 /* Externally powered */
#define PDO_FIXED_COMM_CAP      26 /* USB Communications Capable */
/* 22-24 Reserved */
#define PDO_FIXED_PEAK_CURR     20 /* [21..20] Peak current */
#define PDO_FIXED_VOLT          10 /* Voltage in 50mV units */
#define PDO_FIXED_CURR          0  /* Max current in 10mA units */

#define PDO_FIXED(mV, mA, flags)    (PDO_TYPE_FIXED<<PDO_TYPE                       \
                                    | (flags)                                       \
                                    | (unsigned long)(mV) / 50 << PDO_FIXED_VOLT    \
                                    | (unsigned long)(mA) / 10 << PDO_FIXED_CURR)

union tagAllPDOs
{
    struct tagPDO
    {
        unsigned char       :8;
        unsigned char       :8;
        unsigned char       :8;
        unsigned char       :6;
        unsigned char Type  :2;
            #define PDO_TYPE_FIXED      0
            #define PDO_TYPE_BATTERY    1
            #define PDO_TYPE_VARIABLE   2
            #define PDO_TYPE_MASK       3
    }PDO;
    struct tagFixedPDOSource
    {
        unsigned char CurrentL8     :8;
        unsigned char CurrentH2     :2;
        unsigned char VoltageL6     :6;
        unsigned char VoltageH4     :4;
        unsigned char PeakCurrent   :2;
        unsigned char Rsvd1         :2; // Reserved
        unsigned char Rsvd2         :1; // Reserved
        unsigned char DataRoleSwap  :1;
        unsigned char CommCap       :1;
        unsigned char External      :1;
        unsigned char Suspend       :1;
        unsigned char DualRole      :1;
        unsigned char Type          :2;
    }FixedSource;
    struct tagVariablePDOSource
    {
        unsigned char MaxCurrentL8  :8;
        unsigned char MaxCurrentH2  :2;
        unsigned char MinVoltageL6  :6;
        unsigned char MinVoltageH4  :4;
        unsigned char MaxVoltageL4  :4;
        unsigned char MaxVoltageH6  :6;
        unsigned char Type          :2;
    }VariableSource;
    struct tagBatteryPDOSource
    {
        unsigned char MaxPowerL8    :8;
        unsigned char MaxPowerH2    :2;
        unsigned char MinVoltageL6  :6;
        unsigned char MinVoltageH4  :4;
        unsigned char MaxVoltageL4  :4;
        unsigned char MaxVoltageH6  :6;
        unsigned char Type          :2;
    }BatterySource;
    struct tagFixedPDOSink
    {
        unsigned char CurrentL8     :8;
        unsigned char CurrentH2     :2;
        unsigned char VoltageL6     :6;
        unsigned char VoltageH4     :4;
        unsigned char Rsvd1         :4; // Reserved
        unsigned char Rsvd2         :1; // Reserved
        unsigned char DataRoleSwap  :1;
        unsigned char CommCap       :1;
        unsigned char External      :1;
        unsigned char HigherCap     :1;
        unsigned char DualRole      :1;
        unsigned char Type          :2;
    }FixedSink;
    struct tagBatteryPDOSink
    {
        unsigned char PowerL8       :8;
        unsigned char PowerH2       :2;
        unsigned char MinVoltageL6  :6;
        unsigned char MinVoltageH4  :4;
        unsigned char MaxVoltageL4  :4;
        unsigned char MaxVoltageH6  :6;
        unsigned char Type          :2;
    }BatterySink;
};

union tagAllRDOs
{
    struct tagRDO
    {
        unsigned char           :8;
        unsigned char           :2;
        unsigned char           :6;
        unsigned char           :4;
        unsigned char           :4;
        unsigned char           :1;
        unsigned char           :1;
        unsigned char           :1;
        unsigned char GiveBack  :1;
        unsigned char ObjPos    :3;
        unsigned char           :1;
    }RDO;
    struct tagFixedRDO
    {// Fixed and Variable Request Data Object without GiveBack Support
        unsigned char MaxOpCurrL8   :8;
        unsigned char MaxOpCurrH2   :2;
        unsigned char OpCurrL6      :6;
        unsigned char OpCurrH4      :4;
        unsigned char Rsvd1         :4; // Reserved
        unsigned char NoSuspend     :1;
        unsigned char CommCap       :1;
        unsigned char MismatchCap   :1;
        unsigned char GiveBack      :1;
        unsigned char ObjPos        :3;
        unsigned char Rsvd2         :1; // Reserved
    }FixedRDO;
    struct tagFixedRDOGiveBack
    {// Fixed and Variable Request Data Object with GiveBack Support
        unsigned char MinOpCurrL8   :8;
        unsigned char MaxOpCurrH2   :2;
        unsigned char OpCurrL6      :6;
        unsigned char OpCurrH4      :4;
        unsigned char Rsvd1         :4; // Reserved
        unsigned char NoSuspend     :1;
        unsigned char CommCap       :1;
        unsigned char MismatchCap   :1;
        unsigned char GiveBack      :1;
        unsigned char ObjPos        :3;
        unsigned char Rsvd2         :1; // Reserved
    }FixedRDOGiveBack;

};

/**
  * BDO : BIST Data Object
  */
struct tagBDO
{
    unsigned char IntL8         :8;
    unsigned char IntH8         :8;
    unsigned char Rsvd1         :8; // Reserved
    unsigned char Rsvd2         :4; // Reserved
    unsigned char RequestType   :4;
        #define BIST_RECEIVER_MODE      0
        #define BIST_TRANSMIT_MODE      1
        #define RETURNED_BIST_COUNTERS  2
        #define BIST_CARRIER_MODE0      3
        #define BIST_CARRIER_MODE1      4
        #define BIST_CARRIER_MODE2      5
        #define BIST_CARRIER_MODE3      6
        #define BIST_EYE_PATTERN        7
        #define BIST_TEST_DATA          8
};

/**
  * @brief build message header
  */
#define PD_HEADER(MessageType, MessageID, NumberOfDataObjects)  ((NumberOfDataObjects)<<NUMBER_OF_DATA_OBJECTS  \
                                                                | MessageID<<MESSAGE_ID                         \
                                                                | PD_POWER_ROLE_SOURCE<<PORT_POWER_ROLE         \
                                                                | PD_REV20<<SPECIFICATION_REVISION              \
                                                                | PD_DATA_ROLE_DFP<<PORT_DATA_ROLE              \
                                                                | MessageType<<MESSAGE_TYPE)
/**
  * @brief structured VDM header bit field
  */
#define VDM_SVID            16
#define VDM_TYPE            15
#define VDM_VERSION         13
/* 11-12 Reserved */
#define VDM_OBJECT_POSITION 8
#define VDM_COMMAND_TYPE    6
/* 5 Reserved */
#define VDM_COMMAND         0

#define SVID_ANALOGIX       0x1F29
#define SVID_GOOGLE         0x18D1
#define SVID_PD             0xFF00
#define SVID_DP             0xFF01
#define SVID_MICROSOFT      0x045E
#define SVID_LENOVO         0x17EF
#define SVID_BIZLINK        0x06C4
#define SVID_PARADE         0x1DA0
#define SVID_NXP            0x1FC9
#define SVID_TI             0x0451
#define SVID_CYPRESS        0x04B4
#define SVID_VIA            0x2109
#define SVID_ETRON          0x1E4E
#define SVID_UNASSIGNED     0x0000

#define VDM_STRUCTURED      1

#define VDM_VERSION_10      0
#define VDM_VERSION_20      1


#define VDM_CMD_DISCOVER_IDENTITY   1
#define VDM_CMD_DISCOVER_SVIDS      2
#define VDM_CMD_DISCOVER_MODES      3
#define VDM_CMD_ENTER_MODE          4
#define VDM_CMD_EXIT_MODE           5
#define VDM_CMD_ATTENTION           6
/* 7-15 Reserved */
#define VDM_CMD_DP_STATUS_UPDATE    16
#define VDM_CMD_DP_CONFIG           17
/* 18-31 Reserved for DP_SID use */

/**
  * @brief VDM header
  */
struct tagVDMHeader
{
    unsigned char Command               :5; // Command
    unsigned char Rsvd1                 :1; // Reserved
    unsigned char CommandType           :2; // Command Type
        #define VDM_INITIATOR       0
        #define VDM_RESPONDER_ACK   1
        #define VDM_RESPONDER_NAK   2
        #define VDM_RESPONDER_BUSY  3
    unsigned char ObjectPosition        :3; // Object Position
    unsigned char Rsvd2                 :2; // Reserved
    unsigned char VDMVersion            :2; // Structured VDM Version
    unsigned char VDMType               :1; // VDM Type
    unsigned char SVIDL8                :8; // Standard or Vendor ID (SVID)
    unsigned char SVIDH8                :8;
};

/**
  * @brief build structured VDM header
  */
#define VDM_HEADER(Command, CommandType, ObjectPosition)    ((SVID_ANALOGIX)<<VDM_SVID              \
                                                            | 1<<VDM_TYPE                           \
                                                            | (VDM_VERSION_10)<<VDM_VERSION         \
                                                            | (ObjectPosition)<<VDM_OBJECT_POSITION \
                                                            | (CommandType)<<VDM_COMMAND_TYPE       \
                                                            | (Command)<<VDM_COMMAND)
#define VDM_HEADER_PD_SID(Command, CommandType, ObjectPosition)    ((SVID_PD)<<VDM_SVID             \
                                                            | 1<<VDM_TYPE                           \
                                                            | (VDM_VERSION_10)<<VDM_VERSION         \
                                                            | (ObjectPosition)<<VDM_OBJECT_POSITION \
                                                            | (CommandType)<<VDM_COMMAND_TYPE       \
                                                            | (Command)<<VDM_COMMAND)

/**
  * @brief Discover Identity ID header
  */
struct tagIDHeader
{
    unsigned char VIDL8         :8; // USB Vendor ID
    unsigned char VIDH8         :8;
    unsigned char Rsvd1         :8; // Reserved
    unsigned char Rsvd2         :2; // Reserved
    unsigned char Modal         :1; // Modal Operation Supported
    unsigned char ProductType   :3; // Product Type
        #define ID_UNDEFINED        0 // Undefined
        #define ID_HUB              1 // Hub
        #define ID_PERIPHERAL       2 // Peripheral
        #define ID_PASSIVE_CABLE    3 // Passive Cable
        #define ID_ACTIVE_CABLE     4 // Active Cable
        #define ID_AMA              5 // Alternate Mode Adapter (AMA)
        /* 111b..110b ¨C Reserved, shall not be used. */
    unsigned char USBDevice     :1; // Data Capable as a USB Device
    unsigned char USBHost       :1; // Data Capable as a USB Host
};

/**
  * @brief Product VDO
  */
struct tagProductVDO
{
    unsigned char bcdDeviceL8   :8;
    unsigned char bcdDeviceH8   :8; // 
    unsigned char USBProductIDL8:8;
    unsigned char USBProductIDH8:8;
};

/**
  * @brief Cert Stat VDO
  */
struct tagCertStatVDO
{
    unsigned char TIDL8         :8; // Test ID (TID)
    unsigned char TIDM8         :8;
    unsigned char TIDH4         :4;
    unsigned char Rsvd1         :4; // Reserved
    unsigned char Rsvd2         :8; // Reserved
};

/**
  * @brief Alternate Mode Adapter (AMA) VDO
  */
struct tagAMAVDO
{
    unsigned char USBSS         :3; // USB Superspeed Signaling Support
    unsigned char VBUS_REQ      :1; // VBUS required
    unsigned char VCONN_REQ     :1; // VCONN required
    unsigned char VCONN_PWR     :3; // VCONN power
    unsigned char SSRX2         :1; // SSRX2 Directionality Support
    unsigned char SSRX1         :1; // SSRX1 Directionality Support
    unsigned char SSTX2         :1; // SSTX2 Directionality Support
    unsigned char SSTX1         :1; // SSTX1 Directionality Support
    unsigned char Rsvd1         :4; // Reserved
    unsigned char Rsvd2         :8; // Reserved
    unsigned char FWVer         :4; // Firmware Version
    unsigned char HWVer         :4; // Hardware Version
};

/**
  * @brief Discover SVIDs Responder VDO
  */
struct tagSVIDSVDO
{
    unsigned char SVID1L8   :8; // SVID n+1
    unsigned char SVID1H8   :8;
    unsigned char SVID0L8   :8; // SVID n
    unsigned char SVID0H8   :8;
};

#define AMA_VCONN_PWR_1W   0
#define AMA_VCONN_PWR_1W5  1
#define AMA_VCONN_PWR_2W   2
#define AMA_VCONN_PWR_3W   3
#define AMA_VCONN_PWR_4W   4
#define AMA_VCONN_PWR_5W   5
#define AMA_VCONN_PWR_6W   6

#define AMA_USBSS_U2_ONLY  0
#define AMA_USBSS_U31_GEN1 1
#define AMA_USBSS_U31_GEN2 2
#define AMA_USBSS_BB_ONLY  3

#define PD_T_UNIT_INTERVAL  3.7 /* between 3.03us and 3.70us */
// 64-bit preamble + (SOP(4 x 4-bit K-code) + (2-byte header + 7 x 4-byte DO + 4-byte CRC) x 8 + EOP(4-bit K-code)) x 5 / 4
#define PD_T_PACKET_MAX (unsigned int)((64 + (4 * 4 + (2 + 4 * 7 + 4) * 8 + 4) * 5 / 4) * PD_T_UNIT_INTERVAL) /* between 429x3.03=1300us and 429x3.7=1587us */

#define PD_T_PREAMBLE_SOP   (unsigned int)((64 + 4 * 5) * PD_T_UNIT_INTERVAL)
#define PD_T_GOODCRC    (unsigned int)((64 + (4 * 4 + (2 + 4) * 8 + 4) * 5 / 4) * PD_T_UNIT_INTERVAL) /* between 149x3.03=452us and 149x3.7=552us */
/* Timers (USB PD Spec Rev2.0 Table 6-30)*/
#define PD_T_NO_RESPONSE            5500000UL   /* between 4.5s and 5.5s */
#define PD_T_RECEIVE                1100        /* between 0.9ms and 1.1ms */
#define PD_T_TRANSMIT               195         /* MAX 195us */

#define PD_T_TYPE_C_SEND_SOURCE_CAP 110000UL    /* between 100ms and 200ms */
#define PD_T_TYPE_C_SEND_SOURCE_CAP_MIN 100000UL    /* MIN 100ms */
#define PD_T_TYPE_C_SEND_SOURCE_CAP_MAX 200000UL    /* MAX 200ms */
#define PD_T_TYPE_C_SINK_WAIT_CAP       250000UL /* between 210ms and 250ms */
#define PD_T_TYPE_C_SINK_WAIT_CAP_MAX   465000UL /* between 310ms and 620ms */
// #define PD_T_SOURCE_ACTIVITY   (45*MSEC) /* between 40ms and 50ms */
#define PD_T_SENDER_RESPONSE        27000UL       /* between 24ms and 30ms */
#define PD_T_VDM_SENDER_RESPONSE    30000UL       /* between 24ms and 30ms */
#define PD_T_VDM_WAIT_MODE_ENTRY    100000UL    /* max 100ms */
#define PD_T_VDM_WAIT_MODE_EXIT     100000UL    /* max 100ms */
#define PD_T_PS_TRANSITION_MAX      500000UL    /* between 450ms and 550ms */
#define PD_T_PS_SOURCE_ON           400000UL    /* between 390ms and 480ms */
#define PD_T_PS_SOURCE_ON_MAX       480000UL    /* between 390ms and 480ms */
// TD.PD.CP.E1 PSSourceOffTimer Deadline, 800ms
// TD.PD.CP.E2 PSSourceOffTimer Timeout, 550ms
#define PD_T_PS_SOURCE_OFF          780000UL    /* between 750ms and 920ms */
#define PD_T_PS_SOURCE_OFF_MAX      920000UL    /* between 750ms and 920ms */
#define PD_T_SWAP_SOURCE_START      20000       /* min 20ms */
#define PD_T_VCONN_SOURCE_ON_MAX    100000      /* max 100ms */
#define PD_T_PS_HARD_RESET          25000       /* between 25ms and 35ms */
#define PD_T_SRC_RECOVER            830000UL    /* between 0.66s and 1s */
#define PD_T_SRC_RECOVER_MAX        1000000UL
#define PD_T_DR_SWAP_HARD_RESET     15000       /* max 15ms */
#define PD_T_BIST_CONT_MODE         45000       /* between 30ms and 60ms */
#define PD_T_SRC_TRANSITION         30000       /* between 25ms and 35ms */
#define PD_T_DISCOVER_IDENTITY      45000       /* between 40ms and 50ms */
#define PD_T_CABLE_MESSAGE          800         /* min 750us */
// #define PD_T_DRP_HOLD         (120*MSEC) /* between 100ms and 150ms */
// #define PD_T_DRP_LOCK         (120*MSEC) /* between 100ms and 150ms */
// /* DRP_SNK + DRP_SRC must be between 50ms and 100ms with 30%-70% duty cycle */
// #define PD_T_DRP_SNK           (40*MSEC) /* toggle time for sink DRP */
// #define PD_T_DRP_SRC           (30*MSEC) /* toggle time for source DRP */
/* PD counter definitions */
#define PD_MESSAGE_ID_COUNT 7
#define PD_RETRY_COUNT      2
#define PD_HARD_RESET_COUNT 2
#define PD_CAPS_COUNT       50
#define PD_DISCOVER_IDENTITY_COUNT  20

#define VSAFE0V_MAX     800 // vSafe0V, in mV, 0.8V

#endif  /* __PD_H__ */

