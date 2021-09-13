#include "Servo.h"

/////////////////////////
// 結論
// arduino nanoは割り込みピンは2本のみ
// 割り込みによって制御することはできない
/////////////////////////


int split(const String data,const char delimiter,String *dst){
    int index = 0;
    int arraySize = (sizeof(data)/sizeof((data)[0]));  
    int datalength = data.length();
    for (int i = 0; i < datalength; i++) {
        char tmp = data.charAt(i);
        if ( tmp == delimiter ) {
            index++;
            if ( index > (arraySize - 1)) return -1;
        }
        else dst[index] += tmp;
    }
    return (index + 1);
}

int servo_num = 4;
Servo servo_handler[4];
int servo_pwm_pin[] = {2,3,4,5}; //もしかしたらpin6も可能かも?知らないけど。
int switch_pin[]    = {7,8,9,10};
int switch_mode[4] = {2,2,2,2}; //mode: 0 off, 1 on, 2 初期化前
int servo_mode[4] = {0,0,0,0}; //mode: 0 off, 1 on
int servo_manual_con[] = {1,1,1,1};

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps

  for (int i = 0; i < servo_num; i++){
    servo_handler[i].attach(servo_pwm_pin[i], 500, 2400);
    pinMode(switch_pin[i], INPUT_PULLUP);
    check_switch(i);
  }
}

void loop(){
  for (int i = 0; i < servo_num; i++){
    check_switch(i);
  }
}

void check_switch(const int id) {
  auto switch_current = digitalRead(switch_pin[id]);
  bool mode_changed = false;
  if(switch_current != switch_mode[id]){
    switch_mode[id] = switch_current;
    servo_manual_con[id] = 1;
    mode_changed = true;
  }
  if(servo_manual_con[id] && mode_changed){
    if (switch_mode[id] == LOW)servo_close(id);
    else servo_open(id);

    servo_mode[id] = switch_mode[id];
  }
}


void serialEvent(){
  if (Serial.available() > 0){
    String buffer = Serial.readStringUntil('\n');
    String lines[10];
    int lines_num = split(buffer, ';', lines);
    for(int i=0; i< lines_num; i++){
      String line_div[10];
      int line_div_num = split(lines[i], ':', line_div);
      if(line_div_num>2){
        Serial.println("syntax error: "+lines[i]);
        continue;
      }
      String cmd = line_div[0];
      String target = line_div[1];
      
      operation_manage(cmd,target);
    }
  }
}

void operation_manage(const String cmd, const String target){
  if(cmd == "read_switch_mode"){
    read_switch_mode();
    return;
  }
  if(cmd == "read_servo_mode"){
    read_servo_mode();
    return;
  }
  if(cmd == "open" || cmd == "close"){
    remote_control(cmd, target);
    return;
  }
}

void read_switch_mode(){
  String buffer = "";
  for(int i=0;i<servo_num; i++){
    buffer += String(switch_mode[i]) + " ";
  }
  Serial.println(buffer);
}

void read_servo_mode(){
  String buffer = "";
  for(int i=0;i<servo_num; i++){
    buffer += String(servo_mode[i]) + " ";
  }
  Serial.println(buffer);
}

void remote_control(const String cmd, const String target){
  String target_div[10];
  int target_div_num = split(target, ' ', target_div);
  for(int i=0; i< target_div_num; i++){
    auto id =target_div[i].toInt();
    if(id <0 || id >=servo_num) continue;
    servo_manual_con[id] = 0;
    if(cmd=="open"){
      servo_mode[id] = 1;
      servo_open(id);
    }
    if(cmd=="close"){
      servo_mode[id] = 0;
      servo_close(id);
    }
  }
}

void servo_open(const int id){
  servo_handler[id].write(90);
}

void servo_close(const int id){
  servo_handler[id].write(0);
}
