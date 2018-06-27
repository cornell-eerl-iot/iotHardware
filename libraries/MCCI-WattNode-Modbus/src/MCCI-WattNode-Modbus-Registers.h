/* Mcci-WattNode-Modbus-Registers.h	Sun Apr 22 2018 14:51:03 tmm */

/*

Module:  Mcci-WattNode-Modbus-Registers.h

Function:
	C++ register definition and manipulation functions
	for WattNode Modbus.

Version:
	V0.1.0	Sun Apr 22 2018 14:51:03 tmm	Edit level 1

Copyright notice:
	This file copyright (C) 2018 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	See LICENSE file accompanying this work.

Author:
	Terry Moore, MCCI Corporation	April, 2018

Revision history:
   0.1.0  Sun Apr 22 2018 14:51:03  tmm
	Module created from Continental Controls spreadsheet.

*/

#ifndef _MCCI_WATTNODE_MODBUS_REGISTERS_H_		/* prevent multiple includes */
#define _MCCI_WATTNODE_MODBUS_REGISTERS_H_

#pragma once

#include <stdint.h>
#include <cstring>

#include "MCCI-WattNode-Modbus-tuple.h"

class WattNodeModbus
	{
public:
	enum class Register : uint16_t
		{
		DummyReg_i32 = 1,
		EnergySum_f32 = 1001,
		EnergyPosSum_f32 = 1003,
		EnergySumNR_f32 = 1005,
		EnergyPosSumNR_f32 = 1007,
		PowerSum_f32 = 1009,
		PowerA_f32 = 1011,
		PowerB_f32 = 1013,
		PowerC_f32 = 1015,
		VoltAvgLN_f32 = 1017,
		VoltA_f32 = 1019,
		VoltB_f32 = 1021,
		VoltC_f32 = 1023,
		VoltAvgLL_f32 = 1025,
		VoltAB_f32 = 1027,
		VoltBC_f32 = 1029,
		VoltAC_f32 = 1031,
		Freq_f32 = 1033,
		EnergyA_f32 = 1101,
		EnerygB_f32 = 1103,
		EnergyC_f32 = 1105,
		EnergyPosA_f32 = 1107,
		EnergyPosB_f32 = 1109,
		EnergyPosC_f32 = 1111,
		EnergyNegSum_f32 = 1113,
		EnergyNegSumNR_f32 = 1115,
		EnergyNegA_f32 = 1117,
		EnergyNegB_f32 = 1119,
		EnergyNegC_f32 = 1121,
		EnergyReacSum_f32 = 1123,
		EnergyReacA_f32 = 1125,
		EnergyReacB_f32 = 1127,
		EnergyReacC_f32 = 1129,
		EnergyAppSum_f32 = 1131,
		EnergyAppA_f32 = 1133,
		EnergyAppB_f32 = 1135,
		EnergyAppC_f32 = 1137,
		PowerFactorAvg_f32 = 1139,
		PowerFactorA_f32 = 1141,
		PowerFactorB_f32 = 1143,
		PowerFactorC_f32 = 1145,
		PowerReacSum_f32 = 1147,
		PowerReacA_f32 = 1149,
		PowerReacB_f32 = 1151,
		PowerReacC_f32 = 1153,
		PowerAppSum_f32 = 1155,
		PowerAppA_f32 = 1157,
		PowerAppB_f32 = 1159,
		PowerAppC_f32 = 1161,
		CurrentA_f32 = 1163,
		CurrentB_f32 = 1165,
		CurrentC_f32 = 1167,
		Demand_f32 = 1169,
		DemandMin_f32 = 1171,
		DemandMax_f32 = 1173,
		DemandApp_f32 = 1175,
		DemandA_f32 = 1177,
		DemandB_f32 = 1179,
		DemandC_f32 = 1181,
		EnergySum_i32 = 1201,
		EnergyPosSum_i32 = 1203,
		EnergySumNR_i32 = 1205,
		EnergyPosSumNR_i32 = 1207,
		PowerSum_i16 = 1209,
		PowerA_i16 /* = 1210 */,
		PowerB_i16 /* = 1211 */,
		PowerC_i16 /* = 1212 */,
		VoltAvgLN_i16 /* = 1213 */,
		VoltA_i16 /* = 1214 */,
		VoltB_i16 /* = 1215 */,
		VoltC_i16 /* = 1216 */,
		VoltAvgLL_i16 /* = 1217 */,
		VoltAB_i16 /* = 1218 */,
		VoltBC_i16 /* = 1219 */,
		VoltAC_i16 /* = 1220 */,
		Freq_i16 /* = 1221 */,
		EnergyA_i32 = 1301,
		EnerygB_i32 = 1303,
		EnergyC_i32 = 1305,
		EnergyPosA_i32 = 1307,
		EnergyPosB_i32 = 1309,
		EnergyPosC_i32 = 1311,
		EnergyNegSum_i32 = 1313,
		EnergyNegSumNR_i32 = 1315,
		EnergyNegA_i32 = 1317,
		EnergyNegB_i32 = 1319,
		EnergyNegC_i32 = 1321,
		EnergyReacSum_i32 = 1323,
		EnergyReacA_i32 = 1325,
		EnergyReacB_i32 = 1327,
		EnergyReacC_i32 = 1329,
		EnergyAppSum_i32 = 1331,
		EnergyAppA_i32 = 1333,
		EnergyAppB_i32 = 1335,
		EnergyAppC_i32 = 1337,
		PowerFactorAvg_i16 = 1339,
		PowerFactorA_i16 /* = 1340 */,
		PowerFactorB_i16 /* = 1341 */,
		PowerFactorC_i16 /* = 1342 */,
		PowerReacSum_i16 /* = 1343 */,
		PowerReacA_i16 /* = 1344 */,
		PowerReacB_i16 /* = 1345 */,
		PowerReacC_i16 /* = 1346 */,
		PowerAppSum_i16 /* = 1347 */,
		PowerAppA_i16 /* = 1348 */,
		PowerAppB_i16 /* = 1349 */,
		PowerAppC_i16 /* = 1350 */,
		CurrentA_i16 /* = 1351 */,
		CurrentB_i16 /* = 1352 */,
		CurrentC_i16 /* = 1353 */,
		Demand_i16 /* = 1354 */,
		DemandMin_i16 /* = 1355 */,
		DemandMax_i16 /* = 1356 */,
		DemandApp_i16 /* = 1357 */,
		DemandA_i16 /* = 1358 */,
		DemandB_i16 /* = 1359 */,
		DemandC_i16 /* = 1360 */,
		IoPinState_i16 /* = 1361 */,
		PulseCount_i32 /* = 1362 */,
		ConfigPasscode_i32 = 1601,
		CtAmps_i16 = 1603,
		CtAmpsA_i16 /* = 1604 */,
		CtAmpsB_i16 /* = 1605 */,
		CtAmpsC_i16 /* = 1606 */,
		CtDirections_i16 /* = 1607 */,
		Averaging_i16 /* = 1608 */,
		PowerIntScale_i16 /* = 1609 */,
		DemPerMins_i16 /* = 1610 */,
		DemSubints_i16 /* = 1611 */,
		GainAdjustA_i16 /* = 1612 */,
		GainAdjustB_i16 /* = 1613 */,
		GainAdjustC_i16 /* = 1614 */,
		PhaseAdjustA_i16 /* = 1615 */,
		PhaseAdjustB_i16 /* = 1616 */,
		PhaseAdjustC_i16 /* = 1617 */,
		CreepLimit_i16 /* = 1618 */,
		PhaseOffset_i16 /* = 1619 */,
		ZeroEnergy_i16 /* = 1620 */,
		ZeroDemand_i16 /* = 1621 */,
		CurrentIntScale_i16 /* = 1622 */,
		IoPinMode_i16 /* = 1623 */,
		ApplyComConfig_i16 = 1651,
		Address_i16 /* = 1652 */,
		BaudRate_i16 /* = 1653 */,
		ParityMode_i16 /* = 1654 */,
		ModbusMode_i16 /* = 1655 */,
		ReplyDelay_i16 /* = 1656 */,
		SerialNumber_i32 = 1701,
		UptimeSecs_i32 = 1703,
		TotalSecs_i32 = 1705,
		Model_i16 = 1707,
		Version_i16 /* = 1708 */,
		Options_i16 /* = 1709 */,
		ErrorStatus_i16 /* = 1710 */,
		PowerFailCount_i16 /* = 1711 */,
		CrcErrorCount_i16 /* = 1712 */,
		FrameErrorCount_i16 /* = 1713 */,
		PacketErrorCount_i16 /* = 1714 */,
		OverrunCount_i16 /* = 1715 */,
		ErrorStatus1_i16 /* = 1716 */,
		ErrorStatus2_i16 /* = 1717 */,
		ErrorStatus3_i16 /* = 1718 */,
		ErrorStatus4_i16 /* = 1719 */,
		ErrorStatus5_i16 /* = 1720 */,
		ErrorStatus6_i16 /* = 1721 */,
		ErrorStatus7_i16 /* = 1722 */,
		ErrorStatus8_i16 /* = 1723 */,
		}; // enum Register

	// convert WattNodeModbus::Register into equivalent address.
	// Addresses on the bus are origin 0; but Modbus documentation
	// is always origin 1; hence the discrepancy.
	static constexpr uint16_t getAddress(Register r)
		{
		return static_cast<uint16_t>(r) - 1;
		}

	// convert address into a WattNodeModbus::Register
	static constexpr Register getRegister(uint16_t address)
		{
		return Register(address + 1);
		}

	static float regval_getfloat(int16_t vLow, int16_t vHigh, bool *pError = nullptr)
		{
		const uint32_t uImage = (static_cast<uint16_t>(vHigh) << 16u) 
				      | static_cast<uint16_t>(vLow);

		/* reject NANs */
		if ((uImage & 0x7FE00000u) == 0x7FE00000u)
			{
			if (pError != nullptr)
				*pError = true;
			return 0.0;
			}

		/* otherwise the answer is the bit pattern as a float */
		float fpResult;
		static_assert(sizeof(fpResult) == sizeof(uImage), "float / uin32_t mismatch");
		std::memcpy(&fpResult, &uImage, sizeof(fpResult));
		return fpResult;
		}

	// Convert float to external form for Modbus.
	// Convenient use: 
	// 	int16_t vLow, vHigh;
	//	std::tie(vLow, vHigh) = WattNodeModbus::float_getregval(vFloat);
	static std::tuple<int16_t, int16_t> float_getregval(float vFloat)
		{
		uint32_t uImage;
		static_assert(sizeof(vFloat) == sizeof(uImage), "float / uint32_t mismatch");
		std::memcpy(&uImage, &vFloat, sizeof(vFloat));
		const int16_t vLow = static_cast<int16_t>(uImage);
		const int16_t vHigh = static_cast<int16_t>(uImage >> 16);
		return std::make_tuple(vLow, vHigh); 
		}

	static int32_t regval_getint32(int16_t vLow, int16_t vHigh)
		{
		const uint32_t uImage = (static_cast<uint16_t>(vHigh) << 16u) 
				      | static_cast<uint16_t>(vLow);
		return (int32_t)(uImage);		
		}

	// Convert int32_t to external form for Modbus.
	// Convenient use: 
	// 	int16_t vLow, vHigh;
	//	std::tie(vLow, vHigh) = WattNodeModbus::int32_getregval(vInt32);
	static std::tuple<int16_t, int16_t> int32_getregval(int32_t vInt32)
		{
		const auto uImage = (uint32_t)(vInt32);
		const int16_t vLow = static_cast<int16_t>(uImage);
		const int16_t vHigh = static_cast<int16_t>(uImage >> 16);
		return std::make_tuple(vLow, vHigh); 
		}

	}; // class WattNodeModbus

#endif /* _MCCI_WATTNODE_MODBUS_REGISTERS_H_ */