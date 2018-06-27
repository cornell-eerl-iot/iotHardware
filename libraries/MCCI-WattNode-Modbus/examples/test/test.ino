#include <MCCI-WattNode-Modbus-Registers.h>

void setup(void)
	{
	while (! Serial)
		/* loop */ yield();

	Serial.println(__FILE__);
	Serial.print("DummyReg_i32 = ");
	Serial.println(static_cast<int>(WattNodeModbus::Register::DummyReg_i32));
	Serial.print("addr(DummyReg_i32) = ");
	Serial.println(WattNodeModbus::getAddress(WattNodeModbus::Register::DummyReg_i32));

	int16_t vLow, vHigh;
	float v; int32_t i32v;

	std::tie(vLow, vHigh) = WattNodeModbus::float_getregval(3.14159);
	Serial.print("float_getregval(3.14159) = ");
	Serial.print(vLow); Serial.print(", "); Serial.print(vHigh); 
	Serial.print(" / hex 0x"); Serial.print(vLow & 0xFFFF, HEX); Serial.print(", 0x"); Serial.println(vHigh & 0xFFFF, HEX);

	bool errflag = false;
	v = WattNodeModbus::regval_getfloat(0, -1, &errflag);
	Serial.print("regval_getfloat(0,-1) = ");
	Serial.print(v); Serial.print(", error = "); Serial.println(errflag);

	errflag = false;
	v = WattNodeModbus::regval_getfloat(0, 0x4000, &errflag);
	Serial.print("regval_getfloat(0, 0x4000) = ");
	Serial.print(v); Serial.print(", errflag = "); Serial.println(errflag);

	errflag = true;
	v = WattNodeModbus::regval_getfloat(0, 0x4000, &errflag);
	Serial.print("errflag = true, regval_getfloat(0, 0x4000) = ");
	Serial.print(v); Serial.print(", errflag = "); Serial.println(errflag);

	std::tie(vLow, vHigh) = WattNodeModbus::int32_getregval(INT32_MIN);
	Serial.print("float_getregval(INT32_MIN) = ");
	Serial.print(vLow); Serial.print(", "); Serial.print(vHigh); 
	Serial.print(" / hex 0x"); Serial.print(vLow & 0xFFFF, HEX); Serial.print(", 0x"); Serial.println(vHigh & 0xFFFF, HEX);

	std::tie(vLow, vHigh) = WattNodeModbus::int32_getregval(INT32_MAX);
	Serial.print("float_getregval(INT32_MAX) = ");
	Serial.print(vLow); Serial.print(", "); Serial.print(vHigh); 
	Serial.print(" / hex 0x"); Serial.print(vLow & 0xFFFF, HEX); Serial.print(", 0x"); Serial.println(vHigh & 0xFFFF, HEX);

	i32v = WattNodeModbus::regval_getint32(0, -1);
	Serial.print("regval_getint32(0,-1) = ");
	Serial.println(i32v);

	i32v = WattNodeModbus::regval_getint32(-1, 0);
	Serial.print("regval_getint32(-1, 0) = ");
	Serial.println(i32v);
	}

void loop(void)
	{
	/* do nothing */
        yield();
	}
