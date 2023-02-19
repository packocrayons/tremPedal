#define UP_TIME_POT 7
#define DOWN_TIME_POT 6
#define DEPTH_POT 5
#define USER_LED 10
#define VARIO_LED 9
#define CHMODE_LED 8
#define USER_SWITCH 2
#define BYPASS_SWITCH 12

#define FLAGS_SETUP 0
#define INCREMENTING_FLAG 0b1
#define CHANGE_MODE 0b10

#define MODE_TRIANGLE 0x10
#define MODE_SQUARE 0x0
#define MODE_OFF 0xf0

#define COUNT_MAX 1023

#define CHANGE_MODE_THRESHOLD 100
#define CHANGE_MODE_TIMEOUT 3000

int count = 0;
uint8_t flags = FLAGS_SETUP;  
uint8_t mode = 0;
int change_mode = 0;
int up_time = 0;
int down_time = 0;
int depth = 0;
  
void setup() {
  // put your setup code here, to run once:
  pinMode(USER_LED, OUTPUT);
  pinMode(VARIO_LED, OUTPUT);
  pinMode(CHMODE_LED, OUTPUT);
  pinMode(USER_SWITCH, INPUT);
  pinMode(BYPASS_SWITCH, INPUT);
  TCCR1A = (1 << COM1A0) | (1 << COM1A1) | (1 << COM1B0) | (1 << COM1B1) | (1 << WGM10) | (1 << WGM11); //toggle OCRs on compare match. Phase correct 10 bit pwm
  TCCR1B = (1 << CS00);
}

float mapfloat(int x, int in_min, int in_max, float out_min, float out_max)
{
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

int get_val(int pot_to_read, int* stored_value){
  static long last_change = 0;
  if ((flags & CHANGE_MODE) && ((millis() - last_change) > CHANGE_MODE_TIMEOUT)){
    flags &= ~(flags & CHANGE_MODE);
  }
  int readval = analogRead(pot_to_read);
  if (abs(readval - *stored_value) > CHANGE_MODE_THRESHOLD){
    flags |= CHANGE_MODE;
    last_change = millis();
    *stored_value = readval;
  }
    
}

void loop() {
  get_val(UP_TIME_POT, &up_time);
  get_val(DOWN_TIME_POT, &down_time);

  if (flags & INCREMENTING_FLAG){
    count += up_time;
    if (count >= COUNT_MAX){
      flags &= ~INCREMENTING_FLAG; //unset the increment flag when we hit the top
      count =COUNT_MAX;
    }
    
  } else {
    count -= down_time;
    if (count <= 0){
      flags |= INCREMENTING_FLAG; //set the increment flag
      count = 0;
    }
  }
  mode = digitalRead(USER_SWITCH) << 4;
  if (digitalRead(BYPASS_SWITCH)) mode = MODE_OFF;

  get_val(DEPTH_POT, &depth);
  float percentDepth = mapfloat(depth, 0, 1023, 0, 1);


  digitalWrite(CHMODE_LED, flags & CHANGE_MODE ? HIGH : LOW); //this ternary statement appears unnecessary, but I don't know the risks of sending a shifted bitstream to digitalwrite. It might blindly write to that port and do weird things
  switch(mode){
    case MODE_TRIANGLE:
      OCR1A = count * percentDepth;
      OCR1B = count * percentDepth;
      
      break;
     case MODE_SQUARE:
      OCR1A = (flags & INCREMENTING_FLAG) * depth;
      OCR1B = (flags & INCREMENTING_FLAG) * depth;
  
      break;
     case MODE_OFF:
     default:
      OCR1A = 1023;
      OCR1B = 1023;   
      break;
  }
  delay(50); //people speed, sand is too fast
}
