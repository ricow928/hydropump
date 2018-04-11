// include the LCD library
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

#define LEVELRESISTOR 560
#define LEVELPIN A1

int led = 13;
int pump = 10;
int floatswitch = 0;
int counter = 0;
int interval = 120; //minutes
int max_pump_sec = 60; //seconds
int elapsed_sec = 0; //seconds
int pump_exit = 0;
int timer_exit = 0;
int float_exit = 0;
const int ph_in = A2;

int pump_it() {
  counter = 0;
  digitalWrite(pump, HIGH); //turn the pump on
  digitalWrite(led, HIGH);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("PUMPING!");
  display.display();
  while((analogRead(floatswitch)>500) && (counter < max_pump_sec)){  // While the float switch isn't floating:
    delay(1000); //keep the pump on
    counter = counter + 1; //count the number of seconds you've been pumping
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("PUMPING!");
    display.print("Elapsed: "); display.print(counter); display.println(" sec.");
    display.print("Limit: "); display.print(max_pump_sec); display.println(" sec.");
    display.display();
  }
  digitalWrite(pump,LOW);
  digitalWrite(led, LOW);
  elapsed_sec = 0;
  if (counter < max_pump_sec) { // float switch triggered exit
    float_exit += 1;
    return counter;
  }
  else { // timer triggered exit
    timer_exit += 1;
    return 0;
  }
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  sensors.begin();
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  display.display();
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello Hydro!");
  display.display();
  delay(2000);
  Serial.println("Welcome to Hydro");

  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  pinMode(pump, OUTPUT);
  pump_exit = pump_it();
}

// the loop routine runs over and over again forever:
void loop() {
  if (elapsed_sec == interval*60) {
    pump_exit = pump_it();
  }
  delay(1000UL);           // wait for a second
  elapsed_sec += 1;
  // Write some stats
  float elapsed_mins = elapsed_sec/60.0;
  float time_to_pump = interval - elapsed_mins;
  int pump_tot = timer_exit + float_exit;

  sensors.requestTemperatures();
  float tmp = sensors.getTempFByIndex(0);

  float reading = analogRead(LEVELPIN);
  // convert the value to resistance
  reading = (1023 / reading)  - 1;
  reading = LEVELRESISTOR / reading;
  // convert resistance to inches (assuming 560 ohm resistor)
  float level = -0.0082*reading+29.82;

  float pH = analogRead(ph_in)*14.0/1024.0;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Since: "); display.print(elapsed_mins); display.println(" min.");
  display.print("Until: "); display.print(time_to_pump); display.println(" min.");
  display.print("Total pumps: "); display.println(pump_tot);
  if (pump_exit == 0){
    display.print("Timer ("); display.print(timer_exit); display.println(")");
  }
  else {
    display.print("Float ("); display.print(float_exit); display.println(")");
  }
  
  display.print(counter); display.print("/"); display.print(max_pump_sec); display.println(" secs pumping.");
  display.print("pH: "); display.print(pH,1); display.print(" @ "); display.print(level, 1); display.println(" in.");
  display.print("Temp: "); display.print(tmp, 0); display.println(" F");
  display.display();
  Serial.print("Since: "); Serial.print(elapsed_mins); Serial.print(" Until: "); Serial.print(time_to_pump);
  Serial.print(" Count: "); Serial.print(pump_tot); Serial.print(" Time: "); Serial.print(counter); Serial.print("/"); Serial.println(max_pump_sec);
  Serial.print("pH: "); Serial.print(pH,1); Serial.print(" TempF: "); Serial.print(tmp,1); Serial.print(" Level: "); Serial.print(level,1); Serial.println(" in.");
  
}
