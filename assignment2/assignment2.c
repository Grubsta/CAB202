// ANSI Tower for a Teensy utilising a PewPew board.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
// check snake game for sprite trailing hero.


// Configuration (sprite L & W).
#define HW 16 // Hero.
#define HH 8
#define TH 20 // Tower.
#define TW 80
#define DH 11 // Door.
#define DW 24
#define EH 5 // Enemy.
#define EW 8
#define KH 3 // Key.
#define KW 8
#define VWH 44 // Vetical wall.
#define VWW 3
#define HWH 3 // Horizontal wall.
#define HWW 44
// Threshold for button presses.
#define thresh (1000)


// TODO : ###
// FIX SCREEN movement
// FIX RANDOM generator
// FIX COLLISION Y value
// ADD ARROWS TO bow
// ADD BOMB FUNCTION
// ADD FORMATTED SENTENCES TO SERIAL DEBUUGGA

// NEED TO KNOWS
// SCREEN = 84x48

// Global variables.
// Location / movement.
float speed = 1.0;
double dx = 0;
double dy = 0;
int dxdy[1];
// Player.
int level = 2;
int lives = 3;
int score = 0;
// Timer.
int seconds = 0;
int minutes = 0;
int timeCounter = 0;
// Sprite amounts.
int enemyAm = 0;
int treasureAm = 0;
int wallAm;
// Gameplay / Collisions.
int XYarray[50];
int screenX = 0;
int screenY = 0;
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
uint16_t closedCon = 0;
uint16_t openCon = 0;
// Shooting mechanism values.
bool shot = false;
int hx; int hy; int cx; int cy;

// int wallX1 = -33, wallX2 = 117;
// int wallY1 = -21, wallY2 = 69;
int wallX1 = -10, wallX2 = 94;
int wallY1 = -10, wallY2 = 58;

// Initialise sprites.
Sprite hero; Sprite tower; Sprite door; Sprite key;
Sprite enemy[5]; Sprite treasure[5]; Sprite wall[5];
Sprite shield; Sprite bow; Sprite bomb; Sprite crosshair;
Sprite arrow;

// Terminal output strings
 char *heroDeathT = ("An enemy has killed the hero.");
 char *enemyDeathT = ("An enemy has been shot till death by the hero.");
 char *sheildCollT = ("The hero has picked up a shield. +1 protection.");
 char *keyCollT = ("The hero has retrieved the key.");
 char *bowCollT = ("The hero has collected the bow.");
 char *bombCollT = ("The hero has located da bomb.");
 char *shieldUsedT = ("An enemy has destroyed the heros shield. -1 protection.");
 // char *keyUsedT = ("The player has unlocked the door with the key.\nCongratulations on finishing floor {0}.", level);
 char *bowUsedT = ("The hero has shot the bow.");
 char *bombUsedT = ("The hero has detonated the bomb.");


// Initialise hero.
void initHero(void) {
	int x = LCD_X / 2 - HW / 2;
	int y = LCD_Y / 2 + HH + 3;
	sprite_init(&hero, x, y, HW, HH, heroBitmap);
}


// Causes the sprites to magically dissapear
void spriteMagic(Sprite sprite) {
  sprite.x = -0;
  sprite.y = -1000;
}


// Colisions for static map edges. ### FIX
void staticMap(void) {
  int x = round(hero.x); int y = round(hero.y);
  if (x < 0 || x + HW >= LCD_X - 1) hero.x -= dx;
  if (y < 0 || y + HH >= LCD_Y - 1) hero.y -= dy;
}


// Sends inputted string via serial connection.
void send_str(const char *string)
{
	char c;
	while (1) {
		c = pgm_read_byte(string++);
		if (!c) break;
		usb_serial_putchar(c);
	}
}


// Collision detection between 2 sprites.
bool gapCollision(Sprite sprite1, Sprite sprite2, int gap) {
  // Sprite 1.
  int spr1Bottom = sprite1.x + sprite1.height + gap;
  int spr1Top = sprite1.y - gap;
  int spr1Left = sprite1.x - gap;
  int spr1Right = sprite1.x + sprite1.width + gap;
  // Sprite 2.
  int spr2Bottom = sprite2.y + sprite2.height + gap;
  int spr2Top = sprite2.y - gap;
  int spr2Left = sprite2.x - gap;
  int spr2Right = sprite2.x + sprite2.width + gap;
  // Creates a perimter arround sprites and checks for collision.
	if (spr1Bottom < spr2Top || spr1Top > spr2Bottom || spr1Right < spr2Left|| spr1Left > spr2Right) {
		return false;
	}
	else {
		return true;
	}
}


// Moves enemy sprite towards hero's location.
void enemyMovement() {
	float enemySpeed = 0.1;
	for (int i = 0; i < enemyAm; i++) {
		if (enemy[i].x < hero.x) enemy[i].x += enemySpeed;
		else if (enemy[i].x > hero.x) enemy[i].x -= enemySpeed;
		if (enemy[i].y < hero.y) enemy[i].y += enemySpeed;
		else if (enemy[i].y > hero.y) enemy[i].y -= enemySpeed;
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
    crosshair.x = cx; crosshair.y = cy;
  }
  if (!crosshairInit) sprite_init(&crosshair, LCD_X * 0.5, LCD_Y * 0.5, 3, 3, crosshairBitmap);
  // User input.
  if ((BIT_IS_SET(PINF, 6) || 	BIT_IS_SET(PINF, 5)) && shot == false) {
    send_str(PSTR("3\n"));
    shot = true;
    if (cx < hx) sx = hx - 5 - 2; // 2 being the shot's width.
    else if (cx > hx) sx = hx + HW + 5;
    if (cy < hy) sy = hy - 5;
    else if (cy > hy) sy = hy + HW + 5;
    if (bowTrailed) {
      sprite_init(&arrow, sx, sy, 2, 2, arrowBitmap);
      sprite_draw(&arrow);
      send_str(PSTR("The arrow has left the building.\n"));
    }
    else {
      sprite_init(&bomb, sx, sy, 2, 2, arrowBitmap);
      sprite_draw(&bomb);
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
    arrow.x += sx; arrow.y += sy;
    // The projectile has hit the target.
    if (xHit && yHit) {
      if (bowTrailed) {
        for (int i = 0; i < enemyAm; i++) {
          if (gapCollision(arrow, enemy[i], 1)) {
            spriteMagic(enemy[i]);
            score += 10;
            send_str(PSTR("An enemy has been shot till death by the hero.\n"));
          }
        }
        spriteMagic(arrow);
      }
      if (bombTrailed) {
        for (int i = 0; i < enemyAm; i++) {
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
    sprite_draw(&bomb);
    sprite_draw(&bow);
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
  //  //### TEMP
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
	if (hero.x < round(LCD_X * 0.15) && hero.x > wallX1 && hero.x < wallX2) x += 1; // ### fix the && of all of these
	else if (hero.x + HW > round(LCD_X * 0.85) && hero.x > -33 && hero.x < wallX2) x -= 1;
	if (hero.y < round(LCD_Y * 0.15) && hero.y > wallY1 && hero.y < wallY2) y += 1;
	else if (hero.y + HH > round(LCD_Y * 0.85) && hero.y > wallY1 && hero.y < wallY2) y -= 1;
  screenX += x; screenY += y;
  if (screenX <= -33 || screenX >= 33) x = 0; // #### DEBATABLY WORKING
  if (screenY <= -21 || screenY >= 21) y = 0;
	moveAll(x, y);
}


// Initialises set amount of walls.
void wallInit(void) {
	wallAm = 6;
	int x, y;
	int drawnWall = 0;
	bool valid = true;
	for (int i = 0; i < wallAm; i++) {
		do {
			x = rand() % wallX2 + (wallX1 * 1) - 12;
      x += wallX1;
			y = rand() % wallY2 + (wallY1 * 1) - 12;
      y += wallY1;
			int direction = rand() % 1;
			if (direction == 1) { // Vertical direction.
				sprite_init(&wall[i], x, y, VWW, VWH, vertWallBitmap);
			} else { // Horizontal direction.
				sprite_init(&wall[i], x, y, HWW, HWH, horWallBitmap);
			}
			if (drawnWall > 0) {
				for (int a = 0; a < drawnWall; a++) { // 5-1 and make the fifth start at the same x1y1 as another
					if (gapCollision(wall[i], wall[a], 15)) valid = false; // ### NEED to compensate for wall gaps.
					else valid = true;
				}
			}
		} while (!valid);
		sprite_draw(&wall[i]);
		drawnWall =+ 1;
	}
	wallInitialised = true;
}


// Initialises defence items.
void defenceInit(void) {
  bool valid = false;
  int gen, x, y;
  // Randomising both the position and chance of spawn.
  gen = rand() % 10; x = rand() % 100; y = rand() % 85;
  x = 5; y = 25; // ### TEMP
  while (!valid) {
    if (valid == false) sprite_init(&bow, x, y, 8, 3, bowBitmap); // ### Add Colision
    valid = true;
  }
  valid = false;
  gen = rand() % 10; x = rand() % 100; y = rand() % 85;
  x = 40; y = 25;
  while (!valid) {
    if (valid == false) sprite_init(&bomb, x, y, 8, 4, bombBitmap); // ### Add Colision
    valid = true;
  }
  valid = false;
  gen = rand() % 10; x = rand() % 100; y = rand() % 85;
  x = 75; y = 25;
  while (!valid) {
    if (gen <= 3) sprite_init(&shield, x, y, 8, 4, shieldBitmap); // ### Add Colision
    valid = true;
  }
}


// Initialises all the enemy sprites.
void enemyInit(void) { ///### inits broken, fix later
  int x, y;
  for (int i = 0; i < enemyAm; i++) {
    x = rand() % 100; y = rand() % 85;
    sprite_init(&enemy[i], x, y, EW, EH, enemyBitmap);
  }
}


// Initialises all the treasure sprites.
void treasureInit(void) { ///### inits broken, fix later
  int x, y;
  for (int i = 0; i < treasureAm; i++) {
    x = rand() % 100; y = rand() % 85;
    sprite_init(&treasure[i], x, y, 8, 3, treasureBitmap);
  }
}


// Initialises all the sprites on the map.
void mapInit(void) {
	for (int i = 0; i < 50; i++) {
		XYarray[i] = rand() % 40;
	}
	sprite_init(&door, XYarray[8], XYarray[43], DW, DH, doorBitmap);
	sprite_init(&key, XYarray[2], XYarray[33], KW, KH, keyBitmap);
  // treasureInit();
	wallInit();
	initHero();
  defenceInit();
	enemyInit();
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
    // crosshairMovement();
  }
	// Randomly generated level sprites.
  else {
		if (!mapInitialised) mapInit();
    enemyMovement();
		for (int i = 0; i < wallAm; i++) sprite_draw(&wall[i]);
		for (int i = 0; i < treasureAm; i++) sprite_draw(&treasure[i]);
		sprite_draw(&door); sprite_draw(&key);
    if (!shieldTrailed) sprite_draw(&shield);
    if (!bowTrailed) sprite_draw(&bow);
    if (!bombTrailed) sprite_draw(&bomb);
  }
}


// Enables sprites to trail sprites.
void spriteTrail(Sprite sprite1) {
  int x = hero.x - 3; int y = hero.y + HH + 3;
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
	char lev[50]; char liv[50]; char scor[50]; char timer[20];
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
	else if (BIT_IS_SET(PINB, 0)){ // Centre switch.
		// Acts as a debouncer whilst user hold switch down.
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
		if (gapCollision(hero, enemy[i], 0)) {
			enColl = true;
		}
	}
  if (level > 1) {
    if (gapCollision(hero, bomb, 1)) {
      if (!bombTrailed) {
        bombTrailed = true; shieldTrailed = false; bowTrailed = false;
        send_str(PSTR("The hero has located da bomb.\r\n"));
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
      // mapInitialised = false;
      // destroyGame(); // ### FIX AFTER DEBUGGA
    }
	}
	else if (gapCollision(hero, door, 1)) {
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
	scrollMap();
	staticMap();
  sprite_draw(&hero);
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


// Game over menu.
void gameOverScreen(void) {
	clear_screen();
	char lev[50]; char scor[50];
	draw_string(0, 0, "You died in ANZI!", FG_COLOUR);
	sprintf(lev, "level: %d", level); draw_string(0, 10, lev, FG_COLOUR);
	sprintf(scor, "final score: %d", score); draw_string(0, 20, scor, FG_COLOUR);
	draw_string(0, 40, "SW2/3 to restart", FG_COLOUR);
	show_screen(); //### Add button press to restart gameplay.
  bool start = false;
  do {
    if (BIT_IS_SET(PINF, 6) || 	BIT_IS_SET(PINF, 5)) start = true;
  } while (!start);
  // setup(); ###
}


// Serial output for common game stats.
void serialOutput(void) {
  // int x = hero.x; int y = hero.y;
  // char gameStatsT[100];
  // // snprintf(gameStatsT, "~~~~~~~~~~~~~~~~~~~~~\r\n\r\n"
  // // " Current run-time: %02d:%02d\r\n\r\n"
  // // "			  		 Score: %d\r\n\r\n"
  // // "		  			 Level: %d\r\n\r\n"s
  // // "	  	X,Y Location: %d,%d\r\n\r\n"
  // // "  Remaining lives: %d", minutes, seconds, score, level, x, y, lives);
  // // send_str(gameStatsT);
}


// Initialise Timer.
void timer(void) {
	timeCounter++;
	if (timeCounter == 10) {
	seconds++;
	timeCounter = 0;
		if (seconds == 60) {
		seconds = 0;
		minutes++;
			if (minutes == 100) {
			}
		}
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
}

// Setup (ran on start).
void setup(void) {
  set_clock_speed(CPU_8MHz);
	usb_init();
  adc_init();
  lcd_init(LCD_DEFAULT_CONTRAST);
  timer();
  DDRD |= (1<<6);
  PORTD |= (1<<6);
  clear_screen();
  draw_string(0, 10, "Connect to a ", FG_COLOUR);
  draw_string(0, 20, "serial terminal", FG_COLOUR);
  draw_string(0, 30, "to continue", FG_COLOUR);
  show_screen();
	while(!usb_configured());
  while(!(usb_serial_get_control() & USB_SERIAL_DTR))
  usb_serial_flush_input();
  PORTD &= ~(1<<6);
  send_str(PSTR("Welcome to ANSI\r\n"));
  initControls();
	// welcomeScreen();
  clear_screen();
  drawLvl();
  sprite_draw(&hero);
  show_screen();
}


// Process (ran every frame).
void process(void) {
	if (lives > 0) {
		clear_screen();
		timer();
		drawLvl();
    if (bombTrailed || bowTrailed) crosshairMovement();
		moveHero();
		show_screen();
	}
	else {
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
