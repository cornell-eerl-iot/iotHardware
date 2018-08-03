#pragma once

#include <Catena_PollableInterface.h> //in Catena_for_arduino library
#include <ModbusRtu.h>
#undef min
#undef max
#include <vector>

namespace McciCatena {

class cCatenaModbusRtu : public Modbus, 
			 public cPollableObject
	{
	using Super = Modbus;
private:
	int8_t lastPollResult = 0;
	
	std::vector<modbus_t> w_telegrams;

	modbus_t *telegrams = new modbus_t[1];
	int telegramsCounter = 0; //counter for the telegram index to add new telegrams by array indexing
	int telegramsSize = 1; //Max size of the current modbus_t telegrams
	int queryCount = -1;

	uint16_t *container = new uint16_t[1];//container to store data from query
	int containerMaxSize = 1;
	int containerCurrSize = 1;

	float *convertedData = new float[1];
	int convertedDataSize = 1;


public:
	cCatenaModbusRtu() {};
	cCatenaModbusRtu(uint8_t u8id) : Super(u8id){};
	cCatenaModbusRtu(uint8_t u8id, uint8_t u8txenpin) : Super(u8id, u8txenpin){};

	// the polling interface 
	// we save the poll() results for the background, or 
	// we might just want to look for completion other ways.
	virtual void poll() {this->Super::poll();};
	
	virtual void query();
  	virtual void query(modbus_t telegram){this->Super::query(telegram);}
	
	void print_telegrams();
	void print_convertedData();
	void print_container();

	void add_telegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil, uint16_t *reg);
	void add_telegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil);
	void add_telegram(modbus_t telegram);

	int getTelegramSize() const {return this->telegramsSize;}
	int getTelegramCounter() const {return this->telegramsCounter;}

	uint16_t *getContainer(){return container;} //get the result from poll.
	int getContainerCurrSize(){return containerCurrSize;}

	int8_t getPollResult() const { return this->lastPollResult; }
	int getQueryCount() const {return this->queryCount;}
	uint16_t getCurrCoil(){return telegrams[queryCount].u16CoilsNo;}

	float *i16b_to_float();

	};


/**
 * @brief
 * This method sends queries to the device from 1st element to the last element in telegram
 *	
 * @return none
 */
void cCatenaModbusRtu::query(){
	if(w_telegrams.size()){
		this->Super::query(w_telegrams.back());
		w_telegrams.pop_back();
	}else{	
		queryCount++;
		if(queryCount>=telegramsCounter){
			queryCount=0;
		}
		this->Super::query(telegrams[queryCount]);
		this->containerCurrSize=telegrams[queryCount].u16CoilsNo;
	}
	
}
/*
 * @brief
 * Prints all the telegram values to the serial monitor
 *	
 * @return none
 */
void cCatenaModbusRtu::print_telegrams(){
	for(int i = 0; i<telegramsSize; i++){
		Serial.print("ID: ");Serial.print(telegrams[i].u8id); // device address
		Serial.print(" Func Code: ");Serial.print(telegrams[i].u8fct); // function code 
		Serial.print(" Address: ");Serial.print(telegrams[i].u16RegAdd); // start address in device
		Serial.print(" Num ele: ");Serial.println(telegrams[i].u16CoilsNo); // number of elements (coils or registers) to read
		//Serial.println(" ");
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
void cCatenaModbusRtu::add_telegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil, uint16_t *reg){
	if(telegramsCounter>=telegramsSize){
		modbus_t *clone = telegrams;
		telegrams = new modbus_t[telegramsSize*2];
		for(int i = 0; i<telegramsSize; i++){
			telegrams[i] = clone[i];
		}
		telegramsSize *=2;
	}
	if(funct>4){
		modbus_t telegram = {id,funct,addr,coil,reg};
		w_telegrams.push_back(telegram);
	}else{
		telegrams[telegramsCounter].u8id = id; // device address
		telegrams[telegramsCounter].u8fct = funct; 
		telegrams[telegramsCounter].u16RegAdd = addr; // start address in device
		telegrams[telegramsCounter].u16CoilsNo = coil; // number of elements (coils or registers) to read
		telegrams[telegramsCounter].au16reg = reg; // pointer to a memory array in the Arduino
		telegramsCounter++;
	}
}
/**
 * @brief
 * Adds new telegram to the telegrams array; no output array needed for this variation
 * 
 * @param
 *	parameter from the modbus_t struct
 *	
 * @return none
 */
void cCatenaModbusRtu::add_telegram(uint8_t id, uint8_t funct, uint16_t addr, uint16_t coil){
	if(coil>this->containerMaxSize){
		this->containerMaxSize = coil;
		delete[] this->container;
		this->container = new uint16_t [this->containerMaxSize];
		for(int i = 0; i<this->telegramsCounter; i++){
			this->telegrams[i].au16reg = this->container;
		}
	}
	this->add_telegram(id,funct,addr,coil,this->container);
}

/**
 * @brief
 * Adds new telegram to the telegrams array.
 * 
 * @param
 *	modbus_t struct
 *	
 * @return none
 */
void cCatenaModbusRtu::add_telegram(modbus_t telegram){
	if(telegram.au16reg == nullptr){
		this->add_telegram(telegram.u8id,telegram.u8fct,telegram.u16RegAdd,telegram.u16CoilsNo);
	}else{
		this->add_telegram(
			telegram.u8id,
			telegram.u8fct,telegram.u16RegAdd,telegram.u16CoilsNo,telegram.au16reg);
	}
	
}

/**
 * @brief
 * Combines the two 16-bit integer registers to 32-bit floats. We use operator "or" to 
 * combine high and low registers together. For modbus, we assume that the first 
 * register is the lower 2 bytes and the second register is the higher 2 bytes.
 * 
 * @return convertedData
 */
float * cCatenaModbusRtu::i16b_to_float(){
	delete[] this->convertedData;
	this->convertedDataSize = telegrams[queryCount].u16CoilsNo/2;
	this->convertedData = new float[this->convertedDataSize];
	uint16_t low;
	uint32_t high;
	uint16_t *au16data = telegrams[queryCount].au16reg;
	
	for (int i = 0; i < telegrams[queryCount].u16CoilsNo; ++i)
	{
		if(i%2 == 0){
			low = au16data[i];
		}else{
			high = ((au16data[i]<<16)&0xFFFF0000) | low;
			memcpy(&this->convertedData[(i-1)/2],&high,sizeof(this->convertedData[(i-1)/2]));
		}
	}
	return this->convertedData;
}

/**
 * @brief
 * Prints the contents of convertedData array.
 *	
 * @return none
 */
void cCatenaModbusRtu::print_convertedData(){
		if(this->queryCount==0) Serial.println("");
		for (int i = 0;i<this->convertedDataSize;i++){
			Serial.print(this->convertedData[i],DEC);
			/*if(!(i==(this->convertedDataSize-1)))
				Serial.print(",");*/
			Serial.print(",");
		}
	
}

/**
 * @brief
 * Prints the contents of container array.
 *	
 * @return none
 */
void cCatenaModbusRtu::print_container(){
		if(this->queryCount==0) Serial.println("");
		for (int i = 0;i<this->containerCurrSize;i++){
			Serial.print(this->container[i],DEC);
			if(!(i==(this->containerCurrSize-1)))
				Serial.print(",");
		}Serial.print(",---,");
	
}
  
// remember to register this with catena framework at startup, e.g.:
//	gCatena.registerObject(&myCatenaModbusRtu);
//
//	After that, gCatena.poll() will include the object in the poll sequence.
//
}; // end namespace McciCatea
