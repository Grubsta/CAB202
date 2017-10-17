// ANSI Tower for a Teensy utilising a PewPew board.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <sprite.h>
#include <macros.h>
#include "bitmaps.h"
#include "usb_serial.h"
#include "cab202_adc.h"

// Initialise timer macros.
#define FREQ 8000000.0
#define PRESCALE 256.0
#define TIMER_SCALE 256.0

// Configuration (sprite L & W) macros.
#define HW 16 // Hero.
#define HH 8
#define TH 20 // Tower.
#define TW 80
#define DH 10 // Door.
#define DW 24
#define EH 5 // Enemy.
#define EW 8
#define KH 3 // Key.
#define KW 8
#define VWH 25 // Vetical wall.
#define VWW 3
#define HWH 3 // Horizontal wall.
#define HWW 25
// Threshold for button presses.
#define thresh (1000)


// TODO : ###
// FIX SCREEN movement
// FIX RANDOM generator
// FIX COLLISION Y value
// ADD ARROWS TO bow
// ADD BOMB FUNCTION

// NEED TO KNOWS
// SCREEN = 84x48

// Global variables.
// Location / movement.
float speed = 1.0;
float dx = 0;
float dy = 0;
int dxdy[1];
// Player.
int level = 2, lives = 3, score = 0;
// Timer.
int seconds = 0, minutes = 0;
int totalSeconds = 0, totalMinutes = 0;
double interval = 0;
// Sprite amounts.
uint8_t enemyAm = 0, treasureAm = 0, wallAm = 2;
// Gameplay / Collisions.
uint8_t herox, heroy;
uint8_t keyx, keyy;
uint8_t grid[6][2] = {
  {-8, 1.5}, {-8, 46.5}, // Left (Top | Bottom)
  {42, 1.5}, {42, 46.5}, // Mid (Top | Bottom)
  {92, 1.5}, {92, 46.5}  // Right (Top | Bottom)
};
int screenX = 0, screenY = 0;
bool keyColl = false;
bool activated = false;
bool lvlInit = false;
bool wallInitialised = false;
bool mapInitialised = false;
bool enemyInitialised = false;
// Defense objects.
bool keySpawn = false;
bool bombSpawn = false;
bool shieldSpawn = false;
bool spriteTrailed = false;
bool bowTrailed = false;
bool bombTrailed = false;
bool shieldTrailed = false;
bool keyTrailed = false;
bool crosshairInit = false;
// Game engine.
uint16_t closedCon = 0;
uint16_t openCon = 0;
bool resetGame = false;
// Shooting mechanism values.
bool shot = false;
int hx, hy, cx, cy;
// String output.
char printArray[20];
// Seed generator.
int seed = 2313;
int wallX1 = -33, wallX2 = 117;
int wallY1 = -21, wallY2 = 69;

// Initialise sprites.
Sprite hero; Sprite tower; Sprite door; Sprite key;
Sprite enemy[5]; Sprite treasure[5]; Sprite wall[12];
Sprite shield; Sprite bow; Sprite bomb; Sprite crosshair;
Sprite arrow;

// Initialise hero.
void initHero(void) {
	int x = LCD_X / 2 - HW / 2;
	int y = LCD_Y / 2 + HH + 3;
  herox = x; heroy = y;
	sprite_init(&hero, x, y, HW, HH, heroBitmap);
}


// Causes the sprites to magically dissapear
void spriteMagic(Sprite sprite) {
  sprite.x = 30;
  sprite.y = -1000;
  free(&sprite); // #####
}


// Colisions for static map edges.
void staticMap(void) {
  uint8_t x = round(hero.x); uint8_t y = round(hero.y);
  if (x < 0 || x + HW >= LCD_X - 1) hero.x -= dx;
  if (y < 0 || y + HH >= LCD_Y - 1) hero.y -= dy;
}

// Sends inputted string via serial connection.
void send_str(const char *string) {
	char c;
	while (1) {
		c = pgm_read_byte(string++);
		if (!c) break;
		usb_serial_putchar(c);
	}
}


// Sends inputted string via serial connection.
void usb_serial_send(char * message) {
	usb_serial_write((uint8_t *) message, strlen(message));
}

// Collision detection between 2 sprites. Added gap incase a extended
// perimeter is necessary.
bool gapCollision(Sprite sprite1, Sprite sprite2, int gap) {
  // Sprite 1.
  uint8_t spr1Bottom = round(sprite1.x) + sprite1.height;
  uint8_t spr1Top = round(sprite1.y); /////##################
  uint8_t spr1Left = round(sprite1.x) - gap;
  uint8_t spr1Right = round(sprite1.x) + sprite1.width + gap;
  // Sprite 2.
  uint8_t spr2Bottom = round(sprite2.y) + sprite2.height;
  uint8_t spr2Top = round(sprite2.y);
  uint8_t spr2Left = round(sprite2.x) - gap;
  uint8_t spr2Right = round(sprite2.x) + sprite2.width + gap;
  // Creates a perimeter arround sprites and checks for collision.
	if (spr1Bottom > spr2Top && spr1Top < spr2Bottom && spr1Right > spr2Left&& spr1Left < spr2Right) {
		return true;
	}
	else {
		return false;
	}
}


// Explosion caused by bomb on impact.
void explosion(void) {
  // Enemy collisions.
  uint8_t bomX = bomb.x; uint8_t bomY = bomb.y;
  sprite_init(&bomb, bomX, bomY, 7, 7, bombBitmap);
  sprite_draw(&bomb);
    for (int i = 0; i < 6; i++) {
      if (gapCollision(bomb, enemy[i], 3)) {
        spriteMagic(enemy[i]);
        score += 10;
      }
    }
    // Destroy bomb.
    spriteMagic(bomb);
    // Flicker LEDs
    SET_BIT(PORTB, 2); _delay_ms(250); CLEAR_BIT(PORTB, 2);
    SET_BIT(PORTB, 3); _delay_ms(250); CLEAR_BIT(PORTB, 3);
    SET_BIT(PORTB, 2); _delay_ms(250); CLEAR_BIT(PORTB, 2);
    SET_BIT(PORTB, 3); _delay_ms(250); CLEAR_BIT(PORTB, 3);
}


// Checking for all sprite collisions.
bool spriteCollision(Sprite sprite) {
  for (int i = 0; i < enemyAm; i++) {
    if (gapCollision(sprite, enemy[i], 1)) {send_str(PSTR("FAILED\r\n")); return true;};
  }
  send_str(PSTR("passed enemy\r\n"));
  for (int i = 0; i < treasureAm; i++) {
    if (gapCollision(sprite, treasure[i], 1)) {send_str(PSTR("FAILED\r\n")); return true;}
  }
  send_str(PSTR("passed treasure\r\n"));
  for (int i = 0; i < wallAm; i++) {
    if (gapCollision(sprite, wall[i], 1)) {send_str(PSTR("FAILED\r\n")); return true;}
  }
  send_str(PSTR("passed wall\r\n"));
  if (gapCollision(sprite, bomb, 1)) {send_str(PSTR("FAILED\r\n")); return true;}
  send_str(PSTR("passed bomb\r\n"));
  if (gapCollision(sprite, bow, 1)) {send_str(PSTR("FAILED\r\n")); return true;};
  send_str(PSTR("passed bow\r\n"));
  if (gapCollision(sprite, shield, 1)) {send_str(PSTR("FAILED\r\n")); return true;}
  send_str(PSTR("passed shield\r\n"));
  if (gapCollision(sprite, key, 1)) {send_str(PSTR("FAILED\r\n")); return true;}
  send_str(PSTR("passed key\r\n"));
  if (gapCollision(sprite, door, 1)) {send_str(PSTR("FAILED\r\n")); return true;}
  send_str(PSTR("passed door\r\n"));
  send_str(PSTR("PASSED COLLISIONS\r\n"));
  return false;

}


// X-value collision detection.
bool xCollision(Sprite sprite1, Sprite sprite2){
  // Sprite 1.
  uint8_t sprite1Left = sprite1.x;
  uint8_t sprite1Right = sprite1.x + sprite1.width;
  // Sprite 2.
  uint8_t sprite2Left = sprite2.x;
  uint8_t sprite2Right = sprite2.x + sprite2.width;
  // If collision is occuring.
  // sprintf(printArray, "S1 L = %d, R = %d, S2 L = %d, R = %d r\n", sprite1Left, sprite1Right, sprite2Left, sprite2Right);
  // usb_serial_send(printArray); //#####
  if (sprite1Left < sprite2Right && sprite1Right > sprite2Left) return true;
  else return false; // Else return no collision.
}


// Y-value collision detection.
bool yCollision(Sprite sprite1, Sprite sprite2){
  // Sprite 1.
  uint8_t sprite1Bottom = round(sprite1.y) + sprite1.height;
  uint8_t sprite1Top = round(sprite1.y);
  // Sprite 2.
  uint8_t sprite2Bottom = round(sprite2.y) + sprite2.height;
  uint8_t sprite2Top = round(sprite2.y);
  // if (sprite1Bottom > sprite2Top && sprite1Top < sprite2Bottom - 2) return true;
  if (sprite1Bottom > sprite2Top && sprite1Top < sprite2Bottom) return true;
  else return false;
}


// Moves enemy sprite towards hero's location.
void enemyMovement(void) {
	float enemySpeed = 0.1;
  if (level == 1) enemyAm = 1;
	for (int i = 0; i < enemyAm; i++) {
    for (int a = 0; a < 6; a++) {
      if (xCollision(enemy[i], wall[a]) && yCollision(enemy[i], wall[a])) {
        enemySpeed = -0.2;
      }
    }
    if (level == 1) {
      if (xCollision(enemy[i], tower) && yCollision(enemy[i], tower)) {
        enemySpeed = -0.2;
      }
    }
    if (xCollision(enemy[i], door) && yCollision(enemy[i], door)) {
      enemySpeed = -0.2;
    }
    if ((enemy[i].x > LCD_X - 84 || enemy[i].x > LCD_X) && (enemy[i].y > LCD_Y - 44 || enemy[i].y < LCD_Y)){
      if (enemy[i].x < hero.x) enemy[i].x += enemySpeed;
      else if (enemy[i].x > hero.x) enemy[i].x -= enemySpeed;
      if (enemy[i].y < hero.y) enemy[i].y += enemySpeed;
      else if (enemy[i].y > hero.y) enemy[i].y -= enemySpeed;
    }
		sprite_draw(&enemy[i]);
	}
}


// Shooting mechanism.
void sendIt(void) {
  int sx = 0; int sy = 0;
  int shotSpeed = 1;
  // Change values whilst no user input.
  if (!shot) {
    hx = hero.x; hy = hero.y;
    cx = crosshair.x; cy = crosshair.y;
  }
  if (!crosshairInit) sprite_init(&crosshair, LCD_X * 0.5, LCD_Y * 0.5, 3, 3, crosshairBitmap);
  // User input.
  if ((BIT_IS_SET(PINF, 6) || 	BIT_IS_SET(PINF, 5)) && shot == false) {
    shot = true;
    // Checks in which direction the crosshair is from player.
    if (cx < hx) sx = hx - 5 - 2; // 2 being the shot's width.
    else if (cx > hx) sx = hx + HW + 5;
    if (cy < hy) sy = hy - 5;
    else if (cy > hy) sy = hy + HW + 5;
    // Determines what projectile is currently equipt.
    if (bowTrailed) {
      sprite_init(&arrow, sx, sy, 3, 3, arrowBitmap);
      send_str(PSTR("The arrow has left the building.\n"));
    }
    else {
      send_str(PSTR("Bombs away.\n"));
    }
  }
  // ran if and while the projectile is functional.
  if (shot) {
    bool xHit = false;
    bool yHit = false;
    if (bowTrailed) {
      sx = arrow.x;
      sy = arrow.y;
    }
    else {
      sx = bomb.x;
      sy = bomb.y;
    }
    if (cx < sx) sx -= shotSpeed;
    else if (cx > sx) sx += shotSpeed;
    else xHit = true;
    if (cy < sy) sy -= shotSpeed;
    else if (cy > sy) sy += shotSpeed;
    else yHit = true;
    arrow.x = sx; arrow.y = sy;
    // The projectile has hit the target.
    if (xHit && yHit) {
      if (bowTrailed) {
        for (int i = 0; i < 6; i++) { // 6 being enemyAm - 1
          if (gapCollision(arrow, enemy[i], 1)) {
            spriteMagic(enemy[i]);
            score += 10;
            send_str(PSTR("An enemy has been shot till death by the hero.\n"));
          }
        }
        spriteMagic(arrow);
      }
      if (bombTrailed) {
        for (int i = 0; i < 6; i++) {
          if (gapCollision(bomb, enemy[i], 3)) {
            spriteMagic(enemy[i]);
            score += 10;
            send_str(PSTR("An enemy has died in a horrific explosion.\n"));
          }
        }
        spriteMagic(bomb);
      }
      send_str(PSTR("The projectile has been destoyed.\r\n"));
      shot = false;
    }
    send_str(PSTR("ass.\n"));
    sprite_draw(&arrow);
  }
}


// Moves crosshair dependent on petentiometer input.
void crosshairMovement(void) {
  int left_adc = adc_read(0);
  int right_adc = adc_read(1);
  crosshair.x = (double) left_adc * (LCD_X - crosshair.width) / 1024;
  crosshair.y = (double) right_adc * (LCD_Y - crosshair.height) / 1024;
  sprite_draw(&crosshair);
  sendIt();
}


// Initialises all sprites on first level.
void level1Init(void) {
	int midX = LCD_X / 2;
	initHero();
  sprite_init(&wall[0], 2, 0 - VWH, VWW, VWH, vertWallBitmap);
  sprite_init(&wall[1], 80, 0 - VWH, VWW, VWH, vertWallBitmap);
  sprite_init(&tower, 2, 0, TW, TH, towerBitmap);
	sprite_init(&enemy[0], LCD_X * 0.85, LCD_Y * 0.50, EW, EH, enemyBitmap);
	sprite_init(&key, LCD_X * 0.15 - KW, LCD_Y * 0.50, KW, KH, keyBitmap);
 	sprite_init(&door, midX - DW / 2, TH - DH, DW, DH, doorBitmap);
	lvlInit = true;
}


// Moves all sprites dependent on xy values.
void moveAll(int x, int y) {
	if (level == 1) {
		tower.x += x; tower.y += y;
    wall[0].x += x; wall[0].x += y;
    wall[1].x += x; wall[1].x += y;
	}
	else {
		for(int i = 0; i < wallAm; i++) {
			wall[i].x += x; wall[i].y += y;
		}
	}
	for(int i = 0; i < treasureAm; i++) {
		treasure[i].x += x; treasure[i].y += y;
	}
	for(int i = 0; i < enemyAm; i++) {
		enemy[i].x += x; enemy[i].y += y;
	}
	key.x += x; key.y += y;
	door.x += x; door.y += y;
  shield.x += x; shield.y += y;
  bomb.x += x; bomb.y += y;
  bow.x += x; bow.y += y;
	hero.x += x; hero.y += y;
}


// Scrolling map feature.
void scrollMap(void) {
  int x = 0;
	int y = 0;
  if (screenX > 33) screenX = 33;
  else if (screenX < -33) screenX = -33;
  if (screenY > 21) screenY = 21;
  else if (screenY < -21) screenY = -21;
  // bool l = false, r = false, u = false, d = false;
	if (hero.x < round(LCD_X * 0.20)) x += 1;
	if (hero.x + HW > round(LCD_X * 0.80)) x -= 1;
	if (hero.y < round(LCD_Y * 0.20)) y += 1;
	if (hero.y + HH > round(LCD_Y * 0.80)) y -= 1;
  screenX += x; screenY += y;
  if (screenX <= -33 || screenX >= 33) x = 0; // #### DEBATABLY WORKING
  if (screenY <= -21 || screenY >= 21) y = 0;
	moveAll(x, y);
}


// Produces random x value between game size.
int randX(void) {
  seed += 1;
  srand(interval * seed * seconds / minutes);
  int x = rand() % (wallX2 + (wallX1 + 8 * 1));
  return x;
}


// Produces random y value between game size.
int randY(void) {
  seed += 1;
  srand(interval * seed / seconds + minutes);
  int y = rand() % (wallY2 + (wallY1 * 1));
  return y;
}


// Shuffles an entered array.
void shuffle(int *array, size_t n) {
  if (n > 1) {
    size_t i;
    for (i = 0; i < n - 1; i++) {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

// Psuedo random wall generation.
void wallInit(void) {
  // Initialising random wall formations.
  wallAm = 6;
  // uint8_t positions[6] = {0, 1, 2, 3, 4, 5}; // Grid position
  // float WG = 0.5; // Gap between walls.
  uint8_t hw = 10; // Half wall size.
  // uint8_t verGap = VWH * WG;
  // uint8_t horGap = HWW * WG;
  // int verMid = VWH * 0.5; int horMid = HWW * 0.5;
  // for (uint8_t i = 0; i < 6; i++){
  uint8_t gridX = grid[2][0]; uint8_t gridY = grid[2][1];
  uint8_t chained[13][4] = { // x1, y1, x1, y1 (Vertical, Horizontal).
    {gridX, gridY, gridX, gridY + hw}, // T shape (90 D counter clockwise).
    {gridX - hw, gridY, gridX - hw, gridY}, // L shape (flipped).
    {gridX - hw, gridY, gridX - hw + 3, gridY + 22}, // L shape.
    {gridX, gridY, gridX - 11, gridY}, // T shape.
    {gridX + hw + 2, gridY, gridX - hw, gridY}, // L shape (upside down).
    {gridX, gridY, gridX, gridY + hw},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
  };
  uint8_t i = 5;
  sprintf(printArray, "VWH: %d\r\n", gridY);
  usb_serial_send(printArray);
  uint8_t x1 = chained[i][0], y1 = chained[i][1];
  uint8_t x2 = chained[i][2], y2 = chained[i][3];
  sprite_init(&wall[0], x1, y1, VWW, VWH, vertWallBitmap);
  sprite_init(&wall[1], x2, y2, HWW, HWH, horWallBitmap);

  int x = wall[0].x; int y = wall[0].y;
  sprintf(printArray, "X: %d Y: %d~~~~~~~~~~~~~~~~\r\n", x, y);
  usb_serial_send(printArray);
}


// Initialises defence items.
void defenceInit(void) {
  bool valid = false;
  int gen, x, y;
  // Randomising both the position and chance of spawn.
  gen = rand() % 100;
  gen = 29;
  if (gen <= 29) {
    while (!valid) {
      x = randX(); y = randY();
      // x = LCD_X * 0.8; y = LCD_Y * 0.3; // ####
      sprite_init(&bow, x, y, 8, 3, bowBitmap);
      if (!spriteCollision(bow)) valid = true;
      valid = true;
    }
  }
  valid = false;
  gen = rand() % 100;
  gen = 29;
  if (gen <= 29) {
    while (!valid) {
      x = randX(); y = randY();
      // x = LCD_X * 0.8; y = LCD_Y * 0.6; // ####
      sprite_init(&bomb, x, y, 6, 4, bombBitmap);
      if (!spriteCollision(bomb)) valid = true;
      valid = true;
    }
  }
  valid = false;
  gen = rand() % 100;
  if (gen <= 29) {
    while (!valid) {
      sprite_init(&shield, x, y, 8, 4, shieldBitmap);
      if (!spriteCollision(shield)) valid = true;
    }
  }
}


// Initialises all the enemy sprites.
void enemyInit(void) { ///### inits broken, fix later
  int x, y;
  // srand(minutes * seed - seconds);
  enemyAm = rand() % 6;
  // enemyAm = 2;
  for (int i = 0; i < enemyAm; i++) {
    bool valid = false;
    sprintf(printArray, "ENEMY no. = %d\r\n", i);
    usb_serial_send(printArray);
    while (!valid) {
      x = randX(); y = randY();
      // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      // x += wallX1; y += wallY1;
      sprite_init(&enemy[i], x, y, EW, EH, enemyBitmap);
      if (!spriteCollision(enemy[i])) valid = true;
      send_str(PSTR("FAILED\r\n"));
    }
  }
}


// Initialises all the treasure sprites.
void treasureInit(void) {
  int x, y;
  // srand(minutes * seed - seconds);
  treasureAm = rand() % 6;
  for (int i = 0; i < treasureAm; i++) {
    bool valid = false;
    while (!valid) {
      sprintf(printArray, "TREASURE no. = %d\r\n", i);
      usb_serial_send(printArray);
      x = randX(); y = randY();
      // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      // x += wallX1; y += wallY1;
      sprite_init(&treasure[i], x, y, 8, 3, treasureBitmap);
      if (!spriteCollision(treasure[i])) valid = true;
      send_str(PSTR("FAILED\r\n"));
    }
  }
}


// Initialises door sprite.
void doorInit(void) {
  int x, y;
  bool valid = false;
  while (!valid) {
    x = randX(); y = randY();
      // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      // x += wallX1; y += wallY1;
    sprite_init(&door, x, y, DW, DH, doorBitmap);
    if (!spriteCollision(door)) break;
    send_str(PSTR("FAILED\r\n"));
  }
}


// Initialises key sprite.
void keyInit(void) {
  int x, y;
  // srand(minutes * seed - seconds);
  bool valid = false;
  while (!valid) {
    x = randX(); y = randY();
    // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
    // x += wallX1; y += wallY1;
    sprite_init(&key, x, y, KW, KH, keyBitmap);
    if (!spriteCollision(key)) break;
    send_str(PSTR("FAILED\r\n"));
  }
}


// Initialises all the sprites on the map.
void mapInit(void) {
  // send_str(PSTR("WALL\r\n"));
  wallInit();
  // send_str(PSTR("DOOR\r\n"));
  // doorInit();
  // send_str(PSTR("KEY\r\n"));
  // keyInit();
  // send_str(PSTR("TREASURE\r\n"));
  // treasureInit();
  // send_str(PSTR("ENEMY\r\n"));
  // enemyInit();
  // send_str(PSTR("HERO\r\n"));
	initHero();
  // send_str(PSTR("DEFENCE\r\n"));
  // defenceInit(); #### broken
	mapInitialised = true;
}


// Initialises level skeleton and draws it.
void drawLvl(void) {
	// Static level 1 sprites.
  if (level == 1) {
		if (!lvlInit) level1Init();
    sprite_draw(&tower); sprite_draw(&door); sprite_draw(&key);
    sprite_draw(&wall[0]); sprite_draw(&wall[1]);
		enemyMovement();
  }
	// Randomly generated level sprites.
  else {
		if (!mapInitialised) mapInit();
    enemyMovement();
    // int X = wall
    // sprintf(printArray, "VWH: %d %d\r\n", verGap);
    // usb_serial_send(printArray);
    // sprite_draw(&wall[0]); sprite_draw(&wall[1]);
		for (int i = 0; i < 6; i++) sprite_draw(&wall[i]); // ###
		for (int i = 0; i < treasureAm; i++) sprite_draw(&treasure[i]);
		sprite_draw(&door); sprite_draw(&key);
    if (!shieldTrailed) sprite_draw(&shield);
    if (!bowTrailed) sprite_draw(&bow);
    if (!bombTrailed) sprite_draw(&bomb);
  }
}


// Enables sprites to trail sprites.
void spriteTrail(Sprite sprite1) {
  int x, y;
  if (sprite1.bitmap == keyBitmap) {
     x = hero.x + HW + 2;  y = hero.y + HH + 3;
  }
  else {
     x = hero.x - sprite1.width - 3;  y = hero.y + HH + 3;
  }
  sprite1.x = x; sprite1.y = y;
  sprite_draw(&sprite1);
}


// Destroys entire level. ###
void destroyGame(void) {
	if (level == 1) {
		spriteMagic(tower); spriteMagic(enemy[0]);
	}
	else {
		for (int i = 0; i < wallAm; i++) spriteMagic(wall[i]);
	}
	for (int i = 0; i < enemyAm; i++) spriteMagic(enemy[i]);
  for (int i = 0; i < treasureAm; i++) spriteMagic(treasure[i]);
	spriteMagic(hero); spriteMagic(key); spriteMagic(door);
  spriteMagic(bow); spriteMagic(bomb); spriteMagic(shield);
	mapInitialised = false; lvlInit = false; keyColl = false;
  keySpawn = false; bombSpawn = false; shieldSpawn = false;
  spriteTrailed = false; bowTrailed = false; bombTrailed = false;
  shieldTrailed = false; keyTrailed = false;
}


// Loading screen between levels.
void loadingScreen(void) {
	clear_screen();
	char lev[50];char scor[50];
	sprintf(lev, "Next level %d", level); draw_string(0, 20, lev, FG_COLOUR);
	sprintf(scor, "Score: %d", score); draw_string(0, 40, scor, FG_COLOUR);
	show_screen();
	_delay_ms(2000);
}


// Display Menu during gameplay.
void displayMenu(void) {
	clear_screen();
	char lev[20]; char liv[20]; char scor[20]; char timer[20];
	sprintf(lev, "Level: %d", level); draw_string(0, 0, lev, FG_COLOUR);
	sprintf(liv, "Lives: %d", lives); draw_string(0, 10, liv, FG_COLOUR);
	sprintf(scor, "Score: %d", score); draw_string(0, 20, scor, FG_COLOUR);
	sprintf(timer, "Time: %02d:%02d", minutes, seconds); draw_string(0, 30, timer, FG_COLOUR);
	show_screen();
}


// User input from PewPew switches.
void userControlls(void) {
	if (BIT_IS_SET(PIND, 1)){ // Up switch.
		dy = -speed;
	}
	else if (BIT_IS_SET(PINB, 7)){ // Down switch.
		dy = speed;
	}
	else if (BIT_IS_SET(PINB, 1)){ // Left switch.
		dx = -speed;
	}
	else if (BIT_IS_SET(PIND, 0)){ // Right switch.
		dx = speed;
	}
	if (BIT_IS_SET(PINB, 0)){ // Centre switch.
		// Acts as a debouncer whilst user hold switch down.
    while (BIT_IS_SET(PINB, 0)) {
      closedCon++;
    	openCon = 0;
    	if (closedCon > thresh) {
    		if (!activated) {
    			closedCon = 0;
    		}
    		activated = true;
    	}
    	else {
    		openCon++;
    		closedCon = 0;
    		if (openCon > thresh) {
    			if (activated) {
    				openCon = 0;
    			}
    			activated = false;
    		}
    		displayMenu();
    	}
    }
  }
	dxdy[0] = dx; dxdy[1] = dy;
}


// Hero movement.
void moveHero(void) {
	// Useful variables.
	float xx = dx; float yy = dy;
	dx = 0; dy = 0;
	dxdy[0] = 0; dxdy[1] = 0;
	bool enColl = false;
	// User Input.
	userControlls();
	dx = dxdy[0];
	dy = dxdy[1];
	// Array sprite Collisions.
	for (int i = 0; i < enemyAm; i++) {
		// if (gapCollision(hero, enemy[i], 0)) {
		// 	enColl = true;
		// }
    if (xCollision(hero, enemy[i])) {
      send_str(PSTR("DA X.\r\n"));
      if (yCollision(hero, enemy[i])) {
        enColl = true;
        send_str(PSTR("DA FUCKN COLLISION YA HERE.\r\n"));
      }
    }
	}
  for (int i = 0; i < 6; i++) {
    if (gapCollision(hero, treasure[i], 1)) {
      spriteMagic(treasure[i]);
      score += 10;
    }
  }
  if (level > 1) {
    if (gapCollision(hero, bomb, 1)) {
      if (!bombTrailed) {
        bombTrailed = true; shieldTrailed = false; bowTrailed = false;
        send_str(PSTR("The hero has located da bomb.\r\n")); //####
      }
      spriteTrail(bomb);
    }
    if (gapCollision(hero, bow, 1)) {
      if (!bowTrailed) {
        bowTrailed = true; bombTrailed = false; shieldTrailed = false;
        send_str(PSTR("The hero has collected the bow.\r\n"));
      }
      spriteTrail(bow);
    }

    if (gapCollision(hero, shield, 1)) {
      if (!shieldTrailed) {
        shieldTrailed = true; bombTrailed = false; bowTrailed = false;
        send_str(PSTR("The hero has picked up a shield. +1 protection.\r\n"));
      }
      spriteTrail(shield);
    }
  }
	if (gapCollision(hero, key, 1)) {
    send_str(PSTR("The hero has retrieved the key.\r\n"));
		keyColl = true;
    spriteTrailed = true;
		key.x = 150;
		key.y = 150;
	}
	if (level == 1) {
		if (gapCollision(hero, tower, 0)) {
			hero.y -= yy;
			hero.x -= xx;
			dx = 0;
			dy = 0;
		}
	}
	// Checking for collisions.
	if (enColl) {
    if (shieldTrailed){
      hero.y -= yy * 3;
      hero.x -= xx * 3;
      shieldTrailed = false;
      send_str(PSTR("An enemy has destroyed the heros shield. -1 protection."));
      shield.x = 1000;
      shield.y = 1000;
    }
    else {
      lives -= 1;
      send_str(PSTR("An enemy has killed the hero.\r\n"));
      initHero(); // ###
      screenX = 0; screenY = 0;
      // mapInitialised = false;
      // destroyGame(); // ### FIX AFTER DEBUGGA
    }
	}
	// else if (gapCollision(hero, door, 0)) {
  else if (xCollision(hero, door) && yCollision(hero, door)) {
		if (keyColl) {
			level += 1;
			score += 100;
			destroyGame();
			loadingScreen();
		}
		else {
      send_str(PSTR("This seems to require a key.\r\n"));
			hero.y -= yy;
			hero.x -= xx;
		}
	}
	// If no collisions occur, move hero.
	else {
		hero.y += dy;
		hero.x += dx;
	}
  if (keyColl) spriteTrail(key);

	scrollMap();
	staticMap();
  sprite_draw(&hero);
}


// Reset required global variables to restart the game.
void resetVars(void) {
  level = 1, lives = 3, score = 0;
  seconds = 0, minutes = 0;
  screenX = 0, screenY = 0;
  keyColl = false;
  activated = false;
  lvlInit = false;
  wallInitialised = false;
  mapInitialised = false;
  enemyInitialised = false;
  keySpawn = false;
  bombSpawn = false;
  shieldSpawn = false;
  spriteTrailed = false;
  bowTrailed = false;
  bombTrailed = false;
  shieldTrailed = false;
  keyTrailed = false;
  crosshairInit = false;
  resetGame = true;
}


// Welcome Screen.
void welcomeScreen(void) {
  clear_screen();
  draw_string(LCD_X / 2 - 25, LCD_Y / 2 - 6, "Corey Hull", FG_COLOUR);
  draw_string(LCD_X / 2 - 23, LCD_Y / 2 + 6, "N10007164", FG_COLOUR);
  show_screen();
  _delay_ms(2000);
	char* countDwn[3] = {"3", "2", "1"};
	bool start = false;
	do {
		if (BIT_IS_SET(PINF, 6) || 	BIT_IS_SET(PINF, 5)) start = true;
	} while (!start);
	for (int i = 0; i <= 2; i++) {
		clear_screen();
		draw_string(LCD_X / 2 - (9 / 2), LCD_Y / 2, countDwn[i], FG_COLOUR);
		show_screen();
		_delay_ms(333); // ### CHANGE to 3Hz frequency
	}
}


// Serial output for common game stats.
void serialOutput(void) {
  int x = round(hero.x); int y = round(hero.y);
  int x2 = x + hero.width; int y2 = y + hero.height;
  int min = totalMinutes + minutes;
  int sec = totalSeconds + seconds;
  char printString[40];
  sprintf(printString, "run-time(m:s): %02d:%02d\r\n", min, sec);
  usb_serial_send(printString);
  sprintf(printString, "Score: %d\r\n", score);
  usb_serial_send(printString);
  sprintf(printString, "Level: %d\r\n", level);
  usb_serial_send(printString);
  sprintf(printString, "(X1,X2|Y1,Y2) Location: %d,%d|%d,%d\r\n", x, x2, y, y2);
  usb_serial_send(printString);
  sprintf(printString, "Remaining lives: %d\r\n\r\n", lives);
  usb_serial_send(printString);
}


// 8-bit Overflow timer.
ISR(TIMER0_OVF_vect) {
	interval += TIMER_SCALE * PRESCALE / FREQ;
	if ( interval >= 1.0 ) {
		interval = 0;
    seconds++;
    serialOutput();
	}
  if (interval > 0.495 && interval < 0.505) {
    serialOutput();
  }
  if (seconds < 59) {
    seconds++;
  }
  else {
    seconds = 0;
    minutes++;
  }
}


// Enables input from PewPew switches.
void initControls(void) {
  // D-pad Controlls.
  CLEAR_BIT(DDRB, 0); // Centre.
  CLEAR_BIT(DDRB, 1); // Left.
  CLEAR_BIT(DDRB, 7); // Down.
  CLEAR_BIT(DDRD, 0); // Right.
  CLEAR_BIT(DDRD, 1); // Up.
	// SW2 & SW3 Controlls.
	CLEAR_BIT(DDRF, 6); // Left.
	CLEAR_BIT(DDRF, 5); // Right.
  // LEDs.
  SET_BIT(DDRB, 2);
  SET_BIT(DDRB, 3);
}

// Setup (ran on start).
void setup(void) {
  // srand(45387 * seconds * minutes);
  // Set CPU speed.
  set_clock_speed(CPU_8MHz);
  // Set Timer 0 to overflow approx 122 times per second.
  TCCR0B |= 4;
  TIMSK0 = 1;
  // Enable interrupts.
  sei();
  // Initialise different connected devices.
  if (!resetGame){
    usb_init();
    adc_init();
    lcd_init(LCD_DEFAULT_CONTRAST);
    // timer();
    DDRD |= (1<<6);
    PORTD |= (1<<6);
    // Display menu.
    clear_screen();
    draw_string(0, 10, "Connect to a ", FG_COLOUR);
    draw_string(0, 20, "serial terminal", FG_COLOUR);
    draw_string(0, 30, "to continue", FG_COLOUR);
    show_screen();
    // Wait for usb Configuration.
    while(!usb_configured());
    while(!(usb_serial_get_control() & USB_SERIAL_DTR)) usb_serial_flush_input();
    // Turn light off.
    PORTD &= ~(1<<6);
    send_str(PSTR("Welcome to ANSI\r\n"));
    initControls();
  }
	// welcomeScreen();
  clear_screen();
  drawLvl();
  // sprite_draw(&hero);
  show_screen();
}


// Game over menu.
void gameOverScreen(void) {
	clear_screen();
	char lev[50]; char scor[50];
	draw_string(0, 0, "You died in ANZI!", FG_COLOUR);
	sprintf(lev, "level: %d", level); draw_string(0, 10, lev, FG_COLOUR);
	sprintf(scor, "score: %d", score); draw_string(0, 20, scor, FG_COLOUR);
	draw_string(0, 40, "SW2/3 to restart", FG_COLOUR);
	show_screen(); //### Add button press to restart gameplay.
  bool start = false;
  do {
    if (BIT_IS_SET(PINF, 6) || 	BIT_IS_SET(PINF, 5)) start = true;
  } while (!start);
  totalMinutes += minutes; totalSeconds += seconds;
  resetVars();
  setup();
}


// Process (ran every frame).
void process(void) {
	if (lives > 0) {
		clear_screen();
		drawLvl();
    if (bombTrailed || bowTrailed) crosshairMovement();
		moveHero();
		show_screen();
	}
	else {
    send_str(PSTR("Game ova red nova.\n"));
		gameOverScreen();
	}
}


// Main loop.
int main(void) {
  setup();
  for ( ;; ) {
    process();
    _delay_ms(10);
	}
}
