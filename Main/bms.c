#include "bms.h"
#include "iic.h"
#include "init.h"
#include "app.h"
/* USER CODE BEGIN PD */
 /*
 
	be carefull!!! FW_VERSION, HW_VERSION defined twice.
	
 #define FW_VERSION 0x0002 =>> #define FW_VERSION_BMS 0x0002
 #define HW_VERSION 0x0003 =>> #define HW_VERSION_BMS 0x0003
 */
/* Functions Declare ---------------------------------------------------------*/

uint8_t RX_data [2] = {0x00, 0x00}; // used in several functions to store data read from BQ769x2
uint8_t RX_32Byte [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//used in Subcommands read function
// Global Variables for cell voltages, temperatures, Stack voltage, PACK Pin voltage, LD Pin voltage, CC2 current
uint16_t CellVoltage [16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//float Temperature [3] = {0,0,0};
uint16_t Stack_Voltage = 0x00;
uint16_t Pack_Voltage = 0x00;
uint16_t LD_Voltage = 0x00;
uint16_t Pack_Current = 0x00;

uint16_t AlarmBits = 0x00;
uint16_t AlarmBits2 = 0x00;
uint8_t value_SafetyStatusA;  // Safety Status Register A
uint8_t value_SafetyStatusB;  // Safety Status Register B
uint8_t value_SafetyStatusC;  // Safety Status Register C
uint8_t value_PFStatusA;   // Permanent Fail Status Register A
uint8_t value_PFStatusB;   // Permanent Fail Status Register B
uint8_t value_PFStatusC;   // Permanent Fail Status Register C
uint8_t FET_Status;  // FET Status register contents  - Shows states of FETs
uint16_t CB_ActiveCells;  // Cell Balancing Active Cells

uint8_t	UV_Fault = 0;   // under-voltage fault state
uint8_t	OV_Fault = 0;   // over-voltage fault state
uint8_t	SCD_Fault = 0;  // short-circuit fault state
uint8_t	OCD_Fault = 0;  // over-current fault state
uint8_t	OCC_Fault = 0;  // over-current fault state
uint8_t ProtectionsTriggered = 0; // Set to 1 if any protection triggers

uint8_t LD_ON = 0;	// Load Detect status bit
uint8_t DSG = 0;   // discharge FET state
uint8_t CHG = 0;   // charge FET state
uint8_t PCHG = 0;  // pre-charge FET state
uint8_t PDSG = 0;  // pre-discharge FET state

uint32_t AccumulatedCharge_Int; // in BQ769x2_READPASSQ func
uint32_t AccumulatedCharge_Frac;// in BQ769x2_READPASSQ func
uint32_t AccumulatedCharge_Time;// in BQ769x2_READPASSQ func

//uint16_t	bms_int_ass;
//uint8_t error_long;
uint16_t error_long_flag=0;
bms_pro_flag_t bms_pro_flag={0};
uint32_t pin_io_mode_cache;
/* USER CODE END PV */



uint8_t IIC_WriteData(unsigned char u8DeviceAddr, unsigned char u8Address, unsigned char *pu8Data, unsigned int u32DataLen)
{
	//i2c_master_write(DEV_ADDR, u8Address, pu8Data, u32DataLen);
	I2C_WriteMulti(DEV_ADDR, u8Address, pu8Data, u32DataLen);
	return 0;
	
}


uint8_t IIC_ReadData(unsigned char u8DeviceAddr, unsigned char u8Address , unsigned char *pu8Data, unsigned int u32DataLen)
{
	
	//i2c_master_read(DEV_ADDR, u8Address, pu8Data, u32DataLen);
	I2C_ReadMulti(DEV_ADDR, u8Address, pu8Data, u32DataLen);
	return 0;
}
	

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t copyIndex = 0;
    for (copyIndex = 0; copyIndex < count; copyIndex++)
    {
        dest[copyIndex] = source[copyIndex];
    }
}

unsigned char Checksum(unsigned char *ptr, unsigned char len)
// Calculates the checksum when writing to a RAM register. The checksum is the inverse of the sum of the bytes.	
{
	unsigned char i;
	unsigned char checksum = 0;

	for(i=0; i<len; i++)
		checksum += ptr[i];

	checksum = 0xff & ~checksum;

	return(checksum);
}

unsigned char CRC8(unsigned char *ptr, unsigned char len)
//Calculates CRC8 for passed bytes. Used in i2c read and write functions 
{
	unsigned char i;
	unsigned char crc=0;
	while(len--!=0)
	{
		for(i=0x80; i!=0; i/=2)
		{
			if((crc & 0x80) != 0)
			{
				crc *= 2;
				crc ^= 0x107;
			}
			else
				crc *= 2;

			if((*ptr & i)!=0)
				crc ^= 0x107;
		}
		ptr++;
	}
	return(crc);
}


uint16_t write_nack=0;
void I2C_WriteReg(uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
	// MAX_BUFFER_SIZE
	uint8_t TX_Buffer [20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t max_cont=5;
	uint8_t crc_count = 0;
		crc_count = count * 2;
		uint8_t crc1stByteBuffer [3] = {0x10, reg_addr, reg_data[0]};
		unsigned int j;
		unsigned int i;
		uint8_t temp_crc_buffer [3];

		TX_Buffer[0] = reg_data[0];
		TX_Buffer[1] = CRC8(crc1stByteBuffer,3);

		j = 2;
		for(i=1; i<count; i++)
		{
			TX_Buffer[j] = reg_data[i];
			j = j + 1;
			temp_crc_buffer[0] = reg_data[i];
			//printf("j:%d\n",j);
			TX_Buffer[j] = CRC8(temp_crc_buffer,1); //TODO
			j = j + 1;
		}
		 while(IIC_WriteData(DEV_ADDR, reg_addr, TX_Buffer, crc_count)&&(max_cont--)){
				//sys_clr_wdt();
		 }
//		if(!max_cont){
//			write_nack++;
//		}
	
}

uint16_t RX_CRC_Fail = 0;  // reset to 0. If in CRC Mode and CRC fails, this will be incremented.
	
uint8_t I2C_ReadReg(uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
	//uint8_t RX_Buffer [MAX_BUFFER_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	RX_CRC_Fail=0;
	IIC_ReadData(DEV_ADDR, reg_addr, RX_32Byte, count*2);
	
	uint8_t crc1stByteBuffer [4] = {0x10, reg_addr, 0x11, RX_32Byte[0]};
	if( CRC8(crc1stByteBuffer,4)!=RX_32Byte[1])RX_CRC_Fail++;
	reg_data[0]=RX_32Byte[0];
	for(uint8_t i=1;i<count;i++){
			reg_data[i]=RX_32Byte[i*2];
		if(RX_32Byte[i*2+1]!=CRC8(reg_data+i,1)){
			RX_CRC_Fail++;
		}
	}
//	if(RX_CRC_Fail) togo_IO();
//	Uart_PutChar(0xfc);
//	Uart_PutChar(0xfc);
//	Uart_PutChar(0xfc);
	return RX_CRC_Fail;
}

uint16_t u16_nack_flag = 0;
void delay_us_2ms(void)
{
	__IO uint32_t t = 500;
	while(t--);
}
void BQ769x2_SetRegister(uint16_t reg_addr, uint32_t reg_data, uint8_t datalen)
{
	uint8_t TX_Buffer[2] = {0x00, 0x00};
	uint8_t TX_RegData[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	//TX_RegData in little endian format
	TX_RegData[0] = reg_addr & 0xff; 
	TX_RegData[1] = (reg_addr >> 8) & 0xff;
	TX_RegData[2] = reg_data & 0xff; //1st byte of data

	switch(datalen)
    {
		case 1: //1 byte datalength
      		I2C_WriteReg(0x3E, TX_RegData, 3);
			///延时/////
			if(u16_nack_flag) break;
			delay_us_2ms();
			TX_Buffer[0] = Checksum(TX_RegData, 3); 
			TX_Buffer[1] = 0x05; //combined length of register address and data
      		I2C_WriteReg(0x60, TX_Buffer, 2); // Write the checksum and length
			///延时/////
			delay_us_2ms();
			break;
		case 2: //2 byte datalength
			TX_RegData[3] = (reg_data >> 8) & 0xff;
			I2C_WriteReg(0x3E, TX_RegData, 4);
			///延时/////
			if(u16_nack_flag) break;
			delay_us_2ms();
			TX_Buffer[0] = Checksum(TX_RegData, 4); 
			TX_Buffer[1] = 0x06; //combined length of register address and data
      		I2C_WriteReg(0x60, TX_Buffer, 2); // Write the checksum and length
			///延时/////
			delay_us_2ms();
			break;
		case 4: //4 byte datalength, Only used for CCGain and Capacity Gain
			TX_RegData[3] = (reg_data >> 8) & 0xff;
			TX_RegData[4] = (reg_data >> 16) & 0xff;
			TX_RegData[5] = (reg_data >> 24) & 0xff;
			I2C_WriteReg(0x3E, TX_RegData, 6);
			///延时/////
			if(u16_nack_flag) break;
			delay_us_2ms();
			TX_Buffer[0] = Checksum(TX_RegData, 6); 
			TX_Buffer[1] = 0x08; //combined length of register address and data
      		I2C_WriteReg(0x60, TX_Buffer, 2); // Write the checksum and length
			///延时/////
			delay_us_2ms();
			break;
    }
}

void BMS_FET_ENABLE(uint8_t sw){
	uint8_t TX_Reg[2]; 

	//TX_Reg in little endian format
	TX_Reg[0] = FET_ENABLE & 0xff;
	TX_Reg[1] = (FET_ENABLE >> 8) & 0xff;
	Subcommands(0x0057,0x0,R);
	if(((RX_32Byte[0]>>4)&1)!=sw){
		I2C_WriteReg(0x3E,TX_Reg,2);
	}
	
}
void CommandSubcommands(uint16_t command) //For Command only Subcommands
// See the TRM or the BQ76952 header file for a full list of Command-only subcommands
{	//For DEEPSLEEP/SHUTDOWN subcommand you will need to call this function twice consecutively
	
	uint8_t TX_Reg[2] = {0x00, 0x00};

	//TX_Reg in little endian format
	TX_Reg[0] = command & 0xff;
	TX_Reg[1] = (command >> 8) & 0xff;
	I2C_WriteReg(0x3E,TX_Reg,2); 
	
	//delay_us(2000,10);///延时/////
}

//uint8_t uart_cache[20];
uint8_t bms_init(){
			BQ769x2_RESET_DSG_OFF ();
			BQ769x2_RESET_CHG_OFF ();
			BQ769x2_ReleaseShutdownPin();
			//CommandSubcommands(BQ769x2_RESET);
            delay_us_2ms();
            delay_us_2ms();
            delay_us_2ms();                    // Resets the BQ769x2 registers
            CommandSubcommands(SLEEP_DISABLE); // Sleep mode is enabled by default. For this example, Sleep is disabled to 
			
			//delay_us(1000,10);///延时//////
			
//									delay_us(1000,10);///延时//////
//									CommandSubcommands(FET_ENABLE); // Enable the CHG and DSG FETs
			if(!BQ769x2_Init()){  // Configure all of the BQ769x2 register settings
				printf("here1");
				return 0;
			}//delay_us(1000,10);///延时//////
			delay_us_2ms();					

//			set_low_cell_predsg(2000,1);
			//delay_us(1000,10);///延时//////
			delay_us_2ms();
//			Subcommands(0x9304,0x00,R);
//			uart_cache[0]=RX_32Byte[0];
//			uart_cache[1]=RX_32Byte[1];
//			delay_us(2000,10);///延时/////
//			Subcommands(0x9234,0x00,R);
//			uart_cache[2]=RX_32Byte[0];
//			uart_cache[3]=RX_32Byte[1];
//			delay_us(2000,10);///延时/////
//			Subcommands(0x9261,0x00,R);
//			uart_cache[4]=RX_32Byte[0];
//			uart_cache[5]=RX_32Byte[1];
//			delay_us(2000,10);///延时/////
//			Subcommands(0x9275,0x00,R);
//			uart_cache[7]=RX_32Byte[0];
//			uart_cache[8]=RX_32Byte[1];
//			Subcommands(0x0057,0x0,R);
            
            CommandSubcommands(FET_ENABLE);
            CommandSubcommands(SLEEP_DISABLE); // Sleep mode is enabled by default. For this example, Sleep is disabled to
			CommandSubcommands(ALL_FETS_ON);
			//Subcommands(FET_CONTROL, 0x07, W);
            BMS_FET_ENABLE(1);
            Subcommands(0x0057, 0x0, R);

            //			write_error_long_flag();
            write_nack = 0;
            if (RX_32Byte[0] & 0x10)
            {
                return 1;
		}else{
			return 0;
		}
}
uint8_t t_power_on_cnt_s=1;
void read_data_cmd_long(uint16_t command,uint16_t len){
	
	uint8_t TX_Reg[2];
	TX_Reg[0] = command & 0xff;
	TX_Reg[1] = (command >> 8) & 0xff; 
	I2C_WriteReg(0x3E,TX_Reg,2);
	///延时/////
	delay_us_2ms();
	I2C_ReadReg(0x40, RX_32Byte, len); //RX_32Byte is a global variable
}
void Subcommands(uint16_t command, uint16_t data, uint8_t type)
// See the TRM or the BQ76952 header file for a full list of Subcommands
{
	//security keys and Manu_data writes dont work with this function (reading these commands works)
	//max readback size is 32 bytes i.e. DASTATUS, CUV/COV snapshot
	uint8_t TX_Reg[4] = {0x00, 0x00, 0x00, 0x00};
	uint8_t TX_Buffer[2] = {0x00, 0x00};

	//TX_Reg in little endian format
	TX_Reg[0] = command & 0xff;
	TX_Reg[1] = (command >> 8) & 0xff; 

	if (type == R) {//read
		I2C_WriteReg(0x3E,TX_Reg,2);
		///延时/////
		delay_us_2ms();
		I2C_ReadReg(0x40, RX_32Byte, 2); //RX_32Byte is a global variable
	}
	else if (type == W) {
		//FET_Control, REG12_Control
		TX_Reg[2] = data & 0xff; 
		I2C_WriteReg(0x3E,TX_Reg,3);
		///延时/////
		delay_us_2ms();
		TX_Buffer[0] = Checksum(TX_Reg, 3);
		TX_Buffer[1] = 0x05; //combined length of registers address and data
		I2C_WriteReg(0x60, TX_Buffer, 2);
		///延时///// 
		delay_us_2ms();
	}
	else if (type == W2){ //write data with 2 bytes
		//CB_Active_Cells, CB_SET_LVL
		TX_Reg[2] = data & 0xff; 
		TX_Reg[3] = (data >> 8) & 0xff;
		I2C_WriteReg(0x3E,TX_Reg,4);
		///延时/////
		delay_us_2ms();
		TX_Buffer[0] = Checksum(TX_Reg, 4); 
		TX_Buffer[1] = 0x06; //combined length of registers address and data
		I2C_WriteReg(0x60, TX_Buffer, 2);
		///延时///// 
		delay_us_2ms();
	}
}

void DirectCommands(uint8_t command, uint16_t data, uint8_t type)
// See the TRM or the BQ76952 header file for a full list of Direct Commands
{	//type: R = read, W = write
	uint8_t TX_data[2] = {0x00, 0x00};

	//little endian format
	TX_data[0] = data & 0xff;
	TX_data[1] = (data >> 8) & 0xff;

	if (type == R) {//Read
		I2C_ReadReg(command, RX_data, 2); //RX_data is a global variable
		///延时/////
		delay_us_2ms();	
	}
	if (type == W) {//write
    //Control_status, alarm_status, alarm_enable all 2 bytes long
		I2C_WriteReg(command,TX_data,2);
		///延时/////
		delay_us_2ms();
	}
}
uint16_t write_res=0;
typedef union{
	float float_type;
	uint32_t uint_type;
}bms_gain_t;
bms_gain_t bms_ccGain={3.7842*2};
bms_gain_t bms_capGain={1128681.61*2};
void BSWBq769x2ReadBatteryStatus(void) 
{
	DirectCommands(BatteryStatus, 0x00, R);
	 write_res= (RX_data[1]<<8) + RX_data[0];
}
#define BMS_INIT_ARRAY_EN  0
#if BMS_INIT_ARRAY_EN == 0
bms_init_value_t bms_init_array[] = {
    // After entering CONFIG_UPDATE mode, RAM registers can be programmed. When programming RAM, checksum and length must also be
    // programmed for the change to take effect. All of the RAM registers are described in detail in the BQ769x2 TRM.
    // An easier way to find the descriptions is in the BQStudio Data Memory screen. When you move the mouse over the register name,
    // a full description of the register and the bits will pop up on the screen.

    // 'Power Config' - 0x9234 = 0x2D80
    // Setting the DSLP_LDO bit allows the LDOs to remain active when the device goes into Deep Sleep mode
    // Set wake speed bits to 00 for best performance
    {PowerConfig, 0x2D80, 2},
    //	{ShutdownCellVoltage,2000,2},//休眠前设置2.5V shutdown电压

    // 'REG0 Config' - set REG0_EN bit to enable pre-regulator
    {REG0Config, 0x01, 1},

    // 'REG12 Config' - Enable REG1 with 3.3V output (0x0D for 3.3V, 0x0F for 5V)
    {REG12Config, 0x0D, 1},

    // Set DFETOFF pin to control BOTH CHG and DSG FET - 0x92FB = 0x42 (set to 0x00 to disable)
    {DFETOFFPinConfig, 0x02, 1},
    {CFETOFFPinConfig, 0x02, 1},

    // Set up ALERT Pin - 0x92FC = 0x2A
    // This configures the ALERT pin to drive high (REG1 voltage) when enabled.
    // The ALERT pin can be used as an interrupt to the MCU when a protection has triggered or new measurements are available
    {ALERTPinConfig, 0x2A, 1},

    // Set TS1 to measure Cell Temperature - 0x92FD = 0x07
    {TS1Config, 0x0B, 1},
    {TS2Config, 0x0B, 1},

    // Set TS3 to measure FET Temperature - 0x92FF = 0x0F
    {TS3Config, 0x0B, 1},

    //	// Set HDQ to measure Cell Temperature - 0x9300 = 0x07
    {HDQPinConfig, 0x0B, 1}, // No thermistor installed on EVM HDQ pin, so set to 0x00

    // 'VCell Mode' - Enable 16 cells - 0x9304 = 0x0000; Writing 0x0000 sets the default of 16 cells
    {VCellMode, 0x0157, 2},

    // Enable protections in 'Enabled Protections A' 0x9261 = 0xBC
    // Enables SCD (short-circuit), OCD1 (over-current in discharge), OCC (over-current in charge),
    // COV (over-voltage), CUV (under-voltage)
    {EnabledProtectionsA, 0xBC, 1},

    // Enable all protections in 'Enabled Protections B' 0x9262 = 0xF7
    // Enables OTF (over-temperature FET), OTINT (internal over-temperature), OTD (over-temperature in discharge),
    // OTC (over-temperature in charge), UTINT (internal under-temperature), UTD (under-temperature in discharge), UTC (under-temperature in charge)
    {EnabledProtectionsB, 0xF7, 1},
    {EnabledProtectionsC, 0x00, 1},
    // 'Default Alarm Mask' - 0x..82 Enables the FullScan and ADScan bits, default value = 0xF800
    {DefaultAlarmMask, 0xF882, 2},

    // Set up Cell Balancing Configuration - 0x9335 = 0x03   -  Automated balancing while in Relax or Charge modes
    // Also see "Cell Balancing with BQ769x2 Battery Monitors" document on ti.com
    {BalancingConfiguration, 0x03, 1},
    //	{CellBalanceInterval,240,1},
    {MinCellTemp, 0, 1},
    {MaxCellTemp, 60, 1},
    {MaxInternalTemp, 70, 1},
    //	{CellBalanceInterval,20,1},//default
    {CellBalanceMaxCells, 1, 1},
    {CellBalanceMinCellVCharge, 3000, 2},
    {CellBalanceMinDeltaCharge, 50, 1},
    {CellBalanceStopDeltaCharge, 30, 1},
    {CellBalanceMinCellVRelax, 3000, 2},
    {CellBalanceMinDeltaRelax, 50, 1},
    {CellBalanceStopDeltaRelax, 30, 1},

    // Set up CUV (under-voltage) Threshold - 0x9275 = 0x31 (2479 mV)
    // CUV Threshold is this value multiplied by 50.6mV
    {CUVThreshold, 35, 1}, //
    {CUVDelay, 1515, 2},   //{CUVDelay,1,2},//
    {CUVRecoveryHysteresis, 6, 1},

    // Set up COV (over-voltage) Threshold - 0x9278 = 0x55 (4301 mV)
    // COV Threshold is this value multiplied by 50.6mV
    {COVThreshold, 75, 1}, // 3.795V
    {COVDelay, 1515, 2},
    {COVRecoveryHysteresis, 2, 1}, // 3.7V

    // Set up OCC (over-current in charge) Threshold - 0x9280 = 0x05 (10 mV = 10A across 1mOhm sense resistor) Units in 2mV
    {OCCThreshold, 3, 1},
    {OCCDelay, 128, 1}, // 425ms
    {OCCRecoveryThreshold, 1000, 2},
    {OCCPACKTOSDelta, 10, 2},

    // Set up OCD1 Threshold - 0x9282 = 0x0A (20 mV = 20A across 1mOhm sense resistor) units of 2mV
    {OCD1Threshold, 8, 1}, // 16A
    {OCD1Delay, 128, 1},   // 425MS 3.3ms/lsb

    {OCD2Threshold, 16, 1}, // 32A
    {OCD2Delay, 12, 1},   // 40MS 3.3ms/lsb

    // Set up SCD Threshold - 0x9286 = 0x05 (100 mV = 100A across 1mOhm sense resistor)  0x05=100mV
    {SCDThreshold, 3, 1}, //60A

    // Set up SCD Delay - 0x9287 = 0x03 (30 us) Enabled with a delay of (value - 1) * 15 μs; min value of 1
    {SCDDelay, 21, 1},  //300us
    //   {SCDDelay, 21, 1},
    // Set up SCDL Latch Limit to 1 to set SCD recovery only with load removal 0x9295 = 0x01
    // If this is not set, then SCD will recover based on time (SCD Recovery Time parameter).
    {SCDLLatchLimit, 0x01, 1},

    {OCDRecoveryThreshold, (uint16_t)-100, 2},
    {DAConfiguration, 0x01, 1},
    {EnabledProtectionsC, 0x00, 1},
    {CHGFETProtectionsA, 0x98, 1},
    {CHGFETProtectionsB, 0x50, 1},
    {CHGFETProtectionsC, 0x00, 1},

    {DSGFETProtectionsA, 0xE4, 1}, // default
    {DSGFETProtectionsB, 0x60, 1},
    {DSGFETProtectionsC, 0x00, 1},
    {BodyDiodeThreshold, 2000, 2},
    {SFAlertMaskA, 0xFC, 1},
    {SFAlertMaskB, 0x70, 1},
    {SFAlertMaskC, 0x00, 1},
    {PFAlertMaskA, 0x5F, 1},
    {PFAlertMaskB, 0x9F, 1},
    {PFAlertMaskC, 0x07, 1},
    {PFAlertMaskD, 0x00, 1},
    {EnabledPFA, 0x00, 1},
    {EnabledPFB, 0x00, 1},
    {EnabledPFC, 0x00, 1},
    {EnabledPFD, 0x00, 1},
    {FETOptions, 0x1F, 1},
    //	{PrechargeStartVoltage,2500,2},  //Settings:FET:Precharge Start Voltage
    //	{PrechargeStopVoltage,2550,2},     //Settings:FET:Precharge Stop Voltage
    {PredischargeTimeout, 1, 1}, // Settings:FET:Predischarge Timeout
    {PredischargeStopDelta, 200, 1},
    {ChgPumpControl, 0x01, 1},
    {SCDRecoveryTime, 255, 1},
    {ProtectionsRecoveryTime, 3, 1}, // default
    {HWDDelay, 120, 1},
};
uint8_t load_bms_init_array(void){
	uint8_t err_cnt=0;
	for(int i=0;i<sizeof(bms_init_array)/sizeof(bms_init_array[0]);i++){
		BQ769x2_SetRegister(bms_init_array[i].addr,bms_init_array[i].value,bms_init_array[i].len);
		if(!u16_nack_flag){
#if DEBUG_INFO
			Uart_PutChar(0x33);
			Uart_PutChar(bms_init_array[i].addr>>8);
			Uart_PutChar(bms_init_array[i].addr);
			
#endif
			err_cnt++;
		}
		//sys_clr_wdt();
	}
	return 1;//!err_cnt;
}
#endif
uint8_t turn_to_CFGUPDATE(){
	uint16_t u16ReTryCnt=50;
	CommandSubcommands(SET_CFGUPDATE);	
	/* After entering CONFIG_UPDATE mode, RAM registers can be programmed. 
		When programming RAM, checksum and length must also be programmed for the change to take effect. 
		All of the RAM registers are described in detail in the BQ769x2 TRM. 
		An easier way to find the descriptions is in the BQStudio Data Memory screen. 
		When you move the mouse over the register name,
		a full description of the register and the bits will pop up on the screen.*/

	/* 'Power Config' - 0x9234 = 0x2D80
		Setting the DSLP_LDO bit allows the LDOs to remain active when the device goes into Deep Sleep mode
		Set wake speed bits to 00 for best performance */
	//delay_us(35000,12);
	BSWBq769x2ReadBatteryStatus();
	while((!(write_res&0x0001))&&(u16ReTryCnt > 0))
	{
		CommandSubcommands(SET_CFGUPDATE);
		BSWBq769x2ReadBatteryStatus();
		u16ReTryCnt --;
	//	sys_clr_wdt();
	}
	if(!u16ReTryCnt){
		CommandSubcommands(EXIT_CFGUPDATE);
		return 0;
	}else{
		return 1;
	}
}
void set_low_cell_predsg(uint16_t Vcell,uint16_t en){
	turn_to_CFGUPDATE();
	BQ769x2_SetRegister(ShutdownCellVoltage, Vcell, 2);
	if(en)
		BQ769x2_SetRegister(FETOptions,0x1F,1);
	else
		BQ769x2_SetRegister(FETOptions,0x0F,1);
	CommandSubcommands(EXIT_CFGUPDATE);
}
uint8_t BQ769x2_Init()
{
    // Configures all parameters in device RAM
    // Enter CONFIGUPDATE mode (Subcommand 0x0090) - It is required to be in CONFIG_UPDATE mode to program the device RAM settings
    // See TRM for full description of CONFIG_UPDATE mode
    //	CommandSubcommands(SET_CFGUPDATE);
    //CommandSubcommands(BQ769x2_RESET);
    if (!turn_to_CFGUPDATE())
    {
        return 0;
    }
#if BMS_INIT_ARRAY_EN == 0
    if (load_bms_init_array())
    {
		CommandSubcommands(ALL_FETS_ON);
        BQ769x2_SetRegister(CCGain, bms_ccGain.uint_type, 4);
        BQ769x2_SetRegister(CapacityGain, bms_capGain.uint_type, 4);

        CommandSubcommands(EXIT_CFGUPDATE);
		
        return 1;
    }
    else
    {
        // Exit CONFIGUPDATE mode  - Subcommand 0x0092
        CommandSubcommands(EXIT_CFGUPDATE);
        return 0;
    }
#else
    // Configures all parameters in device RAM



    // 'Power Config' - 0x9234 = 0x2D80
    // 置位DPSLP_LDO ：进入休眠模式，让LDOs保持激活状态
    // 设置 wake speed bits to 00 ： 全速模式
    BQ769x2_SetRegister(PowerConfig, 0x2D80, 2);

    // 'REG0 Config' - set REG0_EN bit ： 启用前置稳压器
    BQ769x2_SetRegister(REG0Config, 0x01, 1);

    // 'REG12 Config' - 使能 REG1 输出3.3V
    BQ769x2_SetRegister(REG12Config, 0x0D, 1);

    // Set DFETOFF pin to control BOTH CHG and DSG FET 输出 - 0x92FB = 0x42 (set to 0x00 to disable)
    BQ769x2_SetRegister(DFETOFFPinConfig, 0x82, 1);
    BQ769x2_SetRegister(DFETOFFPinConfig, 0x82, 1);

    // Set up ALERT Pin - 0x92FC = 0x2A
    // This configures the ALERT pin to drive high (REG1 voltage) when enabled.
    // 在 ALERT 引脚上生成警报信号的功能， 该信号可用作主机处理器的中断。
    BQ769x2_SetRegister(ALERTPinConfig, 0x2A, 1);

    // Set TS1 to measure Cell Temperature - 0x92FD = 0x07
    BQ769x2_SetRegister(TS1Config, 0x07, 1);
    BQ769x2_SetRegister(TS2Config, 0x07, 1);
    // Set TS3 to measure FET Temperature - 0x92FF = 0x0F
    BQ769x2_SetRegister(TS3Config, 0x0F, 1);

    // Set HDQ to measure Cell Temperature - 0x9300 = 0x07
    BQ769x2_SetRegister(HDQPinConfig, 0x00, 1); // No thermistor热敏电阻 installed on EVM评估版 HDQ pin, so set to 0x00

    // 'VCell Mode' - Enable 16 cells - 0x9304 = 0x0000; Writing 0x0000 sets the default of 16 cells
    BQ769x2_SetRegister(VCellMode, 0x0157, 2);

    // 使能保护功能： 'Enabled Protections A' 0x9261 = 0xBC
    // Enables SCD (short-circuit), OCD1 (over-current in discharge), OCC (over-current in charge),
    // COV (over-voltage), CUV (under-voltage)
    BQ769x2_SetRegister(EnabledProtectionsA, 0x00, 1);

    // 使能所有保护： 'Enabled Protections B' 0x9262 = 0xF7
    // Enables OTF (over-temperature FET), OTINT (internal over-temperature), OTD (over-temperature in discharge),
    // OTC (over-temperature in charge), UTINT (internal under-temperature), UTD (under-temperature in discharge), UTC (under-temperature in charge)
    BQ769x2_SetRegister(EnabledProtectionsB, 0x00, 1);
    BQ769x2_SetRegister(EnabledProtectionsC, 0x00, 1);

    // 'Default Alarm Mask' - 0x..82 Enables the FullScan and ADScan bits, default value = 0xF800
    BQ769x2_SetRegister(DefaultAlarmMask, 0x0000, 2);

    // 设置 Balancing Configuration - 0x9335 = 0x03   -  Automated balancing while in Relax or Charge modes
    //  Also see "Cell Balancing with BQ769x2 Battery Monitors" document on ti.com
    BQ769x2_SetRegister(BalancingConfiguration, 0x03, 1);

    // Set up CUV (under-voltage) Threshold - 0x9275 = 0x31 (2479 mV)
    // CUV Threshold is this value multiplied by 50.6mV
    BQ769x2_SetRegister(CUVThreshold, 0x31, 1);

    // Set up COV (over-voltage) Threshold - 0x9278 = 0x55 (4301 mV)
    // COV Threshold is this value multiplied by 50.6mV
    BQ769x2_SetRegister(COVThreshold, 0x55, 1);

    // Set up OCC (over-current in charge) Threshold - 0x9280 = 0x05 (10 mV = 10A across 1mOhm sense resistor) Units in 2mV
    BQ769x2_SetRegister(OCCThreshold, 0x05, 1);

    // Set up OCD1 Threshold - 0x9282 = 0x0A (20 mV = 20A across 1mOhm sense resistor) units of 2mV
    BQ769x2_SetRegister(OCD1Threshold, 0x0A, 1);

    // Set up SCD Threshold - 0x9286 = 0x05 (100 mV = 100A across 1mOhm sense resistor)  0x05=100mV
    BQ769x2_SetRegister(SCDThreshold, 0x05, 1);

    // Set up SCD Delay - 0x9287 = 0x03 (30 us) Enabled with a delay of (value - 1) * 15 ?s; min value of 1
    BQ769x2_SetRegister(SCDDelay, 0x03, 1);

    // Set up SCDL Latch Limit to 1 to set SCD recovery only with load removal 0x9295 = 0x01
    // If this is not set, then SCD will recover based on time (SCD Recovery Time parameter).
    BQ769x2_SetRegister(SCDLLatchLimit, 0x01, 1);


    BQ769x2_SetRegister(CCGain, bms_ccGain.uint_type, 4);
    BQ769x2_SetRegister(CapacityGain, bms_capGain.uint_type, 4);
    CommandSubcommands(FET_ENABLE);
    CommandSubcommands(EXIT_CFGUPDATE);
    
    CommandSubcommands(SLEEP_DISABLE);
    return 1;
#endif
        
}
#if 1
//  ********************************* FET Control Commands  ***************************************
void BQ769x2_DSG_OFF () {
	// Disables all FETs using the DFETOFF (BOTHOFF) pin
	// The DFETOFF pin on the BQ76952EVM should be connected to the MCU board to use this function
	__GPIO_PIN_SET(DFETOFF_PORT, DFETOFF_PIN);
	
	//GPIO_WriteBit(GPIOB, GPIO_PinSource10, Bit_SET);					//BMS - DIS MOS
	//GPIO_ModeConfig(GPIOB, GPIO_PinSource10,GPIO_Mode_OUT_PP);
}

void BQ769x2_RESET_DSG_OFF () {
	// Resets DFETOFF (BOTHOFF) pin
	// The DFETOFF pin on the BQ76952EVM should be connected to the MCU board to use this function
	
	__GPIO_PIN_RESET(DFETOFF_PORT, DFETOFF_PIN);
	//GPIO_WriteBit(GPIOB, GPIO_PinSource10, Bit_RESET);					//BMS - DIS MOS
	//GPIO_ModeConfig(GPIOB, GPIO_PinSource10,GPIO_Mode_OUT_PP);
}


void BQ769x2_CHG_OFF () {
	// Disables all FETs using the DFETOFF (BOTHOFF) pin
	// The DFETOFF pin on the BQ76952EVM should be connected to the MCU board to use this function
	
	__GPIO_PIN_SET(CFETOFF_PORT,CFETOFF_PIN);
	//GPIO_WriteBit(GPIOA, GPIO_PinSource3, Bit_SET);					//BMS - DIS MOS
	//GPIO_ModeConfig(GPIOA, GPIO_PinSource3,GPIO_Mode_OUT_PP);
}

void BQ769x2_RESET_CHG_OFF () {
	// Resets DFETOFF (BOTHOFF) pin
	// The DFETOFF pin on the BQ76952EVM should be connected to the MCU board to use this function

    __GPIO_PIN_RESET(CFETOFF_PORT, CFETOFF_PIN);
    //GPIO_WriteBit(GPIOA, GPIO_PinSource3, Bit_RESET);					//BMS - DIS MOS
	//GPIO_ModeConfig(GPIOA, GPIO_PinSource3,GPIO_Mode_OUT_PP);
}

void BQ769x2_ReadFETStatus() { 
	// Read FET Status to see which FETs are enabled
	DirectCommands(FETStatus, 0x00, R);
	FET_Status = (RX_data[1]*256 + RX_data[0]);
	DSG = ((0x4 & RX_data[0])>>2);// discharge FET state
  	CHG = (0x1 & RX_data[0]);// charge FET state
  	PCHG = ((0x2 & RX_data[0])>>1);// pre-charge FET state
  	PDSG = ((0x8 & RX_data[0])>>3);// pre-discharge FET state
	if(DSG==0){
		//dc_reset_flag=1;
	}
}
#endif
// ********************************* End of FET Control Commands *********************************

// ********************************* BQ769x2 Power Commands   *****************************************

#if 1
uint16_t  shutdown_bms_flag;
uint16_t shutdown_bms_cnt;
extern uint16_t pack_pin_voltage;
void BQ769x2_ShutdownPin() 
{
	// Puts the device into SHUTDOWN mode using the RST_SHUT pin
	// The RST_SHUT pin on the BQ76952EVM should be connected to the MCU board to use this function	
	__GPIO_PIN_SET(RST_SHUT_PORT,RST_SHUT_PIN);
	
}
void BQ769x2_ReleaseShutdownPin() 
{
	// Releases the RST_SHUT pin
	// The RST_SHUT pin on the BQ76952EVM should be connected to the MCU board to use this function
    __GPIO_PIN_RESET(RST_SHUT_PORT, RST_SHUT_PIN);
    // shutdown_bms_flag=0;
}
#endif
// ********************************* End of BQ769x2 Power Commands   *****************************************



// ********************************* BQ769x2 Status and Fault Commands   *****************************************

uint16_t BQ769x2_ReadAlarmStatus() { 
	// Read this register to find out why the ALERT pin was asserted
	DirectCommands(AlarmStatus, 0x00, R);
	return (RX_data[1]*256 + RX_data[0]);//*(uint16_t*)RX_data;//
}

uint16_t BQ769x2_ReadAlarmRaw(){
	DirectCommands(AlarmRawStatus, 0x00, R);
	return RX_data[1]*256 + RX_data[0];//*((uint16_t*)RX_data);//
}
void BQ769x2_ReadSafetyStatusA(){ //good example functions
	// Read Safety Status A/B/C and find which bits are set
	// This shows which primary protections have been triggered
	DirectCommands(SafetyStatusA, 0x00, R);
	if(RX_CRC_Fail) return;
	value_SafetyStatusA =(RX_data[1]*256 + RX_data[0]);// *(uint16_t*)RX_data;//
	//Example Fault Flags
	UV_Fault = ((0x4 & RX_data[0])>>2); 
	OV_Fault = ((0x8 & RX_data[0])>>3);
	SCD_Fault = ((0x80 & RX_data[0])>>7);
	OCD_Fault = ((0x20 & RX_data[0])>>5);
	OCC_Fault = ((0x10 & RX_data[0])>>4);
//	if(RX_data[0]){
//		Uart_PutChar(0xe1);
//		Uart_PutChar(0xea);
//		Uart_PutChar(RX_data[0]);
//	}
}
void BQ769x2_ReadSafetyStatusB(){
	DirectCommands(SafetyStatusB, 0x00, R);
	if(RX_CRC_Fail) return;
	value_SafetyStatusB = (RX_data[1]*256 + RX_data[0]);//*(uint16_t*)RX_data;//
}
void BQ769x2_ReadSafetyStatusC(){
	DirectCommands(SafetyStatusC, 0x00, R);
	if(RX_CRC_Fail) return;
	value_SafetyStatusC = (RX_data[1]*256 + RX_data[0]);//*(uint16_t*)RX_data;//
	if ((value_SafetyStatusA + value_SafetyStatusB + value_SafetyStatusC) > 1) {
		ProtectionsTriggered = 1; }
	else {
		ProtectionsTriggered = 0; }

}
void BQ769x2_ReadPFStatusA() {
	// Read Permanent Fail Status A/B/C and find which bits are set
	// This shows which permanent failures have been triggered
	DirectCommands(PFStatusA, 0x00, R);
	value_PFStatusA =(RX_data[1]*256 + RX_data[0]);// *(uint16_t*)RX_data;//
}
void BQ769x2_ReadPFStatusB(){
	DirectCommands(PFStatusB, 0x00, R);
	value_PFStatusB = (RX_data[1]*256 + RX_data[0]);// *(uint16_t*)RX_data;//
}
void BQ769x2_ReadPFStatusC(){
	DirectCommands(PFStatusC, 0x00, R);
	value_PFStatusC = (RX_data[1]*256 + RX_data[0]);// *(uint16_t*)RX_data;//
} 
// ********************************* End of BQ769x2 Status and Fault Commands   *****************************************


// ********************************* BQ769x2 Measurement Commands   *****************************************


uint16_t BQ769x2_ReadVoltage(uint8_t command)
// This function can be used to read a specific cell voltage or stack / pack / LD voltage
{
	//RX_data is global var
	DirectCommands(command, 0x00, R);
	if(command >= Cell1Voltage && command <= Cell16Voltage) {//Cells 1 through 16 (0x14 to 0x32)
		return (RX_data[1]*256 + RX_data[0]); //voltage is reported in mV
	}
	else {//stack, Pack, LD
		return (RX_data[1]*256 + RX_data[0]); //voltage is reported in 0.01V units
	}

}
void BQ769x2_ReadAllVoltages()
// Reads all cell voltages, Stack voltage, PACK pin voltage, and LD pin voltage
{
  int cellvoltageholder = Cell1Voltage; //Cell1Voltage is 0x14
  for (int x = 0; x < 16; x++){//Reads all cell voltages
    CellVoltage[x] = BQ769x2_ReadVoltage(cellvoltageholder);
    cellvoltageholder = cellvoltageholder + 2;
  }
  Stack_Voltage = BQ769x2_ReadVoltage(StackVoltage);
  sys.bat.vol = Stack_Voltage; // TODO
  Pack_Voltage = BQ769x2_ReadVoltage(PACKPinVoltage);
  LD_Voltage = BQ769x2_ReadVoltage(LDPinVoltage);
}

uint16_t V_cells[6];
void BQ769x2_Read_Vcells_123(){
	I2C_ReadReg(0x14,(uint8_t*)V_cells,6);
}
void BQ769x2_Read_Vcells_5(){
	I2C_ReadReg(0x14+8,(uint8_t*)V_cells+6,2);
}
void BQ769x2_Read_Vcells_7(){
	I2C_ReadReg(0x14+12,(uint8_t*)V_cells+8,2);
}
void BQ769x2_Read_Vcells_9(){
	I2C_ReadReg(0x14+16,(uint8_t*)V_cells+10,2);
}
int16_t BQ769x2_ReadCurrent() //2mA/lsb
// Reads PACK current 
{
	DirectCommands(CC2Current, 0x00, R);
	return (RX_data[1]*256 + RX_data[0]);// *(uint16_t*)RX_data;//  // current is reported in mA
}

uint16_t BQ769x2_ReadTemperature(uint8_t command) 
{
	DirectCommands(command, 0x00, R);
	//RX_data is a global var
	return (RX_data[1]*256 + RX_data[0]);// *(uint16_t*)RX_data;//  // converts from 0.1K to Celcius
}

void BQ769x2_ReadPassQ(){ // Read Accumulated Charge and Time from DASTATUS6 
	//Subcommands(DASTATUS6, 0x00, R);
	read_data_cmd_long(DASTATUS6,12);
	AccumulatedCharge_Int = ((RX_32Byte[3]<<24) + (RX_32Byte[2]<<16) + (RX_32Byte[1]<<8) + RX_32Byte[0]); //Bytes 0-3
	AccumulatedCharge_Frac = ((RX_32Byte[7]<<24) + (RX_32Byte[6]<<16) + (RX_32Byte[5]<<8) + RX_32Byte[4]); //Bytes 4-7
	AccumulatedCharge_Time = ((RX_32Byte[11]<<24) + (RX_32Byte[10]<<16) + (RX_32Byte[9]<<8) + RX_32Byte[8]); //Bytes 8-11
}

// ********************************* End of BQ769x2 Measurement Commands   *****************************************

void bq76942_reset(void)
{
    while (bms_init() == 0 || xbms.nack_cnt != 0)
    {
        xbms.nack_cnt = 0;
        xbms.ack_total = 0;
        printf("bms_init fail\n");
    };
    printf("bms_init OK\n");
}