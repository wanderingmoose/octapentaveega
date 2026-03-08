/*Octapentaveega_LARGE_DIGIT_CLOCK_V5
 * This is a test of larger numbers using the OctaPentaVeega board. 3 wide x 5 down characters.
 * Simple but shows some of the good things one can do with the simple 32x16 character screen.
 * Wanderingmoose Tinkering March 2026.
 *Nano with Octapentaveega module.
 Simple clock format with simple flip up of numbers.
 Date and temperature.
 DS1307 RTC I2C A4 and A5
 DS18B20 temperature on the D2 pin.
 Addin border around the time.
 Added mechincal roll and update order of digit order of roll.
 Works 
//********************************************************
//********************************************************
Copy this into a new sketch and run to set DS1307 clock to compiled time.
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
//Set time to when code is complied. So will be a second or two behind actual time.
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;
void setup () {
  Serial.begin(57600); // Start serial communication
  Wire.begin(); // Initialize I2C communication
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); // Halt if RTC is not found
  }
  // Uncomment the following line to set the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Use F() macro for flash memory optimization if needed
  Serial.println("Time set to the compiled time. Now upload the reading sketch.");
}
void loop () {
  // Nothing needed in the loop for setting the time
}
//********************************************************
//********************************************************
*/


#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>

SoftwareSerial vga(9,8);
RTC_DS1307 rtc;

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//////////////////////////////////////////////////////
// TERMINAL CONTROL
//////////////////////////////////////////////////////

const byte ESC[2] = {27,'['};
const byte CLR[4] = {27,'[','2','J'};

void SetCursor(int r,int c)
{
  vga.write(ESC,2);
  vga.print(r);
  vga.write(';');
  vga.print(c);
  vga.write('H');
}

void NoWrap()
{
  vga.write(ESC,2);
  vga.print("?7l");
}

//////////////////////////////////////////////////////
// LARGE DIGITS
//////////////////////////////////////////////////////

const byte digits[11][5][3] =
{
//Squares
{{128,128,128},{128,32,128},{128,32,128},{128,32,128},{128,128,128}}, //0
{{32,32,128},{32,32,128},{32,32,128},{32,32,128},{32,32,128}},        //1
{{128,128,128},{32,32,128},{128,128,128},{128,32,32},{128,128,128}},  //2
{{128,128,128},{32,32,128},{32,128,128},{32,32,128},{128,128,128}},   //3
{{128,32,128},{128,32,128},{128,128,128},{32,32,128},{32,32,128}},    //4
{{128,128,128},{128,32,32},{128,128,128},{32,32,128},{128,128,128}},  //5
{{128,32,32},{128,32,32},{128,128,128},{128,32,128},{128,128,128}},   //6
{{128,128,128},{32,32,128},{32,32,128},{32,32,128},{32,32,128}},      //7
{{128,128,128},{128,32,128},{128,128,128},{128,32,128},{128,128,128}},//8
{{128,128,128},{128,32,128},{128,128,128},{32,32,128},{32,32,128}},   //9
{{32,32,32},{32,32,32},{32,32,32},{32,32,32},{32,32,32}}              //blank
};

//////////////////////////////////////////////////////
// COLON
//////////////////////////////////////////////////////

const byte colon[5][3] =
{
{32,32,32},
{32,128,32},
{32,32,32},
{32,128,32},
{32,32,32}
};

//////////////////////////////////////////////////////
// BOX GRAPHICS
//////////////////////////////////////////////////////

const byte UPBOX[32] = {137,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,151};
const byte MIDBOX[32] = {138,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,138};
const byte LOWBOX[32] = {136,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,133,135};
const byte CROSSBOX[32] = {144, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 140};

//////////////////////////////////////////////////////
// DRAW DIGIT
//////////////////////////////////////////////////////

void drawDigit(byte d,int row,int col)
{
  for(int r=0;r<5;r++)
  {
    SetCursor(row+r,col);
    vga.write(digits[d][r],3);
  }
}

//////////////////////////////////////////////////////
// ROLL ANIMATION
//////////////////////////////////////////////////////

void rollDigit(byte oldD, byte newD, int row, int col)
{
  for(int step=0; step<=5; step++)
  {
    for(int r=0;r<5;r++)
    {
      int src=r+step;

      SetCursor(row+r,col);

      if(src<5)
        vga.write(digits[oldD][src],3);
      else
        vga.write(digits[newD][src-5],3);
    }

    delay(45);
  }
}

//////////////////////////////////////////////////////
// COLON
//////////////////////////////////////////////////////

void drawColon(int row,int col,bool state)
{
  for(int r=0;r<5;r++)
  {
    SetCursor(row+r,col);

    if(state)
      vga.write(colon[r],3);
    else
      vga.write("   ");
  }
}

//////////////////////////////////////////////////////
// CLOCK
//////////////////////////////////////////////////////

int prevDigits[4] = {-1,-1,-1,-1};

void drawClock(int h,int m,bool colonState)
{

  int row=1;

  int d[4];
  d[0]=h/10;
  d[1]=h%10;
  d[2]=m/10;
  d[3]=m%10;

  if(d[0]==0)
    d[0]=10;

//  int col[4]={3,7,15,19};
    int col[4]={2,6,14,18};
  // update digits right → left (mechanical cascade)
  for(int i=3;i>=0;i--)
  {
    if(prevDigits[i]==-1)
      drawDigit(d[i],row,col[i]);

    else if(prevDigits[i]!=d[i])
    {
      if(prevDigits[i]==10 || d[i]==10)
        drawDigit(d[i],row,col[i]);
      else
        rollDigit(prevDigits[i],d[i],row,col[i]);

      // mechanical cascade delay
      delay(120);
    }
  }

  drawColon(row,11,colonState);

  for(int i=0;i<4;i++)
    prevDigits[i]=d[i];
}

//////////////////////////////////////////////////////
// DATE
//////////////////////////////////////////////////////

void drawDate(DateTime now)
{

  const char* days[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  const char* months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

  char buf[20];

  sprintf(buf,"%s %s %d %d",
  days[now.dayOfTheWeek()],
  months[now.month()-1],
  now.day(),
  now.year());

  int col=(32-strlen(buf))/2 +1;

  SetCursor(7,col);
  vga.print(buf);
}

//////////////////////////////////////////////////////
// SECONDS
//////////////////////////////////////////////////////

void drawSeconds(int s)
{
  char buf[3];
  sprintf(buf,"%02d",s);

  SetCursor(3,25);
  vga.print(buf);
}

//////////////////////////////////////////////////////
// AM/PM
//////////////////////////////////////////////////////

void drawAMPM(bool pm)
{
  SetCursor(1,25);

  if(pm)
    vga.print("PM");
  else
    vga.print("AM");
}

//////////////////////////////////////////////////////
// TEMPERATURE
//////////////////////////////////////////////////////

void drawTemp()
{
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);

  SetCursor(5,25);
  vga.print(t,1);
  vga.print("C ");
}

//////////////////////////////////////////////////////
// TICKER
//////////////////////////////////////////////////////

String ticker=" OCTAPENTAVEEGA Large Digit Clock with Temperature ";

int tickerPos=0;
unsigned long tickerTimer=0;
int tickerDelay=250; //Scroll Speed

void drawTicker()
{
  if(millis()-tickerTimer < tickerDelay)
    return;
  tickerTimer=millis();
  SetCursor(14,1);
  for(int i=0;i<30;i++)
  {
    int index=(tickerPos+i)%ticker.length();
    vga.write(ticker[index]);
  }
  tickerPos++;
 }

//////////////////////////////////////////////////////
// COLOR
//////////////////////////////////////////////////////

void SetColor (int fore, int back)
{
  vga.write (ESC, 2);
  vga.print (fore);
  vga.write (";");
  vga.print (back);
  vga.write ("m");
}

//////////////////////////////////////////////////////
// INIT SCREEN
//////////////////////////////////////////////////////

void InitScreen()
{
  vga.write(CLR,4);

  SetCursor(0,0);
  SetColor(37,40);
  vga.write(UPBOX,32);

  for(int f=1;f<15;f++)
  {
    SetCursor(f,0);
    vga.write(MIDBOX,32);
  }

  SetCursor(15,0);
  vga.write(LOWBOX,32);
SetCursor(6,0);
vga.write(CROSSBOX,32);
SetCursor(8,0);
vga.write(CROSSBOX,32);
SetCursor(13,0);
vga.write(CROSSBOX,32);
}

//////////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////////

void setup()
{
  vga.begin(9600);
  Wire.begin();
  rtc.begin();
  sensors.begin();

  delay(500);

  NoWrap();

  if(!rtc.isrunning())
    rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
// Uncomment the following line to set the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Use F() macro for flash memory optimization if needed
  InitScreen();
}

//////////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////////

int lastSecond=-1;

void loop()
{

  DateTime now = rtc.now();

  if(now.second()!=lastSecond)
  {

    int hour = now.hour();
    bool pm=false;

    if(hour>=12) pm=true;

    hour = hour % 12;
    if(hour==0) hour=12;

    bool colonState = (now.second()%2==0);

    drawClock(hour,now.minute(),colonState);
    drawAMPM(pm);
    drawSeconds(now.second());
    drawTemp();
    drawDate(now);

    lastSecond=now.second();
  }

  drawTicker();
}