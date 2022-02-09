
#include <SW_SPI.h>
#include <LiquidCrystal.h>

//SCLK = A3 = 9
//CS = A4 = 8
//MOSI = DIN = A5 = 7

//Declaration for Button Switch Statement
enum Button {
  Idle,
  Wait,
  Low
};


//Declaration of Global Variables
boolean incEncode = false;

int newPosition = 0;
int oldPosition = 0;




int VerorHoriz = 0; 
int DisplayRow;
int DisplayColumn;
const int interrupt0 = 2;
const int interrupt1 = 3;
volatile int encoderPosition = 0;
volatile int number = 0;
Button ButtonState = Idle;
unsigned long ButtonTime; 
unsigned long Timer;
LiquidCrystal LedDriver (11,9,5,6,7,8);

//Code for handling LED display
//MAX7219 SPI LED Driver
#define MAX7219_TEST   0x0f00 //Display in test mode
#define MAX7219_BRIGHTNESS  0x0a00 //Set brightness of the display
#define MAX7219_SCAN_LIMIT  0x0b00 //Set Scan limit
#define MAX7219_DECODE_MODE 0x0900 //Sets chip to accept bit patterns
#define MAX7219_SHUTDOWN    0x0C00 //Code for shutdown chip


int VerticalHorizontal (int input){
  if (input == 2) 
  {
    return input;
       
   } else if (input == 3){
       return input;      
      }
  }

int ButtonNextState(int input)//function that is called to loop to service the switch
{
  switch(ButtonState) {
    case Idle: //State where nothing is happening.
        if (input == LOW){
        ButtonTime = millis();
        ButtonState = Wait;
        digitalWrite(13, HIGH);
      }
      break;
    case Wait: //Button has gone low and now we are waiting for it to remain low for 5 milliseconds. 
        if (input == HIGH){
        ButtonState = Idle;
      } else if (millis() - ButtonTime >= 5) {
        ButtonState = Low;
        digitalWrite(13, LOW);
      }
      break;
    case Low: 
       if (input == HIGH){
        ButtonState = Idle;
      } else if (millis() - ButtonTime < 500) {
        return 2; //short press
      } else {
        return 3; //long press
      }
      break; 
      
  }
}

void MonitorA() {
  if (digitalRead(2) == digitalRead(3)) {
    encoderPosition ++;
  } else {
    encoderPosition --;
  }
  
}
void MonitorB() {
  if (digitalRead(2) == digitalRead(3)) {
    encoderPosition --;
  } else {
    encoderPosition ++;
  }
}


void setup() {
 

  
  SW_SPI_Initialize();
  //Set up display 
  SW_SPI_16(MAX7219_TEST + 0x01); //Turn on all the LEDS
  delay (100);   //delay 100 ms  
  SW_SPI_16(MAX7219_TEST + 0x00); // Turn the LEDS off
  SW_SPI_16(MAX7219_DECODE_MODE + 0x00); //Disable BCD mode
  SW_SPI_16(MAX7219_BRIGHTNESS + 0x03); //Use lower intensity
  SW_SPI_16(MAX7219_SCAN_LIMIT + 0x0f); //Scan all digits
  SW_SPI_16(MAX7219_SHUTDOWN + 0x01);  //Turn on chip
  

  DisplayColumn = 0; 
  DisplayRow = 0;
  
  ButtonTime = millis();
  Timer = millis();

  //Setup LCD screen
  LedDriver.begin(16,2);
  LedDriver.clear();

  //Setup encoder
  pinMode (interrupt0,INPUT);
  pinMode (interrupt1, INPUT);
  pinMode (4, INPUT);

 attachInterrupt(digitalPinToInterrupt(interrupt0), MonitorA, CHANGE);
 attachInterrupt(digitalPinToInterrupt(interrupt1), MonitorB, CHANGE);
  
}



void loop() {
  //Every 200 ms update the LED display and blink the LED at row, column
  //if 500 ms have passed
  
  
  if (millis() - Timer >= 500) {
    LedDriver.clear();
    
    int newPosition = VerticalHorizontal(ButtonNextState(digitalRead(4)));

    if (newPosition > oldPosition) {
      encoderPosition += 1; 
      oldPosition = newPosition;
      }
    if (newPosition < oldPosition) {
      encoderPosition -= 1;
      oldPosition = newPosition;
      }
      
   
    
    if (VerticalHorizontal(ButtonNextState(digitalRead(4)))==2) {
      //Display VERTICAL on the first line of LCD
        LedDriver.clear();
        LedDriver.setCursor(0,0);
        LedDriver.print("VERTICAL");
        
        //Display column value on second line of LCD
        LedDriver.setCursor(0,1);
        DisplayColumn = number;
        LedDriver.print(DisplayColumn);
       
     
    }
    if (VerticalHorizontal(ButtonNextState(digitalRead(4))) == 3) {
       //Display HORIZONTAL on the first line of LCD screen
        LedDriver.clear();
        LedDriver.setCursor(0,0);
        LedDriver.print("HORIZONTAL");
        
        //Display row value on the second line of the LCD 
        LedDriver.setCursor(0,1);
        DisplayRow = number;
        LedDriver.print(DisplayRow);
        DisplayRow = DisplayRow;
    }

   
   

    //if DisplayColumn is greater than 8
    if (DisplayColumn > 8) {
      DisplayColumn = 1; //change the value of the variable to 1

      Timer += 500;
      
   
    } 
    
  }//end of if statement

  
    //if bit 7 of DisplayRow is set
    if (DisplayRow & 0x80) {
      //Shift Display row 1 bit to the left
      DisplayRow = DisplayRow << 1;
    
    } else {
      //Shift display 1 bit to the left 
      DisplayRow = DisplayRow <<1;
      //Set bottom bit (|1)
      DisplayRow = DisplayRow |= 1;
    
      }
    //Make sure the DisplayRow only affects the lower 8 or data bits
    DisplayRow &= 0x00ff;


    for (int i = 1; i <= 8; i ++) {
     int row = 0;
      SW_SPI_16((i << 8) + row);
      
      
    }
    //Have the SPI place column number in bits 11 - 8 and display the pattern in the bottom 8 bits

    int row = 0;
    bitSet(row, DisplayRow - 1);
    
    SW_SPI_16((DisplayColumn << 8) + row);
  
    
  }//end of Timer if Statement
  
   



 



