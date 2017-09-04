#include <math.h>
#include <stdlib.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <curses.h>

// Configuration.
#define DELAY (10)
#define HERO_WIDTH (3)
#define HERO_HEIGHT (3)
#define ZOMBIE_WIDTH (4)
#define ZOMBIE_HEIGHT (4)
#define N (10)

// Initialising Game state.
bool game_over = false; /* Set this to true when game is over */
bool update_screen = true; /* Set to false to prevent screen update. */
int seconds = 0;
int minutes = 0;
int timeCounter = 0;
int levelTime = 0;
int score = 0;
int level = 1;
int lives = 10;

// Initialising platform Sprite Locations
// double initialX[];
double initialX[20];
double initialX1[20];
double platformWidth[] = {10, 10};
int platformSprite[20];

// Declaring interactive charcters.
char * charGround =
/**/	" o "
/**/	"/|\\"
/**/	"/ \\";

char * lvlOneEnemy =
/**/	"ZZZZ"
/**/	"  Z  "
/**/	" Z   "
/**/	"ZZZZ";

char * platformBase =
/**/ "========================================================================================================================";

// Declare a sprites.
sprite_id hero;
sprite_id enemy;
sprite_id platform[N];

// Windows main borders.
void drawArena(void) {
  draw_line(0, 2, screen_width(), 2, '-');
  draw_line(screen_width(), 1, screen_width() - 1, screen_height(), '|');
  draw_line(0, 2, 0, screen_height(), '|');
}

// Platform entitys in game.
sprite_id initPlatforms(double x1, double y1, double width) {
  sprite_id platformSprite = sprite_create(x1, y1, 1, width, platformBase);
  return platformSprite;
}

// Draw platform/s.
void DrawPlatforms() {
  if (level == 1)
    platform[0] = initPlatforms(0, round(screen_height()), round(screen_width()));
    platform[1] = initPlatforms((screen_width() * 0.3), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
    sprite_draw(platform[0]), sprite_draw(platform[1]);
  }
  else if (level == 2) {
    platform[0] = initPlatforms(0, screen_height(), screen_width());
    platform[1] = initPlatforms((screen_width() * 0.4), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
    platform[2] = initPlatforms((screen_width() * 0.3), screen_height() - (2 + (HERO_HEIGHT * 7)), (screen_width() * 0.2));
    sprite_draw(platform[0]), sprite_draw(platform[1]), sprite_draw(platform[2]);
  }
  else if (level == 3) {
    return;
  }
  else if (level == 4) {
    return;
  }
  else if (level == 5) {
    return;
  }
}

// Draws components of game.
void drawGame(void) {
	drawArena();
  DrawPlatforms();
	sprite_draw(hero);
}

// Game hud.
void gameHud(void) {
  int width = screen_width() / 4;
  draw_formatted(2, 1, "Time: %02d:%02d", minutes, seconds);
  draw_formatted(width, 1, "Lives: %d", lives);
  draw_formatted(width * 2, 1, "Level: %d", level);5
  draw_formatted(width * 3, 1, "Score: %d", score);
}

// Initialise Timer
void timer(void) {
	timeCounter++;
	if (timeCounter == 100) {
		seconds++;
		levelTime++;
		timeCounter = 0;
		if (seconds == 60) {
			seconds = 0;
			minutes++;
      if (minutes == 100) {
        game_over = true;
      }
		}
	}
}

// Collision between charcter and platform.
// bool check_collision(sprite_id s1, sprite_id s2) {
// // Check collsion between sprites s1 and s2
// // Get the top, bottom, left and right of each sprite
//     int sprite1Bottom = sprite_y(s1) + 1;
//     int sprite1Top = sprite_y(s1);
//     int sprite1Left = sprite_x(s1);
//     int sprite1Right = sprite_x(s1) + sprite_width(s1) - 1;
//
//     int sprite2Bottom = sprite_y(s2) + 1;
//     int sprite2Top = sprite_y(s2);
//     int sprite2Left = sprite_x(s2);
//     int sprite2Right = sprite_x(s2) + sprite_width(s2) - 1;
//
// // if any of these do not coincide, then return true
//     if (sprite1Bottom < sprite2Top || sprite1Top > sprite2Bottom || sprite1Right < sprite2Left || sprite1Left > sprite2Right) {
//         return false;
//     }
//         else {
//             return true;
//     }
// }

// Character movement
void moveChar(void) {
  // Get a character code user input.
  int keypress = get_char();
  // Integer coordinates of hero.
  int heroX = round(sprite_x(hero));
  int heroY = round(sprite_y(hero));
  // Jump the hero.
  if ((keypress == KEY_UP) && heroY > 10 ) sprite_move(hero, 0, -5);
  // Move hero left.
  if ((keypress == KEY_LEFT) && heroX > 1 ) sprite_move(hero, -1, 0);
  // Move hero right.
  if ((keypress == KEY_RIGHT) && heroX < width - sprite_width(hero) - 2) sprite_move(hero, +1, 0);
}

// Play one turn of game.
void process(void) {
	int width = screen_width(), height = screen_height();
  moveChar();

  // Collision detection.
  // if (spriteCollision(hero, platform)) sprite_move(hero, +1, 0);

  // Clear current Frame & then redraw.
  clear_screen();
  drawGame();
  // Display Screen.
  show_screen();
}

// Initial setup.
void setup(void) {
  // Useful variables.
  int width = screen_width(), heroWidth = HERO_WIDTH, zombieWidth = ZOMBIE_WIDTH;
  int height = screen_height(), heroHeight = HERO_HEIGHT, zombieHeight = ZOMBIE_HEIGHT;
  // Setup arena
  drawArena();
  // Keep the next line intact.
  show_screen();
  // (d) Set up the hero at the centre of the screen.
	hero = sprite_create(2 + heroWidth, height-heroHeight-1, heroWidth, heroHeight, charGround);
  // (e) Draw the
  drawGame();
  // Display Screen.
	show_screen();
}

// Main loop.
int main(void) {
  // Draw game environment
  setup_screen();
	setup();
	show_screen();

	while ( !game_over ) {
    // Continue drawing frames
    process();
    timer();
    gameHud();

		if ( update_screen ) {
			show_screen();

		}

		timer_pause(DELAY);
	}

	return 0;
}
