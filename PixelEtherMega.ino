// Pixel code for EtherMega and Shield

String today = "ERGO PixelEtherMega 2013-11-21";

// add trim to server response
// add pinMode(10,OUTPUT); and digitalWrite(10,HIGH) for SD Card Fix.
// Fixed the if else error return in state 7 2013-09-20
// Added some death trap restarts - asm volatile ("  jmp 0"); // WTF we can't go anyhow.
//#include <MemoryFree.h>
#include <IniFile.h>
#include <SD.h>
#include <Ethernet.h>
#include <SPI.h>

const int refreshPin = 39;
#define analog 0// This simply creates "0" for the Analog field on the server
#define UBX_MAX_SIZE 60 
#define SD_SELECT 22
//#define ETHERNET_SELECT 10 // for other than a 'Mega'
#define ETHERNET_SELECT 53 // for the EtherMega
File BirthCertificate;
const int led1 = 36;      // IO pin for the LSD
const int led2 = 32;      //  
const int led4 = 28;      //  
const int led8 = 24;      // IO pin for the MSD
const size_t bufferLen = 80;
char buffer[bufferLen];
int port;
String strmac;
unsigned long WaitCnt = 0;// Waiting time for dead man timer


char server[80]; // Name of the server goes here as read from pixel birth certificate.
String strHost;

boolean Fix3D;
long wnR;
uint8_t NumSats; 
long UBX_ecefVZ;
long iTOW;
long lon;
long lat;
long height;
long hMSL;
long towMsR;
long towSubMsR;

int i;
int State = 99 ;
int NextState = 99;

int numEvent;
EthernetClient client;

void setup()
{
	Serial.begin(115200);
	Serial1.begin(9600);// Start at the default baud rate 

	Serial.println  (today);
	pinMode(refreshPin, OUTPUT);
	pinMode(led8, OUTPUT); 
	pinMode(led8+2, OUTPUT);
	pinMode(led4, OUTPUT); 
	pinMode(led4+2, OUTPUT);
	pinMode(led2, OUTPUT); 
	pinMode(led2+2, OUTPUT);
	pinMode(led1, OUTPUT); 
	pinMode(led1+2, OUTPUT);
	ledState(0xFF);// LED Test...
	delay(1000);
	ledState(0x00);
	delay(1000);
	ledState(0xFF);
	delay(1000);

	Serial.println("1.Baud to Arduino 115200, to Shield 9600");
	ledState(0x1);

	// On the Ethernet Shield, CS is pin 4. It's set as an output by default. Note that even if it's not used as the CS pin, the hardware SS pin 
	// (10 on most Arduino boards, 53 on the Mega) must be left as an output or the SD library functions will not work. 	
	// pinMode(10, OUTPUT); //This for non-Mega cards
	pinMode(53, OUTPUT);// This is required for the SD card to work on the EtherMega
	pinMode(10,OUTPUT);
	digitalWrite(10,HIGH);// Shut off Ethernet chip to start.
	delay(1000);
	if (!SD.begin(4)) 
	{
		Serial.println("SD card initialization failed!   ");
		Serial.println();
		delay(5000);
	}
	Serial.println("2.SD card initialization complete");
	ledState(0x2);
	const char *filename = "/cert.ini";// Name of the birth certificate is fixed and must be at the root of the SD card
	IniFile ini(filename);
	if (!ini.open()) 
	{
		Serial.print("Ini file ");
		Serial.print(filename);
		Serial.println(" does not exist on the SD Card.  ");
		delay(5000);
	}
	Serial.println("3.Ini file exists on the SD Card...");
	ledState(0x3);
	const size_t bufferLen = 80;	// Check the file is valid. This can be used to warn if any lines are longer than the buffer.
	char buffer[bufferLen];

	if (ini.getValue("Pixel", "PostWebSite", buffer, bufferLen)) // Fetch a value from a key 
	{
		Serial.print("4.'Post to' Web Site is: ");
		ledState(0x4);
		Serial.println(buffer);
		strHost = buffer;
		strHost.toCharArray(server,bufferLen);// move the host name to the server name buffer
	}
	else 
	{
		Serial.print("Could not read 'host' from section 'network's ");
	}

	ini.getValue("Pixel", "mac", buffer, bufferLen);// Pick the mac (password) from the INI file
	strmac = buffer;
	Serial.print("5.I am: ");
	ledState(0x5);
	Serial.println(strmac);

	byte mac[6];// Six byte buffer that holds the MAC address used

	ini.getValue("Pixel", "mac5", buffer, bufferLen);// Pick up MAC values from the INI file
	mac[5] =atoi(buffer);

	ini.getValue("Pixel", "mac4", buffer, bufferLen);
	mac[4] =atoi(buffer); 

	ini.getValue("Pixel", "mac3", buffer, bufferLen);
	mac[3] =atoi(buffer); 

	ini.getValue("Pixel", "mac2", buffer, bufferLen);
	mac[2] =atoi(buffer); 

	ini.getValue("Pixel", "mac1", buffer, bufferLen);
	mac[1] =atoi(buffer); 

	ini.getValue("Pixel", "mac0", buffer, bufferLen);
	mac[0] =atoi(buffer);  
	//Serial.println(buffer);
	Serial.println("6.Starting the Ethernet connection");
	ledState(0x6);
	if (Ethernet.begin(mac) == 0) 
	{
		Serial.println("Failed to configure Ethernet using DHCP. ");// WTF might as well attempt a restart
		delay(5000);
	}
	delay(1000);// give the Ethernet shield a second to initialize:

	numEvent = 0;
	//	Serial.print(" freeMem = ");// Report initial memory usage
	//	Serial.println(freeMemory());
	WaitCnt = 0;
	NextState = 1;
	Serial.println("7.Ready to initialize the shield");
}

void wakeTheDead()
{
	//Serial.println("ref...");
	digitalWrite(refreshPin, HIGH);
	delay(100);
	digitalWrite(refreshPin, LOW);
}


void ledState(byte state)
{
	//Serial.print("state: ");
	//Serial.println(state);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led4,LOW);
	digitalWrite(led8,LOW);
	if (state & 0x1) digitalWrite(led1,HIGH);
	if (state & 0x2) digitalWrite(led2,HIGH);
	if (state & 0x4) digitalWrite(led4,HIGH);
	if (state & 0x8) digitalWrite(led8,HIGH);

}
void loop()// Main Loop of the Pixel
{

	if(NextState != State)// Report state change
	{

		State = NextState;
		Serial.print("Enter State: ");
		Serial.print(State);
		switch (State)
		{
		case 0 : Serial.println(" Debug state"); break;
		case 1 : Serial.println(" Initializing Shield");break;
		case 2 : Serial.println(" Waiting for NAV_SOL & Fix3D ");break;
		case 3 : Serial.println(" Waiting for NAV_POSLHH for position");break;
		case 4 :
			{ 
				Serial.print (" Waiting for TM2 Event ");
				Serial.println (numEvent);
				break;
			}

		case 5 : Serial.println(" Place holder state");break;
		case 6 : Serial.println(" Sending GET to server");break;
		case 7 : Serial.println(" Waiting for server response");break;
		}
		ledState(State+8);

	}

	switch (State)
	{
	case 0:

		NextState = 3;
		break;
	case 1:

		ShieldInit();
		NextState = 2;	// Go Positional Fix
		//NextState = 0;	// Go loop out for debugging
		break;

	case 2:

		if (MessageReceived() ==  0x0106) // Check to see if Nav_Sol and 3D fix receive
		{
			if (Fix3D)// Got a SOL is it 3D Fix
			{
				NextState =  3 ;// Go wait for NAVPOSLHH 
			}
		}

		break;
	case 3:

		if (MessageReceived() == 0x0102); // NAV POSLLH
		{

			NextState = 4;
		}
		break;
	case 4:// Wait for event


		if (MessageReceived()== 0x0D03)
		{

			if (Fix3D)// it has 3D fix so send it off
			{
				numEvent++;
				NextState =  5 ;// Go build Clear/Crypto and POST
			}
		}

		break;
	case 5:
		Serial.print("Waited for: ");
		Serial.print(WaitCnt);
		Serial.println(" (3E6 ~ 1 min).");
		WaitCnt = 0;
		NextState =  6 ;//  go POST it

		break;
	case 6:// Make call to server
		{
			if (client.connect(server,80))// Make a HTTP request:

			{

				String GetString;// The GetString to be sent is built from stuff from the shield

				GetString = "GET /php/setEvent.php?mac=";
				GetString += strmac;// Password is the MAC

				GetString +="&";GetString += "lat="; GetString += lat;// 
				GetString +="&";GetString += "lon=";GetString += lon;
				GetString +="&";GetString += "hMSL=";GetString += hMSL;				
				GetString +="&";GetString += "wnR=";GetString +=  wnR;
				GetString +="&";GetString += "towMsR=";GetString += towMsR;
				GetString +="&";GetString += "towSubMsR=";GetString += towSubMsR;

				GetString +="&";GetString += "analog=";GetString += analog;

				//GetString +="&";GetString += "analog=";GetString += analog;
				GetString +="&";GetString += "analog=";GetString += numEvent;//  Temp change of analog field to test dead man.

				//GetString += " HTTP/1.0";
				GetString += " HTTP/1.1";
				//Serial.println(GetString);
				//http://www.seti.net/php/setEvent.php?mac=66-66-66-66-66-66&lat=329497313&lon=-1172483644&hMSL=126972&wnR=1749&towMsR=232975825&towSubMsR=452419&analog=666

				client.println(GetString);

				String PostToWeb;
				PostToWeb = "Host: ";
				PostToWeb = PostToWeb += strHost;
				client.println(PostToWeb);
				client.println("Connection: close\r\n");

				NextState = 7;// go get the response
			}
			else 
			{
				Serial.println("Connection to dB server failed Retrying.");
				NextState = 1;

				//Serial.println(freeMemory());

				delay(500);
			}
		}
		break;
	case 7:// Recover server response

		{
			int connectLoop = 0;
			while(client.connected())
			{
				while(client.available())
				{
					connectLoop = 0;// Server did respond so clear the loop counter
					String ServerResponse;
					ServerResponse = client.readStringUntil(0x0A);// Read server response until the first Line Feed

					ServerResponse.toUpperCase();
					ServerResponse.trim();


					//Serial.print("Server Response: ");
					//Serial.println(ServerResponse);

					if(ServerResponse == "HTTP/1.1 200 OK" )				 
					{
						//Serial.println("found OK");
						NextState = 4;// Finished processing one event go wait for next  event
						wakeTheDead();// Pump up the Dead Man

						client.stop();
					}
					else
					{
						Serial.println("Error response from server");
						NextState = 1;
						delay(1000);
						client.stop();
					}

				}

				delay(1);
				connectLoop++;
				if(connectLoop > 10000)
				{
					Serial.println();
					Serial.println(F("Timeout"));
					client.stop();
					NextState = 1;// Time out so start from scratch

					//Serial.println(freeMemory());

					//Serial.println(freeMemory());

					delay(500);
					//asm volatile ("  jmp 0");  
				}
			}
		}
		break;
	}
}
