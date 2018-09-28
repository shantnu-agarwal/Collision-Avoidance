
#define radius 5    //cm
#define theta 0.523   //radians
#define halfwidth 15  //cm
#define length 50     //cm
#define NOS 5     //Number of sensors
int trig[] = { 13,12,11,10,9 };
int echo[] = { 7,6,5,4,3 };
int i;


class Data
{
  int raw[NOS];         //Holds raw values from the sensor in CMs
  int horz[NOS];          //Holds calculated horizontal distances from the sensor. 
  long duration;          //This variable is used in getInput() to store pulsein
  long distance;          //This variable is used in getInput() to store converted distance
  int clearSide[NOS];       //Sets 0 for not clear, 1 for clear
public:
  void getInput();
  void showRaw();
  void showHorz();
  void isClear(); 
  void chooseDirection(); 
}obj;
void Data::getInput()
{
    for (int i = 0; i < NOS; i++)
    {
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
  void Data::showRaw()
  {
    for (i = 0; i < NOS; i++) 
    {
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(raw[i]);
    }
  }
  void Data::showHorz() 
  {
    for (i = 0; i < NOS; i++)
     {
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(horz[i]);
    }
  }
  void Data::isClear() 
  {
    int temp=0;
    for (i = 0; i < NOS; i++) 
    {
      if(i==2)
      {
       if(raw[i]>length)
        clearSide[i]=1; 
       else
        {
        clearSide[i]=0;
        temp++;
        }
       i++;
      }
      if (horz[i] > halfwidth)
        clearSide[i] = 1;
      else
        {
         clearSide[i] = 0;
         temp++;        
        }
      }
   if(temp==0)
    Serial.println("Front Side Clear!!!!")
  }
void Data::chooseDirection() 
{
 int direction=0;
 double d1=0,d2=0;
 d1=(raw[0]+raw[1])/2;
 d2=(raw[3]+raw[4])/2;
 if(d1>d2) 
  direction=1;
 else
  direction=-1;

}
void setup() 
{
  for (i = 0; i < NOS; i++)
  {
    pinMode(trig[i], OUTPUT);
    pinMode(echo[i], INPUT);
  }
  Serial.begin(9600);
}
// the loop function runs over and over again until power down or reset
void loop() 
{
  obj.getInput();
  obj.showRaw(); 
  obj.showHorz();
  //obj.isClear();
  //obj.chooseDirection();
}