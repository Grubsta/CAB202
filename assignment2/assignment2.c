#include "cpu_speed.h"
#include "sprite.h"
#include "lcd.h"
#include <avr/io.h>
#include "graphics.h"
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <sprite.h>

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

// Configuration
#define DELAY (10)
#define HERO_WIDTH (3)
#define HERO_HEIGHT (3)

// Bitmaps
uint8_t heroBitmap[] = {
  0b00000111, 0b11100000,
	0b00000110, 0b01100000,
	0b00000111, 0b11100000,
	0b01111111, 0b11111110,
  0b01100011, 0b11000110,
	0b01000011, 0b11000010,
	0b00001100, 0b00110000,
	0b00111000, 0b00011100,
};

// Initialise Sprites
sprite_id hero;


// Creates a single Sprite
void initCharacter(sprite_id character, uint8_t bitmap) {
  int x = rand() % (LCD_X - 4);
	int y = rand() % (LCD_Y - 4);
  sprite_init(&character, x, y, 4, 4, bitmap);
}


void setup(void) {
  set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();
  initCharacter(hero, heroBitmap);
  sprite_draw(&hero);
}


int main(void) {
	setup();

	for ( ;; ) {
		// process();
		_delay_ms(10);
	}
}

// Main loop.
// int main(void) {
//   // Game environment.
// 	while ( !game_over ) {
//     // Draw frame.
//
// 		if ( update_screen ) {
// 			show_screen();
// 		}
// 		timer_pause(DELAY);
// 	}
// 	return 0;
// }
