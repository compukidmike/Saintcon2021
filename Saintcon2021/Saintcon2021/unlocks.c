/*
 * unlocks.c
 *
 * Created: 8/26/2021 10:01:41 PM
 *  Author: Professor Plum
 */ 

#include <stdbool.h>
#include "main.h"

#define COMBO_COUNT	12

#pragma GCC push_options
#pragma GCC optimize ("O0")

const static char* _hidden_combo = "Lock Combo: 20-10-0";

#pragma GCC pop_options



const uint8_t combo_locks[COMBO_COUNT][3] = {
{10, 20, 26}, // Keynote
{30, 12, 34}, // Badge art
{20, 10,  0}, // Firmware
{38,  8, 18}, // Flash
{ 3, 21, 35}, // Master lock puzzle
{28, 18,  4}, // Master lock serial number A31304
{23, 25, 15}, // Lanyard
{12, 26,  2},
{30, 32, 14},
{32, 10, 32},
{10, 32, 26},
{16, 38,  4},
};
//28, 21, 8 Wrong but Give hint


//Fudging user input if they are close to help improve user experience
int comboFudge(int round, int value, int lastvalue) {
	if (round > 0) {
		for (int i=0; i<COMBO_COUNT; ++i) {
			if (lastvalue == combo_locks[i][round-1]) 
				if ((value+1 == combo_locks[i][round]) ||
					(value-1 == combo_locks[i][round]))
					return combo_locks[i][round];
		}	
	}
	return value;
}

// -1 Already Used
//  0 Invalid
//	1 Valid (marks used)
// -2 Hint 
int isValidCombo(uint8_t l1, uint8_t l2, uint8_t l3) {
	for (int i=0; i<COMBO_COUNT; ++i) {
		if ((combo_locks[i][0] == l1) &&
			(combo_locks[i][1] == l2) &&
			(combo_locks[i][2] == l3)) { //found a match
				if (g_state.combos_bitmask & (1<<i)) {
					//already found this one
					return -1;
				}
				g_state.combos_bitmask |= (1<<i);
				return 1;
		}
		if ((0x1C == l1) &&
		(0x15 == l2) &&
		(0x8 == l3)) {
			return -2;
		}
		
	}
	return 0;
}