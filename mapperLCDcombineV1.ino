#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8
#define readPot 15
#define button 14
//SCL --> Pin 11, CLK --> pin 13.

int16_t yReading = 0;
int16_t xReading = 0;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
uint16_t colour = 0x1F;
uint16_t shift = 00;
uint16_t red = 11;
uint16_t green = 6;
uint16_t blue = 0;
uint16_t min = 0;
uint16_t max = 0;
uint16_t pixelColour = 0;
uint8_t distance = 0;
int16_t displaceX = 0;
int16_t displaceY = 0;
int16_t xI = 0;
int16_t xF = 0;
int16_t yI = 0;
int16_t yF = 0;
int16_t* frmDim[] = { &xI, &xF, &yI, &yF };  //pointer array






#include <SoftwareSerial.h>    //header file of software serial port
SoftwareSerial Serial1(3, 2);  //define software serial port name as Serial1 and define pin2 as RX and pin3 as TX
                              // 2 is green 3 is white
int dist;                      //actual distance measurements of LiDAR
int UART[9];                   //save data measured by LiDAR
long checkSum = 0;

#include <Wire.h>
#include <Servo.h>  // Servo motor library
Servo xSERVO;       // create servo object to control a servo/Servo ySERVO;
Servo ySERVO;
#define horSERVO 7
#define vertSERVO 6
#define PACE 3

int16_t frmMin[] = { 0, 0, 0, 0 };  // ensure the F point is greater than  I

int16_t xDispSize = 180;  //# of x pixels (240 rows)
int16_t yDispSize = 180;  //# of y pixels (180 columns)
int16_t dispDim[] = { xDispSize, xDispSize, yDispSize, yDispSize };

void createLayout() {
  tft.drawRect(94, 96, 65, 31, ST7735_WHITE);
  delay(3);
  tft.fillRect(95, 97, 63, 29, colour);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(96, 98);
  tft.print("DIMENSIONS");
  tft.setCursor(96, 107);
  tft.print("x:");
  tft.setCursor(107, 107);
  tft.print(xI);
  tft.setCursor(130, 107);
  tft.print("/");
  tft.setCursor(135, 107);
  tft.print(xF);
  tft.setCursor(96, 116);
  tft.print("y:");
  tft.setCursor(107, 116);
  tft.print(yI);
  tft.setCursor(130, 116);
  tft.print("/");
  tft.setCursor(135, 116);
  tft.print(yF);
}

void potLineX() {
  //determine xI
  while (digitalRead(button) == LOW) {
    int16_t prevReading = xI;
    xI = analogRead(readPot);
    xI = map(xI, 60, 1023, 0, 158);
    if (prevReading != xI) {
      tft.setTextSize(1);
      tft.fillRect(107, 107, 21, 7, colour);  //clear number
      tft.setCursor(107, 107);                //reset cursor
      tft.print(xI);                          //print new number
      tft.drawLine(xI, 1, xI, 9, colour);     //draw initial rectangle
      delay(3);
      tft.fillRect(xI + 1, 1, 159 - xI, 9, 0x00);
      tft.fillRect(0, 1, xI - 1, 9, 0x00);
    }
  }
  while (digitalRead(button) == HIGH) {
    delay(3);
  }
  while (digitalRead(button) == LOW) {
    int16_t prevReading = xF;
    xF = analogRead(readPot);
    xF = map(xF, 60, 1023, xI, 159);
    if (prevReading != xF) {
      tft.setTextSize(1);
      tft.fillRect(135, 107, 21, 7, colour);    //clear number
      tft.setCursor(135, 107);                  //reset cursor
      tft.print(xF);                            //print new number
      tft.fillRect(xI, 1, xF - xI, 9, colour);  //draw initial rectangle
      delay(3);
      tft.fillRect(xF + 1, 1, 159 - xF, 9, 0x00);
    }
  }
}

void potLineY() {
  tft.fillRect(xI, 1, xF - xI + 1, 9, 0x00);  //erase Xpotline visual, so it can be moved around now.
  while (digitalRead(button) == LOW) {
    int16_t prevReading = yI;
    yI = analogRead(readPot);
    yI = map(yI, 60, 1023, 0, 126);

    if (prevReading != yI) {
      tft.fillRect(xI, 0, xF - xI, yI - 1, 0x00);
      tft.fillRect(xI, yI + 9, xF - xI, 135 - yI, 0x00);
      tft.fillRect(xI, yI, xF - xI, 9, colour);
      createLayout();

      tft.setTextSize(1);
      tft.fillRect(107, 116, 21, 7, colour);
      tft.setCursor(107, 116);
      tft.print(yI);
    }
  }

  while (digitalRead(button) == HIGH) {
    delay(10);
  }

  while (digitalRead(button) == LOW) {
    int16_t prevReading = yF;
    yF = analogRead(readPot);
    yF = map(yF, 60, 1023, yI, 126);
    if (prevReading != xF) {
      tft.fillScreen(0x00);
      tft.fillRect(xI, yI, xF - xI, yF - yI, colour);  //draw initial rectangle

      createLayout();
      tft.setTextSize(1);
      tft.fillRect(135, 107, 21, 7, colour);  //clear number
      tft.setCursor(135, 107);                //reset cursor
      tft.print(xF);                          //print new number
    }
  }
}

void getDistance() {
  checkSum = 0;
  Serial1.begin(115200);
  while(Serial1.available() == 0){
    delay(1);
  }

  if(Serial1.available() > 0){
  for (uint8_t i = 0; i < 8; i++) {  //save data in array
    UART[i] = Serial1.read();
    checkSum = checkSum + UART[i];
  }
  UART[8] = Serial1.read();
  if ((checkSum & 0xff) == UART[8]) {
    dist = UART[2] + UART[3] * 256;  //calculate distance value
    //Serial.println("distance = "+String(dist));
  }
  }
  Serial1.end();
  while(Serial1.available() == 1){
    delay(1);
  }
}

void mapWindow() {                               //given the starting degrees and ending degrees of x and y directions, map out every distance
  bool direct = 0;                               //determines direction of back and forth measurement
  for (uint8_t yPos = yI; yPos <= yF; yPos++) {  //move up 1 degree after every row has been read
  ySERVO.attach(vertSERVO);
    ySERVO.write(130-yPos);                         // move up 1 degree
    delay(100);
    ySERVO.detach();

    uint8_t xPos = *frmDim[!direct];                 //if not working, replace from v4     //determine if xPosition is moving left or right
    uint8_t turn = *frmDim[direct];                  // same as above               //determine turning point
    while (xPos != turn) {                       //move servo until it reaches turning point
     xSERVO.write(xPos);                         //move servo
      delay(5);
      getDistance();
      drawPixel(xF-(xPos-xI), yPos, dist);
      direct ? xPos++ : xPos--;
    }
    direct = !direct;                             //switch directions
  }
}





void getColour2() {
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.print("Sel Colour:");

  tft.setTextColor(0xFF << 11);
  tft.setCursor(5, 26);
  tft.print("RED");

  tft.setTextColor(0xFF << 6);
  tft.setCursor(42, 26);
  tft.print("GREEN");

  tft.setTextColor(0xFF << 0);
  tft.setCursor(104, 26);
  tft.print("BLUE");
  tft.setTextColor(0xFFFFFF);

  while (digitalRead(button) == LOW) {
    if (analogRead(readPot) <= 341) {
      colour = 0x1F << red;
      //Serial.println(colour);
      shift = red;
      tft.drawRect(4, 25, 36, 16, 0xFFFFFF);
      tft.drawRect(41, 25, 60, 16, 0x00);
      tft.drawRect(103, 25, 47, 16, 0x00);
      //Serial.println("red");
    } else if (analogRead(readPot) >= 682) {
      colour = (0x1f << blue);
      shift = blue;
      tft.drawRect(4, 25, 36, 16, 0x00);
      tft.drawRect(41, 25, 60, 16, 0x00);
      tft.drawRect(103, 25, 47, 16, 0xFFFFFF);
      //Serial.println("blue");
    }

    else {
      colour = (0x1F << green);
      //Serial.println(colour);
      shift = green;
      tft.drawRect(4, 25, 36, 16, 0x00);
      tft.drawRect(41, 25, 60, 16, 0xFFFFFF);
      tft.drawRect(103, 25, 47, 16, 0x00);
      //Serial.println("green");
    }
  }
  //Serial.println(colour);
}

void createScale() {
  tft.setCursor(5, 5);
  tft.print("Scale:");
  tft.setCursor(5, 26);
  tft.setTextColor(colour);
  tft.print("min:");
  tft.setCursor(5, 47);
  tft.setTextColor(0xFFFFFF);
  tft.print("max:");

  while (digitalRead(button) == HIGH)
    ;
  delay(10);

  tft.setTextColor(colour);
  while (digitalRead(button) == LOW) {  // determine minimum
    uint16_t prevMin = min;
    min = map(analogRead(readPot), 60, 1023, 0, 1023);
    if (prevMin != min) {
      tft.setCursor(49, 26);
      tft.fillRect(49, 26, 85, 14, 0x00);
      tft.print(String(min) + String("CM"));
    }
  }
  while (digitalRead(button) == HIGH)
    ;
  delay(10);

  tft.setTextColor(0xFFFFFF);
  while (digitalRead(button) == LOW) {  //determin Maximum
    uint16_t prevMax = max;
    max = map(analogRead(readPot), 60, 1023, min, 1200);
    if (prevMax != max) {
      tft.setCursor(49, 47);
      tft.fillRect(49, 47, 85, 14, 0x00);
      tft.print(String(max) + String("CM"));
    }
  }
  while (digitalRead(button) == HIGH)
    ;
  delay(10);
}

void set() {

  getColour2();
  tft.fillScreen(0x00);

  createScale();
  tft.fillScreen(0x00);
  createLayout();
  while (digitalRead(button) == LOW) {
    potLineX();
  }
  while (digitalRead(button) == HIGH)
    ;
  delay(10);

  while (digitalRead(button) == LOW) {
    potLineY();
  }
  delay(1000);


  //Centerize Frame:
  displaceX = (159 - (xF - xI)) / 2;
  displaceY = (127 - (yF - yI)) / 2;

  tft.fillScreen(ST7735_BLACK);
  tft.fillRect(displaceX, displaceY, xF - xI, yF - yI, colour);
  //centered frame process complete.
}

void drawPixel(int16_t currentX, int16_t currentY, int16_t cDistance) {
  if (cDistance < min) {
    pixelColour = colour;
  } else if (cDistance > max) {
    pixelColour = 0x00;
  } else {
    pixelColour = map(cDistance, min, max, 31, 0);
    pixelColour = pixelColour << shift;
  }
  int16_t xCoord = displaceX + currentX-xI;
  int16_t yCoord = displaceY + currentY-yI;
  tft.drawPixel(xCoord, yCoord, pixelColour);
}


void setup() {
  pinMode(horSERVO, OUTPUT);
  pinMode(vertSERVO, OUTPUT);
  xSERVO.attach(horSERVO);
  ySERVO.attach(vertSERVO);
  Wire.begin();

  pinMode(readPot, INPUT);
  pinMode(button, INPUT);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(ST7735_BLACK);
  set();
  mapWindow();

}


void loop() {
}
