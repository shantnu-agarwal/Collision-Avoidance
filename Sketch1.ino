const int pwm[4] = { 8,9,10,11 };
const int m[4][2] = { {24,26},{34,36},{38,40},{42,44} };
#define radius 5    //cm
#define theta 0.523   //radians
#define halfwidth 40  //cm
#define length 40    //cm
#define NOS 5    //Number of sensors
#define SPEED 50 //Sets the PWM speed for motors.
int trig[] = { 39,37,35,33,31 };
int echo[] = { 7,6,5,4,3 };
int i;

//int trig[] = { 39,37,35,33,31 };
//int echo[] = { 7,6,5,4,3 };



class Data {

	int raw[NOS];         //Holds raw values from the sensor in CMs
	int horz[NOS];          //Holds calculated horizontal distances from the sensor. 
	long duration;          //This variable is used in getInput() to store pulsein
	long distance;          //This variable is used in getInput() to store converted distance
	int clearSide[NOS];       //Sets 0 for not clear, 1 for clear



public:
	void getInput() {
		for (int i = 0; i < NOS; i++) {

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
	void showRaw()			/***********ONLY TO PRINT RAW VALUES*********/
	{
		for (i = 0; i < NOS; i++)
		{
			Serial.print(i);
			Serial.print(" : ");
			Serial.println(raw[i]);
		}
	}
	void showHorz() {		/***********ONLY TO PRINT CALCULATED HORIZONTAL VALUES*********/
		for (i = 0; i < NOS; i++) {
			Serial.print(i);
			Serial.print(" : ");
			Serial.println(horz[i]);
		}
	}
	void isClear()			/*CHECKS AND DECIDES WHICH SIDE IS CLEAR*/
	{
		getInput();
		int temp = 0;
		for (i = 0; i < NOS; i++)
		{
			if (i == NOS / 2)
			{
				if (raw[i] > length)
					clearSide[i] = 1;
				else
				{
					clearSide[i] = 0; /*THIS SIDE IS NOT CLEAR, OBJECT IS THERE*/
					temp++;
				}
				i++;
			}
			if (horz[i] > halfwidth) /*DETECTED OBJECT WILL NOT COLLIDE*/
				clearSide[i] = 1;
			else
			{
				clearSide[i] = 0;
				temp++;
			}
		}

		if (temp == 0)
		{
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

		int dr = 1; //by default direction is set right
		float ls = 0, rs = 0, lh = 0, rh = 0;
		for (int i = 0; i < NOS; i++) //to calculate average sum of ls,lh,rs,rh
		{
			if (i < NOS / 2)
			{
				//lh += horz[i];
				ls += raw[i];
			}
			if (i > NOS / 2)
			{
				//rh += horz[i];
				rs += raw[i];
			}
		}
		ls /= NOS / 2;
		rs /= NOS / 2;
		if (ls > rs)		/*********COMPARING FEASIBILTY OF LEFT OVER RIGHT********/
			left(); 
		else
			right();

		/********************** DIRECTION OF MOTION HAS NOW BEEN DECIDED********************/
	}

	void forward()
	{
		Serial.println("FORWARD FORWARD FORWARD FORWARD FORWARD FORWARD");
		setPwm();
		for(i=0;i<4;i++)
		{
			digitalWrite(m[i][0], HIGH);
			digitalWrite(m[i][1], LOW);
		}
	}

	void left()
	{
		Serial.println("LEFT LEFT LEFT LEFT LEFT LEFT LEFT");
		pwmslow();
		for (i = 0; i < 4; i++)
		{
			if (i < 2) {
				digitalWrite(m[i][0], LOW);
				digitalWrite(m[i][1], HIGH);
			}
			else{
				digitalWrite(m[i][0], HIGH);
				digitalWrite(m[i][1], LOW);
			}
		}
	}

	void right()
	{
		Serial.println("RIGHT RIGHT RIGHT RIGHT RIGHT RIGHT");
		pwmslow();
		for (i = 0; i < 4; i++)
		{
			if (i < 2) {
				digitalWrite(m[i][0], HIGH);
				digitalWrite(m[i][1], LOW);
			}
			else {
				digitalWrite(m[i][0], LOW);
				digitalWrite(m[i][1], HIGH);
			}
		}
	}

	void setPwm()
	{ 
		for (int i = 0; i < 4; i++)
			analogWrite(pwm[i], SPEED);
	}


}obj;






void setup() {
	Serial.begin(9600);
	for (int i = 0; i < 4; i++)
		pinMode(pwm[i], OUTPUT);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 2; j++)
			pinMode(m[i][j], OUTPUT);
	for (i = 0; i < NOS; i++)
	{
		pinMode(trig[i], OUTPUT);
		pinMode(echo[i], INPUT);
	}
}

void loop() {
	//obj.showRaw();
	//obj.showHorz();
	obj.isClear();
	
}