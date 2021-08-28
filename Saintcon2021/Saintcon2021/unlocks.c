/*
 * unlocks.c
 *
 * Created: 8/26/2021 10:01:41 PM
 *  Author: Professor Plum
 */ 

#include <stdbool.h>
#include "main.h"

#define COMBO_COUNT	12

const char* _hidden_combo = "Lock Combo: 20-10-0";

const uint8_t combo_locks[COMBO_COUNT][3] = {
{10, 20, 26}, // Keynote
{30, 12, 34}, // Badge art
{20, 10,  0}, // Firmware
{38,  8, 18}, // Flash
{ 3, 21, 35}, // Master lock puzzle
{28, 18,  4}, // Master lock serial number A31304
{23, 25, 15},
{12, 26,  2},
{30, 32, 14},
{32, 10, 32},
{10, 32, 26},
{16, 38,  4},
};


// -1 Already Used
//  0 Invalid
//	1 Valid (marks used)
int isValidCombo(uint8_t l1, uint8_t l2, uint8_t l3) {
	for (int i=0; i<COMBO_COUNT; ++i) {
		if ((combo_locks[i][0] == l1) &&
			(combo_locks[i][1] == l2) &&
			(combo_locks[i][1] == l3)) { //found a match
				if (g_state.combos_bitmask & (1<<i)) {
					//already found this one
					return -1;
				}
				g_state.combos_bitmask |= (1<<i);
				return 1;
		}
	}
	return 0;
}