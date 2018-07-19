#pragma once

#include <Catena_PollableInterface.h> //in Catena_for arduino library
#include "ModbusRtu.h"
//#include "f2sflt16.cpp"

namespace McciCatena {

class cCatenaModbusRtu : public Modbus, 
			 public cPollableObject
	{
	using Super = Modbus;
public:
		cCatenaModbusRtu() {};
    	cCatenaModbusRtu(uint8_t u8id) : Super(u8id) {};
    	cCatenaModbusRtu(uint8_t u8id, uint8_t u8txenpin) : Super(u8id, u8txenpin) {};

	// the polling interface 
	// we save the poll() results for the background, or 
	// we might just want to look for completion other ways.
	virtual void poll() { lastPollResult = this->Super::poll(); };
	int8_t getPollResult() const { return this->lastPollResult; }
	virtual void poll_multiple_regs();

private:
	int8_t lastPollResult = 0;
	modbus_t *telegrams;

	};

void cCatenaModbusRtu::poll_multiple_regs()
{
}



// remember to register this with catena framework at startup, e.g.:
//	gCatena.registerObject(&myCatenaModbusRtu);
//
//	After that, gCatena.poll() will include the object in the poll sequence.
//
}; // end namespace McciCatea
