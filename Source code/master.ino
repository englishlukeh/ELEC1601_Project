#include <SoftwareSerial.h>                     // Software Serial Port

#define RxD 7
#define TxD 6

#define DEBUG_ENABLED  1

// Make sure you modify this address to the one in your Slave Device!!
String slaveAddr = "0,6A,8E,16,C4,24";

SoftwareSerial blueToothSerial(RxD,TxD);

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data

boolean newData = false;

void setup()
{
  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();
  //wait 1s and flush the serial and btooth buffers
  delay(1000);
  Serial.flush();
  blueToothSerial.flush();
}

void loop()
{
    recvWithEndMarker();
    showNewData();
}

void recvWithEndMarkerBlueTooth() {
	static byte ndx = 0;
	char endMarker = '*';
	char rc;
 
	while (blueToothSerial.available() > 0 && newData == false) {
		rc = blueToothSerial.read();
		if (rc != endMarker) {
			receivedChars[ndx] = rc;
			ndx++;
			if (ndx >= numChars) {
				ndx = numChars - 1;
			}
		}
		else {
			receivedChars[ndx] = '\0'; // terminate the string
			ndx = 0;
			newData = true;
		}
	}
}

void recvWithEndMarker() {
	static byte ndx = 0;
	char endMarker = '*';
	char rc;
 
	while (Serial.available() > 0 && newData == false) {
		rc = Serial.read();
		if (rc != endMarker) {
			receivedChars[ndx] = rc;
			ndx++;
			if (ndx >= numChars) {
				ndx = numChars - 1;
			}
		}
		else {
			receivedChars[ndx] = '\0'; // terminate the string
			ndx = 0;
			newData = true;
		}
	}
}

void showNewData() {
 if (newData == true) {
  
    //autosweep
  if (String(receivedChars).equals("auto")){
      Serial.println("Beginning auto-sweep");
      blueToothSerial.print('a');
      blueToothSerial.print('u');
      blueToothSerial.print('t');
      blueToothSerial.print('o');

	  newData = false;
	  
	  //loop three times for light, temperature and radiation
	  for (int i = 0; i < 20; i++){
		for (int j = 0; j < 3; j++){
		    while(!newData){
			    recvWithEndMarkerBlueTooth();
		    }
		    Serial.println(String(receivedChars));
		    newData = false;
	    } 
		Serial.println();
	  }  
  }
    //forwards
  else if (String(receivedChars).equals("frwd")){
    blueToothSerial.print('f');
    blueToothSerial.print('r');
    blueToothSerial.print('w');
    blueToothSerial.print('d');
  }
    //backwards
  else if (String(receivedChars).equals("bkwd")){
    blueToothSerial.print('b');
    blueToothSerial.print('k');
    blueToothSerial.print('w');
    blueToothSerial.print('d');
  }
    //left
  else if (String(receivedChars).equals("left")){
    blueToothSerial.print('l');
    blueToothSerial.print('e');
    blueToothSerial.print('f');
    blueToothSerial.print('t');
  }
    //right
  else if (String(receivedChars).equals("rght")){
    blueToothSerial.print('r');
    blueToothSerial.print('g');
    blueToothSerial.print('h');
    blueToothSerial.print('t');
  }
    //measure
  else if (String(receivedChars).equals("msre")){
      blueToothSerial.print('m');
      blueToothSerial.print('s');
      blueToothSerial.print('r');
      blueToothSerial.print('e');
	  
	  newData = false;
	  
	  //loop three times for light, temperature and radiation
	  for (int i = 0; i < 3; i++){
      while (!newData){
			  recvWithEndMarkerBlueTooth();
      }
		  Serial.println(String(receivedChars));
		  newData = false;
	  }
   Serial.println();
	  

	}
	//unkown command
	else{
	Serial.println("Unknown command entered.");
	}
	newData = false;
 }
}

// Function to start the connection. Make sure slaveAddr is set to the right
// value.
void setupBlueToothConnection()
{
  // Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.begin(38400);
  // set the bluetooth work in master mode
  blueToothSerial.print("\r\n+STWMOD=1\r\n");
  // set the bluetooth name (irrelevant)
  blueToothSerial.print("\r\n+STNA=Master20\r\n");
  // Auto-connection is forbidden here
  blueToothSerial.print("\r\n+STAUTO=0\r\n");
  // This delay is required.
  delay(2000);
  blueToothSerial.flush();
  // This delay is required.
  delay(2000);

  //form the full connection command
  Serial.print("Connecting to slave: ");
  Serial.println(slaveAddr);
  //send connection command
  blueToothSerial.print("\r\n+CONN=" + slaveAddr + "\r\n");
  delay(5000);
}

