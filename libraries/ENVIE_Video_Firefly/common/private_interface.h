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

#ifndef PRIVATE_INTERFACE_H
#define PRIVATE_INTERFACE_H

#define IF_RAM xdata

#define PD_ONE_DATA_OBJECT_SIZE  4
#define PD_MAX_DATA_OBJECT_NUM  7
#define VDO_SIZE (PD_ONE_DATA_OBJECT_SIZE * PD_MAX_DATA_OBJECT_NUM)

#define PDO_FIXED_FLAGS (PDO_FIXED_DUAL_ROLE | PDO_FIXED_DATA_SWAP|PDO_FIXED_EXTERNAL)


/*5000mv voltage*/
#define PD_VOLTAGE_5V 5000

#define PD_MAX_VOLTAGE_20V 20000
#define PD_MAX_VOLTAGE_21V 21000
/*0.9A current */
#define PD_CURRENT_900MA   900
#define PD_CURRENT_1500MA 1500
#define PD_CURRENT_100MA 100

#define PD_CURRENT_3A   3000

#define PD_POWER_15W  15000

#define PD_POWER_60W  60000
				  
//define max request current 900mA  and voltage 5V
#define MAX_REQUEST_VOLTAGE 5000
#define MAX_REQUEST_CURRENT 900

/**
 * @desc:   The interface AP will set(fill) the source capability to Ohio 
 *
 * @param:  
 *              src_caps: PDO buffer pointer of source capability whose bit formats follow the rules:
 *                         it's little-endian, defined in USB PD spec 5.5 Transmitted Bit Ordering
 *                         source capability's specific format defined in USB PD spec 6.4.1 Capabilities Message
 *                          PDO refer to Table 6-4 Power Data Object
 *                          Variable PDO : Table 6-8 Variable Supply (non-battery) PDO  --source
 *                          Battery PDO : Table 6-9 Battery Supply PDO --source 
 *                          Fixed PDO refer to  Table 6-6 Fixed Supply PDO --source
 *                     eg: default5Vsafe src_cap(5V, 0.9A fixed) --> PDO_FIXED(5000,900, PDO_FIXED_FLAGS) 
 *         	     Low address : from current lowes 10bits begin ---------------------------------------------------------------------------------->  High address   
 *                   Name:        MaxCurrent(10mA units)      |            (Voltage 50mV units)          |PeakCur|      reserved      | DataRoleSwap |USBCapable |Externally Powered | USB Suspend | Dual-Role Power  |  Fixed supply 
 *         		 Bit: 	   0 1 2 3 4 5 6 7 8           9  10 11 12 13 14 15              16 17 18 19   20  21   22 23              24      25                     26                  27                            28               29                          30    31
 *                   Value:      0 1 0 1 1 0 1 0 0 0         0   0   1  0   0   1   1               1  0    0   0    0    0    0   0                0       1                     0                      0                             0                1                            0     0 
 *                   Byte:       0x0x5A=900/10uA         0x90                                     0x01                                               0x22
 *                                       
 * 
 *          	   src_caps_size: source capability's size, if the source capability obtains
 *                                   one PDO object, src_caps_size is 4, two PDO objects, src_caps_size is 8.
 *           		
 * @return:  1: success 0: fail 
 *  
 */ 
unsigned char send_src_cap(const unsigned char *src_caps, unsigned char src_caps_size);
unsigned char send_snk_cap(unsigned char *snk_caps, unsigned char snk_caps_size);
unsigned char send_svid(const unsigned char *svid, unsigned char size);
unsigned char send_dp_snk_cfg(const unsigned char *dp_snk_caps, unsigned char dp_snk_caps_size);
unsigned char send_dp_snk_identity(const unsigned char *dp_snk_identity, unsigned char dp_snk_identity_size);
unsigned char send_vdm(const unsigned char *vdm, unsigned char size);
unsigned char send_rdo(const unsigned char *rdo, unsigned char size);
unsigned char send_data_swap(void);
unsigned char send_power_swap(void);
unsigned char send_vconn_swap(void);
unsigned char send_gotomin();
unsigned char send_sop_prime(const unsigned char *sop_prime, unsigned char size);
unsigned char send_sop_double_prime(const unsigned char *sop_double_prime, unsigned char size);
unsigned char send_soft_rst();
unsigned char send_hard_rst();
unsigned char send_accept();
unsigned char send_reject();
unsigned char send_get_pd_status();
unsigned char send_get_sink_cap();
unsigned char send_get_sink_cap_ext();
unsigned char send_get_dp_snk_cap();
unsigned char send_frswap_signal();
#if 1 // debug
unsigned char send_get_var(unsigned char type, unsigned int addr, unsigned char len);
unsigned char send_set_var(unsigned char type, unsigned int addr, unsigned char *buf, unsigned char len);
#endif
#ifdef USE_PD30
unsigned char send_ext_msg(unsigned char is_ext, unsigned char type, unsigned char *pbuf, unsigned char buf_len,unsigned char type_sop);
#endif

unsigned char recv_pd_pwr_object_req_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_dswap_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_sink_caps_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_source_caps_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_cmd_rsp_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_goto_min_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_accept_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_reject_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_dp_alt_enter_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_dp_alt_exit_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_sop_prime_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_sop_double_prime_default_callback(void * para, unsigned char para_len);
unsigned char recv_pd_source_caps_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_sink_caps_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_dswap_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_pswap_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_vdm_defalut_callback(void *para, unsigned char para_len);
unsigned char recv_pd_cmd_rsp_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_dp_sink_identity_default_callback(void *para, unsigned char para_len);
unsigned char recv_pd_svid_default_callback(void *para, unsigned char para_len);
#ifdef USE_PD30
unsigned char recv_ext_msg_callback(unsigned char type, unsigned char *para, unsigned char para_len);
#endif
#if 1 // debug
unsigned char recv_debug_callback(unsigned char type, unsigned char *para, unsigned char para_len);
#endif

char *interface_to_str(unsigned char header_type);
char *result_to_str(unsigned result_type);
void interface_init(void);
void interface_recvd_msg(void);
unsigned char send_msg_process(void);
void send_initialized_setting(bit src_flag);
unsigned char interface_send_msg();
unsigned char interface_send_ctr_msg();
unsigned long change_bit_order(unsigned char *pbuf);

unsigned char i2c_ReadReg(unsigned char dev,unsigned char offset);

/*
* For SKIP highest voltage
* Maximum Voltage for Request Data Object
* 100mv units
*/
#define MAX_VOLTAGE 0x29 // 0x7E:0x29 // 0xD0
/*
* For selection PDO
* Maximum Power for Request Data Object
* 500mW units
*/
#define MAX_POWER 0x2A // 0x7E:0x2A // 0xD1
/*
* For mismatch
* Minimum Power for Request Data Object
* 500mW units
*/
#define MIN_POWER 0x2B // 0x7E:0x2B // 0xD2
/*Show Maximum voltage of RDO*/
#define RDO_MAX_VOLTAGE 0x2C // 0x7E:0x2C // 0xD3
/*Show Maximum Powe of RDO*/
#define RDO_MAX_POWER 0x2D // 0x7E:0x2D // 0xD4
/*Show Maximum current of RDO*/
#define RDO_MAX_CURRENT 0x2E // 0x7E:0x2E // 0xD5

#define FIRMWARE_CTRL 0x2F // 0x7E:0x2F
#define no_skip_check_vbus _BIT0
#define auto_pd_en _BIT1
#define trysrc_en _BIT2
#define trysnk_en _BIT3
#define support_goto_min_power _BIT4
#define slimport_mode_mode _BIT5

#define FW_STATE_MACHINE 0x30 // 0x7E:0x30

#define INT_MASK 0x43 // 0x7E:0x43
/*same with 0x28 interrupt mask*/
#define CHANGE_INT 0x44 // 0x7E:0x44
#define RECEIVED_MSG _BIT0
#define RECEIVED_ACK _BIT1
#define VCONN_CHANGE _BIT2
#define VBUS_CHANGE _BIT3
#define CC_STATUS_CHANGE _BIT4
#define DATA_ROLE_CHANGE _BIT5
#define PR_CONSUMER_GOT_POWER _BIT6
#define DP_HPD_CHANGE _BIT7

#define SYSTEM_STSTUS 0x45 // 0x7E:0x45
/*0: VCONN off; 1: VCONN on*/
#define VCONN_STATUS _BIT2
/*0: vbus off; 1: vbus on*/
#define VBUS_STATUS _BIT3
/*1: host; 0:device*/
#define S_DATA_ROLE _BIT5
/*0: HPD low; 1: HPD high*/
#define HPD_STATUS _BIT7

#define NEW_CC_STATUS 0x46 // 0x7E:0x46


/*control cmd*/
#define interface_pr_swap()  \ 
	do{ \
		InterfaceSendBuf[1] = TYPE_PSWAP_REQ; \
    	return interface_send_ctr_msg( ); \ 
	} while(0)
#define interface_dr_swap() \
	do{ \
		InterfaceSendBuf[1] = TYPE_DSWAP_REQ; \
    	return interface_send_ctr_msg( ); \ 
	} while(0)

    
#define interface_vconn_swap() \
	do{ \
		InterfaceSendBuf[1] = TYPE_VCONN_SWAP_REQ; \
     return interface_send_ctr_msg(  );\
	}while(0)
	
#define interface_get_dp_caps() \
	do{ \
		InterfaceSendBuf[1] = TYPE_DP_SNK_CFG; \
    return interface_send_ctr_msg( ); \
	}while(0)
	
#define interface_send_gotomin() \
	do{ \
		InterfaceSendBuf[1] = TYPE_GOTO_MIN_REQ; \
    return interface_send_ctr_msg(  ); \
	}while(0)
	
#define interface_send_soft_rst() \
	do { \
		InterfaceSendBuf[1] = TYPE_SOFT_RST; \
    return interface_send_ctr_msg( );\
	}while(0)
	
#define interface_send_hard_rst() \
	do{ \
		InterfaceSendBuf[1] = TYPE_HARD_RST; \
    return interface_send_ctr_msg( );\
	}while(0)
	
#define interface_send_restart() \
	do{ \
		InterfaceSendBuf[1] = TYPE_RESTART; \
    return interface_send_ctr_msg( ); \
 	}while(0)
 	
#define interface_send_accept() \
	do{ \
		InterfaceSendBuf[1] = TYPE_ACCEPT; \
    return interface_send_ctr_msg( );\
		}while(0)
		
#define interface_send_reject() \
	do{ \
		InterfaceSendBuf[1] = TYPE_REJECT; \	
    return interface_send_ctr_msg( ); \
    }while(0)
    
 #define interface_get_pd_status()\
 	do{ \
		InterfaceSendBuf[1] = TYPE_PD_STATUS_REQ; \
    return interface_send_ctr_msg( ); \
 		}while(0)
 #define interface_send_dp_enter()\
 	do{ \
		InterfaceSendBuf[1] = TYPE_DP_ALT_ENTER; \
    return interface_send_ctr_msg( ); \
 		}while(0)
 #define interface_send_dp_exit()\
 	do{ \
		InterfaceSendBuf[1] = TYPE_DP_ALT_EXIT; \
    return interface_send_ctr_msg( ); \
 		}while(0)
 		
 #define interface_send_get_sink_cap()\
 	do{ \
		InterfaceSendBuf[1] = TYPE_GET_SNK_CAP; \
 	return interface_send_ctr_msg(); \
 		}while(0)
 		
  #define interface_send_get_sink_cap_ext()\
 	do{ \
		InterfaceSendBuf[1] = TYPE_GET_SINK_CAP_EXT; \
 	return interface_send_ctr_msg(); \
 		}while(0)
 		
#define interface_send_get_dp_snk_cap() \
	do{ \
		InterfaceSendBuf[1] = TYPE_GET_DP_SNK_CAP; \
		return interface_send_ctr_msg(); \
		}while(0)

// send Fast Role Swap signal		
#define interface_send_frswap_signal() \
			do{ \
				InterfaceSendBuf[1] = TYPE_FR_SWAP_SIGNAL; \
				return interface_send_ctr_msg(); \
				}while(0)


/* check soft interrupt happens or not */ 
#define is_msg_recved_interrupt() (ReadReg(CHANGE_INT) & RECEIVED_MSG)
#define is_vbus_changed_interrupt() (ReadReg(CHANGE_INT) & VBUS_CHANGE)
#define is_got_power() (ReadReg(CHANGE_INT) & PR_CONSUMER_GOT_POWER)

/* clear the Ohio's soft  interrupt bit */  
#define clear_recvd_msg_interrupt() WriteReg(CHANGE_INT, (ReadReg(CHANGE_INT) & (~RECEIVED_MSG)))
#define clear_vbus_changed_interrupt() WriteReg(CHANGE_INT, (ReadReg(CHANGE_INT) &(~VBUS_CHANGE)))
#define clear_got_power() WriteReg(CHANGE_INT, (ReadReg(CHANGE_INT) &(~PR_CONSUMER_GOT_POWER)))

#define RESPONSE_REQ_TYPE() InterfaceRecvBuf[2]
#define RESPONSE_REQ_RESULT() InterfaceRecvBuf[3]

////////////////////////////////////////////////////////////////////////////////
/*Comands status*/
enum interface_status {
    CMD_SUCCESS,
    CMD_REJECT,
    CMD_FAIL,
    CMD_BUSY,
    CMD_STATUS
};

#define INTERFACE_SEND_BUF_SIZE 32
#define INTERFACE_RECV_BUF_SIZE 32

#endif  // end of PRIVATE_INTERFACE_H definition
