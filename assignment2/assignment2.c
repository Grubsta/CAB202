// ANSI Tower for a Teensy utilising a PewPew board. This
// board utilises a 84 x 48 LCD screen. Model: Nokia5110 LCD.
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

// Global variables.
// Location / movement.
float speed = 1.0;
float dx = 0;
float dy = 0;
int dxdy[1];
// Player.
uint16_t level = 2, lives = 3;
int score = 0;
// Timer.
int seconds = 0, minutes = 0;
uint16_t totalSeconds = 0, totalMinutes = 0;
double interval = 0;
// Sprite amounts.
int enemyAm = 0, treasureAm = 0, wallAm = 8;
// Map grid.
int grid[6][2] = { // 1.5, 46.5 Y
  {-8, -7}, {-8, 31}, // Left (Top | Bottom)
  {42, -6}, {42, 32}, // Mid (Top | Bottom)
  {83, -4}, {83, 33}  // Right (Top | Bottom)
};
int position[5] = {1, 2, 3, 4, 5}; // Grid position
// Camera position.
int screenX = 0, screenY = 0;
// Gameplay bools.
bool keyColl = false;
bool activated = false;
bool lvlInit = false;
bool wallInitialised = false;
bool mapInitialised = false;
bool enemyInitialised = false;
// Defense objects.
int arrows = 5;
bool bowTrailed = false;
bool bombTrailed = false;
bool shieldTrailed = false;
bool crosshairInit = false;
// Button press objects.
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
// Map border locations.
int wallX1 = -33, wallX2 = 117;
int wallY1 = -21, wallY2 = 69;

// Initialise sprites.
Sprite hero; Sprite tower; Sprite door; Sprite key;
Sprite enemy[5]; Sprite treasure[5]; Sprite wall[12];
Sprite shield; Sprite bow; Sprite bomb; Sprite crosshair;
Sprite arrow;

// Initialise hero.
void initHero(void) {
	uint8_t x = LCD_X / 2 - HW / 2;
	uint8_t y = LCD_Y / 2;
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
  int spr1Bottom = round(sprite1.x) + sprite1.height;
  uint8_t spr1Top = round(sprite1.y);
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


// X-value collision detection.
bool xCollision(Sprite sprite1, Sprite sprite2){
  // Sprite 1.
  uint8_t sprite1Left = sprite1.x;
  uint8_t sprite1Right = sprite1.x + sprite1.width;
  // Sprite 2.
  uint8_t sprite2Left = sprite2.x;
  uint8_t sprite2Right = sprite2.x + sprite2.width;
  if (sprite1Left < sprite2Right && sprite1Right > sprite2Left) return true;
  else return false; // No collision.
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


// Checking for all sprite collisions.
bool spriteCollision(Sprite sprite) {
  for (int i = 0; i < enemyAm; i++) {
    if (xCollision(sprite, enemy[i]) && yCollision(sprite, enemy[i])) return true;
  }
  for (int i = 0; i < treasureAm; i++) {
    if (xCollision(sprite, treasure[i]) && yCollision(sprite, treasure[i])) return true;
  }
  for (int i = 0; i < wallAm; i++) {
    if (xCollision(sprite, wall[i]) && yCollision(sprite, wall[i])) return true;
  }
  if (xCollision(sprite, bomb) && yCollision(sprite, bomb)) return true;
  if (xCollision(sprite, bow) && yCollision(sprite, bow)) return true;
  if (xCollision(sprite, shield) && yCollision(sprite, shield)) return true;
  if (xCollision(sprite, key) && yCollision(sprite, key)) return true;
  if (xCollision(sprite,door) && yCollision(sprite, door)) return true;
  return false;
}


// Moves enemy sprite towards hero's location.
void enemyMovement(void) {
	float enemySpeed = 0.1;
  if (level == 1) enemyAm = 1;
	for (int i = 0; i < enemyAm; i++) {
    for (int a = 0; a < wallAm; a++) {
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
    if ((enemy[i].x > LCD_X - 84 || enemy[i].x > LCD_X) && (enemy[i].y > LCD_Y - 44 || enemy[i].y < LCD_Y)
    && (enemy[i].x > -33 && enemy[i].x < 117 && enemy[i].y > -21 && enemy[i].y < 69)){
      if (enemy[i].x < hero.x) enemy[i].x += enemySpeed;
      else if (enemy[i].x > hero.x) enemy[i].x -= enemySpeed;
      if (enemy[i].y < hero.y) enemy[i].y += enemySpeed;
      else if (enemy[i].y > hero.y) enemy[i].y -= enemySpeed;
    }
		sprite_draw(&enemy[i]);
	}
}


// Explosion caused by bomb on impact.
void explosion(void) {
  // Enemy collisions.
  uint8_t bomX = bomb.x; uint8_t bomY = bomb.y;
  sprite_init(&bomb, bomX, bomY, 7, 7, bombBitmap);
  sprite_draw(&bomb);
  _delay_ms(2000);
  for (int i = 0; i < 6; i++) {
    if (gapCollision(bomb, enemy[i], 3)) {
      spriteMagic(enemy[i]);
      enemy[i].x = -300; enemy[i].y = -300;
      score += 10;
      send_str(PSTR("An enemy has died in a horrific explosion.\n"));
      usb_serial_send(printArray);
    }
  }
  // Destroy bomb.
  spriteMagic(bomb);
  bombTrailed = false;
  // Flicker LEDs
  SET_BIT(PORTB, 2); _delay_ms(250); CLEAR_BIT(PORTB, 2);
  SET_BIT(PORTB, 3); _delay_ms(250); CLEAR_BIT(PORTB, 3);
  SET_BIT(PORTB, 2); _delay_ms(250); CLEAR_BIT(PORTB, 2);
  SET_BIT(PORTB, 3); _delay_ms(250); CLEAR_BIT(PORTB, 3);
}


// Shooting mechanism.
void sendIt(void) {
  int sx = 0; int sy = 0;
  uint8_t shotSpeed = 1;
  // Utilised so if a projectile is shot, it won't trace the moving crosshair.
  if (!shot) {
    hx = hero.x; hy = hero.y;
    cx = crosshair.x + 4; cy = crosshair.y + 4;
  }
  if (!crosshairInit) sprite_init(&crosshair, LCD_X * 0.5, LCD_Y * 0.5, 8, 8, crosshairBitmap);
  // User input.
  if ((BIT_IS_SET(PINF, 6) || BIT_IS_SET(PINF, 5)) && shot == false && (bombTrailed || arrows > 0) &&
  !(xCollision(hero, crosshair) && yCollision(hero, crosshair))) {
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
    bool xHit = false; bool yHit = false;
    if (bowTrailed) {
      sx = arrow.x;
      sy = arrow.y;
    }
    else {
      sx = bomb.x;
      sy = bomb.y;
    }
    // Moves projectile in direction of crosshair.
    if (cx < sx) sx -= shotSpeed;
    else if (cx > sx) sx += shotSpeed;
    else xHit = true;
    if (cy < sy) sy -= shotSpeed;
    else if (cy > sy) sy += shotSpeed;
    else yHit = true;
    // Collisions and movement.
    if (bowTrailed) {
      arrow.x = sx; arrow.y = sy;
      for (int i = 0; i < 12; i++) {
        if (xCollision(arrow, wall[i]) && yCollision(arrow, wall[i])) {
          shot = false;
          break;
        }
      }
      sprite_draw(&arrow);
    }
    else {
      bomb.x = sx; bomb.y = sy;
      for (int i = 0; i < 12; i++) {
        if (xCollision(bomb, wall[i]) && yCollision(bomb, wall[i])) {
          shot = false;
          explosion();
        }
      }
      sprite_draw(&bomb);
    }
    // The projectile hit the target.
    if (xHit && yHit) {
      if (bowTrailed) {
        for (int i = 0; i < 6; i++) { // 6 being enemyAm - 1
          if (gapCollision(arrow, enemy[i], 1)) {
            spriteMagic(enemy[i]);
            enemy[i].x = -300; enemy[i].y = -300;
            score += 10;
            send_str(PSTR("An enemy has been shot till death by the hero.\n"));
          }
        }
        arrows--;
        spriteMagic(arrow);
      }
      if (bombTrailed) {
        explosion();
      }
      send_str(PSTR("The projectile has been destoyed.\r\n"));
      shot = false;
    }
    send_str(PSTR("transit.\n"));
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
  sprite_init(&tower, 2, 0, TW, TH, towerBitmap);
	sprite_init(&enemy[0], LCD_X * 0.85, LCD_Y * 0.50, EW, EH, enemyBitmap);
	sprite_init(&key, LCD_X * 0.15 - KW, LCD_Y * 0.50, KW, KH, keyBitmap);
 	sprite_init(&door, midX - DW / 2, TH - DH + 1, DW, DH, doorBitmap);
	lvlInit = true;
}


// Moves all sprites in the opposite direction of player.
void moveAll(int x, int y) {
	if (level == 1) {
		tower.x += x; tower.y += y;
    wall[0].x += x; wall[0].x += y;
    wall[1].x += x; wall[1].x += y;
	}
	else {
		for(int i = 0; i < 12; i++) {
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


// Scrolling map feature. Works by moving the sprites instead
// of the player if at a certain point on the LCD.
void scrollMap(void) {
  int x = 0;
	int y = 0;
  if (screenX > 33) screenX = 33;
  else if (screenX < -33) screenX = -33;
  if (screenY > 21) screenY = 21;
  else if (screenY < -21) screenY = -21;
	if (hero.x < round(LCD_X * 0.20)) x += 1;
	if (hero.x + HW > round(LCD_X * 0.80)) x -= 1;
	if (hero.y < round(LCD_Y * 0.20)) y += 1;
	if (hero.y + HH > round(LCD_Y * 0.80)) y -= 1;
  screenX += x; screenY += y;
  if (screenX <= -33 || screenX >= 33) x = 0;
  if (screenY <= -21 || screenY >= 21) y = 0;
	moveAll(x, y);
}


// Produces random x value between game size.
int randX(void) {
  seed += 142;
  int x = rand() % (wallX2 + (wallX1 + 8 * 1));
  x += wallX1;
  return x;
}


// Produces random y value between game size.
int randY(void) {
  seed += 283;
  int y = rand() % (wallY2 + (wallY1 * 1));
  y += wallY1; //###
  return y;
}


// Shuffles an inputted array.
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
  int hw = 10; // Half wall size.
  int pos = 0;
  shuffle(position, sizeof(position));
  for (int i = 0; i <= wallAm; i += 2){
    int gridX = grid[position[pos]][0]; int gridY = grid[position[pos]][1];
    int chained[11][4] = { // x1, y1, x1, y1 (Vertical, Horizontal).
      {gridX - 12, gridY - 5, gridX - 12, gridY + hw - 5}, // T shape (90 D counter clockwise).
      {gridX - hw, gridY, gridX - hw + 3, gridY + 22}, // L shape.
      {gridX, gridY - 8, 200, 200},
      {gridX, gridY, gridX - 11, gridY}, // T shape.
      {200, 200, gridX, gridY + 2},
      {gridX - 8, gridY, gridX - hw, gridY}, // L shape (flipped).
      {gridX + hw + 2, gridY, gridX - hw, gridY}, // L shape (upside down).
      {gridX, gridY, gridX, gridY + 11},
      {gridX, gridY, 200, 200},
      {200, 200, gridX - 8, gridY},
    };
    int x1 = chained[pos][0], y1 = chained[pos][1];
    int x2 = chained[pos][2], y2 = chained[pos][3];
    sprite_init(&wall[i], x1, y1, VWW, VWH, vertWallBitmap);
    sprite_init(&wall[i + 1], x2, y2, HWW, HWH, horWallBitmap);
    pos++;
  }
}


// Initialises defence items.
void defenceInit(void) {
  bool valid = false;
  int gen, x, y;
  // Randomising both the position and chance of spawn(30%).
  gen = rand() % 99;
  if (gen <= 100) {
    x = randX(); y = randY();
    sprite_init(&bow, x, y, 8, 3, bowBitmap);
  }
  valid = false;
  gen = rand() % 99;
  if (gen <= 29) {
    x = randX(); y = randY();
    sprite_init(&bomb, x, y, 6, 4, bombBitmap);
  }
  valid = false;
  gen = rand() % 99;
  if (gen <= 29) {
    x = randX(); y = randY();
    sprite_init(&shield, x, y, 8, 4, shieldBitmap);
  }
}


// Initialises all the enemy sprites.
void enemyInit(void) {
  int x, y;
  seed *= 2313 * interval;
  enemyAm = 1 + (rand() % 4);
  for (int i = 0; i < enemyAm; i++) {
    seed *= 2313 * interval;
    x = randX(); y = randY();
    sprite_init(&enemy[i], x, y, EW, EH, enemyBitmap);
  }
}


// Initialises all the treasure sprites.
void treasureInit(void) {
  int x, y;
  treasureAm = rand() % 5;
  for (int i = 0; i < treasureAm; i++) {
    x = randX(); y = randY();
    sprite_init(&treasure[i], x, y, 8, 3, treasureBitmap);
  }
}


// Initialises door sprite.
void doorInit(void) {
  int x = -0, y = -10;
  sprite_init(&door, x, y, DW, DH, doorBitmap);
}


// Initialises key sprite.
void keyInit(void) {
  int x, y; seed += 74;
  x = randX(); y = randY();
  sprite_init(&key, x, y, KW, KH, keyBitmap);
}


// Initialises all the sprites on the map.
void mapInit(void) {
  wallInit();
  doorInit();
  keyInit();
  treasureInit();
  enemyInit();
	initHero();
  defenceInit();
	mapInitialised = true;
}


// Initialises level skeleton and draws it.
void drawLvl(void) {
	// Static level 1 sprites.
  if (level == 1) {
		if (!lvlInit) level1Init();
    sprite_draw(&tower);
  }
	// Randomly generated level sprites.
  else {
		if (!mapInitialised) mapInit();
		for (int i = 0; i <= wallAm; i++) sprite_draw(&wall[i]); // ###
		for (int i = 0; i < treasureAm; i++) sprite_draw(&treasure[i]);
    if (!shieldTrailed) sprite_draw(&shield);
    if (!bowTrailed) sprite_draw(&bow);
    if (!bombTrailed) sprite_draw(&bomb);
  }
  sprite_draw(&door); sprite_draw(&key);
  enemyMovement();
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
  bowTrailed = false; bombTrailed = false;
  shieldTrailed = false; arrows = 5;
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


// Respawn hero.
void respawnHero(void) {
  int x = (randX() % 40) + 24; int y = (randY() % 20) + 15;
  sprite_init(&hero, x, y, HW, HH, heroBitmap);
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


// Character movement.
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
	for (uint8_t i = 0; i < enemyAm; i++) {
    if (xCollision(hero, enemy[i]) && yCollision(hero, enemy[i])) {
      enColl = true;
    }
	}
  for (uint8_t i = 0; i < 6; i++) {
    if (xCollision(hero, treasure[i]) && yCollision(hero, treasure[i])) {
      spriteMagic(treasure[i]);
      treasure[i].y = 300;
      score += 10;
    }
  }
  for (int i = 0; i < wallAm; i++) {
    if (xCollision(hero, wall[i]) && yCollision(hero, wall[i])) {
      hero.y -= yy * 3;
			hero.x -= xx * 3;
			dx = 0;
			dy = 0;
      send_str(PSTR("WALLI COLL\r\n"));
    }
  }
  if (level > 1) {
    if (xCollision(hero, bomb) && yCollision(hero, bomb)) {
      if (!bombTrailed) {
        bombTrailed = true; shieldTrailed = false; bowTrailed = false;
        send_str(PSTR("The hero has located da bomb.\r\n"));
      }
      spriteTrail(bomb);
    }
    if (xCollision(hero, bow) && yCollision(hero, bow)) {
      if (!bowTrailed) {
        bowTrailed = true; bombTrailed = false; shieldTrailed = false;
        send_str(PSTR("The hero has collected the bow.\r\n"));
      }
      spriteTrail(bow);
    }

    if (xCollision(hero, shield) && yCollision(hero, shield)) {
      if (!shieldTrailed) {
        shieldTrailed = true; bombTrailed = false; bowTrailed = false;
        send_str(PSTR("The hero has picked up a shield. +1 protection.\r\n"));
      }
      spriteTrail(shield);
    }
  }
	if (xCollision(hero, key) && yCollision(hero, key)) {
    send_str(PSTR("The hero has retrieved the key.\r\n"));
		keyColl = true;
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
      hero.y -= yy * 7;
      hero.x -= xx * 7;
      shieldTrailed = false;
      send_str(PSTR("An enemy has destroyed the heros shield. -1 protection."));
      shield.x = 1000;
      shield.y = 1000;
    }
    else {
      bowTrailed = false; bombTrailed = false; keyColl = false;
      lives -= 1;
      send_str(PSTR("An enemy has killed the hero.\r\n"));
      respawnHero();
      screenX = 0; screenY = 0;
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
  bowTrailed = false;
  bombTrailed = false;
  shieldTrailed = false;
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
		_delay_ms(333);
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
    if (seconds < 59) {
  		seconds ++;
  	}
  	else {
  		seconds = 0;
  		minutes ++;
  	}
    serialOutput();
  }
  if (interval > 0.495 && interval < 0.505) {
    serialOutput();
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
  // Seed generator.
  srand(seed * seconds * level * minutes);
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
	show_screen();
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
    if (bombTrailed || bowTrailed) crosshairMovement();
    drawLvl();
		moveHero();
		show_screen();
	}
	else {
    send_str(PSTR("Game over\n"));
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
