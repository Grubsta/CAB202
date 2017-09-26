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

// Configuration (sprite L & W).
#define HW 16 // Hero.
#define HH 8
#define TH 21 // Tower.
#define TW 80
#define DH 11 // Door.
#define DW 24
#define EH 5 // Enemy.
#define EW 8
#define KH 3 // Key.
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

// Colisions for static map edges. ### FIX
void staticMap(void) {
  int x = round(hero.x); int y = round(hero.y);
  if (x < 0 || x + HW >= LCD_X - 1) hero.x -= dx;
  if (y - 2  < 0 || y + HH >= LCD_Y - 1) hero.y -= dy;

}

// Collision detection between 2 sprites.
void spriteCollision(sprite_id sprite1, sprite_id sprite2) {
  // // Sprite 1.
  // int spr1Bottom = round(sprite1.y + sprite1->height);
  // int spr1Top = round(sprite1.y);
  // int spr1Left = round(sprite1.x);
  // int spr1Right = round(sprite1.x + sprite1->width);
  // // Sprite 2.
  // int spr2Bottom = round(sprite2.y + sprite2->height);
  // int spr2Top = round(sprite2.y);
  // int spr2Left = round(sprite2.x);
  // int spr2Right = round(sprite2.x + sprite2->width);
  // // Creates a perimter arround sprites and checks for collision.
  // if (spr1Bottom < spr2Top || spr1Top > spr2Bottom || spr1Right < spr2Left|| spr1Left > spr2Right) {
  //   return false;
  // }
  // else return true;
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
    // while(true) {
    //   enemy.x = random_range(0, LCD_X - EW);
    //   enemy.y = random_range(0, LCD_Y - EH);
    //   if (!collision) {
    //     break;
    //   }
    // }
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
    // DISPLAY STATS! hero = lives, score, level
    // break;
  }
  hero.y += dy;
  hero.x += dx;
  if (level == 1) {
    staticMap();
  }
}

// Welcome Screen. ### Not FINISHED! NEVER CALLED
void welcomeScreen(void) {
  clear_screen();
  draw_string(LCD_X / 2 - (10 / 2), LCD_Y / 2 - 2, "Corey Hull", FG_COLOUR);
  draw_string(LCD_X / 2 - (9 / 2), LCD_Y / 2 + 2, "N10007164", FG_COLOUR);
  show_screen();
  _delay_ms(2000);
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
