#define UP_TIME_POT 7
#define DOWN_TIME_POT 6
#define DEPTH_POT 5
#define USER_LED 6
#define VARIO_LED 5
#define USER_SWITCH 2
#define BYPASS_SWITCH 12

#define FLAGS_SETUP 0
#define INCREMENTING_FLAG 0b1

#define MODE_TRIANGLE 0x0
#define MODE_SQUARE 0x10
#define MODE_OFF 0xf0

int count = 0;
uint8_t flags = FLAGS_SETUP;  
uint8_t mode = 0;
  
void setup() {
  // put your setup code here, to run once:
  pinMode(USER_LED, OUTPUT);
  pinMode(VARIO_LED, OUTPUT);
  pinMode(USER_SWITCH, INPUT);
  pinMode(BYPASS_SWITCH, INPUT);
  Serial.begin(9600);
}

float mapfloat(int x, int in_min, int in_max, float out_min, float out_max)
{
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (flags & INCREMENTING_FLAG){
    count += map(analogRead(UP_TIME_POT), 0, 1023, 0, 255);
    if (count >= 0xff){
      flags &= ~INCREMENTING_FLAG; //unset the increment flag when we hit the top
      count =255;
    }
    
  } else {
    count -= map(analogRead(DOWN_TIME_POT), 0, 1023, 0, 255);
    if (count <= 0){
      flags |= INCREMENTING_FLAG; //set the increment flag
      count = 0;
    }
  }
  mode = digitalRead(USER_SWITCH) << 4;
  if (digitalRead(BYPASS_SWITCH)) mode = MODE_OFF;

  uint8_t depth = map(analogRead(DEPTH_POT), 0, 1023, 0, 255);
  float percentDepth = mapfloat(depth, 0, 255, 0, 1);

  switch(mode){
    case MODE_TRIANGLE:
      analogWrite(USER_LED, count * percentDepth);
      analogWrite(VARIO_LED, count * percentDepth);
      break;
     case MODE_SQUARE:
      analogWrite(USER_LED, (flags & INCREMENTING_FLAG) * depth);
      analogWrite(VARIO_LED, (flags & INCREMENTING_FLAG) * depth);
      break;
     case MODE_OFF:
      Serial.println("in OFF mode");
     default:
      digitalWrite(USER_LED, 0);
      digitalWrite(VARIO_LED, 0);
      break;
  }
  delay(50); //people speed, sand is too fast
}
