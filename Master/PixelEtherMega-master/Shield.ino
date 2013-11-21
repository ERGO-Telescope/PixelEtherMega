// File: ErgoEtherMegaShield2.ino    2013-08-09
#include <pgmspace.h>

int ClassID;
long ch;
long flags;	
long checksum;
long Ground_Speed;
long Speed_3d;     
long Ground_Course;

long GPS_timer;
uint8_t ck_a;
uint8_t ck_b;
uint8_t step;
uint8_t UBX_Class;
uint8_t UBX_ID;
uint8_t UBX_length_hi;
uint8_t UBX_length_lo;
uint8_t UBX_counter;
uint8_t UBX_buffer[UBX_MAX_SIZE];
uint8_t UBX_ck_a;
uint8_t UBX_ck_b;

byte setCFG_CFG_Revert[] = {0xB5, 0x62,0x06 ,0x09, 0x0D ,0x00 ,0xFF, 0xFF, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x02, 0x1A, 0x99};// Factory Reset condition
//                             B5    62   06    00    14    00    01    00    00    00    D0    08    00    00    80    25    00    00    01    00    01    00    00    00    00    00    9A    79  //#1 Turn UART 1 on for USB at 9600 baud
byte setPRT_UART1_UBX_1[] = {0xB5, 0x62,0x06 ,0x00, 0x14 ,0x00 ,0x01, 0x00, 0x00 ,0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x9A, 0x79};// UART1 to UBX only 9600 baud
//                             B5    62   06    00    14    00    01    00    00    00    C0    08    00    00    80    25    00    00    01    00    01    00    00    00    00    00    8A    79 
byte setPRT_UART1_UBX_2[] = {0xB5, 0x62,0x06 ,0x00, 0x14 ,0x00 ,0x01, 0x00, 0x00 ,0x00, 0xC0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x8A, 0x79};// UART1 to UBX only 9600 baud

byte setPRT_USB_UBX[] = {0xB5, 0x62,0x06 ,0x00, 0x14 ,0x00 ,0x03, 0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x1F, 0x92}; // USB to UBX only for debugging
byte setTIM2_On[] ={0xB5, 0x62,0x06,0x01, 0x08,0x00,0x0D,0x03, 0x00,  0x01,  0x00,  0x01,  0x00,  0x00, 0x21, 0x28} ; // Turn UART1 and USB ON for TIM TM2
byte setTIM2_Off[] ={0xB5, 0x62,0x06,0x01, 0x08,0x00,0x0D,0x03, 0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 0x1F, 0x20} ; // Turn UART only OFF for TIM TM2
byte setNavPOSLHH_Off[] ={0xB5, 0x62,0x06,0x01, 0x08,0x00,0x01,0x02, 0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 0x12, 0xB9 } ; // Turn UART1 only off for NAV-POSLHH
byte setNavPOSLHH_On[] ={0xB5, 0x62,0x06,0x01, 0x08,0x00,0x01,0x02, 0x00,  0x01,  0x00,  0x01,  0x00,  0x00, 0x14, 0xC1} ;  // Turn UART1 and USB ON for NAV-POSLHH
byte setNavSol_On[] ={0xB5, 0x62,0x06,0x01, 0x08,0x00,0x01,0x06, 0x00,  0x01,  0x00,  0x01,  0x00,  0x00, 0x18, 0xDD } ;// Turn UART1 and USB ON for NAV SOL
byte setNavSol_Off[] ={0xB5, 0x62,0x06,0x01, 0x08,0x00,0x01,0x06, 0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 0x16, 0xD5 };// Turn UART1 only OFF for NAV SOL
byte setRATE_1000[] ={0xB5, 0x62,0x06,0x08, 0x06,0x00,0xE8,0x03, 0x01, 0x00,  0x01,  0x00,  0x01,  0x39  } ;
byte setRATE_200[] = {0xB5, 0x62,0x06,0x08, 0x06,0x00,0xC8,0x00, 0x01, 0x00,  0x01,  0x00,   0xDE, 0x6A, } ;
//byte pollTIM_TM2[] = {0xB5, 0x62,0x0D,0x03, 0x00,0x00,0x10,0x3D} ; PROGMEM prog_uchar pollTIM_TM2[] = {0xB5, 0x62,0x0D,0x03, 0x00,0x00,0x10,0x3D} ;

int NewMessage;

// Send a Configuration Message to the ERGO Shieldvoid CFG (byte * cmd,int size)
void CFG (byte * cmd,int size)
{	

	for (int i=0;i<size;i++)
	{
		//	Serial.println(cmd[i],HEX);
		Serial1.write(cmd[i]); 
		delay(10);
	}
}


void ShieldInit() // STATE 1
{
	CFG(setCFG_CFG_Revert,sizeof(setCFG_CFG_Revert));// Set ublox to factory reset condition
	Serial.println("....ublox now in factory reset condition ");
	CFG( setPRT_UART1_UBX_1,sizeof( setPRT_UART1_UBX_1));// UART1 to UBX at 9600 baud
	delay(200);
	CFG( setPRT_UART1_UBX_2,sizeof( setPRT_UART1_UBX_2));// UART1 to UBX at 9600 baud
	//CFG(setPRT_UART1_UBX,sizeof(setPRT_UART1_UBX));// setPRT_UART1_UBX sets UART1 Protocol IN to UBX and OUT to UBX,115200 baud, N8
	//Serial1.begin(115200);// Now kick the Arduino serial port up to 115200 baud
	//Serial1.begin(38400);// Now kick the Arduino serial port up to 38400 baud
	//CFG(setPRT_UART1_UBX,sizeof(setPRT_UART1_UBX));// Do set PRT_UART1_UBX  a second time at 115200 baud to be sure
	CFG(setPRT_USB_UBX,sizeof(setPRT_USB_UBX));// setPRT1 sets USB Protocol IN to UBX and OUT to UBX 
	CFG( setNavSol_On,sizeof(setNavSol_On));  // Turn on all three messages.
	CFG( setNavPOSLHH_On,sizeof(setNavPOSLHH_On));
	CFG(setTIM2_On,sizeof(setTIM2_On));
	//CFG(setRATE_200,sizeof(setRATE_200));// Run the rate 'balls to the wall'


}

// Collect Serial1 data from Ergo Shield
int  MessageReceived( )
{

	static unsigned long GPS_timer=0;
	byte data; 
	int ret = 0;// Preset the return in case of no message parsed
	if (  Serial1.available() )
	{
		data = Serial1.read();

		switch (step)
		{

		case 0:    
			if(data==0xB5) // Synch Char 1 (dec 181)
				step++;
			break;
		case  1: 
			if(data==0x62)  // Synch Char 2 (dec 98)
				step ++; 
			else 
				step=0; 
			break;
		case 2:
			UBX_Class=data;	// Class byte
			Shieldubx_checksum(UBX_Class);
			step++;
			break;
		case 3:
			UBX_ID=data;	// ID
			Shieldubx_checksum(UBX_ID);
			step++;
			break;
		case 4:
			UBX_length_hi=data;	// Length
			Shieldubx_checksum(UBX_length_hi);
			step++;
			if (UBX_length_hi>=UBX_MAX_SIZE)
			{   
				step=0;   
				ck_a=0;
				ck_b=0;
			}
			break;
		case 5: 
			UBX_length_lo=data;
			Shieldubx_checksum(UBX_length_lo);
			step++;
			UBX_counter=0;
			break;
		case 6 :
			if (UBX_counter < UBX_length_hi)  
			{
				UBX_buffer[UBX_counter] = data;
				Shieldubx_checksum(data);
				UBX_counter++;
				if (UBX_counter==UBX_length_hi)
					step++;
			}
			break;
		case 7: 
			UBX_ck_a=data; 
			step++;
			break;
		case 8:
			UBX_ck_b=data; 

			if((ck_a==UBX_ck_a)&&(ck_b==UBX_ck_b)) 
			{
				step = 0;
				ck_a = 0;
				ck_b = 0;

				//	==================================================     ParseMessage();          ==========================
				int j;
				if(UBX_Class==0x01 && UBX_ID==0x02)// NAV POSLHH
				{

					j=0;
					iTOW = join_4_bytes(&UBX_buffer[j]); // ms Time of week
					j+=4;
					lon = join_4_bytes(&UBX_buffer[j]); // lon*10000000
					j+=4;							
					lat = join_4_bytes(&UBX_buffer[j]);
					j+=4;
					height = join_4_bytes(&UBX_buffer[j]);

					j+=4;

					hMSL = join_4_bytes(&UBX_buffer[j]);  // MSL heigth mm

					ret = 0x0102;
				}


				if (UBX_Class==0x01 && UBX_ID == 0x06 )//  N A V - S O L 

					if ( UBX_buffer[10]	== 0x03)  
					{//and FIX3D

						UBX_ecefVZ=join_4_bytes(&UBX_buffer[36]);  //Vertical Speed in cm/s
						NumSats=UBX_buffer[47];                    //Number of sats...   
						Fix3D = true;
						ret = 0x0106;
					}
					else
					{

						Fix3D = false;
					}


					if (UBX_Class == 0x05)// Class Ack/Nak
					{	
						if(UBX_ID==0x01)//Checking the UBX ID
						{
							// Ignore ACKs and NAKs
						}
						if(UBX_ID== 0x00) //ID = Nak
						{
							// Ignore ACKs and NAKs
						}
					}

					if (UBX_Class ==0x0D && UBX_ID==0x03)//	 TIM used to collect the event parameters )
					{

						ch = done_byte(&UBX_buffer[0]);
						flags = done_byte(&UBX_buffer[1]);	
						wnR = join_2_bytes(&UBX_buffer[4]);
						towMsR = join_4_bytes(&UBX_buffer[8]);
						towSubMsR = join_4_bytes(&UBX_buffer[12]);
						checksum = join_2_bytes(&UBX_buffer[28]);

						ret = 0x0D03; 

					}

			}
			else
			{
				step = 0;
				ck_a = 0;
				ck_b = 0;
			}
			break;
		}

	}//end Serial Available

	return ret;
}

// Join 4 bytes into a long
long join_4_bytes(unsigned char Buffer[])
{
	union long_union {
		int32_t dword;
		uint8_t  byte[4];
	} longUnion;

	longUnion.byte[0] = *Buffer;
	longUnion.byte[1] = *(Buffer+1);
	longUnion.byte[2] = *(Buffer+2);
	longUnion.byte[3] = *(Buffer+3);
	return(longUnion.dword);
}
// join 2 bytes into a long
long join_2_bytes(unsigned char Buffer[])
{
	union long_union {
		int32_t dword;
		uint8_t  byte[2];
	} longUnion;

	longUnion.byte[0] = *Buffer;
	longUnion.byte[1] = *(Buffer+1);
	return(longUnion.dword);
}
long done_byte(unsigned char Buffer[])
{
	union long_union {
		int32_t dword;
		uint8_t  byte[1];
	} longUnion;

	longUnion.byte[0] = *Buffer;
	return(longUnion.dword);
}
// Ublox checksum algorithm
void Shieldubx_checksum(byte ubx_data)
{
	ck_a+=ubx_data;
	ck_b+=ck_a; 
}
