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

// Initialising game variables.
bool game_over = false; /* Set this to true when game is over */
bool update_screen = true; /* Set to false to prevent screen update. */
bool right = false;
bool left = false;
bool air = false;
bool collision = true;
double dx = 0;
double dy = 0;
double velocity = 0;
double gravity = 0.1;
int seconds = 0;
int minutes = 0;
int timeCounter = 0;
int levelTime = 0;
int score = 0;
int level = 1;
int lives = 10;

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

// Declaring sprites.
sprite_id hero; sprite_id enemy; sprite_id platform[10];

// Windows main borders.
void drawArena(void) {
  draw_line(0, 2, screen_width(), 2, '-');
  draw_line(screen_width(), 1, screen_width() - 1, screen_height(), '|');
  draw_line(0, 2, 0, screen_height(), '|');
}

// Creates platform sprites in game.
sprite_id initPlatforms(double x1, double y1, double width) {
  sprite_id platformSprite = sprite_create(x1, y1, width, 1, platformBase);
  return platformSprite;
}

// Draw platform/s.
void DrawPlatforms() {
  switch (level) {
    case 1:
      // level 1.
      platform[0] = initPlatforms(0, screen_height() - 1, screen_width()); sprite_draw(platform[0]);
      platform[1] = initPlatforms((screen_width() * 0.3), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3)); sprite_draw(platform[1]);
    case 2:
      // level 2.
      platform[0] = initPlatforms(0, screen_height(), screen_width());
      platform[1] = initPlatforms((screen_width() * 0.4), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
      platform[2] = initPlatforms((screen_width() * 0.3), screen_height() - (2 + (HERO_HEIGHT * 7)), (screen_width() * 0.2));
      sprite_draw(platform[0]); sprite_draw(platform[1]); sprite_draw(platform[2]);
    case 3:
      // level 3.
      platform[0] = initPlatforms(0, screen_height(), screen_width());
      platform[1] = initPlatforms((screen_width() * 0.4), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
      platform[2] = initPlatforms((screen_width() * 0.3), screen_height() - (2 + (HERO_HEIGHT * 7)), (screen_width() * 0.2));
      sprite_draw(platform[0]); sprite_draw(platform[1]); sprite_draw(platform[2]);
  }
}

// Collision between charcter and platform.
bool spriteCollision(sprite_id sprite1, sprite_id sprite2) {
  // Get perimiter values of sprites.
  int sprite1Bottom = sprite_y(sprite1) + 1;
  int sprite1Top = sprite_y(sprite1);
  int sprite1Left = sprite_x(sprite1);
  int sprite1Right = sprite_x(sprite1) + sprite_width(sprite1) - 1;
  //sprite 2
  int sprite2Bottom = sprite_y(sprite2) + 1;
  int sprite2Top = sprite_y(sprite2);
  int sprite2Left = sprite_x(sprite2);
  int sprite2Right = sprite_x(sprite2) + sprite_width(sprite2) - 1;
  // Collision occurance will output false.
  if (sprite1Bottom + 2 < sprite2Top || sprite1Top > sprite2Bottom || sprite1Bottom < sprite2Top && sprite1Right < sprite2Left
    ||  sprite1Top > sprite2Bottom && sprite1Left > sprite2Right) {
    return false;
  }
  else {
    return true;
  }
}

// Wall collision detection.
bool wallCollision(sprite_id sprite){
  int spriteLeft = sprite_x(sprite);
  int spriteRight = sprite_x(sprite) + sprite_width(sprite) - 1;
  // Return true on collision occurance.
  if (spriteRight > screen_width() - 3 || spriteLeft < 1) {
    return true;
  }
	else {
  	return false;
	}
}

//check the keys
void moveChar(void){
  int key = get_char();
  if (key == 'l') level ++;
  //if (!spriteCollision(hero, platform)) collision = false;
  if (wallCollision(hero)) {
    dx = 0;
    velocity = 0;
    sprite_back(hero);

  }
  else{
    if (air == false){
      if (key == KEY_UP){
          air = true;
          dy = -2;

      }
      //check right arrrow key
      if (key == KEY_RIGHT){
        if (right == true) velocity = 1;
        else if (left == true && velocity == 1) velocity = 0.5;
        else if (left == true && velocity == 0.5){
          left = false;
          right = false;
          velocity = 0;
        }
        else {
          left = false;
          right = true;
          velocity = 0.5;
        }

        if (velocity != 0 ){
          dx = velocity;

        }
        else {
          dx = 0;

        }
      }
      //check left arrow key
      else if (key == KEY_LEFT){
        if (left == true){
          left = true;
          right = false;
          velocity = 1;

        }
        else if (right == true && velocity == 1){
          velocity = 0.5;

        }
        else if (right == true && velocity == 0.5){
          left = false;
          right = false;
          velocity = 0;

        }
        else {
          left = true;
          right = false;
          velocity = 0.5;

        }
        if (velocity != 0 ){
          dx = velocity * -1;

        }
        else {
            dx = 0;

        }
      }
    }
    // if (air == true && collision == false) {
      // for (int i = 0; i <= 10; i++)
      // {
      //   if (spriteCollision(hero, platform[i])) {
      //     air = false;
      //     collision == true;
      //     dy = 0;
      //   }
      if (air == true) dy += gravity;
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
    draw_formatted(width * 2, 1, "Level: %d", level);
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

// Play one turn of game.
void process(void) {
  // Clear current Frame & then redraw.
  clear_screen();
  // Character movement.
  moveChar();
  sprite_turn_to(hero, dx, dy);
  sprite_step(hero);
  // Create game
  drawGame();
  // Display Screen.
  show_screen();
}

// Initial setup.
void setup(void) {
  // Useful variables.
  int width = screen_width(), heroWidth = HERO_WIDTH, zombieWidth = ZOMBIE_WIDTH;
  int height = screen_height(), heroHeight = HERO_HEIGHT, zombieHeight = ZOMBIE_HEIGHT;
  // (d) Set up the hero at the centre of the screen.
	hero = sprite_create(2 + heroWidth, height-heroHeight-1, heroWidth, heroHeight, charGround);
  // (e) Draw the game.
  drawGame();
  // Display Screen.
	show_screen();
}

// Main loop.
int main(void) {
  // Draw game environment
  setup_screen();
	setup();

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
