// PROJECT  : 2D Servo Sonar Mapper
// PURPOSE  : to map a window with 2 servos, a LiDAR sensor, and print on LCD
// AUTHOR   :J. Kafka
// COURSE   :ICS4U-E
// DATE     :2025 01 18
// MCU      :328P
// STATUS   :Working   
// REFERENCE: http://darcy.rsgc.on.ca/ACES/TEI4M/2021/ISPs.html


#include <Adafruit_ST7735.h>                                   //library for LCD communication
#include <Adafruit_GFX.h>                                      //graphics library for LCD
#include <SPI.h>                                               //library for Servo communication
#include <SoftwareSerial.h>                                    //header file of software serial port
#include <Wire.h>                                              //I2C library
#include <Servo.h>                                             // Servo motor library

#define horSERVO 7                                             //define horizontal servo pin
#define vertSERVO 6                                            //define vertical servo pin
#define PACE 3                                                 //define common space
#define TFT_CS 10                                              //define LCD CS pin
#define TFT_RST 9                                              //define LCD reset pin
#define TFT_DC 8                                               //define LCD DC pin
#define readPot 15                                             //define potentiometer pin
#define button 14                                              //define button pin
#define red  11                                                //fixed bitshift for red quantity
#define green 6                                                //fixed bitshift for green quantity
#define blue 0                                                 //fixed bitshift for blue quantity



SoftwareSerial Serial1(3, 2);                                  //begin software serial port for Serial1
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);// begin TFT display
Servo xSERVO;                                                  //activate xSERVO pin as a servo
Servo ySERVO;                                                  //activate xSERVO pin as a servo
int dist;                                                      //actual distance measurements of LiDAR
int UART[9];                                                   //save data measured by LiDAR
long checkSum = 0;                                             //used to ensure accuracy of Serial Communication

uint16_t colour = 0x1F;                                        //fixed "Maximum" for colour range
uint16_t shift = 0;                                            //will either hold red, green, or blue
uint16_t min = 0;                                              //declared minimum for distance sensor
uint16_t max = 0;                                              //declared maximum for distance sensor
uint16_t pixelColour = 0;                                      //declared colour for current pixel
int16_t displaceX = 0;                                         //soon to be fixed value for horizontal displaceemnt
int16_t displaceY = 0;                                         //soon to be fixed value for vertical displaceemnt
int16_t reSize = 0;
int16_t xI = 0;                                                //soon determined leftMost x position
int16_t xF = 0;                                                //soon determined rightMost x position       
int16_t yI = 0;                                                //soon determined leftMost y position
int16_t yF = 0;                                                //soon determined rightMost y position
int16_t* frmDim[] = { &xI, &xF, &yI, &yF };                    //pointer array for all positions


void setup() {
  pinMode(horSERVO, OUTPUT);                                   //declare horizontal servo pin output
  pinMode(vertSERVO, OUTPUT);                                  //declare vertical servo pin output
  pinMode(readPot, INPUT);                                     //declare potentiometer pin output
  pinMode(button, INPUT);                                      //declare button pin output
  xSERVO.attach(horSERVO);                                     //begin horizontal servo
  ySERVO.attach(vertSERVO);                                    //begin vertical servo
  Wire.begin();                                                //begin i2C communication
  tft.initR(INITR_BLACKTAB);                                   //initiate LCD
  tft.setRotation(3);                                          //set rotation due to PCB configuration
  tft.fillScreen(ST7735_BLACK);                                //fill screen black

  set();                                                       //call set function
  mapWindow();                                                 //call map window function
}

void createLayout() {   
  tft.drawRect(94, 96, 65, 31, ST7735_WHITE);                 //draw "dimensions" outline
  tft.fillRect(95, 97, 63, 29, colour);                       //draw "dimensions" background
  tft.setTextColor(ST7735_WHITE);                             //set colour of text
  tft.setTextSize(1);                                         //set text size
  tft.setCursor(96, 98);                                      //set text initial position
  tft.print("DIMENSIONS");                                    //write out:
  tft.setCursor(96, 107);                                     //move text position
  tft.print("x:");                                            //""
  tft.setCursor(107, 107);                                    //""
  tft.print(xI);                                              //""
  tft.setCursor(130, 107);                                    //""
  tft.print("/");                                             //""
  tft.setCursor(135, 107);                                    //""
  tft.print(xF);                                              //""
  tft.setCursor(96, 116);                                     //""
  tft.print("y:");                                            //""
  tft.setCursor(107, 116);                                    //""
  tft.print(yI);                                              //""
  tft.setCursor(130, 116);                                    //""
  tft.print("/");                                             //""
  tft.setCursor(135, 116);                                    //""
  tft.print(yF);
}

void potLineX() {
  while (digitalRead(button) == LOW) {                        //While the button is not being pressed...
    int16_t prevReading = xI;                                 //integer used to reduce LCD noise
    xI = analogRead(readPot);                                 //determine leftmost x value
    xI = map(xI, 60, 1023, 0, 158);                           //map it out to pixel dimensions
    if (prevReading != xI) {                                  //used to reduce noise of LCD
      tft.fillRect(107, 107, 21, 7, colour);                  //clear number
      tft.setCursor(107, 107);                                //reset cursor
      tft.print(xI);                                          //print new number
      tft.drawLine(xI, 1, xI, 9, colour);                     //draw initial rectangle
      delay(PACE);                                            // PACE
      tft.fillRect(xI + 1, 1, 159 - xI, 9, 0x00);             //fill rectangle black
      tft.fillRect(0, 1, xI - 1, 9, 0x00);                    //fill rectangle black
    }
  }
  while (digitalRead(button) == HIGH) {                       //once button is pressed...
    delay(PACE);                                              //wait for it to be let go
  }
  while (digitalRead(button) == LOW) {                        //While button is not being pressed...
    int16_t prevReading = xF;                                 //integer used to reduce LCD noise
    xF = analogRead(readPot);                                 //determine rightmost x value
    xF = map(xF, 60, 1023, xI, 159);                          //map it out to pixel dimensions
    if (prevReading != xF) {                                  //used to reduce noise of LCD
      tft.fillRect(135, 107, 21, 7, colour);                  //clear number
      tft.setCursor(135, 107);                                //reset cursor
      tft.print(xF);                                          //print new number
      tft.fillRect(xI, 1, xF - xI, 9, colour);                //draw initial rectangle
      tft.fillRect(xF + 1, 1, 159 - xF, 9, 0x00);             //fill rest black
    }
  }
}

void potLineY() {
  tft.fillRect(xI, 1, xF - xI + 1, 9, 0x00);                  //erase Xpotline visual, so it can be moved around now.
  while (digitalRead(button) == LOW) {                        //While button is not being pressed...
    int16_t prevReading = yI;                                 //integer used to reduce LCD noise
    yI = analogRead(readPot);                                 //determine leftmost y value
    yI = map(yI, 60, 1023, 0, 126);                           //map it out to pixel dimensions
    if (prevReading != yI) {                                  //used to reduce noise on LCD
      tft.fillRect(xI, 0, xF - xI, yI - 1, 0x00);             //clear previous number
      tft.fillRect(xI, yI + 9, xF - xI, 135 - yI, 0x00);      //clear mapping visual
      tft.fillRect(xI, yI, xF - xI, 9, colour);               //print new mapping visual
      createLayout();                                         //recreate "dimension legend"
      tft.fillRect(107, 116, 21, 7, colour);                  //move around "dimension legend" if necessary
      tft.setCursor(107, 116);                                //move cursor
      tft.print(yI);                                          //print yI in dimension Legend
    }
  }
  while (digitalRead(button) == HIGH) {                       //once button is pressed...
    delay(PACE);                                              //wait for button to be released
  }
  while (digitalRead(button) == LOW) {                        //once button is released...
    int16_t prevReading = yF;                                 //integer used to reduce LCD noise
    yF = analogRead(readPot);                                 //determine rightmost y value
    yF = map(yF, 60, 1023, yI, 126);                          //map it out to pixel dimensions
    if (prevReading != yF) {                                  //used to reduce noise on LCD
      tft.fillScreen(0x00);                                   //clear screen
      tft.fillRect(xI, yI, xF - xI, yF - yI, colour);         //draw initial rectangle
      createLayout();                                         //recreate "dimension legend"
      tft.fillRect(135, 107, 21, 7, colour);                  //clear number
      tft.setCursor(135, 107);                                //reset cursor
      tft.print(yF);                                          //print xF in dimension legend
    }
  }
}

void getDistance() {                                         //function to retrieve LIDAR distance
  checkSum = 0;                                              //reset accuracy integer
  Serial1.begin(115200);                                     //begin Serial communication @115200
  while (Serial1.available() == 0) {                         //wait for serial communication to begin
    delay(1);
  }
  if (Serial1.available() > 0) {                             //once serial communication is available...
    for (uint8_t i = 0; i < 8; i++) {                        //loop for each "byte" of LIDAR data
      UART[i] = Serial1.read();                              //save each byte in UART array
      checkSum = checkSum + UART[i];                         //create accuracy value
    }
    UART[8] = Serial1.read();                                //check accuracy
    if ((checkSum & 0xff) == UART[8]) {                      //if accuracy matches...
      dist = UART[2] + UART[3] * 256;                        //calibrate distance value
    }
  }
  Serial1.end();                                             //end serial communication
  while (Serial1.available() == 1) {                         //wait for serial communication to end
    delay(1);
  }
}

void mapWindow() {                                          //given the starting degrees and ending degrees of x and y directions, map out every distance
  bool direct = 0;                                          //determines direction of back and forth measurement
  for (uint8_t yPos = yI; yPos <= yF; yPos++) {             //move up 1 degree after every row has been read
    ySERVO.attach(vertSERVO);                               //begin Servo
    ySERVO.write(130 - yPos);                               //move up 1 degree
    delay(100);                                             //wait before detaching
    ySERVO.detach();                                        //detach servo

    uint8_t xPos = *frmDim[!direct];                        //determine if xPosition is moving left or right
    uint8_t turn = *frmDim[direct];                         //determine turning point
    while (xPos != turn) {                                  //move servo until it reaches turning point
      xSERVO.write(xPos);                                   //move servo
      getDistance();                                        //retrieve distance
      drawPixel(xF - (xPos - xI), yPos, dist);              //print out on screen
      direct ? xPos++ : xPos--;                             //determien horizontal direction
    }
    direct = !direct;                                       //switch directions after one sweep
  }
}
void getColour() {                                          //function to determine user colour
  tft.setTextSize(2);                                       //user aiding display setup:
  tft.setCursor(5, 5);                                      //""
  tft.print("Sel Colour:");                                 //""
  tft.setTextColor(0x1F << 11);                             //Print colour "Red" in Red
  tft.setCursor(5, 26);                                     //move cursor
  tft.print("RED");                                         //print "Red"

  tft.setTextColor(0x1F << 6);                              //repeat for Green
  tft.setCursor(42, 26);                                    //""
  tft.print("GREEN");                                       //""

  tft.setTextColor(0x1F << 0);                              //repeat for Blue
  tft.setCursor(104, 26);                                   //""
  tft.print("BLUE");                                        //""
  tft.setTextColor(0xFFFFFF);                               //set text to white

  while (digitalRead(button) == LOW) {                      //while button is not pressed...
    if (analogRead(readPot) <= 341) {                       //if reading is in bottom third of analog pot...
      colour = 0x1F << red;                                 //user colour = red
      shift = red;                                          //determine bitshift value
      tft.drawRect(4, 25, 36, 16, 0xFFFFFF);                //user aiding outline over selected colour
      tft.drawRect(41, 25, 60, 16, 0x00);                   //""
      tft.drawRect(103, 25, 47, 16, 0x00);                  //""
    } 
    else if (analogRead(readPot) >= 682) {                  //if reading is in top third of analog pot...
      colour = (0x1f << blue);                              //user colour = blue
      shift = blue;                                         //determine bitshift value
      tft.drawRect(4, 25, 36, 16, 0x00);                    //user aiding outline over selected colour
      tft.drawRect(41, 25, 60, 16, 0x00);                   //""
      tft.drawRect(103, 25, 47, 16, 0xFFFFFF);              //""
    }
    else {                                                  //otherwise...
      colour = (0x1F << green);                             //user colour = green
      shift = green;                                        //determine bitshift value
      tft.drawRect(4, 25, 36, 16, 0x00);                    //user aiding outline over selected colour
      tft.drawRect(41, 25, 60, 16, 0xFFFFFF);               //""
      tft.drawRect(103, 25, 47, 16, 0x00);                  //""
    }
  }
}

void createScale() {                                        //determine minimum and maximum LIDAR readings
  tft.setCursor(5, 5);                                      //user aiding display setup:
  tft.print("Scale:");                                      //""
  tft.setCursor(5, 26);                                     //""
  tft.setTextColor(colour);                                 //""
  tft.print("min:");                                        //""
  tft.setCursor(5, 47);                                     //""
  tft.setTextColor(0xFFFFFF);                               //""
  tft.print("max:");                                        //""
  while (digitalRead(button) == HIGH);                      //While button is being pressed...
  delay(PACE);                                              //delay

  tft.setTextColor(colour);                                 //set text colour to chosen value
  while (digitalRead(button) == LOW) {                      //determine minimum
    uint16_t prevMin = min;                                 //integer used to reduce LCD noise
    min = map(analogRead(readPot), 60, 1023, 0, 1023);  //map minimum
    if (prevMin != min) {                                   //reduce LCD noise
      tft.setCursor(49, 26);                                //user aiding display:
      tft.fillRect(49, 26, 85, 14, 0x00);                   //""
      tft.print(String(min) + String("CM"));                //""
    }
  }
  while (digitalRead(button) == HIGH);                      //while button is being pressed:
  delay(PACE);
  tft.setTextColor(0xFFFFFF);                               //set text to white
  while (digitalRead(button) == LOW) {                      //determine Maximum
    uint16_t prevMax = max;                                 //integer used to reduce LCD noise
    max = map(analogRead(readPot), 60, 1023, min, 1200);   //map maximum
    if (prevMax != max) {                                   //reduce LCD noise
      tft.setCursor(49, 47);                                //user aiding display:
      tft.fillRect(49, 47, 85, 14, 0x00);                   //""
      tft.print(String(max) + String("CM"));                //""
    }
  }
  while (digitalRead(button) == HIGH);                       //while button is being pressed...
  delay(PACE);
}

void set() {                                                 //combine all setup functions
  getColour();                                               //retrieve user colour
  tft.fillScreen(0x00);                                      //reset screen
  createScale();                                             //retrieve min and max
  tft.fillScreen(0x00);                                      //reset screen
  createLayout();                                            //print "legend"
  tft.setTextSize(1);                                        //resize text
  while (digitalRead(button) == LOW) {                       //while button is not pressed...
    potLineX();                                              //determine X values
  }
  while (digitalRead(button) == HIGH);                       //while button is being pressed...
  delay(PACE);                                               //wait
  while (digitalRead(button) == LOW) {                       //while button is not pressed...
    potLineY();                                              //determine Y values
  }
  delay(1000);                                               //wait a moment for user
  determineResize();                                         //reSize mapping if necessary
  tft.fillScreen(ST7735_BLACK);                              //reset screen
  tft.fillRect(displaceX, displaceY, reSize*(xF - xI), reSize*(yF - yI), colour);
}

void drawPixel(int16_t currentX, int16_t currentY, int16_t cDistance) {
  if (cDistance < min) {                                     //if mapping is not necessary...
    pixelColour = colour;                                    //autofill to min colour
  } else if (cDistance > max) {                              //if mapping is still not necessary...
    pixelColour = 0x00;                                      //autofill to max colour
  } else {                                                   //otherwise...
    pixelColour = map(cDistance, min, max, 31, 0);           //map pixel colour...
    pixelColour = pixelColour << shift;                      //shift to determined value
  }
  int16_t xCoord = displaceX + (currentX - xI)*reSize;       //create x coordinate for print
  int16_t yCoord = displaceY + (currentY - yI)*reSize;       //create y coordinate for print
  tft.fillRect(xCoord, yCoord, reSize,reSize, pixelColour);  //print pixel
}

void determineResize(){                                      //function to create scaling
  for(uint8_t r = 1; r <=20; r++){                           //maximum of 20x zoom
    if((yF-yI) * r <= 130 && (xF-xI) * r <= 160){            //keep expanding until no longer able to.
      reSize = r;                                            //set reSize value to R
    }
  }
  displaceX = (159-(reSize*(xF-xI)))/2;                      //determine displacement
  displaceY = (129-(reSize*(yF-yI)))/2;                      //determine displaceemnt
}

void loop() {
}
