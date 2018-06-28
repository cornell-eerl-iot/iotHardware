
#include <ModbusRtu.h>


class Maplewood_device : Modbus 
{
	
	
	public:
    Maplewood_device();
    Maplewood_device(uint8_t u8id);
    Maplewood_device(uint8_t u8id, uint8_t u8txenpin);
	poll();
}



Maplewood_device::Maplewood_device(){
	Modbus();
}

Maplewood_device::Maplewood_device(uint8_t u8id){
	Modbus(uint8_t u8id);
}

Maplewood_device::Maplewood_device(uint8_t u8id, uint8_t u8txenpin){
	Modbus(uint8_t u8id, uint8_t u8txenpin);
}

Maplewood_device::poll(){
	Modbus::poll();
	
}




