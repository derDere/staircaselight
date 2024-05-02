#include <Arduino.h>

#define GPIO_CLK 1
#define GPIO_D1 2

#define pinA GPIO_CLK
#define pinB GPIO_D1

void setup() {
  // put your setup code here, to run once:
  pinMode(GPIO_CLK, INPUT_PULLUP);
  pinMode(GPIO_D1, INPUT_PULLUP);

  Serial.begin(9600);
}


int lastVal = 0b00;
int segment = 0b00000000;
int lastSegment = 0b00000000;
//*/

void loop() {
  // put your main code here, to run repeatedly:
  bool clk = digitalRead(GPIO_CLK);
  bool d1 = digitalRead(GPIO_D1);
  int val = 0b00;

  if (clk) val |= 0b01;
  if (d1) val |= 0b10;
  
  if (lastVal != val) {
    segment <<= 2;
    segment &= 0b11111100;
    segment |= val;
    segment &= 0b11111111;
  }

  if (segment != lastSegment) {
    //Serial.println("------------");
    //Serial.print(segment, BIN);
    //Serial.println("");
    if (segment == 0b10000111 || segment == 0b1000111) {
      segment = 0b00000000;
      Serial.println("<<<-");
    } else if (segment == 0b1001011) {
      segment = 0b00000000;
      Serial.println("->>>");
    }
  }

  /*
  if (c != counter) {
    if (c ==  1) Serial.println(">>>");
    if (c == -1) Serial.println("<<<");
    if (c ==  0) Serial.println("|||");
  }
  */

  lastVal = val;
  lastSegment = segment;
  //*/

  //Serial.print (digitalRead (pinA)) ; Serial.print (",") ; Serial.println (digitalRead (pinB)) ;

  /*
  if (lastClk != clk) {
    if (digitalRead(GPIO_D1)) {
      counter++;
    } else {
      counter--;
    }
    Serial.print("Position: ");
    Serial.println(counter);
  }
  */

  
  //Serial.println("Hello World");
  //Serial.println(clk);
  //Serial.println(d1);
}
