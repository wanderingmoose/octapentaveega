/*Octapentaveega_XTRA_LARGE_DIGIT_CLOCK_V2
 * This is a test of larger numbers using the OctaPentaVeega board. 5 wide x 7 down characters.
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
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>

SoftwareSerial vga(9,8);  //Octapentaveega module, Just needs D8 is needed. There is no response from this module
RTC_DS1307 rtc;

#define ONE_WIRE_BUS 2  //D2

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
// 7x5 LARGE DIGITS
//////////////////////////////////////////////////////

const byte digits[11][7][5] =
{
{{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128}}, //0
{{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32}}, //1
{{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{128,128,128,128,128},{128,32,32,32,32},{128,32,32,32,32},{128,128,128,128,128}}, //2
{{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{128,128,128,128,128}}, //3
{{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128}}, //4
{{128,128,128,128,128},{128,32,32,32,32},{128,32,32,32,32},{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{128,128,128,128,128}}, //5
{{128,32,32,32,32},{128,32,32,32,32},{128,32,32,32,32},{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128}}, //6
{{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128}}, //7
{{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128}}, //8
{{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128}}, //9
{{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32}} //blank
};

//////////////////////////////////////////////////////
// DRAW DIGIT
//////////////////////////////////////////////////////

void drawDigit(byte d,int row,int col)
{
  for(int r=0;r<7;r++)
  {
    SetCursor(row+r,col);
    vga.write(digits[d][r],5);
  }
}

//////////////////////////////////////////////////////
// ROLL ANIMATION
//////////////////////////////////////////////////////

void rollDigit(byte oldD, byte newD, int row, int col)
{
  for(int step=0; step<=7; step++)
  {
    for(int r=0;r<7;r++)
    {
      int src=r+step;

      SetCursor(row+r,col);

      if(src<7)
        vga.write(digits[oldD][src],5);
      else
        vga.write(digits[newD][src-7],5);
    }

    delay(35);
  }
}

//////////////////////////////////////////////////////
// COLON
//////////////////////////////////////////////////////

void drawColon(int row,int col,bool state)
{
  if(state)
  {
    //SetCursor(row+2,col); vga.print("█");
    //SetCursor(row+4,col); vga.print("█");
    SetCursor(row+2,col); vga.write(128);
    SetCursor(row+4,col); vga.write(128);
 
  }
  else
  {
    //SetCursor(row+2,col); vga.print(" ");
    //SetCursor(row+4,col); vga.print(" ");
    SetCursor(row+2,col); vga.write(32);
    SetCursor(row+4,col); vga.write(32);
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

  int col[4]={2,9,18,25};

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

      delay(120);
    }
  }

  drawColon(row,15,colonState);

  for(int i=0;i<4;i++)
    prevDigits[i]=d[i];
}

//////////////////////////////////////////////////////
// INFO PANEL
//////////////////////////////////////////////////////

void drawInfo(DateTime now,bool pm)
{
  char buf[20];

  SetCursor(9,2);
  vga.print(pm?"PM":"AM");

  sprintf(buf,"SEC %02d",now.second());
  SetCursor(10,2);
  vga.print(buf);

  sensors.requestTemperatures();
  float t=sensors.getTempCByIndex(0);

  SetCursor(11,2);
  vga.print("TEMP ");
  vga.print(t,1);
  vga.print(" C ");

  sprintf(buf,"%04d-%02d-%02d",now.year(),now.month(),now.day());
  SetCursor(12,2);
  vga.print(buf);
}

//////////////////////////////////////////////////////
// TICKER
//////////////////////////////////////////////////////

String ticker=" OCTAPENTAVEEGA LARGE DIGIT CLOCK ";

int tickerPos=0;
unsigned long tickerTimer=0;
int tickerDelay=200;

void drawTicker()
{
  if(millis()-tickerTimer < tickerDelay)
    return;

  tickerTimer=millis();

  SetCursor(14,1);

  for(int i=0;i<32;i++)
  {
    int index=(tickerPos+i)%ticker.length();
    vga.write(ticker[index]);
  }

  tickerPos++;
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

    int hour=now.hour();
    bool pm=false;

    if(hour>=12) pm=true;

    hour=hour%12;
    if(hour==0) hour=12;

    bool colonState=(now.second()%2==0);

    drawClock(hour,now.minute(),colonState);
    drawInfo(now,pm);

    lastSecond=now.second();
  }

  drawTicker();
}