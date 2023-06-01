#include <Stepper.h> 
#include <LiquidCrystal_I2C.h>

#define NUMBER_OF_STEP      2048 //Số bước / 1 vòng quay đối với động cơ bước 28BYJ-48
#define STEP_A_LOOP         8    // Số bước thực hiện trong 1 vòng lặp

#define SPEED_UP_BTN        4 // Nút nhấn tăng tốc
#define SPEED_DOWN_BTN      3 // Nút nhấn giảm tốc
#define FORWARD_BTN         5 // Nút nhấn quay thuận nửa bước
#define REVERSE_BTN         6 // Nút nhấn quay ngược nửa bước
#define STOP_BTN            2 // Nút nhấn dừng
// chân điều khiển động cơ bước
#define MOTOR_PIN_1         8
#define MOTOR_PIN_2         10
#define MOTOR_PIN_3         9
#define MOTOR_PIN_4         11

#define TIME_SHOW_SET_SPEED 2000 // Thời gian hiển thị thông báo điều chỉnh tốc độ (ms)
#define HIGH_SPEED          15   // Tốc độ mức cao
#define LOW_SPEED           7    // Tốc độ mức thấp

//=========================================================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
Stepper myStepper = Stepper(NUMBER_OF_STEP, MOTOR_PIN_1, MOTOR_PIN_2, MOTOR_PIN_3, MOTOR_PIN_4); // Cài đặt động cơ bước và cấu hình chân cắm.
enum Status{
  FORWARD,
  REVERSE,
  SET_SPEED,
  STOP
};
Status state = STOP;
Status current = state;
bool is_high_speed = false;
bool is_forward = true;
bool is_press = false;
bool is_set_speed = false;
int total_step = 0;
unsigned long t = 0;
unsigned long t_set_speed = 0;

//===============================================================================================================

void setup(){
  pinMode(SPEED_UP_BTN, INPUT);
  pinMode(SPEED_DOWN_BTN, INPUT);
  pinMode(FORWARD_BTN, INPUT);
  pinMode(REVERSE_BTN, INPUT);
  pinMode(STOP_BTN, INPUT);
  
  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT);
  pinMode(MOTOR_PIN_3, OUTPUT);
  pinMode(MOTOR_PIN_4, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  myStepper.setSpeed(LOW_SPEED);
  print_LCD("STOP", "");
}

void loop(){
  check_input();
  switch(state){
    case FORWARD:
    case REVERSE:
      myStepper.step((is_forward ? -1 : 1) * STEP_A_LOOP);
      total_step += STEP_A_LOOP;
      if(total_step >= 0.5 * NUMBER_OF_STEP){
        total_step = 0;
        delay_while_check_input(1000);
      }
      if(is_set_speed && (unsigned long)(millis() - t_set_speed) >= TIME_SHOW_SET_SPEED){
        print_LCD(is_forward ? "THUAN 50%" : "NGHICH 50%", string_speed());
        is_set_speed = false;
      }
      break;
    case STOP:
      if(is_set_speed){
        myStepper.step((is_forward ? -1 : 1) * STEP_A_LOOP);
      }else{
      stop_motor();
      }
  } 
}
/*
Giải thích
kiểm tra trạng thái nút nhấn, tương ứng với mỗi loại nút nhấn sẽ set state = FORWARD (thuận), REVERSE (nghịch) or STOP (dừng)
và in ra LCD thông tin tương ứng

kiểm tra giá trị state
nếu là FORWARD hoặc REVERSE quay động cơ bước theo chiều tương ứng
"is_forward ? -1 : 1" gọi là toán tử 3 ngôi. có thể hiểu: nếu is_forward là true thì trả về -1 ngược lại trả về 1
cộng thêm số bước vừa thực hiện vào tổng số bước (mỗi vòng lặp thực hiện STEP_A_LOOP = 8 bước)
khi số bước thực hiện bằng một nửa (0.5 * NUMBER_OF_STEP) thì tạm dừng 1s nhưng vẫn kiểm tra điều kiện input và sẽ kết thúc tạm dừng sớm nếu chuyển trạng thái

kiểm tra nếu set speed khi đang chạy thuận hoặc nghịch (is_set_speed = true) và thời gian hiển thị TANG TOC hoặc GIAM TOC lên LCD lớn hơn TIME_SHOW_SET_SPEED(2s)
thì hiển thị lại THUAN 50% hoặc NGHICH 50% tương ứng bằng toán tử 3 ngôi.
đặt lại is_set_speed = false để k thực hiện lại lệnh in

trường hợp state là STOP
nếu set speed thì động cơ sẽ quay liên tục theo chiều đc đặt trước đó (tương tự cách làm với FORWARD và REVERSE)
ngược lại dừng động cơ
*/
//===============================================================================================================

void check_input(){
  /*
  hàm kiểm tra trạng thái đầu vào
  is_press ngăn việc nhấn giữ khiến điều kiện thực hiện liên tục. chỉ khi tất cả các nút đc nhả thì mới có thể nhấn tiếp
  current ngăn việc reset khi nhấn liên tục 1 nút thuận nghịch hoặc stop. vd nếu nhấn quay thuận liên tục thì động cơ sẽ quay 360 độ k nghỉ
  */
  if(digitalRead(FORWARD_BTN) == HIGH){
    if(is_press || current == FORWARD) return;
    is_press = true;
    is_forward = true;
    is_set_speed = false;
    print_LCD("THUAN 50%", string_speed());
    state = FORWARD;
    current = state;
    total_step = 0;
  }else if(digitalRead(REVERSE_BTN) == HIGH){
    if(is_press || current == REVERSE) return;
    is_press = true;
    is_forward = false;
    is_set_speed = false;
    print_LCD("NGHICH 50%", string_speed());
    state = REVERSE;
    current = state;
    total_step = 0;
  }else if(digitalRead(STOP_BTN) == HIGH){
    if((is_press || current == STOP) && !is_set_speed) return;
    is_press = true;
    is_set_speed = false;
    print_LCD("STOP", "");
    state = STOP;
    current = state;
  }else if(digitalRead(SPEED_UP_BTN) == HIGH){
    if(is_press) return;
    is_press = true;
    if(!is_high_speed){
      is_high_speed = true;
      myStepper.setSpeed(HIGH_SPEED);
      print_LCD("TANG TOC", "");
    }else{
      print_LCD("MAX SPEED", "");
    }
    t_set_speed = millis();
    is_set_speed = true;
  }else if(digitalRead(SPEED_DOWN_BTN) == HIGH){
    if(is_press) return;
    is_press = true;
    if(is_high_speed){
      is_high_speed = false;
      myStepper.setSpeed(LOW_SPEED);
      print_LCD("GIAM TOC", "");
    }else{
      print_LCD("MIN SPEED", "");
    }
    t_set_speed = millis();
    is_set_speed = true;
  }else{
    is_press = false;
  }
}

void delay_while_check_input(int m){
  /*
  tạm dừng nhưng vẫn kiểm tra input bằng cách thực hiện vòng lặp while trong thời gian delay mong muốn
  hàm millis() trả về 1 số kiểu unsigned long là thời gian vi xử lý đã chạy tính bằng mili giây
  lấy thời gian hiện tại trừ đi thời gian bắt đầu ta đc thời gian đã qua tính từ lúc bắt đầu
  */
  Status start_state = state;
  t = millis();
  while((unsigned long)(millis() - t) < m){
    check_input();
    if(state != start_state) return;
    delay(100);
  }
}

String string_speed(){
  // hàm trả về chuỗi tốc tốc độ quay hiện tại + đuôi RPM
  if(is_high_speed)
    return String(HIGH_SPEED) + " RPM";
  return String(LOW_SPEED) + " RPM";
}

void print_LCD(String line1, String line2){
  // hàm in ra màn hình LCD với dòng 1 (line1) và dòng 2 (line2)
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(line1);
  lcd.setCursor(4, 1);
  lcd.print(line2);
}

void stop_motor(){
  digitalWrite(MOTOR_PIN_1, LOW);
  digitalWrite(MOTOR_PIN_2, LOW);
  digitalWrite(MOTOR_PIN_3, LOW);
  digitalWrite(MOTOR_PIN_4, LOW);
}

//=============================================== END =========================================================
