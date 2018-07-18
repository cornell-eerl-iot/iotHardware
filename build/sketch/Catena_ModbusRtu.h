#pragma once

#include <Catena_PollableInterface.h> //in Catena_for_arduino library
#include <ModbusRtu.h>


namespace McciCatena {

class cCatenaModbusRtu : public Modbus, 
			 public cPollableObject
	{
	using Super = Modbus;
public:
		cCatenaModbusRtu() {telegram_init(1);};
    	cCatenaModbusRtu(uint8_t u8id) : Super(u8id) {};
    	cCatenaModbusRtu(uint8_t u8id, uint8_t u8txenpin) : Super(u8id, u8txenpin) {};

	// the polling interface 
	// we save the poll() results for the background, or 
	// we might just want to look for completion other ways.
	virtual void poll() { lastPollResult = this->Super::poll(); };
	int8_t getPollResult() const { return this->lastPollResult; }
	virtual void poll_multiple_regs();
	virtual void query();
  	virtual void query(modbus_t telegram){this->Super::query(telegram);}
	void get_data(int telegram_num);
	void telegram_init(int size);
	void printTelegrams();
	
	void addTelegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil, uint16_t *reg);
	void addTelegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil);
	
	uint16_t *get_container(){return container;} //get the result from poll.


private:
	int8_t lastPollResult = 0;
	int telegramsSize = 1; //Size of the current modbus_t telegrams
	int telegramsCounter = 0; //counter for the telegram index to add new telegrams by array indexing
	modbus_t *telegrams;
	uint16_t container[16];
	int queryCount=0;
	};

void cCatenaModbusRtu::poll_multiple_regs()
{

}
/**
 * @brief
 * This method sends queries to the device from 1st element to the last element in telegram
 *	
 * @return none
 */
void cCatenaModbusRtu::query(){
	if(queryCount>telegramsCounter){
		queryCount=0;
	}
	this->Super::query(telegrams[queryCount]);
	queryCount++;
}

//initalizes the telegrams array to a preset size
void cCatenaModbusRtu::telegram_init(int size){
	telegrams = new modbus_t[size];
	telegramsSize = size;
}
/**
 * @brief
 * Prints all the telegram values to the serial monitor
 *	
 * @return none
 */
void cCatenaModbusRtu::printTelegrams(){
	for(int i = 0; i<telegramsSize; i++){
		Serial.print("ID: ");Serial.print(telegrams[i].u8id); // device address
		Serial.print(" Func Code: ");Serial.print(telegrams[i].u8fct); // function code (this one is registers read)
		Serial.print(" Address: ");Serial.print(telegrams[i].u16RegAdd); // start address in device
		Serial.print(" Num ele: ");Serial.println(telegrams[i].u16CoilsNo); // number of elements (coils or registers) to read
		Serial.println(" ");
	}
}

/**
 * @brief
 * Adds new telegram to the telegrams array
 * If adding new telegram will exceed array size, then we reallocate telegrams and double the size of array
 *	
 * @param
 *	parameter from the modbus_t struct
 */
void cCatenaModbusRtu::addTelegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil, uint16_t *reg){
	if(telegramsCounter>=telegramsSize){
		modbus_t *clone = telegrams;
		telegrams = new modbus_t[telegramsSize*2];
		for(int i = 0; i<telegramsSize; i++){
			telegrams[i] = clone[i];
		}
		delete[] clone;
		telegramsSize*=2;
	}
	telegrams[telegramsCounter].u8id = id; // device address
	telegrams[telegramsCounter].u8fct = funct; // function code (this one is registers read)
	telegrams[telegramsCounter].u16RegAdd = addr; // start address in device
	telegrams[telegramsCounter].u16CoilsNo = coil; // number of elements (coils or registers) to read
	telegrams[telegramsCounter].au16reg = reg; // pointer to a memory array in the Arduino
	telegramsCounter++;
}
/**
 * @brief
 * Adds new telegram to the telegrams array; no output needed for the variation
 * 
 * @param
 *	parameter from the modbus_t struct
 *	
 * @return none
 */
void cCatenaModbusRtu::addTelegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil){
	this->addTelegram(id,funct,addr,coil,container);
}


// remember to register this with catena framework at startup, e.g.:
//	gCatena.registerObject(&myCatenaModbusRtu);
//
//	After that, gCatena.poll() will include the object in the poll sequence.
//
}; // end namespace McciCatea
