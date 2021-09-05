/*
 * machine_common.h
 *
 * Created: 8/21/2021 9:29:04 PM
 *  Author: Professor Plum
 */ 


#ifndef MACHINE_COMMON_H_
#define MACHINE_COMMON_H_

#include <stdint.h>

#define MODULE_CABLE_BASE (1<<0)
#define MODULE_BASE_LEFT (1<<1)
#define MODULE_BASE_RIGHT (1<<2)
#define MODULE_GAUGES (1<<3)
#define MODULE_PANNEL_A (1<<4)
#define MODULE_PANNEL_B (1<<5)
#define MODULE_PANNEL_C (1<<6)
#define MODULE_CHOPPER (1<<7)
#define MODULE_GEARS (1<<8)
#define MODULE_PROJECTOR (1<<9)
#define MODULE_HAMMER (1<<10)
#define MODULE_CONVEYER (1<<11)
#define MODULE_ARM (1<<12)
#define MODULE_MOTOR (1<<13)
#define MODULE_SMOKE (1<<14)
#define MODULE_WIRES (1<<15)
#define MODULE_PIPES (1<<16)
#define MODULE_BROKEN (1<<17)
#define MODULE_HOLOGRAM (1<<18)
#define MODULE_ANTENNA (1<<19)
#define MODULE_TANK (1<<20)
#define MODULE_SPARK (1<<21)
#define MODULE_DRONE (1<<22)
#define MODULE_EXTRAS (1<<23)

#define MODULE_COUNT    24

typedef enum {
	wire,
	gear,
	scrap,
	circuit,
	screen,
	metal,
	lens,
	fluid,
	engine,
	flux,
	tesla,
	fuse,
	none
} part_enum;

typedef struct requirement {
	part_enum part;
	uint8_t count;
} requirement;

typedef struct module {
	uint32_t id, prereqs;
	char name[20];
	requirement reqparts[4];
	int src_x, src_y, src_w, src_h;
	int dest_x, dest_y;
} module;

extern const module module_info[MODULE_COUNT];
extern const char *part_names[12];

#endif /* MACHINE_COMMON_H_ */