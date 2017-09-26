// ANSI Tower for a Teensy utilising a PewPew board.
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <sprite.h>
#include <macros.h>
#include "bitmaps.h"

// Configuration (sprite L & W)
#define HW 16 // Hero.
#define HH 8
#define TH 21 // Tower.
#define TW 80
#define DH 11 // Door.
#define DW 24
#define EH 5 // Enemy.
#define EW 8
#define KH 3 // Key
#define KW 8


// TODO : ###
// 1. Levels
// 2. Character output
// 3. Character movement
// 4. Character collision
// 5. Introduction screen
// 6. Game Countdown
// 7. Status screen
// 8. Enemy/s
// 9. Treasuure
// 10. Door & key
// 11. Loading screen
// 12. Random location generator
// 13. Scrolling map
// 14. Random map generator
// 15. Character attack/defense mechanisms

// NEED TO KNOWS
// SCREEN = 84x48

// Global variables.
double dx = 0;
double dy = 0;
int level = 1;

// Initialise sprites.
Sprite hero; Sprite tower; Sprite door;
Sprite key; Sprite enemy; Sprite treasure;

// Parameters asking for rand or static:
// if static x & y set pos, else rand

// Initialise hero.
void initHero(void) {
	int x = LCD_X / 2 - HW / 2;
	int y = LCD_Y / 2 + HH + 3;
	sprite_init(&hero, x, y, HW, HH, heroBitmap);
}

// Colisions for static map edges.
void staticMap(void) {
  int x = round(hero.x); int y = round(hero.y);
  if (x < 0 || x + HW >= LCD_X - 1) hero.x -= dx;
  if (y - 2  < 0 || y + HH >= LCD_Y - 1) hero.y -= dy;

}

// Draws level skeleton.
void drawLvl(void) {
  int midX = LCD_X / 2;
  int maxY = LCD_Y - 1;
  int maxX = LCD_X - 1;
  if (level == 1) {
    sprite_init(&enemy, LCD_X * 0.85, LCD_Y * 0.40, EW, EH, enemyBitmap);
    sprite_init(&key, LCD_X * 0.15 - KW, LCD_Y * 0.40, KW, KH, keyBitmap);
    sprite_init(&tower, 2, 0, TW, TH, towerBitmap);
    sprite_init(&door, midX - DW / 2, TH - DH, DW, DH, doorBitmap);
    draw_line(0, 0, maxX, 0, FG_COLOUR);
    draw_line(0, 0, 0, maxY, FG_COLOUR);
    draw_line(0, maxY, maxX, maxY, FG_COLOUR);
    draw_line(maxX, 0, maxX, maxY, FG_COLOUR);
    sprite_draw(&tower); sprite_draw(&door); sprite_draw(&enemy);
    sprite_draw(&key);
  }
  else {

  }
}

// Hero movement.
void moveHero(void) {
  double dx = 0; double dy = 0;
  // int x = round(hero.x); int y = round(hero.y);
  // Reads user movement input.
  if (BIT_IS_SET(PIND, 1)){ // Up switch.
    dy -= 1;
  }
  else if (BIT_IS_SET(PINB, 7)){ // Down switch.
    dy += 1;
  }
  else if (BIT_IS_SET(PINB, 1)){ // Left switch.
    dx -= 1;
  }
  else if (BIT_IS_SET(PIND, 0)){ // Right switch.
    dx += 1;
  }
  else if (BIT_IS_SET(PINB, 0)){ // Centre switch.

  }
  hero.y += dy;
  hero.x += dx;
  if (level == 1) {
    staticMap();
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
}

// Setup (ran on start).
void setup(void) {
	set_clock_speed(CPU_8MHz);
  initControls();
	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();
  drawLvl();
	initHero();
	sprite_draw(&hero);
	show_screen();
}

// Process (ran every frame).
void process(void) {
	clear_screen();
  drawLvl();
  moveHero();
	sprite_draw(&hero);
	show_screen();
}

// Main loop.
int main(void) {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(10);
	}
}
