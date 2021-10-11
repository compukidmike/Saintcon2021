/*
 * machine_common.c
 *
 * Created: 8/21/2021 9:27:07 PM
 *  Author: Professor Plum
 */ 

#include "machine_common.h"

const char *part_names[12] =
{
	"Wire",
	"Gears",
	"Scrap",
	"Circuit B.",
	"LCD Screen",
	"Neosteal",
	"Hololens",
	"Eitr Fluid",
	"Ion Engine",
	"Flux Cap.",
	"Tesla Coil",
	"Ray Fuse"
};

const module module_info[MODULE_COUNT] = {
	{
		MODULE_CABLE_BASE,
		0,
		"Broadband Fulcrum",
		{{wire,3},{scrap,2},{gear,1},{none,0}},
		0, 205, 185, 35,
		16,205
	},
	{
		MODULE_BASE_LEFT,
		MODULE_CABLE_BASE,
		"Subphase Compressor",
		{{scrap,2},{gear,2},{circuit,1},{none,0}},
		67, 166, 52, 40,
		64, 169
	},
	{
		MODULE_BASE_RIGHT,
		MODULE_CABLE_BASE,
		"Cyber Modulator",
		{{scrap,2},{gear,1},{circuit,1},{wire,1}},
		118, 174, 75, 32,
		116, 177
	},
	{
		MODULE_GAUGES,
		MODULE_BASE_LEFT,
		"Unilateral Scimeter",
		{{scrap,1},{gear,1},{circuit,1},{wire,1}},
		34, 141, 33, 64,
		45, 144
	},
	{
		MODULE_PANNEL_A,
		MODULE_GAUGES,
		"Biomass Terminal",
		{{circuit,2},{screen,1},{wire,1},{scrap,1}},
		32, 109, 48, 32,
		32, 112
	},
	{
		MODULE_PANNEL_B,
		MODULE_PANNEL_A,
		"Sinosodal Incubator",
		{{screen,2},{circuit,1},{wire,1},{scrap,1}},
		48, 61, 33, 48,
		48, 64
	},
	{
		MODULE_PANNEL_C,
		MODULE_BASE_RIGHT,
		"AI Training Module",
		{{circuit,1},{screen,1},{wire,1},{scrap,1}},
		162, 125, 32, 49,
		160, 144
	},
	{
		MODULE_CHOPPER,
		MODULE_PANNEL_C,
		"Mass Deconstructor",
		{{metal,1},{gear,2},{scrap,1},{none,0}},
		195, 160, 45, 27,
		192, 132
	},
	{
		MODULE_GEARS,
		MODULE_PANNEL_C,
		"Lunarwave Mechanism",
		{{gear,3},{metal,1},{wire,1},{none,0}},
		162, 77, 32, 47,
		160, 97
	},
	{
		MODULE_PROJECTOR,
		MODULE_BASE_RIGHT | MODULE_BASE_LEFT,
		"Holographic Display",
		{{lens,1},{circuit,2},{scrap,1},{wire,1}},
		99, 136, 62, 30,
		96, 148
	},
	{
		MODULE_HAMMER,
		MODULE_PANNEL_B,
		"Deformation Deltoid",
		{{metal,2},{gear,1},{wire,1},{scrap,1}},
		0, 59, 48, 50,
		1, 67
	},
	{
		MODULE_CONVEYER,
		MODULE_CHOPPER,
		"Mineral Transporter",
		{{gear,4},{wire,2},{none,0},{none,0}},
		200, 102, 40, 42,
		200, 102
	},
	{
		MODULE_ARM,
		MODULE_PANNEL_B,
		"Radiated Dingle Arm",
		{{scrap,1},{gear,2},{none,0},{none,0}},
		64, 0, 70, 61,
		79, 25
	},
	{
		MODULE_MOTOR,
		MODULE_GEARS,
		"Quantium Generator",
		{{gear,2},{wire,3},{none,0},{none,0}},
		164, 55, 32, 22,
		162, 75
	},
	{
		MODULE_SMOKE,
		MODULE_MOTOR,
		"Pneumatic Release",
		{{metal,2},{scrap,2},{none,0},{none,0}},
		199, 54, 32, 48,
		190, 48
	},
	{
		MODULE_WIRES,
		MODULE_ARM,
		"Polymorphic Modem",
		{{wire,4},{none,0},{none,0},{none,0}},
		147, 23, 49, 30,
		93, 37
	},
	{
		MODULE_PIPES,
		MODULE_CHOPPER,
		"Synthetic Harvester",
		{{metal,2},{wire,2},{none,0},{none,0}},
		192, 195, 48, 45,
		191, 159
	},
	{
		MODULE_BROKEN,
		MODULE_GAUGES,
		"Legacy Thermomixer",
		{{scrap,3},{screen,1},{wire,1},{none,0}},
		0, 142, 34, 64,
		11, 145
	},
	{
		MODULE_HOLOGRAM,
		MODULE_PROJECTOR,
		"Leet Hacker Beacon",
		{{fuse,1},{none,0},{none,0},{none,0}},
		102, 61, 48, 75,
		99, 72
	},
	{
		MODULE_ANTENNA,
		MODULE_PANNEL_B,
		"Xenoflux Transducer",
		{{scrap,1},{wire,3},{none,0},{none,0}},
		0, 0, 53, 54,
		27, 12
	},
	{
		MODULE_TANK,
		MODULE_PANNEL_A,
		"Cryogenic Tank",
		{{scrap,1},{fluid,1},{none,0},{none,0}},
		5, 121, 26, 16,
		6, 121
	},
	{
		MODULE_SPARK,
		MODULE_ARM,
		"Acute Arc Conductor",
		{{tesla,2},{wire,1},{scrap,1},{none,0}},
		153, 0, 48, 21,
		99, 4
	},
	{
		MODULE_DRONE,
		0,
		"Antimatter Drone",
		{{engine,1},{scrap,1},{circuit,1},{none,0}},
		207, 0, 32, 54,
		160, 17
	},
	{
		MODULE_EXTRAS,
		MODULE_PANNEL_B,
		"Continuum Alterator",
		{{flux,1},{wire,1},{scrap,1},{none,0}},
		82, 91, 19, 64,
		78, 95
	},
};