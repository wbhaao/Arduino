#include <LedControl.h>
#include <LiquidCrystal_I2C.h>
// #include <iostream>
// #include <future>
#include <Wire.h>
// LEFT PIN으로 점프맨 점프 동작작
LiquidCrystal_I2C lcd(0x27, 16, 2);
LedControl lc = LedControl(13, 11, 10, 4); // (dataPin, clockPin, csPin, totalDevices)

// tetris ------------------------------------
int setInt = 0;
void tetris(int upState, int downState, int leftState, int rightState);
void gameOver();
int lc0[] = {0, 0, 0, 0, 0, 0, 0, 0};
int lc1[] = {0, 0, 0, 0, 0, 0, 0, 0};
long active[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
long screen[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int tmpCol = 0;

int figura = 0;
int figuraNext = 0;
int fromLeft = 0;
int fromRight = 0;
int angle = 0;
int colCheck = 0;
int moveCheck = 0;
int score = 0;
int started = 0;
int lcRows = 16;
int lcCols = 8;
int allLines = 0;
int currLines = 0;
int brickDelay = 0;
int defDelay = 500;
int level = 0;

boolean sound = true;

byte X[8] = {0b00000, 0b10001, 0b01010, 0b00100,
             0b01010, 0b10001, 0b00000, 0b00000};

byte O[8] = {0b00000, 0b11111, 0b11111, 0b11111,
             0b11111, 0b11111, 0b00000, 0b00000};

byte L[8] = {0b11000, 0b11000, 0b11000, 0b11000,
             0b11000, 0b11111, 0b11111, 0b00000};

byte J[8] = {0b00011, 0b00011, 0b00011, 0b00011,
             0b00011, 0b11111, 0b11111, 0b00000};

byte T[8] = {0b00000, 0b00000, 0b11111, 0b11111,
             0b01110, 0b01110, 0b00000, 0b00000};

byte I[8] = {0b01100, 0b01100, 0b01100, 0b01100,
             0b01100, 0b01100, 0b01100, 0b00000};

byte Z[8] = {0b00000, 0b00000, 0b11110, 0b11110,
             0b01111, 0b01111, 0b00000, 0b00000};

byte S[8] = {0b00000, 0b00000, 0b01111, 0b01111,
             0b11110, 0b11110, 0b00000, 0b00000};

// Nuty
int length = 99;
char notes[] = "EbCDCbaaCEDCbbCDECaa DFAGFEECEDCbbCDECaa EbCDCbaaCEDCbbCDECaa "
               "DFAGFEECEDCbbCDECaa ECDbCab ECDbCEAJ ";
int beats[] = // Som
    {2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 2, 2, 2, 4, 2, 2, 1, 2, 1,
     1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 2, 2, 2, 4, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1,
     2, 1, 1, 2, 1, 1, 2, 2, 2, 2, 4, 2, 2, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1,
     1, 2, 2, 2, 2, 4, 1, 5, 5, 5, 5, 5, 5, 7, 2, 5, 5, 5, 5, 2, 2, 5, 5, 3};
int tempo1 = 128; // Tempo

void playTone(int tone, int duration) {

}

void playNote(char note, int duration) {
  char names[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C',
                  'D', 'E', 'F', 'G', 'J', 'A', 'B'};
  int tones[] = {1915, 1700, 1519, 1432, 1275, 1136, 1014, 956,
                 850,  760,  716,  637,  603,  568,  507};

  for (int i = 0; i < 14; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void updateColumn(int colnum) {
  lc0[colnum] = active[colnum] >> (lcRows / 2);
  lc1[colnum] = active[colnum];
  lc.setColumn(0, colnum, (screen[colnum] >> (lcRows / 2)) | lc0[colnum]);
  lc.setColumn(1, colnum, screen[colnum] | lc1[colnum]);
}

void buttonDelay(int bdelay) {
  if (brickDelay > bdelay) {
    brickDelay -= bdelay;
  }
  delay(bdelay);
}

void splashScreen() {
  int up[] = {
      B11101110, //   o o o   o o o
      B01001000, //     o     o
      B01001100, //     o     o o
      B01001000, //     o     o
      B01001110, //     o     o o o
      B00000000, //
      B11101110, //   o o o   o o o
      B01001010  //     o     o   o
  };

  int down[] = {
      B01001100, //     o     o o
      B01001010, //     o     o   o
      B01001001, //     o     o     o
      B00000000, //
      B01000111, //     o       o o o
      B01000100, //     o       o
      B01000010, //     o         o
      B01001110  //     o     o o o
  };

  for (int rownum = 0; rownum < 8; rownum++) {
    lc.setRow(0, rownum, up[rownum]);
    lc.setRow(1, rownum, down[rownum]);
  }
}

// ---------BGM 코드---------------------------
#define melodyPin 7

unsigned long playDuration = 0;
int currentMelody = 0;
int melodySize = 78;

#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

int melody[] = {
    NOTE_E7, NOTE_E7, 0,       NOTE_E7, 0,        NOTE_C7,  NOTE_E7, 0,
    NOTE_G7, 0,       0,       0,       NOTE_G6,  0,        0,       0,
    NOTE_C7, 0,       0,       NOTE_G6, 0,        0,        NOTE_E6, 0,
    0,       NOTE_A6, 0,       NOTE_B6, 0,        NOTE_AS6, NOTE_A6, 0,
    NOTE_G6, NOTE_E7, NOTE_G7, NOTE_A7, 0,        NOTE_F7,  NOTE_G7, 0,
    NOTE_E7, 0,       NOTE_C7, NOTE_D7, NOTE_B6,  0,        0,       NOTE_C7,
    0,       0,       NOTE_G6, 0,       0,        NOTE_E6,  0,       0,
    NOTE_A6, 0,       NOTE_B6, 0,       NOTE_AS6, NOTE_A6,  0,       NOTE_G6,
    NOTE_E7, NOTE_G7, NOTE_A7, 0,       NOTE_F7,  NOTE_G7,  0,       NOTE_E7,
    0,       NOTE_C7, NOTE_D7, NOTE_B6, 0,        0};

// Mario main them tempo
int tempo[] = {
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    9,  9,  9,  12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 9,
    9,  9,  12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
};

void sing(int distance) {
  // stop the tone playing:
  noTone(8);
  int sec = 100 - distance * 2 <= 0 ? 0 : 100 - distance * 2;
  if (currentMelody >= melodySize)
    currentMelody = 0;
  // to calculate the note duration, take one second
  // divided by the note type.
  // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
  int noteDuration = 1000 / (tempo[currentMelody]) - (120 - sec);

  tone(melodyPin, melody[currentMelody], noteDuration);
  playDuration = noteDuration * 1.30;

  currentMelody++;
  delay(sec);
}
// -----------------------------------------
// -----------점프맨 코드 -----------------------

#define PIN_BUTTON 2
#define PIN_AUTOPLAY 1
#define PIN_READWRITE 10
#define PIN_CONTRAST 12

#define SPRITE_RUN1 1
#define SPRITE_RUN2 2
#define SPRITE_JUMP 3
#define SPRITE_JUMP_UPPER '.' // Use the '.' character for the head
#define SPRITE_JUMP_LOWER 4
#define SPRITE_TERRAIN_EMPTY ' ' // User the ' ' character
#define SPRITE_TERRAIN_SOLID 5
#define SPRITE_TERRAIN_SOLID_RIGHT 6
#define SPRITE_TERRAIN_SOLID_LEFT 7

#define HERO_HORIZONTAL_POSITION 1 // Horizontal position of hero on screen

#define TERRAIN_WIDTH 16
#define TERRAIN_EMPTY 0
#define TERRAIN_LOWER_BLOCK 1
#define TERRAIN_UPPER_BLOCK 2

#define HERO_POSITION_OFF 0         // Hero is invisible
#define HERO_POSITION_RUN_LOWER_1 1 // Hero is running on lower row (pose 1)
#define HERO_POSITION_RUN_LOWER_2 2 //                              (pose 2)

#define HERO_POSITION_JUMP_1 3  // Starting a jump
#define HERO_POSITION_JUMP_2 4  // Half-way up
#define HERO_POSITION_JUMP_3 5  // Jump is on upper row
#define HERO_POSITION_JUMP_4 6  // Jump is on upper row
#define HERO_POSITION_JUMP_5 7  // Jump is on upper row
#define HERO_POSITION_JUMP_6 8  // Jump is on upper row
#define HERO_POSITION_JUMP_7 9  // Half-way down
#define HERO_POSITION_JUMP_8 10 // About to land

#define HERO_POSITION_RUN_UPPER_1 11 // Hero is running on upper row (pose 1)
#define HERO_POSITION_RUN_UPPER_2 12 //                              (pose 2)

static char terrainUpper[TERRAIN_WIDTH + 1];
static char terrainLower[TERRAIN_WIDTH + 1];
static bool buttonPushed = false;

void initializeGraphics() {
  static byte graphics[] = {
      // Run position 1
      B01100,
      B01100,
      B00000,
      B01110,
      B11100,
      B01100,
      B11010,
      B10011,
      // Run position 2
      B01100,
      B01100,
      B00000,
      B01100,
      B01100,
      B01100,
      B01100,
      B01110,
      // Jump
      B01100,
      B01100,
      B00000,
      B11110,
      B01101,
      B11111,
      B10000,
      B00000,
      // Jump lower
      B11110,
      B01101,
      B11111,
      B10000,
      B00000,
      B00000,
      B00000,
      B00000,
      // Ground
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      // Ground right
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      // Ground left
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
  };
  int i;
  // Skip using character 0, this allows lcd.print() to be used to
  // quickly draw multiple characters
  for (i = 0; i < 7; ++i) {
    lcd.createChar(i + 1, &graphics[i * 8]);
  }
  for (i = 0; i < TERRAIN_WIDTH; ++i) {
    terrainUpper[i] = SPRITE_TERRAIN_EMPTY;
    terrainLower[i] = SPRITE_TERRAIN_EMPTY;
  }
}

// Slide the terrain to the left in half-character increments
void advanceTerrain(char *terrain, byte newTerrain) {
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    char current = terrain[i];
    char next = (i == TERRAIN_WIDTH - 1) ? newTerrain : terrain[i + 1];
    switch (current) {
    case SPRITE_TERRAIN_EMPTY:
      terrain[i] = (next == SPRITE_TERRAIN_SOLID) ? SPRITE_TERRAIN_SOLID_RIGHT
                                                  : SPRITE_TERRAIN_EMPTY;
      break;
    case SPRITE_TERRAIN_SOLID:
      terrain[i] = (next == SPRITE_TERRAIN_EMPTY) ? SPRITE_TERRAIN_SOLID_LEFT
                                                  : SPRITE_TERRAIN_SOLID;
      break;
    case SPRITE_TERRAIN_SOLID_RIGHT:
      terrain[i] = SPRITE_TERRAIN_SOLID;
      break;
    case SPRITE_TERRAIN_SOLID_LEFT:
      terrain[i] = SPRITE_TERRAIN_EMPTY;
      break;
    }
  }
}
bool drawHero(byte position, char *terrainUpper, char *terrainLower,
              unsigned int score) {
  bool collide = false;
  char upperSave = terrainUpper[HERO_HORIZONTAL_POSITION];
  char lowerSave = terrainLower[HERO_HORIZONTAL_POSITION];
  byte upper, lower;
  switch (position) {
  case HERO_POSITION_OFF:
    upper = lower = SPRITE_TERRAIN_EMPTY;
    break;
  case HERO_POSITION_RUN_LOWER_1:
    upper = SPRITE_TERRAIN_EMPTY;
    lower = SPRITE_RUN1;
    break;
  case HERO_POSITION_RUN_LOWER_2:
    upper = SPRITE_TERRAIN_EMPTY;
    lower = SPRITE_RUN2;
    break;
  case HERO_POSITION_JUMP_1:
  case HERO_POSITION_JUMP_8:
    upper = SPRITE_TERRAIN_EMPTY;
    lower = SPRITE_JUMP;
    break;
  case HERO_POSITION_JUMP_2:
  case HERO_POSITION_JUMP_7:
    upper = SPRITE_JUMP_UPPER;
    lower = SPRITE_JUMP_LOWER;
    break;
  case HERO_POSITION_JUMP_3:
  case HERO_POSITION_JUMP_4:
  case HERO_POSITION_JUMP_5:
  case HERO_POSITION_JUMP_6:
    upper = SPRITE_JUMP;
    lower = SPRITE_TERRAIN_EMPTY;
    break;
  case HERO_POSITION_RUN_UPPER_1:
    upper = SPRITE_RUN1;
    lower = SPRITE_TERRAIN_EMPTY;
    break;
  case HERO_POSITION_RUN_UPPER_2:
    upper = SPRITE_RUN2;
    lower = SPRITE_TERRAIN_EMPTY;
    break;
  }
  if (upper != ' ') {
    terrainUpper[HERO_HORIZONTAL_POSITION] = upper;
    collide = (upperSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }
  if (lower != ' ') {
    terrainLower[HERO_HORIZONTAL_POSITION] = lower;
    collide |= (lowerSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }

  byte digits = (score > 9999)  ? 5
                : (score > 999) ? 4
                : (score > 99)  ? 3
                : (score > 9)   ? 2
                                : 1;

  // scene
  terrainUpper[TERRAIN_WIDTH] = '\0';
  terrainLower[TERRAIN_WIDTH] = '\0';
  char temp = terrainUpper[16 - digits];
  terrainUpper[16 - digits] = '\0';
  lcd.setCursor(0, 0);
  lcd.print(terrainUpper);
  terrainUpper[16 - digits] = temp;
  lcd.setCursor(0, 1);
  lcd.print(terrainLower);

  lcd.setCursor(16 - digits, 0);
  lcd.print(score);

  terrainUpper[HERO_HORIZONTAL_POSITION] = upperSave;
  terrainLower[HERO_HORIZONTAL_POSITION] = lowerSave;
  return collide;
}

// Handle the button push as an interrupt
void buttonPush() { buttonPushed = true; }
void jumpman(int cnt);
//------------------------------------------
// ---------------- 기본 동작 -------------
int cnt = 0;
int upPin = 4;
int downPin = 3;

int leftPin = 2;
int rightPin = 5;

int nowState = 2; //대기 화면에서, 현재 어느 게임을 선택했는지 알려주는 변수

int inGame = 0; //게임이 시작했는지 아닌지 알려주는 변수

int menuUp, menuDown = 0; //대기 화면에서, up버튼을 클릭했는지 확인하는 변수
                          //대기 화면에서, down버튼을 클릭했는지 확인하는 변수

int upButtonState, downButtonState, leftButtonState, rightButtonState = 0;

void setup() {

  Serial.begin(9600);
  pinMode(melodyPin, OUTPUT);
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
  // 2번핀이 INPUT되었을 떄 함수 실행, FALLING HIGH->LOW일때 인터럽트 실행
  attachInterrupt(1, buttonPush, FALLING);
  lc.shutdown(0,false);
  lc.shutdown(1,false);
 
  lc.setIntensity(0,5);
  lc.setIntensity(1,5);
  Serial.begin(9600);
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  randomSeed(analogRead(0));
}

void buttonControl(int upState, int downState, int leftState, int rightState);
void loop() {
  cnt += 1;
  // std::future<void> a = std::async(std::launch::async, sing);
  // AsyncIO.loop();
  // 입력 받기
  int upState = digitalRead(upPin);
  int downState = digitalRead(downPin);
  int leftState = digitalRead(leftPin);
  int rightState = digitalRead(rightPin);

  buttonControl(upState, downState, leftState, rightState);
  Serial.println(upState);
  Serial.println(downState);
  Serial.println(leftState);
  Serial.println(rightState);
  Serial.println("============");
  // cnt는 게임 시작 후 시간.
  if (rightState == 1 ) {
    inGame = 1;
  }

  //  ------------- 메뉴State일때 --------------
  if (inGame == 0) {
    // 그냥 sing
    if (cnt % 2 == 0)
      sing(0);
    // Up버튼을 눌렀다면
    if (menuUp == 1 && nowState < 3) {
      lcd.clear();
      nowState++;
      menuUp = 0;
    } else if (menuDown == 1 && nowState > 1) {
      lcd.clear();
      nowState--;
      menuDown = 0;
    }
    lcd.setCursor(0, 0);
    whatGame();
  }
  //  --------------- 게임State일때 --------------
  // 여기서 게임 스테이트라면 각각 다른 게임 동작 // if문으로 구분 (함수로
  // 객체화)
  else if (inGame == 1) {

    // 점프맨
    if (nowState == 1) {
      jumpman(cnt);
    }
    // 테트리스
    else if (nowState == 2) {
      Serial.println("-----------------");
      Serial.println(started);
      Serial.println("-----------------");
      if (started == 0) {
        splashScreen();
        for (int i = 0; i < length; i++) {
          if (upState == 1) {
            started = 1;
            break;
          }
          if (notes[i] == ' ') {
            delay(10); // Pausa
          }
        }

        delay(10);
      }
      else {
        if (setInt == 0){
          setInt = 1;
          lc.clearDisplay(0);
          lc.clearDisplay(1);
          memset(lc0, 0, sizeof(lc0));
          memset(lc1, 0, sizeof(lc1));
          memset(active, 0, sizeof(active));
          memset(screen, 0, sizeof(screen));
          tmpCol = 0;
        }
        tetris(upState, downState, leftState, rightState);
      }
    }
    // 스네이크
    else if (nowState == 3) {
      jumpman(cnt);
    }
  }
}

void whatGame() { //현재 무슨게임인지 알려주는 함수
  if (nowState == 1) {
    lcd.print("snake");
  } else if (nowState == 2) {
    lcd.print("tetris");
  } else if (nowState == 3) {
    lcd.print("jumpman");
  }
}

void buttonControl(int upState, int downState, int rightState, int leftState) {
  //버튼을 꾹 눌러도 1번만 인식되게 하는 명령어
  if (upState == 1) {
    if (upButtonState == 0) {
      delay(10);
      upButtonState = 1;
    }
  }

  if (upState == 0) {
    if (upButtonState == 1) {
      menuUp = 1; // up버튼 누르고 뗐을시 menuUp이 1이 됨
      delay(10);
      upButtonState = 0;
    }
  }

  if (downState == 1) {
    if (downButtonState == 0) {
      delay(10);
      downButtonState = 1;
    }
  }

  if (downState == 0) {
    if (downButtonState == 1) {
      menuDown = 1; // down버튼 누르고 뗐을시 menuDown이 1이 됨
      delay(10);
      downButtonState = 0;
    }
  }

  if (leftState == 1) {
    if (leftButtonState == 0) {
      delay(10);
      leftButtonState = 1;
    }
  }

  if (leftState == 0) {
    if (leftButtonState == 1) {
      delay(10);
      leftButtonState = 0;
    }
  }

  if (rightState == 1) {
    if (rightButtonState == 0) {
      delay(10);
      rightButtonState = 1;
    }
  }

  if (rightState == 0) {
    if (rightButtonState == 1) {
      delay(10);
      rightButtonState = 0;
    }
  }
}

void jumpman(int cnt) {
  static byte heroPos = HERO_POSITION_RUN_LOWER_1;
  static byte newTerrainType = TERRAIN_EMPTY;
  static byte newTerrainDuration = 1;
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;
  // 점점 빨리지는 sing
  if (cnt % 2 == 0)
    sing(distance);
  if (!playing) {
    lcd.clear();
    drawHero((blink) ? HERO_POSITION_OFF : heroPos, terrainUpper, terrainLower,
             distance >> 3);
    if (blink) {
      lcd.setCursor(0, 0);
      lcd.print("Press Start");
    }
    blink = !blink;
    if (buttonPushed) {
      initializeGraphics();
      heroPos = HERO_POSITION_RUN_LOWER_1;
      playing = true;
      buttonPushed = false;
      distance = 0;
    }
    return;
  }

  // Shift the terrain to the left
  advanceTerrain(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK
                                   ? SPRITE_TERRAIN_SOLID
                                   : SPRITE_TERRAIN_EMPTY);
  advanceTerrain(terrainUpper, newTerrainType == TERRAIN_UPPER_BLOCK
                                   ? SPRITE_TERRAIN_SOLID
                                   : SPRITE_TERRAIN_EMPTY);

  // Make new terrain to enter on the right
  if (--newTerrainDuration == 0) {
    if (newTerrainType == TERRAIN_EMPTY) {
      newTerrainType =
          (random(3) == 0) ? TERRAIN_UPPER_BLOCK : TERRAIN_LOWER_BLOCK;
      newTerrainDuration = 2 + random(10);
    } else {
      newTerrainType = TERRAIN_EMPTY;
      newTerrainDuration = 10 + random(10);
    }
  }

  if (buttonPushed) {
    if (heroPos <= HERO_POSITION_RUN_LOWER_2)
      heroPos = HERO_POSITION_JUMP_1;
    buttonPushed = false;
  }

  if (drawHero(heroPos, terrainUpper, terrainLower, distance >> 3)) {
    playing = false; // hero collided with something. Too bad.
  } else {
    if (heroPos == HERO_POSITION_RUN_LOWER_2 ||
        heroPos == HERO_POSITION_JUMP_8) {
      heroPos = HERO_POSITION_RUN_LOWER_1;
    } else if ((heroPos >= HERO_POSITION_JUMP_3 &&
                heroPos <= HERO_POSITION_JUMP_5) &&
               terrainLower[HERO_HORIZONTAL_POSITION] != SPRITE_TERRAIN_EMPTY) {
      heroPos = HERO_POSITION_RUN_UPPER_1;
    } else if (heroPos >= HERO_POSITION_RUN_UPPER_1 &&
               terrainLower[HERO_HORIZONTAL_POSITION] == SPRITE_TERRAIN_EMPTY) {
      heroPos = HERO_POSITION_JUMP_5;
    } else if (heroPos == HERO_POSITION_RUN_UPPER_2) {
      heroPos = HERO_POSITION_RUN_UPPER_1;
    } else {
      ++heroPos;
    }
    ++distance;
    digitalWrite(PIN_AUTOPLAY, terrainLower[HERO_HORIZONTAL_POSITION + 2] ==
                                       SPRITE_TERRAIN_EMPTY
                                   ? HIGH
                                   : LOW);
  }
}
void tetris(int upState, int downState, int leftState, int rightState) {
    if (allLines < 100) {
      level = 0; // Level 0
    } else if (allLines < 200) {
      level = 1; // Level 1
    } else if (allLines < 300) {
      level = 2; // Level 2
    } else if (allLines < 400) {
      level = 3; // Level 3
    } else if (allLines < 500) {
      level = 4; // Level 4
    } else {
      level = 5; // Level 5
    }

    defDelay = (5 - level) * 100;
    brickDelay = defDelay;

    if (figura == 0) {
      figura = random(1, 8);
    } else {
      figura = figuraNext;
    }
    figuraNext = random(1, 8);
    angle = 0;

    switch (figura) {
    case 1:
      //"O"
      active[3] = 131072 + 65536;
      active[4] = 131072 + 65536;
      fromLeft = 3;
      fromRight = 3;
      break;

    case 2:
      //"L"
      active[3] = 262144 + 131072 + 65536;
      active[4] = 65536;
      fromLeft = 3;
      fromRight = 3;
      break;

    case 3:
      //"J"
      active[3] = 65536;
      active[4] = 262144 + 131072 + 65536;
      fromLeft = 3;
      fromRight = 3;
      break;

    case 4:
      //"T"
      active[2] = 131072;
      active[3] = 131072 + 65536;
      active[4] = 131072;
      fromLeft = 2;
      fromRight = 3;
      break;

    case 5:
      //"I"
      active[3] = 524288 + 262144 + 131072 + 65536;
      fromLeft = 3;
      fromRight = 4;
      break;

    case 6:
      //"Z"
      active[2] = 131072;
      active[3] = 131072 + 65536;
      active[4] = 65536;
      fromLeft = 2;
      fromRight = 3;
      break;

    case 7:
      //"S"
      active[2] = 65536;
      active[3] = 131072 + 65536;
      active[4] = 131072;
      fromLeft = 2;
      fromRight = 3;
      break;
    }

    for (int krok = 0; krok < lcRows + 1; krok++) {
      colCheck = 0;

      for (int i = 0; i < (lcCols / 2); i++) {
        if ((leftState == 1) && (fromLeft > 0)) {
            moveCheck = 0;
            for (int colnum = fromLeft; colnum < (lcCols - fromRight);
                  colnum++) {
              if ((active[colnum] & screen[colnum - 1]) == 0) {
                moveCheck++;
              }
            }

            if (moveCheck == (lcCols - fromLeft - fromRight)) {
              for (int colnum = (fromLeft - 1); colnum < (lcCols - fromRight);
                    colnum++) {
                if (colnum < (lcCols - 1)) {
                  active[colnum] = active[colnum + 1];
                } else {
                  active[colnum] = 0;
                }
                updateColumn(colnum);
              }
              fromLeft--;
              fromRight++;
              playNote('E', 10);
              buttonDelay(200);
            }
          }
      }

      for (int i = 0; i < (lcCols / 2); i++) {
        if ((rightState == 1) && (fromRight > 0)) {
          moveCheck = 0;
          for (int colnum = fromLeft; colnum < (lcCols - fromRight);
                colnum++) {
            if ((active[colnum] & screen[colnum + 1]) == 0) {
              moveCheck++;
            }
          }

          if (moveCheck == (lcCols - fromLeft - fromRight)) {
            for (int colnum = (lcCols - fromRight); colnum > (fromLeft - 1);
                  colnum--) {
              if (colnum > 0) {
                active[colnum] = active[colnum - 1];
              } else {
                active[colnum] = 0;
              }
              updateColumn(colnum);
            }
            fromLeft++;
            fromRight--;
            playNote('E', 10);
            buttonDelay(200);
          }
        }
      }

      if (downState == 1) {
        brickDelay = 0;
        playNote('b', 10);
      } else {
        brickDelay = defDelay;
      }

      for (int i = 0; i < (lcCols / 2); i++) {
        if (upState == 1) {

          switch (figura) {
          case 1:
            //"O"
            break;

          case 2:
            //"L"
            switch (angle) {
            case 0:
              // . o .        . . .
              // . o .  --->  o o o
              // . o o        o . .
              if ((fromLeft > 0) &&
                  (((active[fromLeft + 1] | (active[fromLeft + 1] << 1)) &
                    screen[fromLeft - 1]) == 0)) {
                active[fromLeft - 1] =
                    (active[fromLeft + 1] | (active[fromLeft + 1] << 1));
                updateColumn(fromLeft - 1);
                active[fromLeft] = (active[fromLeft + 1] << 1);
                updateColumn(fromLeft);
                active[fromLeft + 1] = (active[fromLeft + 1] << 1);
                updateColumn(fromLeft + 1);
                fromLeft--;
                angle = 1;
              }
              break;

            case 1:
              // . . .        o o .
              // o o o  --->  . o .
              // o . .        . o .
              if ((((active[fromLeft + 2] << 1) & screen[fromLeft]) == 0) &&
                  ((((active[fromLeft + 1] << 1) |
                      (active[fromLeft + 1] >> 1)) &
                    screen[fromLeft + 1]) == 0)) {
                active[fromLeft] = (active[fromLeft + 2] << 1);
                updateColumn(fromLeft);
                active[fromLeft + 1] = active[fromLeft + 1] |
                                        (active[fromLeft + 1] << 1) |
                                        (active[fromLeft + 1] >> 1);
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = 0;
                updateColumn(fromLeft + 2);
                fromRight++;
                angle = 2;
              }
              break;

            case 2:
              // o o .        . . o
              // . o .  --->  o o o
              // . o .        . . .
              if ((fromRight > 0) &&
                  (((active[fromLeft] >> 1) & screen[fromLeft]) == 0) &&
                  ((((active[fromLeft + 1] << 1) & active[fromLeft + 1]) &
                    screen[fromLeft + 1]) == 0)) {
                active[fromLeft] = (active[fromLeft] >> 1);
                updateColumn(fromLeft);
                active[fromLeft + 1] = active[fromLeft];
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] =
                    ((active[fromLeft + 1] << 1) | active[fromLeft + 1]);
                updateColumn(fromLeft + 2);
                fromRight--;
                krok--;
                angle = 3;
              }
              break;

            case 3:
              // . . o        . o .
              // o o o  --->  . o .
              // . . .        . o o
              if (((((active[fromLeft] << 1) | (active[fromLeft] >> 1)) &
                    screen[fromLeft + 1]) == 0) &&
                  (((active[fromLeft] >> 1) & screen[fromLeft + 2]) == 0) &&
                  (krok < lcRows)) {
                active[fromLeft] = 0;
                updateColumn(fromLeft);
                active[fromLeft + 1] =
                    (active[fromLeft + 2] | (active[fromLeft + 2] >> 1));
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = ((active[fromLeft + 2] >> 1) &
                                        (active[fromLeft + 2] >> 2));
                updateColumn(fromLeft + 2);
                fromLeft++;
                krok++;
                angle = 0;
              }
              break;
            }
            break;

          case 3:
            //"J"
            switch (angle) {
            case 0:
              // . o .        o . .
              // . o .  --->  o o o
              // o o .        . . .
              if ((fromRight > 0) &&
                  ((((active[fromLeft] << 2) | (active[fromLeft] << 1)) &
                    screen[fromLeft]) == 0) &&
                  (((active[fromLeft] << 1) & screen[fromLeft + 2]) == 0)) {
                active[fromLeft] =
                    ((active[fromLeft] << 2) | (active[fromLeft] << 1));
                updateColumn(fromLeft);
                active[fromLeft + 1] = ((active[fromLeft + 1] << 1) &
                                        (active[fromLeft + 1] >> 1));
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = active[fromLeft + 1];
                updateColumn(fromLeft + 2);
                fromRight--;
                krok--;
                angle = 1;
              }
              break;

            case 1:
              // o . .        . o o
              // o o o  --->  . o .
              // . . .        . o .
              if ((krok < lcRows) &&
                  ((((active[fromLeft + 1] << 1) |
                      (active[fromLeft + 1] >> 1)) &
                    screen[fromLeft + 1]) == 0) &&
                  (((active[fromLeft + 2] << 1) & screen[fromLeft + 2]) ==
                    0)) {
                active[fromLeft] = 0;
                updateColumn(fromLeft);
                active[fromLeft + 1] =
                    (active[fromLeft + 1] | (active[fromLeft + 1] << 1) |
                      (active[fromLeft + 1] >> 1));
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = (active[fromLeft + 2] << 1);
                updateColumn(fromLeft + 2);
                fromLeft++;
                krok++;
                angle = 2;
              }
              break;

            case 2:
              // . o o        . . .
              // . o .  --->  o o o
              // . o .        . . o
              if ((fromLeft > 0) &&
                  (((active[fromLeft + 1] >> 1) & screen[fromLeft - 1]) ==
                    0) &&
                  ((((active[fromLeft + 1] >> 1) |
                      (active[fromLeft + 1] >> 2)) &
                    screen[fromLeft + 1]) == 0)) {
                active[fromLeft - 1] = (active[fromLeft + 1] >> 1);
                updateColumn(fromLeft - 1);
                active[fromLeft] = active[fromLeft - 1];
                updateColumn(fromLeft);
                active[fromLeft + 1] =
                    (active[fromLeft] | (active[fromLeft + 1] >> 2));
                updateColumn(fromLeft + 1);
                fromLeft--;
                angle = 3;
              }
              break;

            case 3:
              // . . .        . o .
              // o o o  --->  . o .
              // . . o        o o .
              if ((((active[fromLeft] >> 1) & screen[fromLeft]) == 0) &&
                  ((((active[fromLeft] << 1) | (active[fromLeft >> 1])) &
                    screen[fromLeft + 1]) == 0)) {
                active[fromLeft] = (active[fromLeft] >> 1);
                updateColumn(fromLeft);
                active[fromLeft + 1] =
                    ((active[fromLeft + 1] << 1) | active[fromLeft + 2]);
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = 0;
                updateColumn(fromLeft + 2);
                fromRight++;
                angle = 0;
              }
              break;
            }
            break;

          case 4:
            //"T"
            switch (angle) {
            case 0:
              // . . .        . o .
              // o o o  --->  o o .
              // . o .        . o .
              if (((active[fromLeft + 1] << 1) & screen[fromLeft + 1]) == 0) {
                // active[fromLeft]
                active[fromLeft + 1] =
                    active[fromLeft + 1] | (active[fromLeft + 1] << 1);
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = 0;
                updateColumn(fromLeft + 2);
                fromRight++;
                angle = 1;
              }
              break;

            case 1:
              // . o .        . o .
              // o o .  --->  o o o
              // . o .        . . .
              if ((fromRight > 0) &&
                  ((active[fromLeft] & screen[fromLeft + 2]) == 0)) {
                // active[fromLeft]
                active[fromLeft + 1] =
                    active[fromLeft + 1] & (active[fromLeft + 1] << 1);
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = active[fromLeft];
                updateColumn(fromLeft + 2);
                fromRight--;
                krok--;
                angle = 2;
              }
              break;

            case 2:
              // . o .        . o .
              // o o o  --->  . o o
              // . . .        . o .
              if ((((active[fromLeft + 1] >> 1) & screen[fromLeft + 1]) ==
                    0) &&
                  (krok < lcRows)) {
                active[fromLeft] = 0;
                updateColumn(fromLeft);
                active[fromLeft + 1] =
                    active[fromLeft + 1] | (active[fromLeft + 1] >> 1);
                updateColumn(fromLeft + 1);
                // active[fromLeft + 2]
                fromLeft++;
                krok++;
                angle = 3;
              }
              break;

            case 3:
              if ((fromLeft > 0) &&
                  ((active[fromLeft + 1] & screen[fromLeft - 1]) == 0)) {
                active[fromLeft - 1] = active[fromLeft + 1];
                updateColumn(fromLeft - 1);
                active[fromLeft] = active[fromLeft] & (active[fromLeft] >> 1);
                updateColumn(fromLeft);
                fromLeft--;
                angle = 0;
              }
              break;
            }
            break;

          case 5:
            //"I"
            switch (angle) {
            case 0:
              // . o . .        . . . .
              // . o . .  --->  o o o o
              // . o . .        . . . .
              // . o . .        . . . .
              if ((fromLeft > 0) && (fromRight > 1)

                  && ((((((active[fromLeft] >> 1) & (active[fromLeft] << 2)) &
                          screen[fromLeft - 1]) &
                        screen[fromLeft + 1]) &
                        screen[fromLeft + 2]) == 0)) {
                active[fromLeft - 1] =
                    ((active[fromLeft] >> 1) & (active[fromLeft] << 2));
                updateColumn(fromLeft - 1);
                active[fromLeft] = active[fromLeft - 1];
                updateColumn(fromLeft);
                active[fromLeft + 1] = active[fromLeft];
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = active[fromLeft];
                updateColumn(fromLeft + 2);
                fromLeft--;
                fromRight -= 2;
                krok -= 2;
                angle = 1;
              }
              break;

            case 1:
              // . . . .        . . o .
              // o o o o  --->  . . o .
              // . . . .        . . o .
              // . . . .        . . o .
              if ((krok < (lcRows - 1)) &&
                  (((active[fromLeft] << 1) | (active[fromLeft] >> 1) |
                    (active[fromLeft] >> 2)) &
                    screen[fromLeft + 2]) == 0) {
                active[fromLeft] = 0;
                updateColumn(fromLeft);
                active[fromLeft + 1] = 0;
                updateColumn(fromLeft + 1);

                active[fromLeft + 2] =
                    (active[fromLeft + 2] | (active[fromLeft + 2] << 1) |
                      (active[fromLeft + 2] >> 1) |
                      (active[fromLeft + 2] >> 2));
                updateColumn(fromLeft + 2);
                active[fromLeft + 3] = 0;
                updateColumn(fromLeft + 3);
                fromLeft += 2;
                fromRight++;
                krok += 2;
                angle = 2;
              }
              break;

            case 2:
              // . . o .        . . . .
              // . . o .  --->  . . . .
              // . . o .        o o o o
              // . . o .        . . . .
              if ((fromLeft > 1) && (fromRight > 0)

                  && ((((((active[fromLeft] << 1) & (active[fromLeft] >> 2)) &
                          screen[fromLeft - 2]) &
                        screen[fromLeft - 1]) &
                        screen[fromLeft + 1]) == 0)) {
                active[fromLeft - 2] =
                    ((active[fromLeft] << 1) & (active[fromLeft] >> 2));
                updateColumn(fromLeft - 2);
                active[fromLeft - 1] = active[fromLeft - 2];
                updateColumn(fromLeft - 1);
                active[fromLeft] = active[fromLeft - 1];
                updateColumn(fromLeft);
                active[fromLeft + 1] = active[fromLeft];
                updateColumn(fromLeft + 1);
                fromLeft -= 2;
                fromRight--;
                krok--;
                angle = 3;
              }
              break;

            case 3:
              // . . . .        . o . .
              // . . . .  --->  . o . .
              // o o o o        . o . .
              // . . . .        . o . .
              if ((krok < (lcRows)) &&
                  (((active[fromLeft] >> 1) | (active[fromLeft] << 1) |
                    (active[fromLeft] << 2)) &
                    screen[fromLeft + 1]) == 0) {
                active[fromLeft] = 0;
                updateColumn(fromLeft);

                active[fromLeft + 1] =
                    (active[fromLeft + 1] | (active[fromLeft + 1] >> 1) |
                      (active[fromLeft + 1] << 1) |
                      (active[fromLeft + 1] << 2));
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = 0;
                updateColumn(fromLeft + 2);
                active[fromLeft + 3] = 0;
                updateColumn(fromLeft + 3);
                fromLeft++;
                fromRight += 2;
                krok++;
                angle = 0;
              }
              break;
            }
            break;

          case 6:
            //"Z"
            switch (angle) {
            case 0:
              // . . .        . o .
              // o o .  --->  o o .
              // . o o        o . .
              if (((active[fromLeft + 1] & screen[fromLeft]) == 0) &&
                  (((active[fromLeft + 1] << 1) & screen[fromLeft + 1]) ==
                    0)) {
                active[fromLeft] = active[fromLeft + 1];
                updateColumn(fromLeft);
                active[fromLeft + 1] = (active[fromLeft + 1] << 1);
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = 0;
                updateColumn(fromLeft + 2);
                fromRight++;
                angle = 1;
              }
              break;

            case 1:
              // . o .        o o .
              // o o .  --->  . o o
              // o . .        . . .
              if ((fromRight > 0) &&
                  ((((active[fromLeft] << 2) & (active[fromLeft] << 1)) &
                    screen[fromLeft]) == 0) &&
                  (((active[fromLeft] & active[fromLeft + 1]) &
                    screen[fromLeft + 2]) == 0)) {
                active[fromLeft] =
                    ((active[fromLeft] << 2) & (active[fromLeft] << 1));
                updateColumn(fromLeft);
                // active[fromLeft + 1]
                active[fromLeft + 2] = (active[fromLeft] >> 1);
                updateColumn(fromLeft + 2);
                fromRight--;
                krok--;
                angle = 2;
              }
              break;

            case 2:
              // o o .        . . o
              // . o o  --->  . o o
              // . . .        . o .
              if ((krok < lcRows) &&
                  (((active[fromLeft + 1] >> 1) & screen[fromLeft + 1]) ==
                    0) &&
                  (((active[fromLeft + 2] << 1) & screen[fromLeft + 2]) ==
                    0)) {
                active[fromLeft] = 0;
                updateColumn(fromLeft);
                active[fromLeft + 1] = (active[fromLeft + 1] >> 1);
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] =
                    (active[fromLeft + 2] | (active[fromLeft + 2] << 1));
                updateColumn(fromLeft + 2);
                fromLeft++;
                krok++;
                angle = 3;
              }
              break;

            case 3:
              // . . o        . . .
              // . o o  --->  o o .
              // . o .        . o o
              if ((fromLeft > 0) &&
                  (((active[fromLeft] & active[fromLeft + 1]) &
                    screen[fromLeft - 1]) == 0) &&
                  (((active[fromLeft + 1] >> 1) & screen[fromLeft + 1]) ==
                    0)) {
                active[fromLeft - 1] =
                    (active[fromLeft] & active[fromLeft + 1]);
                updateColumn(fromLeft - 1);
                // active[fromLeft]
                active[fromLeft + 1] = (active[fromLeft - 1] >> 1);
                updateColumn(fromLeft + 1);
                fromLeft--;
                angle = 0;
              }
              break;
            }
            break;

          case 7:
            //"S"
            switch (angle) {
            case 0:
              // . . .        o . .
              // . o o  --->  o o .
              // o o .        . o .
              if (((active[fromLeft + 1] << 1) & screen[fromLeft]) == 0) {
                active[fromLeft] = (active[fromLeft + 1] << 1);
                updateColumn(fromLeft);
                // active[fromLeft + 1]
                active[fromLeft + 2] = 0;
                updateColumn(fromLeft + 2);
                fromRight++;
                angle = 1;
              }
              break;

            case 1:
              // o . .        . o o
              // o o .  --->  o o .
              // . o .        . . .
              if ((fromRight > 0) &&
                  (((active[fromLeft + 1] << 1) & screen[fromLeft + 1]) ==
                    0) &&
                  (((active[fromLeft] & (active[fromLeft] << 1)) &
                    screen[fromLeft + 2]) == 0)) {
                active[fromLeft] = (active[fromLeft] & active[fromLeft + 1]);
                updateColumn(fromLeft);
                active[fromLeft + 1] = (active[fromLeft + 1] << 1);
                updateColumn(fromLeft + 1);
                active[fromLeft + 2] = (active[fromLeft] << 1);
                updateColumn(fromLeft + 2);
                fromRight--;
                krok--;
                angle = 2;
              }
              break;

            case 2:
              // . o o        . o .
              // o o .  --->  . o o
              // . . .        . . o
              if ((krok < lcRows) && (((active[fromLeft + 1] >> 1) &
                                        screen[fromLeft + 2]) == 0)) {
                active[fromLeft] = 0;
                updateColumn(fromLeft);
                // active[fromLeft + 1]
                active[fromLeft + 2] = (active[fromLeft + 1] >> 1);
                updateColumn(fromLeft + 2);
                fromLeft++;
                krok++;
                angle = 3;
              }
              break;

            case 3:
              // . o .        . . .
              // . o o  --->  . o o
              // . . o        o o .
              if ((fromLeft > 0) &&
                  ((active[fromLeft + 1] & ((active[fromLeft + 1] >> 1)) &
                    screen[fromLeft - 1]) == 0) &&
                  ((active[fromLeft + 1] & screen[fromLeft]) == 0)) {
                active[fromLeft - 1] =
                    (active[fromLeft + 1] & (active[fromLeft + 1] >> 1));
                updateColumn(fromLeft - 1);
                active[fromLeft] = active[fromLeft + 1];
                updateColumn(fromLeft);
                active[fromLeft + 1] = (active[fromLeft - 1] << 1);
                updateColumn(fromLeft + 1);
                fromLeft--;
                angle = 0;
              }
              break;
            }
            break;
          }
          playNote('E', 10);
          buttonDelay(200);
        }
      }

      // Restart
      if (upState == 1) {
        memset(lc0, 0, sizeof(lc0));
        memset(lc1, 0, sizeof(lc1));
        memset(active, 0, sizeof(active));
        memset(screen, 0, sizeof(screen));
        score = 0;
        allLines = 0;
        figura = 0;
        break;
      }

      for (int colnum = 0; colnum < lcCols; colnum++) {

        if ((screen[colnum] & (active[colnum] >> 1)) == 0) {
          colCheck++;
        }

        else {
          colCheck = 0;
          if (krok == 0) {
            started = 0;
          }
        }
      }

      if ((colCheck == lcCols) && (krok < lcRows)) {
        for (int colnum = 0; colnum < lcCols; colnum++) {
          active[colnum] = active[colnum] >> 1;
          updateColumn(colnum);
        }
      } else {
        break;
      }
      delay(10);
    }

    for (int colnum = 0; colnum < lcCols; colnum++) {
      screen[colnum] = screen[colnum] | (lc0[colnum] << (lcRows / 2));
      screen[colnum] = screen[colnum] | lc1[colnum];
      lc0[colnum] = 0;
      lc1[colnum] = 0;
      active[colnum] = 0;
    }

    currLines = 0;
    for (int rownum = 0; rownum < lcRows; rownum++) {
      colCheck = 0;
      for (int colnum = 0; colnum < lcCols; colnum++) {
        if (((screen[colnum] >> rownum) & 1) == 1) {
          colCheck++;
        }
      }
      if (colCheck == lcCols) {
        // Animacja kasowania
        for (int colnum = 0; colnum < lcCols; colnum++) {
          tmpCol = ~((int)round(pow(2, rownum)));
          screen[colnum] = screen[colnum] & tmpCol;
          updateColumn(colnum);

          switch (currLines) {
          case 0:
            playNote('b', 20);
            break;
          case 1:
            playNote('D', 20);
            break;
          case 2:
            playNote('F', 20);
            break;
          case 3:
            playNote('A', 20);
            break;
          }
          delay(30);

          tmpCol = (int)(round(pow(2, rownum)) - 1);
          tmpCol = screen[colnum] & tmpCol;
          screen[colnum] = (screen[colnum] >> (rownum + 1));
          screen[colnum] = (screen[colnum] << rownum);
          screen[colnum] = screen[colnum] | tmpCol;
        }

        for (int colnum = 0; colnum < lcCols; colnum++) {
          updateColumn(colnum);
        }
        rownum--;
        currLines++;
        allLines++;
      }
    }

    if (currLines > 0) {
      score += (int)round(pow(4, currLines - 1));
    }
  gameOver();
  // == Game Over ==
}
void gameOver() {
  playNote('F', 80);
  playNote('A', 60);
  playNote('F', 80);
  playNote('A', 60);

  int cima[] = {
      B11111111, //     o     o o o
      B11111111, //   o o o    o
      B11111111, //   o   o     o o
      B11111111, //     o     o
      B11111111, //     o     o o o
      B11111111, //
      B11111111, //   o o o   o o o
      B11111111  //     o     o   o
  };

  int baixo[] = {
      B11111111, //     o     o o
      B11111111, //     o     o   o
      B11111111, //     o     o     o
      B11111111, //
      B11111111, //     o       o o o
      B11111111, //     o       o
      B11111111, //     o         o
      B11111111  //     o     o o o
  };

  for (int rownum = 8; rownum >= 0; rownum--) {
    lc.setRow(1, rownum, baixo[rownum]);
    delay(10);
  }

  for (int rownum = 8; rownum >= 0; rownum--) {
    lc.setRow(0, rownum, cima[rownum]);
    delay(10);
  }

}
