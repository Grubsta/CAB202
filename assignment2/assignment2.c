#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <sprite.h>

// Configuration
#define HW 16
#define HH 8

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

// Bitmaps.
uint8_t heroBitmap[] = {
  0b00000111, 0b11100000,
	0b00000110, 0b01100000,
	0b00000111, 0b11100000,
	0b00111111, 0b11111100,
  0b01100011, 0b11000110,
	0b01000011, 0b11000010,
	0b00001100, 0b00110000,
	0b00111000, 0b00011100,
};

// Initialise sprites.
Sprite hero;

// Initialise hero.
void initHero(void) {
	int x = rand() % (LCD_X - HW);
	int y = rand() % (LCD_Y - HH);
	sprite_init(&hero, x, y, HW, HH, heroBitmap);
}

// Setup (ran on start).
void setup(void) {
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();
	initHero();
	sprite_draw(&hero);
	show_screen();
}

// Process (ran every frame).
void process(void) {
	clear_screen();
	sprite_draw(&hero);
	show_screen();
}

// Main loop.
int main(void) {
	setup();

	for ( ;; ) {
		// process();
		_delay_ms(10);
	}
}


// // Creates a single Sprite
// // void initCharacter(sprite_id character, uint8_t bitmap) {
// //   int x = rand() % (LCD_X - 4);
// // 	int y = rand() % (LCD_Y - 4);
// //   sprite_init(&character, x, y, 4, 4, bitmap);
// // }
