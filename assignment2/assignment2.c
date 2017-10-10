// ANSI Tower for a Teensy utilising a PewPew board.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <sprite.h>
#include <macros.h>
#include "bitmaps.h"
#include "usb_serial.h"

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
#define HWH 3 // horizontal wall.
#define HWW 44

#define thresh (1000)


// TODO : ###
// 12. Random location generator (apply to all array sprites)
// 13. Scrolling map (half done)
// 15. Character attack/defense mechanisms
// 16. USB Serial Debugging Interface

// NEED TO KNOWS
// SCREEN = 84x48

// Global variables.
// Location / movement.
float speed = 1.0;
double dx = 0;
double dy = 0;
int dxdy[1];
// Player.
int level = 1;
int lives = 3;
int score = 0;
// Timer.
int seconds = 0;
int minutes = 0;
int timeCounter = 0;
// Sprite amounts.
int enemyAm = 5;
int treasureAm = 0;
int wallAm;
// Gameplay / Collisions.
int XYarray[50];
int screenHeight = 48;
int screenWidth = 84;
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

uint16_t closedCon = 0;
uint16_t openCon = 0;

// int wallX1 = -33, wallX2 = 117;
// int wallY1 = -21, wallY2 = 69;
int wallX1 = -10, wallX2 = 94;
int wallY1 = -10, wallY2 = 58;

// Initialise sprites.
Sprite hero; Sprite tower; Sprite door; Sprite key;
Sprite enemy[5]; Sprite treasure[5]; Sprite wall[5];
Sprite shield; Sprite bow; Sprite bomb;

// Terminal output strings
// sprintf(*gameStatsT, "~~~~~~~~~~~~~~~~~~~~~" +
// " Current run-time: {0}:{1}" +
// "			  		 Score: {2}" +
// "		  			 Level: {3}" +
// "	  	X,Y Location: {4},{5}" +
// "  Remaining lives: {6}", minutes, seconds, score, level, hero.x, hero.y, lives);
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

 // void setup_usb_serial( void );
 // void usb_serial_send(char * message);

// Initialise hero.
void initHero(void) {
	int x = LCD_X / 2 - HW / 2;
	int y = LCD_Y / 2 + HH + 3;
	sprite_init(&hero, x, y, HW, HH, heroBitmap);
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


// Collision detection between 2 sprites.
bool gapCollision(Sprite sprite1, Sprite sprite2, int gap) {
  // Sprite 1.
  int spr1Bottom = round(sprite1.x + sprite1.height + gap);
  int spr1Top = round(sprite1.y - gap);
  int spr1Left = round(sprite1.x - gap);
  int spr1Right = round(sprite1.x + sprite1.width + gap);
  // Sprite 2.
  int spr2Bottom = round(sprite2.y + sprite2.height + gap);
  int spr2Top = round(sprite2.y - gap);
  int spr2Left = round(sprite2.x - gap);
  int spr2Right = round(sprite2.x + sprite2.width + gap);
  // Creates a perimter arround sprites and checks for collision.
	if (spr1Bottom < spr2Top || spr1Top > spr2Bottom || spr1Right < spr2Left|| spr1Left > spr2Right) {
		return false;
	}
	else {
		return true;
	}
}


// Initialises all sprites on first level.
void level1Init(void) {
	int midX = LCD_X / 2;
	initHero();
  sprite_init(&wall[0], 2, 0 - VWH, VWW, VWH, vertWallBitmap);
  sprite_init(&wall[1], 80, 0 - VWH, VWW, VWH, vertWallBitmap);
  sprite_init(&tower, 2, 0, TW, TH, towerBitmap);
	sprite_init(&enemy[0], LCD_X * 0.85, LCD_Y * 0.50, EW, EH, enemyBitmap); // ### relocate enemy to allow for movement
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
	if (hero.x < round(LCD_X * 0.15) && hero.x > wallX1 && hero.x < wallX2) x += 1;
	else if (hero.x + HW > round(LCD_X * 0.85) && hero.x > -33 && hero.x < wallX2) x -= 1;
	if (hero.y < round(LCD_Y * 0.15) && hero.y > wallY1 && hero.y < wallY2) y += 1;
	else if (hero.y + HH > round(LCD_Y * 0.85) && hero.y > wallY1 && hero.y < wallY2) y -= 1;
  if ( round(hero.x) < -33 ||  round(hero.x) + HW >= 117 ) hero.x -= x; // ###
  if (round(hero.y) - 2  < -21 || round(hero.y) + HH >= 69) hero.y -= y; // ###
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
			x = rand() % 100;
			y = rand() % 90;
			int direction = rand() % 2;
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
    sprite_init(&treasure[i], x, y, TW, TH, treasureBitmap);
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
	// wallInit();
	initHero();
  defenceInit();
	// enemyInit();
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
		for (int i = 0; i < wallAm; i++) sprite_draw(&wall[i]);
		for (int i = 0; i < treasureAm; i++) sprite_draw(&treasure[i]);
		sprite_draw(&door); sprite_draw(&key);
    sprite_draw(&shield); sprite_draw(&bow); sprite_draw(&bomb);
  }
}


// Enables sprites to trail sprites.
void spriteTrail(Sprite sprite1) {
  int x = hero.x - 3; int y = hero.y + HH * 1.5;
  // if (spriteTrailed) x += HW + 3;
  sprite1.x = x; sprite1.y = y;
}


// Destroys entire level. ###
void destroyGame(void) {
	if (level == 1) {
		free(&tower); free(&enemy[0]);
	}
	else {
		for (int i = 0; i < wallAm; i++) free(&wall[i]);
	}
	for (int i = 0; i < enemyAm; i++) free(&enemy[i]);
  for (int i = 0; i < treasureAm; i++) free(&treasure[i]);
	free(&hero); free(&key); free(&door);
  free(&bow); free(&bomb); free(&shield);
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
	if (gapCollision(hero, key, 2)) {
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
		destroyGame();
		lives -= 1;
		enColl = false;
	}
  else if (gapCollision(hero, bow, 1)) {
    // spriteTrail(bow);
  }
  else if (gapCollision(hero, bomb, 1)) {
    // spriteTrail(bomb);
  }
  else if (gapCollision(hero, shield, 1)) {
    hero.y -= yy;
    hero.x -= xx;
    // spriteTrail(shield);
  }
	else if (gapCollision(hero, door, 1)) {
		if (keyColl) {
			level += 1;
			score += 100;
			destroyGame();
			loadingScreen();
		}
		else {
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
	// Wall collision.
	// staticMap();
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
	char lev[50];char scor[50];
	draw_string(0, 0, "You died in ANZI!", FG_COLOUR);
	sprintf(lev, "level: %d", level); draw_string(0, 10, lev, FG_COLOUR);
	sprintf(scor, "final score: %d", score); draw_string(0, 20, scor, FG_COLOUR);
	draw_string(0, 40, "SW2/3 to restart", FG_COLOUR);
	show_screen(); //### Add button press to restart gameplay.
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
			  // game_over = true;
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
	// // ###
	// // Set Timer 0 to overflow approx 122 times per second.
	// TCCR0B |= 4;
	// TIMSK0 = 1;
	// Enable interrupts.
	// sei();
	// // ###
	// while(!usb_configured()) {
	// 	draw_string(0, 30, "Connect to a \nserial terminal", FG_COLOUR);
	// };
	// char *ANSI = "Welcome to ANSI";
	// usb_serial_send(ANSI);
  initControls();
  lcd_init(LCD_DEFAULT_CONTRAST);
	timer();
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
