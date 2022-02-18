#include <Wire.h>
#include <Arduino.h>
#include <MCP23018.h>

// #define NUM_MCP2301x_ICs   1

//Input Pins
const uint8_t TX = 1;
const uint8_t RX = 0;

//IO Expander Pins
const uint8_t FOut = 14;
const uint8_t Dir = 15;

const uint8_t POS_DETECT = 12;
const uint8_t SENSE_EN = 13;

const uint8_t DATA_EN1 = 4;
const uint8_t EN1 = 0;
const uint8_t FAULT1 = 8;

//Configured for pins on the Particle Boron
//const uint8_t EXT_EN = 8;
//const uint8_t SDA_CTRL = 2;

const uint8_t EXT_EN = 13;
const uint8_t SDA_CTRL = 5;

#define MARKING_PERIOD 9 //>8.33ms for standard marking period
const unsigned long TimeoutStandard = 380; //Standard timeout period for most commands is 380ms 

char ReadArray[25] = {0};

MCP23018 io(0x26); //FIX??
// MCP2301x io;

// #define IOEXP_MODE  (IOCON_INTCC | IOCON_INTPOL | IOCON_ODR | IOCON_MIRROR)
// #define ADDRESS      (MCP2301X_CTRL_ID+7)
// #define LED_PIN      D4   // WeMOS D1 & D1 mini
// #define IN_PORT      0
// #define OUT_PORT     1

// #define RELAY_ON      LOW
// #define RELAY_OFF     HIGH
// #define ALL_OFF       ALL_LOW

// #define DIR 0x00
// #define GPIO 0x12
// #define LAT 0x14
// #define PU 0x0C
// #define ADR 0x20

///////////// VOLTAGE SENSE /////////////
#include <SparkFun_PCA9536_Arduino_Library.h>
#include <MCP3421.h>

MCP3421 adc(0x6B); //Initialize MCP3421 with A3 address

PCA9536 ioSense;

//IO Sense pins
const uint8_t MUX_EN = 3;
const uint8_t MUX_SEL0 = 0;
const uint8_t MUX_SEL1 = 1;
const uint8_t MUX_SEL2 = 2;

const float VoltageDiv = 6; //Program voltage divider
const float CurrentDiv = 0.243902439; //82mOhm, 50V/V Amp

///////////// AUX VOLTAGE SENSE /////////////
#include <MCP3221.h>

MCP3221 adcAux(0x4D); //Instantiate MCP3221 with A5 address 

void setup() {

	// pinMode(EXT_EN, OUTPUT); 
	// digitalWrite(EXT_EN, HIGH);
	Serial.begin(115200);
	// delay(5000);
	// pinMode(SDA_CTRL, OUTPUT);
	// digitalWrite(SDA_CTRL, LOW);
	delay(5000);

	Wire.begin();
 //DEBUG1!
	SetDefaultPins(); //Set the default configuration for sensing 
	// SetPinMode();
	// SetPullups();
	// io.digitalWrite(6, HIGH); //Turn on 5V
	// io.digitalWrite(7, HIGH); //Turn on 12V
	// io.digitalWrite(2, LOW); //Enable Data 1
	// io.digitalWrite(3, LOW); //Enable Data 2
	// io.digitalWrite(4, LOW); //Enable Data 3
	//DEBUG1!
	// Wire.beginTransmission(0x20);
	// Wire.write(0x0A);
	// Wire.write(0x80);
	// Wire.endTransmission();

	// io.begin();	
	// Serial.println(io.init(0x20, IOEXP_MODE, I2C_FREQ100K));

	// io.pinPolarityPort(0, ALL_NORMAL);
 //  	io.pinModePort(0,ALL_OUTPUT); 
 //  	// io.pinModePort(IN_PORT,ALL_INPUT);
 //  	io.digitalWritePort(0, ALL_HIGH);
	pinMode(TX, OUTPUT); //DEBUG1!

	// io.pinPullup(0, DATA_EN1, PU_ENABLE);
	// io.pinPullup(0, DATA_EN2, PU_ENABLE);
	// io.pinPullup(0, DATA_EN3, PU_ENABLE);
	// io.pullUp(DATA_EN1, true);
	// io.pullUp(DATA_EN2, true);
	// io.pullUp(DATA_EN3, true);

	// io.pinMode(0, DATA_EN1, OUTPUT);
	// io.pinMode(0, DATA_EN2, OUTPUT);
	// io.pinMode(0, DATA_EN3, OUTPUT);

	// io.digitalWrite( DATA_EN1, HIGH);
	// io.digitalWrite( DATA_EN2, HIGH);
	// io.digitalWrite( DATA_EN3, HIGH);

	// io.pullUp(FOut, true);
	// io.pullUp(Dir, true);
	// io.pinPullup(0, FOut, true);
	// io.pinPullup(0, Dir, true);
	// io.pinMode(0, FOut, OUTPUT);
	// io.pinMode(0, Dir, OUTPUT);
	io.pinMode( Dir, OUTPUT); //Set to output 
	io.pinMode( FOut, OUTPUT); //Set to output 
	io.digitalWrite( Dir, HIGH); //Set to transmit 
//  io.digitalWrite( FOut, LOW); //DEBUG!
	Serial1.begin(1200, SERIAL_7E1);
	
	delay(1000);
	Serial.println("Begin SDI Demo...");
}

void loop() {
	// io.pinMode(0, TX, LOW);
	// io.pinMode(0, FOut, LOW);
	// delay(1000);
	// io.pinMode(0, FOut, LOW);
	// io.pinMode(0, TX, HIGH);
	// delay(1000);
	// io.pinMode(0, FOut, HIGH);
	// io.pinMode(0, TX, LOW);
	// delay(1000);
	// io.pinMode(0, FOut, HIGH);
	// io.pinMode(0, TX, HIGH);
	// delay(1000);
	static int ReadLength = 0;
  	String ReadString;
	if(Serial.available() > 0) {
    char Input = Serial.read();

    // Increment counter while waiting for carrage return or newline
    // if(Input != 13 || Input != 10) {
    if(Input != '#' && Input != '!') { //Wait for SDI12 line end or control line end 
//      Serial.println(Input); //DEBUG!
      ReadArray[ReadLength] = Input;
//      Serial.println(ReadArray[ReadLength]); //DEBUG!
      ReadLength++;
    }

    // if(Input == 13 || Input == 10) { // carriage or newline
    if(Input == '!') { //SDI12 line ending
		ReadArray[ReadLength] = Input; //Append final character
		ReadLength++;

		ReadString = String(ReadArray);
		ReadString.trim();
		memset(ReadArray, 0, sizeof(ReadArray));
		ReadLength = 0;

		// while(Serial.peek() != '\n'); //Wait for new command
		// String NewCommand = Serial.readStringUntil('\n');
		Serial.print(">");
		Serial.println(ReadString); //Echo back to serial monitor
		Serial.println(SendCommand(ReadString));
	}
	//Format - ppCs#
	//pp - two digit pin number (base 10)
	//C - command ('M' - Mode, 'R' - Read, 'W' - Write)
	//s - state (0 - Output/Off, 1 - Input/On), required only for write and mode operations 
	if(Input == '#') { //Control line ending
		ReadString = String(ReadArray);
		ReadString.trim();
		memset(ReadArray, 0, sizeof(ReadArray));
		ReadLength = 0;
		int Pin = (ReadString.substring(0,2)).toInt(); //Grab the pin to operate on
		String Operation = ReadString.substring(2,3); //Grab the middle char
		int State = (ReadString.substring(3,4)).toInt(); //Grab the state to set
		int Result = 0; //Used to grab result of either setting of pin mode or response from a read

		if(Operation.equals("M")) { //if call is for pinmode setting
			Result = io.pinMode(Pin, !State); //Use inverse of state to corespond with 1 = input, 0 = output
		}
		
		if(Operation.equals("R")) { //If call is for digital read
			Result = io.digitalRead(Pin);
		}

		if(Operation.equals("W")) { //If call is for digital write
			Result = io.digitalWrite(Pin, State);
		}

		if(!Operation.equals("S") && !Operation.equals("s")) { //If any function but voltage sense called, report normally
			Serial.print(">");
			Serial.println(ReadString); //Echo back to serial monitor
			Serial.print(">");
			Serial.println(Result); //Return various result 
		}

		if(Operation.equals("S")) { //Run voltage sense program
			VoltageSense();
		}

		if(Operation.equals("s")) { //Run Apogee port voltage sense program
			AuxVoltageSense();
		}
		
	}
	// GetAddress();
	// delay(5000);

}
}

char GetAddress()
{
	SendBreak(); //Send break to start message
	Mark(MARKING_PERIOD); //Mark for standard period before command
	Serial1.begin(1200, SERIAL_7E1);
	Serial1.print("0I!");
	Serial1.flush(); //Make sure data is transmitted before releasing the bus
	delay(1);
	ReleaseBus(); //Switch bus to recieve 
	// char Var = Serial.re

	unsigned long LocalTime = millis();
	// bool GotData = false;
	while(Serial1.available() < 3 && (millis() - LocalTime) <  TimeoutStandard);
	String Val = Serial1.readStringUntil('\r');
	Serial.println(Val);
	return 0; //DEBUG!
}

void SendBreak()
{
	// io.pinMode(0, Dir, OUTPUT); //Make sure direction is set to output
	// io.pinMode(0, FOut, OUTPUT); //Make sure the force out pin is in output mode
	io.digitalWrite( Dir, HIGH); //Set direction to output
	io.digitalWrite( FOut, LOW); //Set low to force output high
	delay(13); //Wait for bus to acnowledge action
	io.digitalWrite( FOut, HIGH); //Stop forcing output
}

void Mark(unsigned long Period)
{
	// io.pinMode(0, Dir, OUTPUT); //Make sure direction control pin is set as an output
	digitalWrite(TX, 1); //Preempt value before turning output on
	delay(1);
	pinMode(TX, OUTPUT); //Make sure transmit pin is set as output
	io.digitalWrite( Dir, HIGH); //Set direction to output
	digitalWrite(TX, 1); //Begin marking condition
	delay(Period); //Wait for a given marking period
	// digitalWrite(TX, 0); //Stop marking  
}

void Space(unsigned long Period)
{
	// io.pinMode(0, Dir, OUTPUT); //Make sure direction control pin is set as an output
	pinMode(TX, OUTPUT); //Make sure transmit pin is set as output
	io.digitalWrite( Dir, HIGH); //Set direction to output
	digitalWrite(TX, 0); //Begin spacing condition
	delay(Period); //Wait for a given marking period
	digitalWrite(TX, 1); //Stop spacing
}

// void SendCommand()

void ReleaseBus() 
{
	// io.pinMode(0, Dir, OUTPUT); //Make sure direction pin is set as an output
	io.digitalWrite( Dir, LOW); //Set direction to input
}

String SendCommand(String Command) 
{
	SendBreak(); //Send break to start message
	Mark(MARKING_PERIOD); //Mark for standard period before command
	// pinMode(TX, INPUT);
	// delay(1);
	Serial1.begin(1200, SERIAL_7E1);
	Serial1.print(Command); 
	Serial1.flush(); //Make sure data is transmitted before releasing the bus
	delay(1); //DEBUG! Return to 1ms??
	ReleaseBus(); //Switch bus to recieve 
//	io.digitalWrite(4, HIGH); //DEBUG! Turn off output buffer!
	// char Var = Serial.re

	unsigned long LocalTime = millis();
	char Data[100] = {0}; //Make data array for storage FIX! Change length to not be arbitrary
	bool GotData = false; //Used to keep track of terminating character has been recieved 
	int Pos = 0; //Keep track of position in data array
	while(!GotData && (millis() - LocalTime) <  TimeoutStandard) {
		if(Serial1.available() > 0) {
			Data[Pos] = Serial1.read(); //If byte is available, read it in
			Pos++; //Increment position
		}
		if(Data[Pos] == '\n') GotData = true; //Set flag if end character is read in
	}
	String Val = String(Data); //Convert to String
	Val.trim(); //Get rid of any trailing characters 
	// while(Serial1.available() < 3 && (millis() - LocalTime) <  TimeoutStandard);
	// String Val = Serial1.readStringUntil('\r');
	// Serial.println(Val);
//	io.digitalWrite(4, LOW); //DEBUG! Turn on output buffer!
	return Val; 
}

///////////////////DUMB PATCH INCOMING!//////////////////////////////////
// void SetPinMode() { 
// 	Wire.beginTransmission(ADR);
// 	Wire.write(DIR);
// 	Wire.write(0x00);
// 	Wire.endTransmission();
// }

// void SetPullups() {
// 	Wire.beginTransmission(ADR);
// 	Wire.write(PU);
// 	Wire.write(0xFF);
// 	Wire.endTransmission();
// }

// // void io.digitalWrite(uint8_t Pin, uint8_t State) {
// // 	uint8_t Val = ReadByte(LAT);
// // 	if(State) Val = Val | (1 << Pin);
// // 	if(!State) Val = Val & (~(1 << Pin)); 

// // 	Wire.beginTransmission(ADR);
// // 	Wire.write(LAT);
// // 	Wire.write(Val);
// // 	Wire.endTransmission();
// // }

// uint8_t ReadByte(uint8_t Reg) {
// 	Wire.beginTransmission(ADR);
// 	Wire.write(Reg);
// 	Wire.endTransmission();

// 	Wire.requestFrom(ADR, 1);
// 	return Wire.read();
// }

void SetDefaultPins()
{
	Serial.println(io.begin()); //DEBUG!
	io.pinMode(SENSE_EN, OUTPUT); //Set sense control as output
	io.digitalWrite(SENSE_EN, HIGH); //Default sense to on - NOTE: For proper sequencing, this must be done BEFORE each enable line is driven high
	for(int i = 0; i < 8; i++) { //Enable all power, then all data
		io.pinMode(i, OUTPUT); 
		io.digitalWrite(i, HIGH); 
	} 

	for(int i = 8; i < 12; i++) { //Set FAULT lines as input pullups
		io.pinMode(i, INPUT_PULLUP);
	}

	io.pinMode(POS_DETECT, INPUT); //Set position detect as normal pullup (has external pullup)
	// io.pinMode(SENSE_EN, OUTPUT); //Set sense control as output
	// io.digitalWrite(SENSE_EN, HIGH); //Default sense to on 
}

void VoltageSense() //Voltage sense (AKA Sensor0)
{
	Serial.print(">Voltage Sense:\n");
	// digitalWrite(I2C_EN, LOW); //Turn off external I2C
	io.digitalWrite(SENSE_EN, HIGH); //Make sure sense power is turned on
	ioSense.begin(); //Initalize voltage sensor IO expander
	for(int i = 0; i < 4; i++) { //Set all pins to output
		ioSense.pinMode(i, OUTPUT); 
	}
	ioSense.digitalWrite(MUX_EN, LOW); //Turn MUX on 
	int SenseError = adc.Begin(); //Initialize ADC 
	if(SenseError == 0) { //Only proceed if ADC connects correctly
		adc.SetResolution(18); //Set to max resolution (we paid for it right?) 

		ioSense.digitalWrite(MUX_SEL2, HIGH); //Read voltages
		for(int i = 0; i < 4; i++){ //Increment through 4 voltages
			ioSense.digitalWrite(MUX_SEL0, i & 0b01); //Set with lower bit
			ioSense.digitalWrite(MUX_SEL1, (i & 0b10) >> 1); //Set with high bit
			delay(1); //Wait for voltage to stabilize
	  		Serial.print("\tPort");
	  		Serial.print(i);
	  		Serial.print(":");
	  		Serial.print(adc.GetVoltage(true)*VoltageDiv, 6); //Print high resolution voltage
	  		Serial.print(" V\n");  
		}
		ioSense.digitalWrite(MUX_SEL2, LOW); //Read currents
		for(int i = 0; i < 4; i++){ //Increment through 4 voltages
			ioSense.digitalWrite(MUX_SEL0, i & 0b01); //Set with lower bit
			ioSense.digitalWrite(MUX_SEL1, (i & 0b10) >> 1); //Set with high bit
			delay(1); //Wait for voltage to stabilize
	  		Serial.print("\tPort");
	  		Serial.print(i);
	  		Serial.print(":");
	  		Serial.print(adc.GetVoltage(true)*CurrentDiv*1000, 6); //Print high resolution current measure in mA
	  		Serial.print(" mA\n");  
		}
	}
	else Serial.print("\tFAIL!\n");
	ioSense.digitalWrite(MUX_EN, HIGH); //Turn MUX off
	// digitalWrite(I2C_EN, HIGH); //Turn on external I2C
}

void AuxVoltageSense()
{
	io.digitalWrite(DATA_EN1 + 3, LOW); //Set MUX to analog output
	Serial.print(">Aux Voltage Sense:\n");
	adcAux.begin(); //Initalize the MCP3221
	Serial.print("\t");
	Serial.print(adcAux.getVoltage(3.3)*1000.0); //Print voltage in mV, calculate for 5V supply 
	Serial.print(" mV\n"); 
}
