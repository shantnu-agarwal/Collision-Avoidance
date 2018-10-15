#include<SPI.h>
const int pwm[4] = { 5,6,7,8 };
const int m[4][2] = { {51,53},{47,49},{22,24},{26,28} };
#define radius 5    //cm // old 5
#define theta 0.523   //radians // old 0.523
#define halfwidth 45  //cm
#define length 45    //cm
#define NOS 5    //Number of sensors
#define SPEED 100 //Sets the PWM speed for motors.
int trig[] = { 52,50,34,32,30 };  //same
int echo[] = { 13,12,11,10,9 };   //left to right
int i;

//int trig[] = { 39,37,35,33,31 };
//int echo[] = { 7,6,5,4,3 };



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
		else
		{
			Serial.println("Can't go Straight !!!!!!!");
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

void sendData(int value) {
	SPI.begin();
	SPI.transfer((char)value);

}

void setup() {
	Serial.begin(9600);
	for (int i = 0; i < 4; i++)
	    pinMode(pwm[i], OUTPUT);
	for (int i = 0; i < 4; i++)
	    for (int j = 0; j < 2; j++)
			pinMode(m[i][j], OUTPUT);
	for (i = 0; i < NOS; i++){
		pinMode(trig[i], OUTPUT);
		pinMode(echo[i], INPUT);
	}
}

void loop() {
	//obj.showRaw();
	//obj.showHorz();
	obj.isClear();
}