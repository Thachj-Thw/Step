#include <Stepper.h> 
#include <LiquidCrystal_I2C.h>

#define MAX_SPEED           19
#define MIN_SPEED           1
#define STEP_PER_REVOLUTION 2048 //Số bước / 1 vòng quay đối với động cơ bước 28BYJ-48

#define SPEEDUP_BTN         2 // nút nhấn tăng tốc
#define SPEEDDOWN_BTN       4 // nút nhấn giảm tốc
#define FORWARD_BTN         5 // nút nhấn quay thuận nửa bước
#define REVERSE_BTN         6 // nút nhấn quay ngược nửa bước
#define STOP_BTN            3 // nút nhấn dừng
// chân điều khiển động cơ bước
#define IN1                 8
#define IN2                 9
#define IN3                 10
#define IN4                 11

#define TIME_SHOW_SET_SPEED 2000
#define HIGH_SPEED          19
#define LOW_SPEED           13

//=========================================================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
Stepper myStepper = Stepper(STEP_PER_REVOLUTION, IN1, IN3, IN2, IN4); // Cài đặt động cơ bước và cấu hình chân cắm.
bool is_high_speed = false;
int Step = 0;
enum Mode{
  Forward,
  Reverse,
  SetSpeed,
  Stop
};
Mode current_print = Stop;
Mode Status = Stop;
unsigned long t = 0;
unsigned long t_set_speed = 0;

//===============================================================================================================

void setup(){
  pinMode(SPEEDUP_BTN, INPUT);
  pinMode(SPEEDDOWN_BTN, INPUT);
  pinMode(FORWARD_BTN, INPUT);
  pinMode(REVERSE_BTN, INPUT);
  pinMode(STOP_BTN, INPUT);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  myStepper.setSpeed(LOW_SPEED);
  print_LCD("STOP");
}

void loop() {
  check_input();
  switch(Status){
    case Forward:
      print_status(Forward, "THUAN 50% " + is_high_speed ? String(HIGH_SPEED) : String(LOW_SPEED));
      for(int i = 0; i <= 100; ++i){
        myStepper.step(0.005 * STEP_PER_REVOLUTION);
        check_input();
        print_status(Forward, "THUAN 50% " + is_high_speed ? String(HIGH_SPEED) : String(LOW_SPEED));
        if(Status != Forward) break;
      }
      delay_while_check_input(1000);
      break;
    case Reverse:
      print_status(Reverse, "NGHICH 50% " + is_high_speed ? String(HIGH_SPEED) : String(LOW_SPEED));
      for(int i = 0; i <= 100; ++i){
        myStepper.step(-0.005 * STEP_PER_REVOLUTION);
        check_input();
        print_status(Reverse, "NGHICH 50% " + is_high_speed ? String(HIGH_SPEED) : String(LOW_SPEED));
        if(Status != Reverse) break;
      }
      delay_while_check_input(1000);
    case Stop:
      print_status(Stop, "STOP");
      stop_motor();
  } 
}
//===============================================================================================================

void check_input(){
  if(digitalRead(FORWARD_BTN) == HIGH){
     Status = Forward;
  }
  else if(digitalRead(REVERSE_BTN) == HIGH){
     Status = Reverse;
  }
  else if(digitalRead(STOP_BTN) == HIGH){
    Status = Stop; 
  }
  else if(digitalRead(SPEEDUP_BTN) == HIGH){
    if(!is_high_speed){
      is_high_speed = true;
      myStepper.setSpeed(HIGH_SPEED);
      print_LCD("TANG TOC");
    }else{
      print_LCD("MAX SPEED");
    }
    t_set_speed = millis();
    current_print = SetSpeed;
  }
  else if(digitalRead(SPEEDDOWN_BTN) == HIGH){
    if(is_high_speed){
      is_high_speed = false;
      myStepper.setSpeed(LOW_SPEED);
      print_LCD("GIAM TOC");
    }else{
      print_LCD("MIN SPEED");
    }
    t_set_speed = millis();
    current_print = SetSpeed;
  }
}

void delay_while_check_input(int m){
  t = millis();
  while((unsigned long)(millis() - t) < m){
    check_input();
    delay(100);
  }
}

void print_LCD(String s){
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print(s);
}

void print_status(Mode m, String p){
  if(current_print != m){
    if(current_print == SetSpeed && (unsigned long)(millis() - t_set_speed) >= TIME_SHOW_SET_SPEED){
      print_LCD(p);
      current_print = m;
    }else if(current_print != SetSpeed){
      print_LCD(p);
      current_print = m;
    }
  }
}

void stop_motor(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

//=============================================== END =========================================================
