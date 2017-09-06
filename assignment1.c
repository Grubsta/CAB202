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
bool topCollision = false;
bool ground = false;
bool roof = false;
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
int platformAmount = 1;

int sprite1Bottom;
int sprite1Top;
int sprite1Left;
int sprite1Right;

int sprite2Bottom;
int sprite2Top;
int sprite2Left;
int sprite2Right;

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
      platform[0] = initPlatforms(0, screen_height() - 1, screen_width());
      platform[1] = initPlatforms((screen_width() * 0.3), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
      sprite_draw(platform[0]); sprite_draw(platform[1]); break;
    case 2:
      // level 2.
      platform[0] = initPlatforms(0, screen_height(), screen_width());
      platform[1] = initPlatforms((screen_width() * 0.4), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
      platform[2] = initPlatforms((screen_width() * 0.3), screen_height() - (2 + (HERO_HEIGHT * 7)), (screen_width() * 0.2));
      sprite_draw(platform[0]); sprite_draw(platform[1]); sprite_draw(platform[2]); break;
    case 3:
      // level 3.
      platform[0] = initPlatforms(0, screen_height(), screen_width());
      platform[1] = initPlatforms((screen_width() * 0.4), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
      platform[2] = initPlatforms((screen_width() * 0.3), screen_height() - (2 + (HERO_HEIGHT * 7)), (screen_width() * 0.2));
      sprite_draw(platform[0]); sprite_draw(platform[1]); sprite_draw(platform[2]); break;
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
  // int sprite2Bottom = sprite_y(sprite2) + 1;
  // int sprite2Top = sprite_y(sprite2);
  // int sprite2Left = sprite_x(sprite2);
  // int sprite2Right = sprite_x(sprite2) + sprite_width(sprite2) - 1;
  int sprite2Bottom = sprite_y(sprite2) + 1;
  int sprite2Top = sprite_y(sprite2);
  int sprite2Left = sprite_x(sprite2);
  int sprite2Right = sprite_x(sprite2) + sprite_width(sprite2) - 1;
  // Collision occurance will output false.
  if (sprite1Bottom < sprite2Top || sprite1Bottom < sprite2Top && sprite1Right < sprite2Left
    ||  sprite1Top > sprite2Bottom && sprite1Left > sprite2Right) {
    if  (sprite1Top > sprite2Bottom) topCollision = true;
    return false;
  }
  else {
    return true;
  }
}

bool xCollision(sprite_id sprite1, sprite_id sprite2){
  sprite1Left = sprite_x(sprite1);
  sprite1Right = sprite_x(sprite1) + sprite_width(sprite1) - 1;
  sprite2Left = sprite_x(sprite2);
  sprite2Right = sprite_x(sprite2) + sprite_width(sprite2) - 1;
  if (sprite1Left <= sprite2Right + 1 && sprite1Right >= sprite2Left - 1) return true;
  else return false;
}

bool yCollision(sprite_id sprite1, sprite_id sprite2){
  sprite1Bottom = sprite_y(sprite1);
  sprite1Top = sprite_y(sprite1);
  sprite2Bottom = sprite_y(sprite2);
  sprite2Top = sprite_y(sprite2);
  if (sprite1Bottom == sprite2Top - 1) {
    roof = false; ground = true; return true;
  }
  else if (sprite1Top == sprite2Bottom + 1) {
    roof = true; ground = false; return true;
  }
  else {
    roof = false; ground = false; return false;
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
  // User input.
  int key = get_char();
  // Level indicator.
  if (key == 'l') level ++;
  // Checking each platform on level.
  // for (int i = 0; i < platformAmount; i++) {
  //   // i screws with collision halfway through platform.
  //   if (spriteCollision(hero, platform[i])) {
  //     if (topCollision == true) {
  //       dy = 0;
  //       velocity = 0;
  //       topCollision = false;
  //       sprite_back(hero);
  //     }
  //   }
  // }
  // If hero collides with wall.

  // for (int i = 0; i <= platformAmount; i++){
  if (xCollision(hero, platform[0])){
      if (yCollision(hero, platform[0])){
        if(roof == true){
          dy += gravity;
          velocity = 0;
          sprite_back(hero);
        }
        else if(ground ==  true){
          dy = 0;
          air = false;
          sprite_back(hero);
        }
        // else sprite_back(hero);
      }
    }
  if (xCollision(hero, platform[1])){
      if (yCollision(hero, platform[1])){
        if(roof == true){
          dy += gravity;
          velocity = 0;
        }
        else if(ground ==  true){
          dy = 0;
          air = false;
          sprite_back(hero);
        // }
      }
    }
  }
  if (wallCollision(hero)) {
    dx = 0;
    velocity = 0;
    sprite_back(hero);
  }
  // If the characters head hits platform.
  // else if (topCollision == true) dy += gravity;
  // // If character is in air and has collided.
  // else if (air == true && spriteCollision(hero, platform[0])){
  //   air = false;
  //   collision == true;
  //   dy = 0;
  //   sprite_back(hero);
  // }
  // If no collisions occur but char in air increase gravity.
  else if (air == true) dy += gravity;
  // Else, continue with user controls.
  else{
    if (air == false){
      if (key == KEY_UP){
          air = true;
          dy = -1.7;

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

// Prints Debug information to screen.
void display_debug_data() {
    double dx = sprite_dx(hero);
    double dy = sprite_dy(hero);
    int x = sprite_x(hero);
    int y = sprite_y(hero);
    draw_string(5,3, "Debug Data");
    draw_string(5,4, "Player dx: ");
    draw_double(20, 4, dx);
    draw_string(5,5, "Player dy: ");
    draw_double(20, 5, dy);
    draw_string(5,6, "Player X pos: ");
    draw_int(20, 6, x);
    draw_string(5,7, "Player Y pos: ");
    draw_int(20, 7, y);
    draw_formatted(5, 8, "S2: bottom: %02d , top: %02d, right: %02d, left: %02d", sprite2Top, sprite2Bottom, sprite2Right, sprite2Left);
    draw_formatted(5, 9, "S1: bottom: %02d , top: %02d, right: %02d, left: %02d", sprite1Top, sprite1Bottom, sprite1Right, sprite1Left);
    if (ground == false) {
     draw_string(50, 5, "Player colliding with ground");
   } else {
     draw_string(50, 5, "Player is off ground");
   }
   if (air == true) {
       draw_string(50, 6, "Player is jumping");
   } else {
       draw_string(50, 6, "Player is not jumping");
   }
  }

// Play one turn of game.
void process(void) {
  // Clear current Frame & then redraw.
  clear_screen();
  // Character movement.
  moveChar();
  sprite_turn_to(hero, dx, round(dy));
  sprite_step(hero);
  // Create game.
  drawGame();
  // Debugger.
  display_debug_data();

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
