/**
  ******************************************************************************
  * @file    anx7625.h
  * @author  
  * @version 
  * @date    
  * @brief 	
  ******************************************************************************
  */

#ifndef _ANX7625_H
#define _ANX7625_H

/* Includes ------------------------------------------------------------------*/
extern "C" {
#include <edid.h>
}

/* Exported functions --------------------------------------------------------*/
int 	anx7625_dp_start(uint8_t bus, const struct edid *edid, enum edid_modes mode = EDID_MODE_AUTO);
int 	anx7625_dp_get_edid(uint8_t bus, struct edid *out);
int 	anx7625_init(uint8_t bus);
void 	anx7625_wait_hpd_event(uint8_t bus);
int 	anx7625_get_cc_status(uint8_t bus, uint8_t *cc_status);
int 	anx7625_read_system_status(uint8_t bus, uint8_t *sys_status);
bool 	anx7625_is_power_provider(uint8_t bus);

#endif  /* _ANX7625_H */
