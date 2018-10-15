#include <SPI.h>
#include <Wire.h>
			/************CONSTANT FOR ULTRASONICS***************/
const int pwm[4] = { 5,6,7,8 };
const int m[4][2] = { {51,53},{47,49},{22,24},{26,28} };
#define radius 5		//cm // old 5
#define theta 0.523		//radians // old 0.523
#define halfwidth 45	//cm
#define length 45		//cm
#define NOS 7			//Number of sensors
#define SPEED 100		//Sets the PWM speed for motors.
int trig[] = { 52,50,34,32,30 };  //same
int echo[] = { 13,12,11,10,9 };   //left to right
int i;

			/************CONSTANT FOR GYROSCOPE***************/
int gyro_x, gyro_y, gyro_z;
long gyro_x_cal, gyro_y_cal, gyro_z_cal;
boolean set_gyro_angles;

long acc_x, acc_y, acc_z, acc_total_vector;
float angle_roll_acc, angle_pitch_acc;

float angle_pitch, angle_roll;
int angle_pitch_buffer, angle_roll_buffer;
float angle_pitch_output, angle_roll_output;

long loop_timer;
int temp;
void gyroMaths();
void sendData(int);



class Data {

	int raw[NOS];         //Holds raw values from the sensor in CMs
	int horz[NOS];          //Holds calculated horizontal distances from the sensor. 
	long duration;          //This variable is used in getInput() to store pulsein
	long distance;          //This variable is used in getInput() to store converted distance


public:
	void getInput() {
		for (int i = 0; i < NOS; i++){
			digitalWrite(trig[i], LOW);
			delayMicroseconds(5);
			digitalWrite(trig[i], HIGH);
			delayMicroseconds(5);
			digitalWrite(trig[i], LOW);
			duration = pulseIn(echo[i], HIGH);
			distance = duration * 0.034 / 2;
			if (distance > 150)
				distance = 150;
			raw[i] = distance;

			horz[i] = abs((raw[i] + radius)*cos(((PI - (4 * theta)) / 2) + (i*theta)));
		}
	}
	void showRaw(){      /***********ONLY TO PRINT RAW VALUES*********/
	    for (i = 0; i < NOS; i++){
			Serial.print(i);
			Serial.print(" : ");
			Serial.println(raw[i]);
		}
	}
	void showHorz() {   /***********ONLY TO PRINT CALCULATED HORIZONTAL VALUES*********/
		for (i = 0; i < NOS; i++) {
			Serial.print(i);
			Serial.print(" : ");
			Serial.println(horz[i]);
		}
	}
	void isClear(){      /*CHECKS AND DECIDES WHICH SIDE IS CLEAR*/
	    getInput();
	    int temp = 0;
	    for (i = 0; i < NOS; i++){
			if (i == NOS / 2){
				if (raw[i] < length){
						/*THIS SIDE IS NOT CLEAR, OBJECT IS THERE*/
				temp++;
				}
			//i++; /*This line might be needed, need to test with and without it.*/
			}
			if (horz[i] < halfwidth) /*DETECTED OBJECT WILL COLLIDE*/  
			{
				temp++;
			}
		}

		if (temp == 0){
			Serial.println("Front Side Clear!!!!");
			forward();
		}
		else{
			Serial.println("Can't go Straight !!!!!!!");
			gyroMaths();
			if(angle_pitch_output > 45)
				chooseDirection();
		}
	}

	void chooseDirection() {
		float ls = 0, rs = 0, lh = 0, rh = 0;
		for (int i = 0; i < NOS; i++){ //to calculate average sum of ls,lh,rs,rh
			if (i < NOS / 2){
				lh += horz[i];
				ls += raw[i];
			}
			if (i > NOS / 2){
				rh += horz[i];
				rs += raw[i];
			}
		}
		ls /= NOS / 2;
		rs /= NOS / 2;
	    lh /= NOS / 2;
		rh /= NOS / 2;
		int difference = ls-rs; //+ve when left is more clear
    
		if (difference < 25 && difference > -25)    /*********COMPARING FEASIBILTY OF LEFT OVER RIGHT********/
			right();  
		else if(difference > 25 )
			left();
		else 
			right();

    /********************** DIRECTION OF MOTION HAS NOW BEEN DECIDED********************/
	}

	void forward(){
		Serial.println("FORWARD FORWARD FORWARD FORWARD FORWARD FORWARD");
		sendData(128);
	}

	void left(){
		Serial.println("LEFT LEFT LEFT LEFT LEFT LEFT LEFT");
		sendData(64);
    }

	void right(){
	    Serial.println("RIGHT RIGHT RIGHT RIGHT RIGHT RIGHT");
		sendData(16);
    }

}obj;

void sendData(int value) {					//SENDS SPI DATA
	SPI.begin();
	SPI.transfer((char)value);
	Serial.print("Value sent is ");
	Serial.println((char)value);
}

void gyroMaths() {
	
	read_mpu_data();

	gyro_x -= gyro_x_cal;
	gyro_y -= gyro_y_cal;
	gyro_z -= gyro_z_cal;


	angle_pitch += gyro_x * 0.0000611;                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
	angle_roll += gyro_y * 0.0000611;                                    //Calculate the traveled roll angle and add this to the angle_roll variable

	angle_pitch += angle_roll * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the roll angle to the pitch angel
	angle_roll -= angle_pitch * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the pitch angle to the roll angel


	acc_total_vector = sqrt((acc_x*acc_x) + (acc_y*acc_y) + (acc_z*acc_z));  //Calculate the total accelerometer vector

	angle_pitch_acc = asin((float)acc_y / acc_total_vector)* 57.296;       //Calculate the pitch angle
	angle_roll_acc = asin((float)acc_x / acc_total_vector)* -57.296;       //Calculate the roll angle

	angle_pitch_acc -= 0.0;                                              //Accelerometer calibration value for pitch
	angle_roll_acc -= 0.0;                                               //Accelerometer calibration value for roll

	if (set_gyro_angles) {													//If the IMU is already started
		angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;		//Correct the drift of the gyro pitch angle with the accelerometer pitch angle
		angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;			//Correct the drift of the gyro roll angle with the accelerometer roll angle
	}
	else {																	//At first start
		angle_pitch = angle_pitch_acc;										//Set the gyro pitch angle equal to the accelerometer pitch angle 
		angle_roll = angle_roll_acc;										//Set the gyro roll angle equal to the accelerometer roll angle 
		set_gyro_angles = true;												//Set the IMU started flag
	}

	angle_pitch_output = angle_pitch_output * 0.9 + angle_pitch * 0.1;
	angle_roll_output = angle_roll_output * 0.9 + angle_roll * 0.1;
	//Serial.print(" | Angle  = "); Serial.print(angle_pitch_output);
	//Serial.print(" | Roll   = "); Serial.print(angle_roll_output);
	while (micros() - loop_timer < 4000);									//Wait until the loop_timer reaches 4000us (250Hz) before starting the next loop
	loop_timer = micros();
}


void setup_mpu_registers() {
	
	Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
	Wire.write(0x6B);                                                    //Send the requested starting register
	Wire.write(0x00);                                                    //Set the requested starting register
	Wire.endTransmission();
	
	Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
	Wire.write(0x1C);                                                    //Send the requested starting register
	Wire.write(0x10);                                                    //Set the requested starting register
	Wire.endTransmission();
	
	Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
	Wire.write(0x1B);                                                    //Send the requested starting register
	Wire.write(0x08);                                                    //Set the requested starting register
	Wire.endTransmission();
}


void read_mpu_data() {                                             //Subroutine for reading the raw gyro and accelerometer data
	Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
	Wire.write(0x3B);                                                    //Send the requested starting register
	Wire.endTransmission();                                              //End the transmission
	Wire.requestFrom(0x68, 14);                                           //Request 14 bytes from the MPU-6050
	while (Wire.available() < 14);                                        //Wait until all the bytes are received
	acc_x = Wire.read() << 8 | Wire.read();
	acc_y = Wire.read() << 8 | Wire.read();
	acc_z = Wire.read() << 8 | Wire.read();
	temp = Wire.read() << 8 | Wire.read();
	gyro_x = Wire.read() << 8 | Wire.read();
	gyro_y = Wire.read() << 8 | Wire.read();
	gyro_z = Wire.read() << 8 | Wire.read();
}

void setup() {

	/*******************SETUP THE ULTRASONICS*************/
	for (int i = 0; i < 4; i++)
		pinMode(pwm[i], OUTPUT);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 2; j++)
			pinMode(m[i][j], OUTPUT);
	for (i = 0; i < NOS; i++) {
		pinMode(trig[i], OUTPUT);
		pinMode(echo[i], INPUT);
	}
	/************SETUP THE GYRO***************/
	Wire.begin();
	setup_mpu_registers();
	for (int cal_int = 0; cal_int < 1000; cal_int++) {                  //Read the raw acc and gyro data from the MPU-6050 for 1000 times
		read_mpu_data();
		gyro_x_cal += gyro_x;                                              //Add the gyro x offset to the gyro_x_cal variable
		gyro_y_cal += gyro_y;                                              //Add the gyro y offset to the gyro_y_cal variable
		gyro_z_cal += gyro_z;                                              //Add the gyro z offset to the gyro_z_cal variable
		delay(3);                                                          //Delay 3us to have 250Hz for-loop
	}

	gyro_x_cal /= 1000;
	gyro_y_cal /= 1000;
	gyro_z_cal /= 1000;
	Serial.begin(115200);
	loop_timer = micros();                                               //Reset the loop timer
}


void loop() {
	//obj.showRaw();
	//obj.showHorz();
	obj.isClear();
}
