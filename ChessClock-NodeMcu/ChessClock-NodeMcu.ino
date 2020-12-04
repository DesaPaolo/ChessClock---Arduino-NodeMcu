#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//I2C pin define
#define I2C_ADDR 0x27
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

// macros from DateTime.h for getting elapsed time

#define SECS_PER_MIN  (60UL)
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)

// LCD init
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

unsigned long start_turn1_timer;
unsigned long start_turn2_timer;
unsigned int timer1;
unsigned int timer2;
unsigned int turn;
unsigned int pot;
unsigned int time_mode[12] = {1, 2, 3, 3, 5, 5, 10, 10, 15, 30, 30, 59}; //min
unsigned int time_increment[12] = {0, 1, 0, 2, 0, 3, 0, 5, 10, 0, 20, 0}; //sec
unsigned int alarm_time[12] = {10, 10, 20, 20, 30, 30, 60, 60, 60, 120, 90, 180}; //sec
unsigned int alarm_mode = 1;
unsigned int time_mode_index;
unsigned int old_time_mode_index;

uint8_t LED1_PIN = D5;
uint8_t LED2_PIN = D7;
uint8_t BUTT1_PIN = D6;
uint8_t BUTT2_PIN = D8;
uint8_t BUZZ_PIN = D3;
uint8_t POTENTIOMETER = A0;

//GAME OVER func
void game_over(int led_pin) {
  for (int i = 0; i < 10; i++) {
    digitalWrite(led_pin, LOW);
    digitalWrite(BUZZ_PIN, HIGH);
    delay(250);
    digitalWrite(led_pin, HIGH);
    digitalWrite(BUZZ_PIN, LOW);
    delay(250);
  }
}


//Reset
void reset_mode() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    delay(250);
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    delay(250);
  }
}

void alarm() {

  if(timer1 == alarm_time[time_mode_index]) {
    while((millis() - start_turn1_timer) <= 250) {
      digitalWrite(BUZZ_PIN, HIGH);
    }
    digitalWrite(BUZZ_PIN, LOW);
  } else {
    while((millis() - start_turn2_timer) <= 250) {
      digitalWrite(BUZZ_PIN, HIGH);
    }
    digitalWrite(BUZZ_PIN, LOW);
  }
}

//Time handling functions
String printDigits(byte digits) {
  // Utility function for digital clock display: prints colon and leading 0
  String string = String(":");
  if (digits < 10)
    string += String("0");
  string += String(digits, DEC);
  return string;
}

String millis_to_date(unsigned int timer) {

  String date;

  int minutes = numberOfMinutes(timer);
  int seconds = numberOfSeconds(timer);

  String minutes_str;
  if (minutes < 10)
    minutes_str = String("0") + String(minutes, DEC);
  else
    minutes_str = String(minutes, DEC);

  String seconds_str = String(":");
  if (seconds < 10)
    seconds_str = seconds_str + String("0") + String(seconds, DEC);
  else
    seconds_str = seconds_str + String(seconds, DEC);

  date = String(minutes_str + seconds_str);

  return date;
}


void setup() {

  //Butt/Led setup
  pinMode(LED1_PIN, OUTPUT);
  pinMode(BUTT1_PIN, INPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTT2_PIN, INPUT);
  pinMode (BUZZ_PIN, OUTPUT);

  //LCD setup
  lcd.begin(16, 2);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();

  //ChessClock variables init
  timer1 = 60 * 10; // 10 min default
  timer2 = 60 * 10;
  turn = 0;
  time_mode_index = 7;
  old_time_mode_index = 7;

  Serial.begin(9600);
}


void loop() {
  Serial.println(turn);

  // Switch turn triggering

  if (timer1 == 0 || timer2 == 0) {
    /*Time: 00:00 -> GAME OVER*/
    if (timer1 == 0) {
      game_over(LED1_PIN);
    } else {
      game_over(LED2_PIN);
    }
    turn = 0;
    lcd.clear();
  } else if (digitalRead(BUTT1_PIN) == HIGH && digitalRead(BUTT2_PIN) == HIGH && turn != 0) {
    /*RESET (holding both buttons)*/
    delay(1000); //checking the holding of both buttons
    if (digitalRead(BUTT1_PIN) == HIGH && digitalRead(BUTT2_PIN) == HIGH) {
      reset_mode();
      turn = 0;
      lcd.clear();
    }
  } else if (digitalRead(BUTT1_PIN) == HIGH && turn != 2 ) {
    if (turn == 0) {
      delay(750);
      turn = 3;
      Serial.println("!!!!!!!!");
    } else {
      /*Timer 1: "I move, it's your turn"*/
      digitalWrite(LED1_PIN, LOW);
      digitalWrite(LED2_PIN, HIGH);

      start_turn2_timer = millis();
      if (turn != 3)
        timer1 += time_increment[time_mode_index];
      turn = 2;
    }

    lcd.clear();

  } else if (digitalRead(BUTT2_PIN) == HIGH && turn != 1) {
    if (turn == 0) {
      turn = 3;
      lcd.clear();
      delay(750);
      Serial.println("!!!!!!!!");
    } else {
      /*Timer 2: "I move, it's your turn"*/
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, LOW);

      start_turn1_timer = millis();
      if (turn != 3)
        timer2 += time_increment[time_mode_index];
      turn = 1;
    }

    lcd.clear();
  }

  switch (turn) {

    case 0:
      /*Setup Mode*/
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);
      pot = analogRead(POTENTIOMETER);
      //Serial.println(pot);

      time_mode_index = map(pot, 90, 1024, 0, 11); // A0 - ADC pin of NODEMCU gives this value mapping
      //Serial.println(time_mode_index);

      if (time_mode_index != old_time_mode_index) {
        lcd.clear();
        old_time_mode_index = time_mode_index;
      }

      timer1 = time_mode[time_mode_index] * 60;
      timer2 = time_mode[time_mode_index] * 60;
      break;
    case 1 :
      /*Turn 1*/
      if ((millis() - start_turn1_timer) >= 1000) {
        start_turn1_timer = millis();
        timer1 -= 1;
      }
      break;
    case 2:
      /*Turn 2*/
      if ((millis() - start_turn2_timer) >= 1000) {
        start_turn2_timer = millis();
        timer2 -= 1;
      }
      break;
    case 3:
      /*Setting alarm mode*/
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);
      pot = analogRead(POTENTIOMETER);

      alarm_mode = map(pot, 90, 1024, 0, 1);
      break;
  }

  //Alarm handling
  if ((turn == 1 or turn == 2) && alarm_mode && (timer1 == alarm_time[time_mode_index] or timer2 == alarm_time[time_mode_index])) {
    alarm();
  }

  //LCD print

  if (turn == 0) {
    lcd.setCursor(5, 0);
    lcd.print(String(time_mode[time_mode_index]) + String("+") + String(time_increment[time_mode_index]));
  } else if (turn == 3) {
    lcd.setCursor(0, 0);
    String alarm_value_str;

    if (alarm_mode)
      alarm_value_str = String("Yes");
    else
      alarm_value_str = String("No ");

    lcd.print("Alarm mode: " + alarm_value_str);
  } else {
    lcd.setCursor(1, 0);
    lcd.print("ChessClockNudo");
  }

  //Timer 1 ---> left
  lcd.setCursor(0, 1);
  String date1 = millis_to_date(timer1);
  lcd.print(date1);

  //Timer 2 ---> right
  lcd.setCursor(11, 1);
  String date2 = millis_to_date(timer2);
  lcd.print(date2);

}
