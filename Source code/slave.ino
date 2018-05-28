//Serial Communication for Radiation Detector Arduino code included; written by Alex Boguslavsky RH Electronics; mail: support@radiohobbystore.com

#include <SoftwareSerial.h>   //Software Serial Port
#include <Servo.h>                      // Include servo library

#define RxD 7
#define TxD 6

#define DEBUG_ENABLED  1

#include <SPI.h>
#define LOG_PERIOD 15000  //Logging period in milliseconds, recommended value 15000-60000.
#define MAX_PERIOD 60000  //Maximum logging period without modifying this sketch

unsigned long counts;     //variable for GM Tube events
unsigned long cpm;        //variable for CPM
unsigned int multiplier;  //variable for calculation CPM in this sketch
unsigned long previousMillis;  //variable for time measurement

Servo servoLeft;                        // Declare left and right servos

Servo servoRight;

SoftwareSerial blueToothSerial(RxD,TxD);

void setup() 
{ 

	//play tone to signify startup procedure
	tone(4, 3000, 1000);                       // Play tone for 1 second
	delay(1000);                               // Delay to finish tone

	//setup for geiger counter
	counts = 0;
	cpm = 0;
	multiplier = MAX_PERIOD / LOG_PERIOD;      //calculating multiplier, depend on your log period
	attachInterrupt(digitalPinToInterrupt(3), tube_impulse, FALLING); //define external interrupts 

	//setup for Bluetooth
	Serial.begin(9600);
	pinMode(RxD, INPUT);
	pinMode(TxD, OUTPUT);
	setupBlueToothConnection();
	//wait 1s and flush the serial and btooth buffers
	delay(1000);
	Serial.flush();
	blueToothSerial.flush();

} 

void setupBlueToothConnection()
{
  //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.begin(38400);
  //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STWMOD=0\r\n");
  //set the bluetooth name (irrelevant)
  blueToothSerial.print("\r\n+STNA=Slave20\r\n"); 
  // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STOAUT=1\r\n");
  // Auto-connection is not allowed
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); 
  delay(2000); // This delay is required.
  //make the slave bluetooth inquirable 
  blueToothSerial.print("\r\n+INQ=1\r\n");
  delay(2000); // This delay is required.
}

void tube_impulse(){       //subprocedure for capturing events from Geiger Kit
  counts++;
}

int detectRadiation(){
  unsigned long currentMillis = millis();
  
  while (currentMillis - previousMillis <= LOG_PERIOD){
    currentMillis = millis();
  } 
   
    previousMillis = currentMillis;
    cpm = counts * multiplier;
    
   counts = 0;
   return cpm;
}

float detectTemperature(){
  
  //evaluation converts the range (0-1023) to voltage, and then to degrees Celsius, with 10mv per degree (with 500mV offset)
  float degreesC = ((analogRead(A0) * 0.004882814) - 0.5) * 100.0;
  return degreesC;
}

float detectLight(){
  
  //lightLevel is a value from [0, 1023]
  float lightLevel = analogRead(A1);
  return lightLevel;
  
}

void measure(){
	float temperature = detectTemperature();
	float lightLevel = detectLight();
	int radiation = detectRadiation();

	blueToothSerial.print("Temperature: " + String(temperature, 4) + "*");
  blueToothSerial.print("Light Level: " + String(lightLevel, 4) + "*");
  blueToothSerial.print("Radiation Level: " + String(radiation) + "*");
	
}



void autoSweep(){
  
	for (int i = 0; i < 20; i++){
    moveForward();
    delay(100);
    int rightDetect = irDetect(2, 5, 38000);
    int leftDetect = irDetect(9, 10, 38000);
    
    if (leftDetect == 0 && rightDetect == 0){
      moveBackwards();
      turnRight();
    }
    else if (leftDetect == 0){
      turnRight();
    }
    else if (rightDetect == 0){
      turnLeft();
    }
    delay(100);
    measure();
	}
}

//IR Object Detection Function
int irDetect(int irLedPin, int irReceiverPin, long frequency)
{
  tone(irLedPin, frequency, 8);              // IRLED 38 kHz for at least 1 ms
  delay(1);                                  // Wait 1 ms
  int ir = digitalRead(irReceiverPin);       // IR receiver -> ir variable
  delay(1);                                  // Down time before recheck
  return ir;                                 // Return 1 no detect, 0 detect
}  

void moveForward(){
	servoLeft.attach(13);               // Attach left signal to pin 13
    servoRight.attach(12);              // Attach right signal to pin 12

    // Full speed forward
    servoLeft.writeMicroseconds(1700);  // Left wheel counterclockwise
    servoRight.writeMicroseconds(1300); // Right wheel clockwise
    delay(2000);                        // ...for 3 seconds
    servoLeft.detach();                 // Stop sending servo signals
    servoRight.detach();
}

void moveBackwards(){
	servoLeft.attach(13);               // Attach left signal to pin 13
    servoRight.attach(12);              // Attach right signal to pin 12

    // Full speed backwards
    servoLeft.writeMicroseconds(1300);  // Left wheel clockwise
    servoRight.writeMicroseconds(1700); // Right wheel counterclockwise
    delay(2000);                        // ...for 3 seconds
    servoLeft.detach();                 // Stop sending servo signals
    servoRight.detach();
}

void turnRight(){
	servoLeft.attach(13);               // Attach left signal to pin 13
    servoRight.attach(12);              // Attach right signal to pin 12

    // Full speed right turn
    servoLeft.writeMicroseconds(1700);  // Left wheel counterclockwise
    servoRight.writeMicroseconds(1700); // Right wheel clockwise
    delay(400);                        // ...for 3 seconds
    servoLeft.detach();                 // Stop sending servo signals
    servoRight.detach();
}

void turnLeft(){
	servoLeft.attach(13);               // Attach left signal to pin 13
    servoRight.attach(12);              // Attach right signal to pin 12

    // Full speed left turn
    servoLeft.writeMicroseconds(1300);  // Left wheel counterclockwise
    servoRight.writeMicroseconds(1300); // Right wheel clockwise
    delay(400);                        // ...for 3 seconds
    servoLeft.detach();                 // Stop sending servo signals
    servoRight.detach();
}

void loop()
{ 
  
  char recvChar;
  int count = 0;
  String command = "";
  
    // If there is data ready in the btooth
  while (true){
    if(blueToothSerial.available()){
        recvChar = blueToothSerial.read();
        command = command + recvChar;
        count++;
    }
    if (count == 4){
      break;
    }
  }
  
  count = 0;
  
  if (command.equals("auto")){
    autoSweep();
  }
  else if (command.equals("frwd")){
    moveForward();
  }
  
  else if (command.equals("bkwd")){
    moveBackwards();
  }
  else if (command.equals("left")){
    turnLeft();
  }
  else if (command.equals("rght")){
    turnRight();
  }
  else if (command.equals("msre")){
    measure();
  }        
} 
