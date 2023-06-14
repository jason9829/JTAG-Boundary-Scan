#include <stdio.h>
#include <inttypes.h>
#include "TAP_LookUpTable.h"
#include "TAP.h"
#include "global.h"
#include "TAP_StateTable.h"
#include "jtagLowLevel.h"


extern TapState currentTapState;

/*  @desc    return the name of TAP state in string instead of number(enum)
    @retval  TAP state in string
*/
char *getTapStateName(TapState tapState){
  switch(tapState){
    case TEST_LOGIC_RESET : return "TEST_LOGIC_RESET"; break;
    case RUN_TEST_IDLE    : return "RUN_TEST_IDLE"; break;
    case SELECT_DR_SCAN   : return "SELECT_DR_SCAN"; break;
    case CAPTURE_DR       : return "CAPTURE_DR"; break;
    case SHIFT_DR         : return "SHIFT_DR"; break;
    case EXIT1_DR         : return "EXIT1_DR"; break;
    case PAUSE_DR         : return "PAUSE_DR"; break;
    case EXIT2_DR         : return "EXIT2_DR"; break;
    case UPDATE_DR        : return "UPDATE_DR"; break;
    case SELECT_IR_SCAN   : return "SELECT_IR_SCAN"; break;
    case CAPTURE_IR       : return "CAPTURE_IR"; break;
    case SHIFT_IR         : return "SHIFT_IR"; break;
    case EXIT1_IR         : return "EXIT1_IR"; break;
    case PAUSE_IR         : return "PAUSE_IR"; break;
    case EXIT2_IR         : return "EXIT2_IR"; break;
    case UPDATE_IR        : return "UPDATE_IR"; break;
    default: return "Invalid TAP state";
  }
}

/*  @param current TAP State and current TMS state
    @desc  Reset to TEST_LOGIC_RESET state no matter
            current state.
*/

TapState updateTapState(TapState currentState, int tms){
	TapState retval;
      if(tms == 1)
    	  retval =  (tapTrackTable[currentState].nextState_tms1);
      else if(tms == 0)
    	  retval =  (tapTrackTable[currentState].nextState_tms0);
    return retval;
}




/*
 * Reset the TAP controller state to TEST_LOGIC_RESET
 * state.
 */
void resetTapState(){
	int i = 0;

	 while(i < 5){
	   jtagClkIoTms(0, 1);
	   currentTapState = updateTapState(currentTapState, 1);
	   i++;
	 }
}

/*
 * Transition of TAP state from parameter "start"
 * to "end"
 */
void tapTravelFromTo(TapState start, TapState end){
    int tmsRequired;

      while(currentTapState != end){
          tmsRequired = getTmsRequired(currentTapState, end);
          jtagClkIoTms(0, tmsRequired);
          currentTapState = updateTapState(currentTapState, tmsRequired);
          }
}

void jtagWriteTms(uint64_t data, int length){
  int dataMask = 1;
  int oneBitData = 0;

  while(length > 0){
    oneBitData = dataMask & data;
    jtagClkIoTms(0, oneBitData);
    length--;
    data = data >> 1;
  }
}

/*
Refer to B5.2.3 Switching from SWD to JTAG operation of https://developer.arm.com/documentation/ihi0031/latest/

To switch SWJ-DP from SWD to JTAG operation:
	1. Send at least 50 SWCLKTCK cycles with SWDIOTMS HIGH. This sequence ensures that the current 
		interface is in its reset state. The SWD interface only detects the 16-bit SWD-to-JTAG sequence when it is 
		in the reset state.
	2. Send the 16-bit SWD-to-JTAG select sequence on SWDIOTMS.
	3. Send at least five SWCLKTCK cycles with SWDIOTMS HIGH. This sequence ensures that if SWJ-DP 
		was already in JTAG operation before sending the select sequence, the JTAG TAP enters the 
		Test-Logic-Reset state.
		
The 16-bit SWD-to-JTAG select sequence is 0b0011 1100 1110 0111, MSB first. This sequence can be represented as 
either of the following:
• 0x3CE7, transmitted MSB first
• 0xE73C, transmitted LSB first.

*/
void switchSwdToJtagMode(){
	jtagWriteTms(0x3FFFFFFFFFFFF, 50);
	jtagWriteTms(0xE73C, 16);
	resetTapState();
}

uint64_t jtagWriteAndReadBits(uint64_t data, int length){
	int dataMask = 1;
	int oneBitData = 0;
	uint64_t tdoData = 0;
	int i = 0;
	int n = 0;
	uint64_t outData = 0;

	// noted that last bit of data must be set at next tap state
	for (n = length ; n > 1; n--) {
	  oneBitData = dataMask & data;
	  tdoData = jtagClkIoTms(oneBitData, 0);
	  currentTapState = updateTapState(currentTapState, 0);
	  outData |= tdoData << (i*1);
	  data = data >> 1;
	  i++;
	}
	oneBitData = dataMask & data;
	tdoData = jtagClkIoTms(oneBitData, 1);
	currentTapState = updateTapState(currentTapState, 1);
	outData |= tdoData << (i*1);;
	return outData;
}

void loadJtagIR(int instructionCode, int length, TapState start){
	tapTravelFromTo(start, SHIFT_IR);
	jtagWriteAndReadBits(instructionCode, length);
	tapTravelFromTo(EXIT1_IR, RUN_TEST_IDLE);
}

uint64_t jtagWriteAndRead(uint64_t data, int length){
  uint64_t outData = 0;
  tapTravelFromTo(RUN_TEST_IDLE, SHIFT_DR);
  outData = jtagWriteAndReadBits(data, length);
  tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
  return outData;
}

uint64_t jtagBypass(int instruction, int instructionLength, int data, int dataLength){
	uint64_t valRead = 0;
	loadJtagIR(instruction, instructionLength, RUN_TEST_IDLE);
	valRead = jtagWriteAndRead(data,dataLength);
	jtagSetIr(BYPASS);
	return valRead;
}

void loadBypassRegister(int instruction, int instructionLength, int data, int dataLength){
	loadJtagIR(instruction, instructionLength, RUN_TEST_IDLE);
	jtagWriteAndRead(data, dataLength);
}

uint64_t jtagReadIdCode(int instructionCode, int instructionLength, int data, int dataLength){
	uint64_t valRead = 0;
	loadJtagIR(instructionCode, instructionLength, RUN_TEST_IDLE);
	valRead = jtagWriteAndRead(data, dataLength);
	jtagSetIr(IDCODE);
	return valRead;
}

uint64_t jtagReadIDCodeResetTAP(int data, int dataLength){
	uint64_t valRead = 0;
	resetTapState();
	tapTravelFromTo(TEST_LOGIC_RESET, SHIFT_DR);
	valRead = jtagWriteAndReadBits(data, dataLength);
	tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
	return valRead;
}
