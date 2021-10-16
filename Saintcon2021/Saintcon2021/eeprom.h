/*
 * eeprom.h
 *
 * Created: 8/19/2021 4:48:48 PM
 *  Author: Professor Plum
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_

void eeprom_init();
void eeprom_erase();
void eeprom_load_state();
void eeprom_save_state();

//TODO: NVMCTRL.SBLK=4; NVMCTRL.PSZ=1;



#endif /* EEPROM_H_ */