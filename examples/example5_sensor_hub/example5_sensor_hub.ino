#include <Wire.h>
#include "SparkFun_ISM330DHCX.h"

#define MAG_ADDR_READ 0x61
#define MAG_ADDR_WRITE 0x30
#define MAG_READ_REG 0x00
#define MAG_READ_LEN 0x07
#define MAG_WRITE_REG 0x09
#define MAG_WRITE_DATA 0x01

SparkFun_ISM330DHCX myISM; 
sfe_ism_data_t accelData; 
sfe_ism_data_t gyroData; 

sfe_hub_sensor_settings_t readMMC, writeMMC;

uint8_t shRawData[MAG_READ_LEN] = {};
uint32_t magXVal; 
uint32_t magYVal; 
uint32_t magZVal; 

void setup(){


	writeMMC.address = MAG_ADDR_WRITE; 
	writeMMC.subAddress = MAG_WRITE_REG; 
	writeMMC.length = MAG_WRITE_DATA; 

	readMMC.address = MAG_ADDR_READ; 
	readMMC.subAddress = MAG_READ_REG; 
	readMMC.length = MAG_READ_LEN; 

	Wire.begin();

	Serial.begin(115200);

	if( !myISM.begin() ){
		Serial.println("Did not begin.");
		while(1);
	}

	myISM.deviceReset();
	myISM.resetSensorHub();

	while( !myISM.getDeviceReset() ){ 
		delay(1);
	} 

	Serial.println("Reset.");
	Serial.println("Applying settings.");
	delay(100);

	myISM.setDeviceConfig();
	myISM.setBlockDataUpdate();

	// ******
	// Might need a soft reset of MMC
	// Enable Passthrough mode, reset MMC and then
	// Move on.......
	// ******

	// Set up write registers
//	myISM.setHubSensorWrite(&writeMMC);

	// Set up read Registers
	// Which sensor (0-3) and settings to apply to given sensor

	// Set the number of peripheral sensor to be read by the 6DoF
	myISM.setNumberHubSensors(0);

	// Enable pullup resistors on SDX/SCX
	myISM.setHubPullUps(); 
	// Output data rate for the external sensor
	myISM.setHubODR(ISM_SH_ODR_52Hz);

	// Enable the 6DoF as a controller I2C
	// All configurations to the sensor hub must occur while the controller I2C 
	// bus is powered down.


	writeControlBit(writeMMC);

	//
	myISM.setHubSensorRead(0, &readMMC);

	myISM.enableSensorI2C(true);

	//Wait 300us
	delayMicroseconds(350);
	
	// Apply acceleromter settings
	myISM.setAccelFullScale(ISM_4g); 
	myISM.setAccelFilterLP2();
	myISM.setAccelSlopeFilter(ISM_LP_ODR_DIV_100);

	// Apply gyroscope settings
	myISM.setGyroFullScale(ISM_500dps); 
	myISM.setGyroFilterLP1();
	myISM.setGyroLP1Bandwidth(ISM_MEDIUM);

	// Enable Sensors
	myISM.setAccelDataRate(ISM_XL_ODR_104Hz);
	myISM.setGyroDataRate(ISM_GY_ODR_104Hz);





}

void loop(){

	if( myISM.checkStatus() ){

		myISM.getAccel(&accelData);
		myISM.getGyro(&gyroData);

		if( myISM.externalSensorNack(0) )
			Serial.println("MMC Nacked...");

		if( myISM.getHubStatus() )
		{
			Serial.println("Hub Comms Done.");
			myISM.readPeripheralSensor(shRawData, (uint8_t)MAG_READ_LEN);

			magXVal = shRawData[0] | shRawData [1] | (shRawData[6] & 0x20); 
			magYVal = shRawData[2] | shRawData [3] | (shRawData[6] & 0x20); 
			magZVal = shRawData[4] | shRawData [5] | (shRawData[6] & 0x20); 

			Serial.print("Magnetometer: ");
			Serial.print("X: ");
			Serial.print(magXVal);
			Serial.print(" ");
			Serial.print("Y: ");
			Serial.print(magYVal);
			Serial.print(" ");
			Serial.print("Z: ");
			Serial.print(magZVal);
			Serial.println(" ");

			writeControlBit(writeMMC);
			myISM.setHubSensorRead(0, &readMMC);
			myISM.enableSensorI2C(true);

			myISM.setAccelDataRate(ISM_XL_ODR_104Hz);
			myISM.setGyroDataRate(ISM_GY_ODR_104Hz);

		}


		Serial.print("Accelerometer: ");
		Serial.print("X: ");
		Serial.print(accelData.xData);
		Serial.print(" ");
		Serial.print("Y: ");
		Serial.print(accelData.yData);
		Serial.print(" ");
		Serial.print("Z: ");
		Serial.print(accelData.zData);
		Serial.println(" ");
		Serial.print("Gyroscope: ");
		Serial.print("X: ");
		Serial.print(gyroData.xData);
		Serial.print(" ");
		Serial.print("Y: ");
		Serial.print(gyroData.yData);
		Serial.print(" ");
		Serial.print("Z: ");
		Serial.print(gyroData.zData);
		Serial.println(" ");
		Serial.print("Magnetometer: ");
		Serial.print("X: ");
		Serial.print(magXVal);
		Serial.print(" ");
		Serial.print("Y: ");
		Serial.print(magYVal);
		Serial.print(" ");
		Serial.print("Z: ");
		Serial.print(magZVal);
		Serial.println(" ");
	}

	delay(100);
}


bool writeControlBit(sfe_hub_sensor_settings_t toWrite)
{

	// Sensors must be off for sensor hub configuration.
	myISM.setAccelDataRate(ISM_XL_ODR_OFF);
	myISM.setGyroDataRate(ISM_GY_ODR_OFF);
	myISM.enableSensorI2C(false);
	// 300us until 6DoF I2C is powered down
	delayMicroseconds(350);

	// Configure a write
	myISM.setHubSensorWrite(&toWrite);
	// Re-enable sensor for write
	myISM.setAccelDataRate(ISM_XL_ODR_104Hz);
	myISM.enableSensorI2C(true);

	// Wait for write to complete
	while( !myISM.getHubStatus() ){
		Serial.print(".");
		delay(1);
	}
	
	
	// Turn off sensor
	myISM.enableSensorI2C(false);
	
	delayMicroseconds(350);

	Serial.println();
	Serial.println("Write Complete");

	return true; 

}
