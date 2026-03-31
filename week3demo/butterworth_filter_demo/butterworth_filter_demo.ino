// #include <math.h>

// double a[2] = {1.95558189, -0.95654717};
// double b[3] = {0.00024132, 0.00048264, 0.00024132};
// double inputs[3] = {0, 0, 0};
// double outputs[3] = {0, 0, 0};

// void setup() {
//   // put your setup code here, to run once:
//   Serial.begin(57600);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   float t = ((float)millis())/1000;
//   inputs[0] = 1 * sin(2.0*3.14*2.0*t) + 0.2 * sin(2.0*3.14*50.0*t);
//   outputs[0] = a[0]*outputs[1] + a[1]*outputs[2] + b[0]*inputs[0] + b[1]*inputs[1] + b[2]*inputs[2];
//   inputs[2] = inputs[1];
//   inputs[1] = inputs[0];
//   outputs[2] = outputs[1];
//   outputs[1] = outputs[0];
  
//   Serial.print("Input:");
//   Serial.print(inputs[0]);
//   Serial.print(",");
//   Serial.print("Time:");
//   Serial.println(outputs[0]);
//   delay(1);
// }

#include <Wire.h>              // for I2C
#include <LiquidCrystal_I2C.h> // the LCD display over I2C
LiquidCrystal_I2C lcd(0x27,16,2);

const int X_PIN = A0;
const int Y_PIN = A1;
const int Z_PIN = A2;
const int ST_PIN = 7;

double a[2] = {-0.13463524, -0.17477906};
double b[3] = {0.32735357, 0.65470715, 0.32735357};
double inputs[3] = {0, 0, 0};
double outputs[3] = {0, 0, 0};

int zero_values[3] = {333, 324, 400};
int test_values[3] = {262, 396, 538};
double scaling_factors[3] = {-325/(300*(double)(test_values[0] - zero_values[0])), 325/(300*(double)(test_values[1] - zero_values[1])), 550/(300*(double)(test_values[2] - zero_values[2]))};

void lcd_init() {
  lcd.init();      // initialize the lcd 
  lcd.backlight(); // turn on backlight
  lcd.clear();     // clear the LCD, i.e. remove all characters
  lcd.setCursor(0,0); // set cursor to 1st column, 1st row (numbering starts from 0)
  lcd.print("Step counter");
  delay(1000);
  lcd.setCursor(8,1); // set cursor to 9th column, 2nd row
  lcd.print("0");
}

void calibration() {
  digitalWrite(ST_PIN, LOW);
  delay(2000);

  zero_values[0] = analogRead(X_PIN);
  zero_values[1] = analogRead(Y_PIN);
  zero_values[2] = analogRead(Z_PIN);

  digitalWrite(ST_PIN, HIGH);
  delay(2000);

  test_values[0] = analogRead(X_PIN);
  test_values[1] = analogRead(Y_PIN);
  test_values[2] = analogRead(Z_PIN);

  scaling_factors[0] = -325/(300*(double)(test_values[0] - zero_values[0]));
  scaling_factors[1] = 325/(300*(double)(test_values[1] - zero_values[1]));
  scaling_factors[2] = 550/(300*(double)(test_values[2] - zero_values[2]));

  digitalWrite(ST_PIN, LOW);
}

double acc_square_magnitude() {
  double x_acc = scaling_factors[0] * (analogRead(X_PIN) - zero_values[0]);
  double y_acc = scaling_factors[1] * (analogRead(Y_PIN) - zero_values[1]);
  double z_acc = scaling_factors[2] * (analogRead(Z_PIN) - zero_values[2]);
  return x_acc * x_acc + y_acc * y_acc + z_acc * z_acc;
}

double butterworth(double input) {
  inputs[0] = input;
  outputs[0] = a[0]*outputs[1] + a[1]*outputs[2] + b[0]*inputs[0] + b[1]*inputs[1] + b[2]*inputs[2];
  inputs[2] = inputs[1];
  inputs[1] = inputs[0];
  outputs[2] = outputs[1];
  outputs[1] = outputs[0];
  return outputs[0];
}

void setup() {
  Serial.begin(57600);
  pinMode(ST_PIN, OUTPUT);
  lcd_init();
  // calibration();
}

unsigned long lastStepTime = millis();
int stepCounter = 0;
bool steppedUp = false;

void loop() {
  // Serial.print("X:");
  // Serial.println(scaling_factors[0] * (analogRead(X_PIN) - zero_values[0]));
  // Serial.print(",");
  // Serial.print("Y:");
  // Serial.println(scaling_factors[1] * (analogRead(Y_PIN) - zero_values[1]));
  // Serial.print(",");
  // Serial.print("Z:");
  // Serial.println(scaling_factors[2] * (analogRead(Z_PIN) - zero_values[2]));
  // Serial.print(",");
  double magnitude = acc_square_magnitude();
  Serial.print("Acc:");
  Serial.println(magnitude);
  Serial.print(",");
  Serial.print("Acc filtered:");
  double filteredMagnitude = butterworth(magnitude);
  Serial.println(filteredMagnitude);
  if (!steppedUp && filteredMagnitude > 0.04) {
    steppedUp = true;
    lastStepTime = millis();
  }
  if (steppedUp && filteredMagnitude < 0.01 && millis() - lastStepTime > 500) {
    steppedUp = false;
    stepCounter ++;
  }
  Serial.println(stepCounter);
  // delay(1);
  lcd.setCursor(8,1); // set cursor to 9th column, 2nd row
  lcd.print(String(stepCounter));
}