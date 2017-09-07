#include <math.h>
#include <stdlib.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <curses.h>

// TODO : ###
// Configuration.
#define DELAY (10)
#define HERO_WIDTH (3)
#define HERO_HEIGHT (3)
#define PLATFORM_HEIGHT (1)

// Initialising game variables.
bool game_over = false;
bool update_screen = true;
bool right = false;
bool left = false;
bool air = false;
bool collision = true;
bool topCollision = false;
bool ground = false;
bool roof = false;
bool switcher = true;
bool spriteDrawn = false;
double dx = 0;
double dy = 0;
double Edx = -0.15;
int Edy = 0;
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


// Declaring interactive sprites/ characters.
char * charGround =
/**/	" o "
/**/	"/|\\"
/**/	"/ \\";

char * charAir =
/**/ "\\o/ "
/**/	" | "
/**/	"/ \\";

char * batEnemy1 =
/**/	"\\ /"
/**/	 " | ";

char * batEnemy2 =
/**/	 " | "
/**/	 "/ \\";

char * boulderEnemy =
/**/	"/''\\"
/**/  "\\__/";

char * zombieEnemy =
/**/	"ZZZZ"
/**/	"  Z "
/**/	" Z  "
/**/	"ZZZZ";

char * platformBase =
/**/ "==========================================================================================================================";

char * exitImg =
/**/ "EXIT"
/**/ "|  |"
/**/ "| .|"
/**/ "|  |";

// Declaring sprites.
sprite_id hero; sprite_id enemy; sprite_id platform[10];
sprite_id door;

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
      if (spriteDrawn == false) {
        // Set up the hero at the left of the screen.
        hero = sprite_create(2 + HERO_WIDTH, screen_height()-HERO_HEIGHT-1, HERO_WIDTH, HERO_HEIGHT, charGround);
        // Set up zombie at the right of the screen.
        enemy = sprite_create(screen_width() - 9, screen_height() - 5, 4, 4, zombieEnemy);
        // Resetting value.
        spriteDrawn = true;
      }
      platform[0] = initPlatforms(0, screen_height() - 1, screen_width());
      platform[1] = initPlatforms((screen_width() * 0.3), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
      sprite_draw(platform[0]); sprite_draw(platform[1]);
      break;
    case 2:
      // level 2.
      if (spriteDrawn == false) {
        // Set up the hero at the left of the screen.
        hero = sprite_create(2 + HERO_WIDTH, screen_height()-HERO_HEIGHT-1, HERO_WIDTH, HERO_HEIGHT, charGround);
        // Set up zombie at the right of the screen.
        enemy = sprite_create(screen_width() - 9, screen_height() - 4, 3, 2, batEnemy1);
        // Resetting value.
        spriteDrawn = true;
      }
      platformAmount = 2;
      platform[0] = initPlatforms(0, screen_height() - 1, screen_width());
      platform[1] = initPlatforms((screen_width() * 0.3), screen_height() - (1 + (HERO_HEIGHT * 2)), (screen_width() * 0.3));
      // platform[1] = initPlatforms((screen_width() * 0.3), screen_height() - (1 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.3));
      platform[2] = initPlatforms((screen_width() * 0.35), screen_height() - (2 + (HERO_HEIGHT * 4)), (screen_width() * 0.2));
      // platform[2] = initPlatforms((screen_width() * 0.3), screen_height() - (2 + (HERO_HEIGHT * 7)), (screen_width() * 0.2));
      sprite_draw(platform[0]); sprite_draw(platform[1]); sprite_draw(platform[2]); break;
    case 3:
      // level 3.
      if (spriteDrawn == false) {
        // Set up the hero at the left of the screen.
        hero = sprite_create(2 + HERO_WIDTH, screen_height()-HERO_HEIGHT-1, HERO_WIDTH, HERO_HEIGHT, charGround);
        // Set up zombie at the right of the screen.
        sprite_create(screen_width() - 9, (screen_height() - (2 + (HERO_HEIGHT * 7) - 1)), 3, 2, batEnemy1);
        // Resetting value.
        spriteDrawn = true;
      }
      platformAmount = 3;
      platform[0] = initPlatforms(0, screen_height() - 1, screen_width() * 0.25);
      platform[1] = initPlatforms(screen_width() * 0.75, screen_height() - 1, screen_width() * 0.25);
      platform[2] = initPlatforms(screen_width() * 0.25, screen_height() - (2 + (HERO_HEIGHT * 3.5)), (screen_width() * 0.5));
      platform[3] = initPlatforms(((screen_width() * 0.5) / 2) * 2, screen_height() - (2 + (HERO_HEIGHT * 7)), (screen_width() * 0.5));
      sprite_draw(platform[0]); sprite_draw(platform[1]); sprite_draw(platform[2]); sprite_draw(platform[3]); break;
    case 4:
      // Level 4.
      break;
    case 5:
      // Level 5.
      break;
  }
}

// Destroys all the sprites.
void destroyGame(void) {
  for (int i = 0; i <= platformAmount; i++){
    sprite_destroy(platform[i]);
  }
  sprite_destroy(enemy); sprite_destroy(hero); sprite_destroy(door);
  spriteDrawn = false;
}

// Map Treasure
void treasure(void) {
  if (level == 3) {

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

// Function for enemy movement.
void enemyMovement(sprite_id opponent) {
  if (wallCollision(enemy)) {
    if (sprite_x(enemy) < screen_width() * 0.5) {
      if (switcher == false) {
        if (level == 2) sprite_create(screen_width() - 9, screen_height() - 4, 3, 2, batEnemy1);
      }
      Edx = 0.15;
      switcher = false;
    }
    else if (sprite_x(enemy) > screen_width() * 0.5) {
      Edx = -0.15;
      switcher = false;
    }
  }
  if (level == 5) {
    // ### add dy physics here.
  }
}

// X-value collision detection.
bool xCollision(sprite_id sprite1, sprite_id sprite2){
  // Sprite 1.
  sprite1Left = sprite_x(sprite1);
  sprite1Right = sprite_x(sprite1) + sprite_width(sprite1);
  // Sprite 2.
  sprite2Left = sprite_x(sprite2);
  sprite2Right = sprite_x(sprite2) + sprite_width(sprite2);
  // If collision is occuring.
  if (sprite1Left <= sprite2Right + 1 && sprite1Right >= sprite2Left - 1) return true;
  else return false; // Else return no collision.
}

// Y-value collision detection.
bool yCollision(sprite_id sprite1, sprite_id sprite2){
  // Sprite 1.
  sprite1Bottom = round(sprite_y(sprite1)) + HERO_HEIGHT; // ### fix here for Y value on plat ( I think )
  sprite1Top = round(sprite_y(sprite1));
  // Sprite 2.
  sprite2Bottom = round(sprite_y(sprite2)); // stops head from glitching
  sprite2Top = round(sprite_y(sprite2));
  // if (air == true && sprite1Bottom == sprite2Top + 1) { // ### collision detected when you jump off
  //   roof = false; ground = true; return true;
  // }
  // else if (sprite1Bottom == sprite2Top) {
  if (sprite1Bottom == sprite2Top) {
    roof = false; ground = true; return true;
  }
  // If collision with bottom of platform.
  else if (sprite1Top == sprite2Bottom + 1) {
    roof = true; ground = false; return true;
  }
  else {
    roof = false; ground = false; return false;
  }
}

// Checks all user movements and inputs.
void moveChar(void){
  // User input.
  int key = get_char();
  // Level indicator.
  if (key == 'l') {
    level ++;
    destroyGame();
  }
  // Collision checks between platform/s and hero.
  for (int i = 0; i <= platformAmount; i++){
    if (xCollision(hero, platform[i])){
      if (yCollision(hero, platform[i])){
          if (roof == true){
            sprite_back(hero);
            dy += gravity;
            velocity = 0;
          }
          else if (ground == true){
            dy = 0;
            air = false;
          }
        }
      }
    }
  // Gravity if character is in air.
  if (ground == false && roof == false) { // ### fix here for bug with jumping off platform.
    dy += gravity;
    // hero = sprite_create(sprite_y(hero), sprite_x(hero), HERO_WIDTH, HERO_HEIGHT, charAir);
    // ### Check to see if this collides with other DY
    // and that is why bug is occuring.
  }
  // Wall collision.
  if (sprite1Right > screen_width() - 3 || sprite1Left < 1){
    if (air == true) sprite_back(hero);
    dx = 0;
    velocity = 0;
    sprite_back(hero);
  } // ### changing this to else if fixes level 1 platform glitch but destroys character movement
  // Roof collision.
  else if (sprite_y(hero) <= 3) {
    dy += gravity;
    velocity = 0;
    sprite_back(hero);
  }
  // Enemy collision.
  else if (xCollision(hero, enemy) && yCollision(hero, enemy)){
    lives -= 1;
    destroyGame();
  }
  // Door collision.
  else if (xCollision(hero, door) && yCollision(hero, door)){
    level += 1;
    score += 100;
    destroyGame();
  }
  // Else, continue with user controls.
  // else if (air == true && ground == false && roof == false && dy >) dy = 0;
  else {
    if (air == false){
      // Checks for left arrow input.
      if (key == KEY_LEFT){
        if (left == true){
          left = true;
          right = false;
          velocity = 1;
        }
        else if (right == true && velocity == 1) velocity = 0.5;
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
        if (velocity != 0) dx = velocity * -1;
        else dx = 0;
      }
      // Checks for right arrow input.
      else if (key == KEY_RIGHT){
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
        if (velocity != 0 ) dx = velocity;
        else dx = 0;

      }
      // Checks for up arrow input.
      else if (key == KEY_UP){
          air = true;
          dy = -1.7;

      }
    }
  }
}

// Creates Exit door.
void createExit(void){
  door = sprite_create(screen_width() - 8, screen_height() - 5, 4, 4, exitImg);
  sprite_draw(door);
 }

// Ends game when player reaches end or runs out of lives.
void endGame(void){
  if (lives == 0 || level >= 6) {
    destroyGame();

  }
}

// Draws components of game.
void drawGame(void) {
  // Creates Arena elements.
	drawArena();
  DrawPlatforms();
  createExit();
  // Draws previously initialised sprites.
	sprite_draw(hero);
  sprite_draw(enemy);
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
  int Edx = sprite_dx(enemy);
  int Edy = sprite_dy(enemy);
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
  draw_formatted(5, 8, "S1: top: %02d , bottom: %02d, right: %02d, left: %02d",sprite1Top, sprite1Bottom, sprite1Right, sprite1Left );
  draw_formatted(5, 9, "S2: bottom: %02d , top: %02d, right: %02d, left: %02d",sprite2Bottom, sprite2Top, sprite2Right, sprite2Left );

  if (xCollision(hero, platform[1]) == true && yCollision(hero, platform[1]) == true) draw_string(50, 3, "Collision detected (X & Y)");
  else if (xCollision(hero, platform[0])) draw_string(50, 3, "Collision detected (X)");
  else if (yCollision(hero, platform[0])) draw_string(50, 3, "Collision detected (Y)");

  if (ground == true) draw_string(50, 5, "colliding with ground");
  else draw_string(50, 5, "not colliding with ground");
  // draw_formatted(35, 7, "Enemy Left: %02d , Enemy Right = %02d", round(sprite_x(enemy)), round(sprite_x(enemy) + sprite_width(enemy)));
  draw_formatted(45, 7, "Enemy dx: %02d , Enemy dy = %02d",Edx, Edy);
  if (roof == true) draw_string(50, 4, "colliding with roof");
  else draw_string(50, 4, "not colliding with roof");

  if (air == true) draw_string(50, 6, "jumping");
  else draw_string(50, 6, "not jumping");
}

// Play one turn of game.
void process(void) {
  // Clear current Frame & then redraw.
  clear_screen();
  // Create game.
  drawGame();
  // Enemy movement.
  enemyMovement(enemy);
  sprite_turn_to(enemy, Edx, round(Edy));
  sprite_step(enemy);
  // Character movement.
  moveChar();
  sprite_turn_to(hero, dx, round(dy));
  sprite_step(hero);
  // Debugger.
  display_debug_data();
}

// Initial setup.
void setup(void) {
  // Set up the hero at the left of the screen.
  hero = sprite_create(2 + HERO_WIDTH, screen_height()-HERO_HEIGHT-1, HERO_WIDTH, HERO_HEIGHT, charGround);
  // Set up zombie at the right of the screen.
  enemy = sprite_create(screen_width() - 9, screen_height() - 5, 4, 4, zombieEnemy);
  // Draw the game.
  drawGame();
  // Display Screen.
	show_screen();
}

// Main loop.
int main(void) {
  // Draw game environment.
  setup_screen();
	setup();

	while ( !game_over ) {
    // Continue drawing frames.
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
