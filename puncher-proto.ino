
#include<Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

const int MPU=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float total_accel;
double pitch,roll;
const float punchThreshold = 7; 
bool punchActive = false;
unsigned long punchStartTime = 0;
unsigned long punchEndTime = 0;
float totalAccelDuringPunch = 0;
int punchCount = 0;
int accelCount = 0;
float speedOne = 0; 
const int increaseButtonPin = 2;
const int decreaseButtonPin = 3; 
const int startButtonPin = 4; 
float weight = 60.0; 

const int maxDisplayLines = 2;
int displayLines = 0;


#define screenHeight 64
#define screenWidth 128

Adafruit_SSD1306 display(screenWidth, screenHeight, &Wire, -1);

//===============================================================================
//  Functions
//===============================================================================




void convert_accel(float Xvalue, float Yvalue, float Zvalue) {
  Xvalue /= 1350;
  Xvalue -= 1; 
 
  Zvalue /= 1850; 
  Zvalue -= 9.86;

  Yvalue /= 1350;
  Yvalue -= 1;

  AcX = Xvalue; 
  AcY = Yvalue; 
  AcZ = Zvalue; 


}

void acceleration_finder(float Xval, float Yval, float Zval) {
  int count = 0;
  if (Yval == 0) {
    Yval += 1;
    count += 1;
  }
  if (Xval == 0) {
    Xval += 1;
    count += 1;
    
  }
  if (count == 1) {
    total_accel = sqrt(pow(Xval, 2) + pow(Yval, 2) - 1);
  }
  else if (count == 2) {
    total_accel = sqrt(pow(Xval, 2) + pow(Yval, 2) - 2);
  }
  else {
    total_accel = sqrt(pow(Xval, 2) + pow(Yval, 2));
  }
}



//===============================================================================
//  Initialization
//===============================================================================




void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  pinMode(increaseButtonPin, INPUT_PULLUP);
  pinMode(decreaseButtonPin, INPUT_PULLUP);
  pinMode(startButtonPin, INPUT_PULLUP);

  

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  // Display static text

  

}





//===============================================================================
//  Main
//===============================================================================




void loop(){
  if (digitalRead(startButtonPin) == HIGH) {
    Serial.println("Ready your hand position.");
    
    display.clearDisplay(); 
    display.setCursor(0, 10); 
    display.println("Ready your hand"); 
    display.setCursor(0, 20); 
    display.println("position");
    display.display();  
    delay(1500);

    display.clearDisplay(); 
    display.setCursor(35 ,20); 
    display.setTextSize(4);
    Serial.println("GO");
    display.println("GO");
    display.display();
  
    while (true) {
      Wire.beginTransmission(MPU);
      Wire.write(0x3B);
      Wire.endTransmission(false);
      Wire.requestFrom(MPU,14,true);

      int AcXoff,AcYoff,AcZoff,GyXoff,GyYoff,GyZoff;
      int temp,toff;
      double t,tx,tf;

      //Acceleration data correction
      AcXoff = -250;
      AcYoff = 36;
      AcZoff = 1200;

      //Temperature correction
      toff = -1400;

      //Gyro correction
      GyXoff = -335;
      GyYoff = 250;
      GyZoff = 170;

      //read accel data and apply correction
      AcX=(Wire.read()<<8|Wire.read()) + AcXoff;
      AcY=(Wire.read()<<8|Wire.read()) + AcYoff;
      AcZ=(Wire.read()<<8|Wire.read()) + AcZoff;

      //read temperature data & apply correction
      temp=(Wire.read()<<8|Wire.read()) + toff;

      //read gyro data & apply correction
      GyX=(Wire.read()<<8|Wire.read()) + GyXoff;
      GyY=(Wire.read()<<8|Wire.read()) + GyYoff;
      GyZ=(Wire.read()<<8|Wire.read()) + GyZoff;

      // Calculate and convert temperature
      tx=temp;
      t = tx/340 + 36.53;     // Formula from data sheet
      tf = (t * 9/5) + 32;    // Standard C to F conversion

      //get pitch/roll
      convert_accel(AcX, AcY, AcZ);
      acceleration_finder(AcX, AcY, AcZ);

      AcZoff = 1200;

      if (total_accel > punchThreshold && !punchActive) {
          punchActive = true;
          punchStartTime = millis();
          totalAccelDuringPunch = 0; // Reset total acceleration for new punch
          punchCount++;
        }

        if (punchActive) {
          totalAccelDuringPunch += total_accel;
          accelCount++; 
        }

        
        if (total_accel < punchThreshold && punchActive) {
          punchActive = false;
          punchEndTime = millis();
          float punchDuration = (punchEndTime - punchStartTime) / 1000.0; // Duration in seconds
          float averageAccel = totalAccelDuringPunch / accelCount;
          speedOne = (0 + (8 + averageAccel * punchDuration)) / 2;

          Serial.print("Punch Speed: "); Serial.print(speedOne); Serial.println("m/s");  

          display.clearDisplay();
          display.setCursor(0, 5); 
          display.setTextSize(1);
          display.println("Punch Speed: "); 
          display.setCursor(0, 20); 
          display.print(speedOne); 
          display.print(" m/s");
          display.display();  



          Serial.print("Punching Power: "); Serial.print(averageAccel * (weight / 100) * 50); Serial.println("N");

         
          display.setCursor(0, 40); 
          display.setTextSize(1);
          display.println("Punching Power: "); 

          display.setCursor(0, 55); 
          display.print(averageAccel * (weight / 100) * 50); 
          display.print("N");
          display.display();  

          delay(3500);
          display.clearDisplay();

          display.setCursor(38, 5); 
          display.setTextSize(1);
          display.println("Press a");
          display.setCursor(42,30);
          display.print("Button");
          display.setCursor(35,55);
          display.setTextSize(1);
          display.println("to Restart");
          display.display();  

          delay(500);
          display.clearDisplay();


          break; 
        }
        //Serial.print("Accelerometer: ");
      // Serial.print("X = "); Serial.print(AcX);
        //Serial.print(" | Y = "); Serial.print(AcY);
      // Serial.print(" | Z = "); Serial.println(AcZ);
        // Serial.print(" Total Accel: "); Serial.println(total_accel);
      

      delay(10);
    }
  } else {
   
    if (digitalRead(increaseButtonPin) == HIGH) {
  
  weight += 3;
  Serial.print("Weight increased to: ");  Serial.print(weight);  Serial.println(" kg");
  

  if (displayLines >= maxDisplayLines) {
    display.clearDisplay(); 
    displayLines = 0; 
  }

  
 
  display.setCursor(0, 10 + (displayLines * 32)); 
  display.println("Weight increased to: "); 
  display.print(weight); 
  display.println(" kg");
  display.display(); 

  displayLines++; 

  delay(500); 
}

    
    else if (digitalRead(decreaseButtonPin) == HIGH) {
      weight -= 3;
      Serial.print("Weight decreased to: "); Serial.print(weight); Serial.println(" kg");
      

   if (displayLines >= maxDisplayLines) {
    display.clearDisplay(); 
    displayLines = 0; 
  }
      
      display.setCursor(0, 10 + (displayLines * 32)); 
      display.println("Weight decreased to: "); 
      display.print(weight); 
      display.println(" kg");
      display.display();

      displayLines++; 

      delay(500);
    }
}
}
//===============================================================================
//  GetAngle - Converts accleration data to pitch & roll
//===============================================================================
//void getAngle(int Vx,int Vy,int Vz) {
//double x = Vx;
//double y = Vy;
//ouble z = Vz;
//pitch = atan(x/sqrt((y*y) + (z*z)));
//roll = atan(y/sqrt((x*x) + (z*z)));
//convert radians into degrees
//pitch = pitch * (180.0/3.14);
//roll = roll * (180.0/3.14) ;
