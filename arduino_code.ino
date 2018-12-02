#include <Adafruit_NeoPixel.h>
#include "Adafruit_MCP23008.h"

#define BUZZER_PIN 2
#define LEFT_PIN 4
#define DOWN_PIN 5
#define UP_PIN 6
#define RIGHT_PIN 7
#define ENTER_PIN 8
#define LED2_R_PIN 9
#define LED2_G_PIN 10
#define LED2_B_PIN 11
#define LED3_PIN 12
#define LED1_PIN 13
#define POT_PIN A1
#define TEMP_PIN A2
#define LIGHT_PIN A3

#define CMD_BEGIN '#'
#define CMD_IGNORE '!'

Adafruit_MCP23008 seg7;
Adafruit_NeoPixel digital_led = Adafruit_NeoPixel(5, LED3_PIN, NEO_GRB + NEO_KHZ800);

String cmd = "";              
bool gotCommand = false;
uint8_t comp_vals[7][3];
bool buttons[5];
int pot;
int light;
float temp;

void serialEvent();
void changeState();
void sendState();

void RGBled();
void misc();        // LED, BUZZER, 7SEG DISPLAY CONTROL
void digitalLed();

void getButtonsState();
void getPot();
void getTemp();
void getLight();

void setup()
{
  seg7.begin(0x4);
  for( int x = 0; x < 8; x++)
  {
    seg7.pinMode(x, OUTPUT);
  }
  digital_led.begin();
  Serial.begin(9600);
  cmd.reserve(13);

  pinMode(LEFT_PIN, INPUT);
  pinMode(DOWN_PIN, INPUT);
  pinMode(UP_PIN, INPUT);
  pinMode(RIGHT_PIN, INPUT);
  pinMode(ENTER_PIN, INPUT);
  
  pinMode(BUZZER_PIN, OUTPUT); 
  pinMode(LED2_R_PIN, OUTPUT);
  pinMode(LED2_G_PIN, OUTPUT);
  pinMode(LED2_B_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);

  RGBled();
  misc();
  digitalLed();
}

void loop() 
{
  if (gotCommand) 
  {
    if(cmd[0] == CMD_BEGIN)
    {
      if(cmd.length() == 13) { changeState(); Serial.print(cmd); }
      else if(cmd.length() == 4) sendState();
    }
  cmd = "";
  gotCommand = false;
  }

  getButtonsState();
  getPot();
  getTemp();
  getLight();
}

void serialEvent() 
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    cmd += inChar;
    if (inChar == '\n') gotCommand = true;
  }
}

void changeState()
{
  int component = int(cmd[1]) - int('A');
  if(component > 6) return;
  
  if(cmd[2] != CMD_IGNORE && cmd[3] != CMD_IGNORE && cmd[4] != CMD_IGNORE)
  {
    comp_vals[component][0] = (cmd[2] - '0')*100 + (cmd[3] - '0')*10 + (cmd[4] - '0');
  }
  if(cmd[5] != CMD_IGNORE && cmd[6] != CMD_IGNORE && cmd[7] != CMD_IGNORE)
  {
    comp_vals[component][1] = (cmd[5] - '0')*100 + (cmd[6] - '0')*10 + (cmd[7] - '0');
  }
  if(cmd[8] != CMD_IGNORE && cmd[9] != CMD_IGNORE && cmd[10] != CMD_IGNORE)
  {
    comp_vals[component][2] = (cmd[8] - '0')*100 + (cmd[9] - '0')*10 + (cmd[10] - '0');
  }
  
  switch (component)
  {
    default: { digitalLed(); break; }
    case 5: { RGBled(); break; }
    case 6: { misc(); break; }
  }
}

void sendState()
{
  int component = int(cmd[1]) - int('a');
  String data = "         ";
  data.reserve(9);
  if(component <= 6)
  {
    data[0] = char((comp_vals[component][0] - comp_vals[component][0]%100)/100) + '0';
    data[1] = char((comp_vals[component][0]%100 - comp_vals[component][0]%10)/10) + '0';
    data[2] = char(comp_vals[component][0]%10) + '0';
    data[3] = char((comp_vals[component][1] - comp_vals[component][1]%100)/100) + '0';
    data[4] = char((comp_vals[component][1]%100 - comp_vals[component][1]%10)/10) + '0';
    data[5] = char(comp_vals[component][1]%10) + '0';
    data[6] = char((comp_vals[component][2] - comp_vals[component][2]%100)/100) + '0';
    data[7] = char((comp_vals[component][2]%100 - comp_vals[component][2]%10)/10) + '0';
    data[8] = char(comp_vals[component][2]%10) + '0';
  }
  else if(component == 8)
  {
    for(int i = 0; i < 5; i++)
    {
      data[i] = char(buttons[i]) + '0';
    }
  }
  else if(component == 9)
  {
    data[0] = char((pot - pot % 1000) / 1000) + '0';
    data[1] = char((pot % 1000 - pot % 100) / 100) + '0';
    data[2] = char((pot % 100 - pot % 10) / 10) + '0';
    data[3] = char(pot % 10) + '0';
  }
  else if(component == 10)
  {
    data[0] = char((light - light % 1000) / 1000) + '0';
    data[1] = char((light % 1000 - light % 100) / 100) + '0';
    data[2] = char((light % 100 - light % 10) / 10) + '0';
    data[3] = char(light % 10) + '0';
  }
  else if(component == 11)
  {
    Serial.println(temp);
    int cnv_temp = temp * 100;
    data[0] = char((cnv_temp - cnv_temp % 1000) / 1000) + '0';
    data[1] = char((cnv_temp % 1000 - cnv_temp % 100) / 100) + '0';
    data[2] = char((cnv_temp % 100 - cnv_temp % 10) / 10) + '0';
    data[3] = char(cnv_temp % 10) + '0';
  }
  Serial.print("@" + data + "\r\n");
}

void digitalLed()
{
  for(int i = 0; i < 5; i++)
  {
    digital_led.setPixelColor(i, digital_led.Color(comp_vals[i][0], comp_vals[i][1], comp_vals[i][2]));
  }
  digital_led.show();
}

void RGBled()
{
  analogWrite(LED2_R_PIN, comp_vals[5][0]);
  analogWrite(LED2_G_PIN, comp_vals[5][1]);
  analogWrite(LED2_B_PIN, comp_vals[5][2]);
}

void misc()
{
  digitalWrite(LED1_PIN, comp_vals[6][0]);  // LED
  digitalWrite(BUZZER_PIN, comp_vals[6][1]);   // BUZZER

  // 7-SEG DISPLAY
  switch (comp_vals[6][2])
  {
    case 0:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, HIGH);
      seg7.digitalWrite(4, HIGH);
      seg7.digitalWrite(5, HIGH);
      seg7.digitalWrite(6, LOW);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 1:
    {
      seg7.digitalWrite(0, LOW);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, LOW);
      seg7.digitalWrite(4, LOW);
      seg7.digitalWrite(5, LOW);
      seg7.digitalWrite(6, LOW);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 2:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, LOW);
      seg7.digitalWrite(3, HIGH);
      seg7.digitalWrite(4, HIGH);
      seg7.digitalWrite(5, LOW);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 3:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, HIGH);
      seg7.digitalWrite(4, LOW);
      seg7.digitalWrite(5, LOW);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 4:
    {
      seg7.digitalWrite(0, LOW);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, LOW);
      seg7.digitalWrite(4, LOW);
      seg7.digitalWrite(5, HIGH);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 5:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, LOW);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, HIGH);
      seg7.digitalWrite(4, LOW);
      seg7.digitalWrite(5, HIGH);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 6:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, LOW);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, HIGH);
      seg7.digitalWrite(4, HIGH);
      seg7.digitalWrite(5, HIGH);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 7:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, LOW);
      seg7.digitalWrite(4, LOW);
      seg7.digitalWrite(5, LOW);
      seg7.digitalWrite(6, LOW);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 8:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, HIGH);
      seg7.digitalWrite(4, HIGH);
      seg7.digitalWrite(5, HIGH);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    case 9:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, HIGH);
      seg7.digitalWrite(2, HIGH);
      seg7.digitalWrite(3, LOW);
      seg7.digitalWrite(4, LOW);
      seg7.digitalWrite(5, HIGH);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
    default:
    {
      seg7.digitalWrite(0, HIGH);
      seg7.digitalWrite(1, LOW);
      seg7.digitalWrite(2, LOW);
      seg7.digitalWrite(3, HIGH);
      seg7.digitalWrite(4, HIGH);
      seg7.digitalWrite(5, HIGH);
      seg7.digitalWrite(6, HIGH);
      seg7.digitalWrite(7, HIGH);
      break;
    }
  }
}

void getButtonsState()
{
  bool temp_buttons[5];
  for(int i = 0; i < 5; i++)
  {
    if(digitalRead(i + 4) == HIGH) temp_buttons[i] = 1;
    else temp_buttons[i] = 0;
  }
  for(int i = 0; i < 5; i++)
  {
    if(buttons[i] != temp_buttons[i])
    {
      delay(70);
      if(digitalRead(i + 4) == temp_buttons[i]) buttons[i] = temp_buttons[i];
    }
  }
}

void getPot()
{
  pot = analogRead(POT_PIN);
}

void getTemp()
{
  temp = analogRead(TEMP_PIN) * 0.125 - 22.0;
}

void getLight()
{
  light = analogRead(LIGHT_PIN);
}
