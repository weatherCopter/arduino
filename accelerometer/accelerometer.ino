//https://www.pololu.com/product/2468
//http://www.st.com/web/en/resource/technical/document/datasheet/DM00057547.pdf
//#define "test1.csv"'
#include <Wire.h>
//#include <SdFat.h>
//#include <SdFatUtil.h>
//#include <L3G.h>
#include <LSM303.h>

LSM303 accelerometer1;
// LSM303 accelerometer2;
//L3G gyro;
//SdFat sd;
//SdFile file;


void setup()
{
	Serial.begin(115200);
	Wire.begin();

	Accel_Init();
	//Gyro_Init();

}

void loop()
{
	while((accelerometer1.readAccReg(LSM303::STATUS_A) & 0x08) == false)
	{}

//		if (!sd.begin(10, SPI_FULL_SPEED)) sd.initErrorHalt();
//	if (!file.open("test8.txt", O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt();
	accelerometer1.read();

	Serial.print(accelerometer1.a.x);
	Serial.print(',');
//	file.print(accelerometer1.a.x);
//	file.print(',');
	Serial.print(accelerometer1.a.y);
	Serial.print(',');
//	file.print(accelerometer1.a.y);
//	file.print(',');
	Serial.print(accelerometer1.a.z);
	Serial.print(',');
//	file.print(accelerometer1.a.z);
//	file.print(',');
//	file.close();

	// while((accelerometer2.readAccReg(LSM303::STATUS_A) & 0x08) == false)
	// {}

//		if (!sd.begin(10, SPI_FULL_SPEED)) sd.initErrorHalt();
//	if (!file.open("test8.txt", O_RDWR | O_CREAT | O_AT_END)) sd.errorHalt();
	// accelerometer2.read();

	// Serial.print(accelerometer2.a.x);
	// Serial.print(',');
//	file.print(accelerometer2.a.x);
//	file.print(',');
	// Serial.print(accelerometer2.a.y);
	// Serial.print(',');
//	file.print(accelerometer2.a.y);
//	file.print(',');
	// Serial.print(accelerometer2.a.z);
	// Serial.println(',');
//	file.println(accelerometer2.a.z);
//	file.print(',');
//	file.close();

}

void Accel_Init()
{
	accelerometer1.init(LSM303::device_D, LSM303::sa0_high);
	accelerometer1.enableDefault();
	switch (accelerometer1.getDeviceType())
	{
		case LSM303::device_D:
			accelerometer1.writeReg(LSM303::CTRL2, 0x18); // 8 g full scale: AFS = 011
			accelerometer1.writeReg(LSM303::CTRL1, 0x77);
			break;
		case LSM303::device_DLHC:
			accelerometer1.writeReg(LSM303::CTRL_REG4_A, 0x28); // 8 g full scale: FS = 10; high resolution output mode
			break;
		default: // DLM, DLH
			accelerometer1.writeReg(LSM303::CTRL_REG4_A, 0x30); // 8 g full scale: FS = 11
	}

	// accelerometer2.init(LSM303::device_D, LSM303::sa0_low);
	// accelerometer2.enableDefault();
	// switch (accelerometer2.getDeviceType())
	// {
	// 	case LSM303::device_D:
	// 		accelerometer2.writeReg(LSM303::CTRL2, 0x18); // 8 g full scale: AFS = 011
	// 		accelerometer2.writeReg(LSM303::CTRL1, 0x77);
	// 		break;
	// 	case LSM303::device_DLHC:
	// 		accelerometer2.writeReg(LSM303::CTRL_REG4_A, 0x28); // 8 g full scale: FS = 10; high resolution output mode
	// 		break;
	// 	default: // DLM, DLH
	// 		accelerometer2.writeReg(LSM303::CTRL_REG4_A, 0x30); // 8 g full scale: FS = 11
	// }
}

//void Gyro_Init()
//{
//  gyro.init();
//  gyro.writeReg(L3G_CTRL_REG4, 0x20); // 2000 dps full scale
//  gyro.writeReg(L3G_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz
//}


