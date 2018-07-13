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
	virtual void query();
	void telegram_init(int size);
	void telegram_serialprint();
	void add_telegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil, uint16_t *reg);
	void add_telegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil);


private:
	int8_t lastPollResult = 0;
	int telegramsSize = 1; //Size of the current modbus_t telegrams
	int telegramsCounter = 0; //counter for the telegram index to add new telegrams by array indexing
	modbus_t *telegrams;
	uint16_t container[16];
	};

void cCatenaModbusRtu::poll_multiple_regs()
{
}

//initalizes the telegrams array to a preset size
void cCatenaModbusRtu::telegram_init(int size){
	telegrams = new modbus_t[size];
	telegramsSize = size;
}

//Prints all the telegram values to the serial monitor
void cCatenaModbusRtu::telegram_serialprint(){
	for(int i = 0; i<telegramsSize; i++){
		Serial.print("ID: ");Serial.print(telegrams[i].u8id); // device address
		Serial.print(" Func Code: ");Serial.print(telegramss[i].u8fct); // function code (this one is registers read)
		Serial.print(" Address: ");Serial.print(telegrams[i].u16RegAdd); // start address in device
		Serial.print(" Num ele: ");Serial.println(telegrams[i].u16CoilsNo); // number of elements (coils or registers) to read
		Serial.println(" ");
	}
}

/**
 * @add_telegram
 * @brief
 * Adds new telegram to the telegrams array
 * If adding new telegram will exceed array size, then we reallocate telegrams and double the size of array
 *
 */
void cCatenaModbusRtu::add_telegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil, uint16_t *reg){
	if(telegramsCounter>=telegramsSize){
		modbus_t *clone = telgrams;
		telegrams = new modbus_t[telegramsSize*2];
		for(int i = 0; i<telegramsSize; i++){
			telegrams[i] = clone[i];
		}
		telegramsSize*=2;
	}
	telegrams[telegramsCounter].u8id = id; // device address
	telegrams[telegramsCounter].u8fct = funct; // function code (this one is registers read)
	telegrams[telegramsCounter].u16RegAdd = addr; // start address in device
	telegrams[telegramsCounter].u16CoilsNo = coil; // number of elements (coils or registers) to read
	telegrams[telegramsCounter].au16reg = reg; // pointer to a memory array in the Arduino
	telegramsCounter++;
}

// remember to register this with catena framework at startup, e.g.:
//	gCatena.registerObject(&myCatenaModbusRtu);
//
//	After that, gCatena.poll() will include the object in the poll sequence.
//
}; // end namespace McciCatea
