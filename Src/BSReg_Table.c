/*
 * BSReg_Table.c
 *
 *  Created on: Aug 14, 2019
 *      Author: user
 */

#include <stdio.h>
#include <string.h>
#include "BSReg_Table.h"
#include "global.h"

#define NO_OF_BOUNDARY_SCAN_REG 	64

extern char tempBuffer[BUFFER_SIZE];

/*
 * This array of BSReg structures defines a lookup table for the different boundary scan registers (BSReg) in the system. 
 * The array is indexed by an enumeration value representing each boundary scan register. 
 * Each element of the array is a BSReg structure, which holds information about a specific boundary scan register,
 * such as its input, output, and control cell numbers, and its pin name.
 */
BSReg	bSRegTable[NO_OF_BOUNDARY_SCAN_REG] = {
		// GPIO_A
		[PA0] = {.inputCellNum = 187, .outputCellNum =  188, .controlCellNum = 189, .pinName = "pa0"},
		[PA1] = {.inputCellNum = 187, .outputCellNum =  185, .controlCellNum = 186, .pinName = "pa1"},
		[PA2] = {.inputCellNum = 181, .outputCellNum =  182, .controlCellNum = 183,	.pinName = "pa2"},
		[PA3] = {.inputCellNum = 178, .outputCellNum =  179, .controlCellNum = 180,	.pinName = "pa3"},
		[PA4] = {.inputCellNum = 175, .outputCellNum =  176, .controlCellNum = 177, .pinName = "pa4"},
		[PA5] = {.inputCellNum = 172, .outputCellNum =  173, .controlCellNum = 174, .pinName = "pa5"},
		[PA6] = {.inputCellNum = 169, .outputCellNum =  170, .controlCellNum = 171, .pinName = "pa6"},
		[PA7] = {.inputCellNum = 166, .outputCellNum =  167, .controlCellNum = 168, .pinName = "pa7"},
		[PA8] = {.inputCellNum = 67, .outputCellNum =  68, .controlCellNum = 69, 	.pinName = "pa8"},
		[PA9] = {.inputCellNum = 64, .outputCellNum =  65, .controlCellNum = 66, 	.pinName = "pa9"},
		[PA10] = {.inputCellNum = 61, .outputCellNum =  62, .controlCellNum = 63, 	.pinName = "pa10"},
		[PA11] = {.inputCellNum = 58, .outputCellNum =  59, .controlCellNum = 60, 	.pinName = "pa11"},
		[PA12] = {.inputCellNum = 55, .outputCellNum =  56, .controlCellNum = 57, 	.pinName = "pa12"},

		// GPIO_B
		[PB0] = {.inputCellNum = 157, .outputCellNum =  158, .controlCellNum = 159, .pinName = "pb0"},
		[PB1] = {.inputCellNum = 154, .outputCellNum =  155, .controlCellNum = 156, .pinName = "pb1"},
		[PB2] = {.inputCellNum = 151, .outputCellNum =  152, .controlCellNum = 153, .pinName = "pb2"},


		[PB5] = {.inputCellNum = 19, .outputCellNum =  20, .controlCellNum = 21, .pinName = "pb5"},
		[PB6] = {.inputCellNum = 16, .outputCellNum =  17, .controlCellNum = 18, .pinName = "pb6"},
		[PB7] = {.inputCellNum = 13, .outputCellNum =  14, .controlCellNum = 15, .pinName = "pb7"},
		[PB8] = {.inputCellNum = 9, .outputCellNum =  10, .controlCellNum = 11,  .pinName = "pb8"},
		[PB9] = {.inputCellNum = 6, .outputCellNum =  7, .controlCellNum = 8, 	 .pinName = "pb9"},
		[PB10] = {.inputCellNum = 121, .outputCellNum =  122, .controlCellNum = 123 , .pinName = "pb10"},
		[PB11] = {.inputCellNum = 118, .outputCellNum =  119, .controlCellNum = 120,  .pinName = "pb11"},
		[PB12] = {.inputCellNum = 115, .outputCellNum =  116, .controlCellNum = 117,  .pinName = "pb12"},
		[PB13] = {.inputCellNum = 112, .outputCellNum =  113, .controlCellNum = 114,  .pinName = "pb13"},
		[PB14] = {.inputCellNum = 109, .outputCellNum =  110, .controlCellNum = 111,  .pinName = "pb14"},
		[PB15] = {.inputCellNum = 112, .outputCellNum =  113, .controlCellNum = 114,  .pinName = "pb15"},

		// GPIO_C
		[PC13] = {.inputCellNum = 214, .outputCellNum =  215, .controlCellNum = 216,  .pinName = "pc13"},
		[PC14] = {.inputCellNum = 211, .outputCellNum =  212, .controlCellNum = 213,  .pinName = "pc14"},
		[PC15] = {.inputCellNum = 208, .outputCellNum =  209, .controlCellNum = 210,  .pinName = "pc15"},

		// Oscillator
		[oscOutPD0] = {.inputCellNum = 205, .outputCellNum =  206, .controlCellNum = 207,  .pinName = "oscOutPD0"},
		[oscOutPD1] = {.inputCellNum = 202, .outputCellNum =  203, .controlCellNum = 204,  .pinName = "oscOutPD1"},

};


/*
 * This function returns a BSReg object from the lookup table based on a string input. 
 * It first skips any whitespace in the input string, then checks each element of the lookup table to see if 
 * the pinName matches with the input string.
 *
 * Parameters:
 *   - *str: A string that needs to be matched with a pinName in the lookup table.
 *
 * Returns:
 *   - A BSReg object fetched from the lookup table based on the input string.
 */
BSReg getBSRegFromTable(char *str){
	skipWhiteSpaces(&str);
	for(int i = 0; i<NO_OF_BOUNDARY_SCAN_REG;i++){
		if(!(strcasecmp(bSRegTable[i].pinName, str))){
			return (bSRegTable[i]);
		}
	}
}


/*
 * This function checks if a given string is a valid boundary scan register. 
 * It first skips any whitespace in the input string, then checks each element of the lookup table to see if 
 * the pinName matches with the input string. If a match is found, the function returns 1, 
 * otherwise, it writes an error message to the temporary buffer and returns 0.
 *
 * Parameters:
 *   - *str: A string that needs to be validated as a pinName in the lookup table.
 *
 * Returns:
 *   - 1 if the input string is a valid pinName in the lookup table.
 *   - 0 if the input string is not a valid pinName in the lookup table.
 */
int isBSRegValid(char *str){
	skipWhiteSpaces(&str);
	for(int i = 0; i<NO_OF_BOUNDARY_SCAN_REG;i++){
		if(!(strcasecmp(bSRegTable[i].pinName, str))){
			return 1;
		}
	}
	sprintf(tempBuffer, "Invalid Boundary Scan Register(pin) '%s' received\n", str);
	return 0;

}
