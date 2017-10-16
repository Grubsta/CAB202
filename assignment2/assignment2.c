// ANSI Tower for a Teensy utilising a PewPew board.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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

// Initialise timer macros.
#define FREQ 8000000.0
#define PRESCALE 256.0
#define TIMER_SCALE 256.0

// Configuration (sprite L & W) macros.
#define HW 16 // Hero.
#define HH 8
#define TH 20 // Tower.
#define TW 80
#define DH 10 // Door.
#define DW 24
#define EH 5 // Enemy.
#define EW 8
#define KH 3 // Key.
#define KW 8
#define VWH 25 // Vetical wall.
#define VWW 3
#define HWH 3 // Horizontal wall.
#define HWW 25
// Threshold for button presses.
#define thresh (1000)


// TODO : ###
// FIX SCREEN movement
// FIX RANDOM generator
// FIX COLLISION Y value
// ADD ARROWS TO bow
// ADD BOMB FUNCTION

// NEED TO KNOWS
// SCREEN = 84x48

// Global variables.
// Location / movement.
float speed = 1.0;
float dx = 0;
float dy = 0;
int dxdy[1];
// Player.
int level = 2, lives = 3, score = 0;
// Timer.
int seconds = 0, minutes = 0;
int totalSeconds = 0, totalMinutes = 0;
double interval = 0;
// Sprite amounts.
uint8_t enemyAm = 0, treasureAm = 0, wallAm = 2;
// Gameplay / Collisions.
int herox, heroy;
int keyx, keyy;
// int XYarray[50];
int screenX = 0, screenY = 0;
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
// Game engine.
uint16_t closedCon = 0;
uint16_t openCon = 0;
bool resetGame = false;
// Shooting mechanism values.
bool shot = false;
int hx, hy, cx, cy;
// String output.
char printArray[20];
// Seed generator.
int seed = 2313;
// Linked list for wall positions.
struct node *end=NULL;
typedef struct node {
    int val;
    struct node * next;
} Node;

Node * wallXCoords = NULL;
Node * wallYCoords = NULL;

int wallX1 = -33, wallX2 = 117;
int wallY1 = -21, wallY2 = 69;

// Initialise sprites.
Sprite hero; Sprite tower; Sprite door; Sprite key;
Sprite enemy[5]; Sprite treasure[5]; Sprite wall[5];
Sprite shield; Sprite bow; Sprite bomb; Sprite crosshair;
Sprite arrow;

Node *insert(int i) {
    Node *last_node = NULL;
    Node *first_node = NULL;
    for (int c = 0; c < i; c++) {
        Node *node = malloc(sizeof(Node));
        node->val = i;
        node->next = NULL;
        if (!last_node) {
            // Remember the first node, so we can return it.
            first_node = node;
        }
        else {
            // Otherwise, append to the existing list.
            last_node->next = node;
        }
        last_node = node;
    }
    return first_node;
}

// Initialise hero.
void initHero(void) {
	int x = LCD_X / 2 - HW / 2;
	int y = LCD_Y / 2 + HH + 3;
  herox = x; heroy = y;
	sprite_init(&hero, x, y, HW, HH, heroBitmap);
}


// Causes the sprites to magically dissapear
void spriteMagic(Sprite sprite) {
  sprite.x = 30;
  sprite.y = -1000;
  free(&sprite); // #####
}


// Colisions for static map edges.
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


// Sends inputted string via serial connection.
void usb_serial_send(char * message) {
	usb_serial_write((uint8_t *) message, strlen(message));
}

// Collision detection between 2 sprites. Added gap incase a extended
// perimeter is necessary.
bool gapCollision(Sprite sprite1, Sprite sprite2, int gap) {
  // Sprite 1.
  int spr1Bottom = round(sprite1.x + sprite1.height);
  int spr1Top = round(sprite1.y) - gap; /////##################
  int spr1Left = round(sprite1.x) - gap;
  int spr1Right = round(sprite1.x) + sprite1.width + gap;
  // Sprite 2.
  int spr2Bottom = round(sprite2.y) + sprite2.height + gap;
  int spr2Top = round(sprite2.y);
  int spr2Left = round(sprite2.x) - gap;
  int spr2Right = round(sprite2.x) + sprite2.width + gap;
  // sprintf(printArray, "S2: B = %d, T = %d\r\n", spr2Bottom, spr2Top);
  // usb_serial_send(printArray);

  // Creates a perimter arround sprites and checks for collision.
	if (spr1Bottom > spr2Top && spr1Top < spr2Bottom && spr1Right > spr2Left&& spr1Left < spr2Right) {
		return true;
	}
	else {
		return false;
	}
}


// Explosion caused by bomb on impact.
void explosion(void) {
  // Enemy collisions.
    for (int i = 0; i < 6; i++) {
      if (gapCollision(bomb, enemy[i], 3)) {
        spriteMagic(enemy[i]);
        score += 10;
      }
    }
    // Destroy bomb.
    spriteMagic(bomb);
    // Flicker LEDs
    SET_BIT(PORTB, 2); _delay_ms(250); CLEAR_BIT(PORTB, 2);
    SET_BIT(PORTB, 3); _delay_ms(250); CLEAR_BIT(PORTB, 3);
    SET_BIT(PORTB, 2); _delay_ms(250); CLEAR_BIT(PORTB, 2);
    SET_BIT(PORTB, 3); _delay_ms(250); CLEAR_BIT(PORTB, 3);
}


// Checking for all sprite collisions.
bool spriteCollision(Sprite sprite) {
  for (int i = 0; i < enemyAm; i++) {
    if (gapCollision(sprite, enemy[i], 0)) return true;
  }
  send_str(PSTR("passed enemy\r\n"));
  for (int i = 0; i < treasureAm; i++) {
    if (gapCollision(sprite, treasure[i], 0)) return true;
  }
  send_str(PSTR("passed treasure\r\n"));
  for (int i = 0; i < wallAm; i++) {
    if (gapCollision(sprite, wall[i], 0)) return true;
  }
  send_str(PSTR("passed wall\r\n"));
  if (gapCollision(sprite, bomb, 0)) return true;
  send_str(PSTR("passed bomb\r\n"));
  if (gapCollision(sprite, bow, 0)) return true;
  send_str(PSTR("passed bow\r\n"));
  if (gapCollision(sprite, shield, 0)) return true;
  send_str(PSTR("passed shield\r\n"));
  if (gapCollision(sprite, key, 0)) return true;
  send_str(PSTR("passed key\r\n"));
  if (gapCollision(sprite, door, 0)) return true;
  send_str(PSTR("passed door\r\n"));
  send_str(PSTR("PASSED COLLISIONS\r\n"));
  return false;

}


// Moves enemy sprite towards hero's location.
void enemyMovement() {
	float enemySpeed = 0.1;
  if (level == 1) enemyAm = 1;
	for (int i = 0; i < enemyAm; i++) {
    if ((enemy[i].x > LCD_X - 84 || enemy[i].x > LCD_X) && (enemy[i].y > LCD_Y - 44 || enemy[i].y < LCD_Y)){
      if (enemy[i].x < hero.x) enemy[i].x += enemySpeed;
      else if (enemy[i].x > hero.x) enemy[i].x -= enemySpeed;
      if (enemy[i].y < hero.y) enemy[i].y += enemySpeed;
      else if (enemy[i].y > hero.y) enemy[i].y -= enemySpeed;
    }
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
    cx = crosshair.x; cy = crosshair.y;
  }
  if (!crosshairInit) sprite_init(&crosshair, LCD_X * 0.5, LCD_Y * 0.5, 3, 3, crosshairBitmap);
  // User input.
  if ((BIT_IS_SET(PINF, 6) || 	BIT_IS_SET(PINF, 5)) && shot == false) {
    shot = true;
    // Checks in which direction the crosshair is from player.
    if (cx < hx) sx = hx - 5 - 2; // 2 being the shot's width.
    else if (cx > hx) sx = hx + HW + 5;
    if (cy < hy) sy = hy - 5;
    else if (cy > hy) sy = hy + HW + 5;
    // Determines what projectile is currently equipt.
    if (bowTrailed) {
      sprite_init(&arrow, sx, sy, 3, 3, arrowBitmap);
      sprite_draw(&arrow);
      send_str(PSTR("The arrow has left the building.\n"));
    }
    else {
      sprite_init(&bomb, sx, sy, 6, 4, arrowBitmap);
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
    arrow.x = sx; arrow.y = sy;
    // The projectile has hit the target.
    if (xHit && yHit) {
      if (bowTrailed) {
        for (int i = 0; i < 6; i++) { // 6 being enemyAm - 1
          if (gapCollision(arrow, enemy[i], 1)) {
            spriteMagic(enemy[i]);
            score += 10;
            send_str(PSTR("An enemy has been shot till death by the hero.\n"));
          }
        }
        spriteMagic(arrow);
      }
      if (bombTrailed) {
        for (int i = 0; i < 6; i++) {
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
  // bool l = false, r = false, u = false, d = false;
	if (hero.x < round(LCD_X * 0.20)) x += 1;
	if (hero.x + HW > round(LCD_X * 0.80)) x -= 1;
	if (hero.y < round(LCD_Y * 0.20)) y += 1;
	if (hero.y + HH > round(LCD_Y * 0.80)) y -= 1;
  screenX += x; screenY += y;
  if (screenX <= -33 || screenX >= 33) x = 0; // #### DEBATABLY WORKING
  if (screenY <= -21 || screenY >= 21) y = 0;
	moveAll(x, y);
}


// Random wall generator which works by added
// each X & Y location the sprite takes up and
// checking if these positions have already been filled.
// NEEEEEEEEEED TOOOO FIX THIS #############
bool wallShiz(Sprite sprite1, int i, int gap) {
	for (int a = sprite1.x - gap; a < sprite1.x + sprite1.width + gap; a++) {
		for (int i = 0; i <= sizeof(wallXCoords); i++) {
			if (a == wallXCoords->val) {
        send_str(PSTR("X COLL\r\n"));
        for (int b = sprite1.y - gap; b < sprite1.y + sprite1.height + gap; b++) {
          for (int i = 0; i <= sizeof(wallYCoords); i++) {
            if (b == wallYCoords->val) {
              send_str(PSTR("X & Y COLL MATE\r\n"));
              return true;
            } wallYCoords = wallYCoords->next;
          }
        }
      }
      wallXCoords = wallXCoords->next;
    }
	}
  send_str(PSTR("APPENDING\r\n"));
  // while (wallXCoords->val != NULL) {
  //   wallYCoords = wallYCoords->next;
  // }
  for (int a = sprite1.x - gap; a < sprite1.x + sprite1.width + gap; a++) {
    wallXCoords->next->val = a;
    if (a > (120 || -50)) send_str(PSTR("ERROR: Wall X out of bounds\r\n"));
    wallXCoords->next = NULL;
  }
  for (int a = sprite1.y - gap; a < sprite1.y + sprite1.height + gap; a++) {
    wallYCoords->next->val = a;
    if (a > (120 || -50)) send_str(PSTR("ERROR: Wall Y out of bounds\r\n"));
    wallYCoords->next = NULL;
  }
  send_str(PSTR("NO COLLISION MATE\r\n"));
  // for (int i = 0; i <= sizeof(wallXCoords); i++) {
  //   int a = wallXCoords->val;
  //   sprintf(printArray, "X - value[%d]: %d\r\n", i, a);
  //   usb_serial_send(printArray);
  //   wallXCoords = wallXCoords->next;
  // }
  // for (int i = 0; i <= sizeof(wallYCoords); i++) {
  //   int a = wallYCoords->val;
  //   sprintf(printArray, "Y - value[%d]: %d\r\n", i, a);
  //   usb_serial_send(printArray);
  //   wallYCoords = wallYCoords->next;
  // }
	return false;
}


// Initialises 6 walls randomly across playable area.
void wallInit(void) {
  send_str(PSTR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n"));
  sprintf(printArray, "WALL AMOUNT = %d\r\n", wallAm);
  usb_serial_send(printArray);
  wallAm = 6;
	int x, y;
	// int drawnWall = 0;
  int gap = 5;
	bool valid = true;
	for (int i = 0; i < wallAm; i++) {
		do {
      sprintf(printArray, "ATTEMPTING WALL no. = %d\r\n", i);
      usb_serial_send(printArray);
			x = rand() % wallX2 + (wallX1 * 1);
      x += wallX1;
			y = rand() % wallY2 + (wallY1 * 1);
      y += wallY1;
			int direction = rand() % 10;
      // if (direction > 5 && x < wallX2 && x > wallX1 && y < wallY2 && y > wall)
			if (direction > 5) { // Vertical direction.
        send_str(PSTR("VERT.\r\n"));
				sprite_init(&wall[i], x, y, VWW, VWH, vertWallBitmap);
				if (wallShiz(wall[i], i, gap)) valid = false;
			} else { // Horizontal direction.
        send_str(PSTR("NOT VERT.\r\n"));
				sprite_init(&wall[i], x, y, HWW, HWH, horWallBitmap);
				if (wallShiz(wall[i], i, gap)) valid = false;
			}
      // if (spriteCollision(wall[i])) valid = false;
		} while (!valid);
    send_str(PSTR("ACCCCCCCCCCCCCTTTTTTTTTTTTUALLLY WORKED WHAT\r\n"));
	}
  send_str(PSTR("FINISHED INITIALISING WALLS - TIME to DRAW\r\n"));
  for (int i = 0; i < wallAm; i++) {
    sprite_draw(&wall[i]);
  }
  send_str(PSTR("DRAWN\r\n"));
	wallInitialised = true;
}


// Produces random x value between game size.
int randX(void) {
  seed += 1;
  srand(interval * seed);
  int x = rand() % (wallX2 + (wallX1 + 8 * 1));
  return x;
}


// Produces random y value between game size.
int randY(void) {
  seed += 1;
  srand(interval * seed);
  int y = rand() % (wallY2 + (wallY1 * 1));
  return y;
}


// Initialises defence items.
void defenceInit(void) {
  bool valid = false;
  int gen, x, y;
  // Randomising both the position and chance of spawn.
  gen = rand() % 100;
  gen = 29;
  if (gen <= 29) {
    while (!valid) {
      x = randX(); y = randY();
      // x = LCD_X * 0.8; y = LCD_Y * 0.3; // ####
      sprite_init(&bow, x, y, 8, 3, bowBitmap);
      if (!spriteCollision(bow)) valid = true;
      valid = true;
    }
  }
  valid = false;
  gen = rand() % 100;
  gen = 29;
  if (gen <= 29) {
    while (!valid) {
      x = randX(); y = randY();
      // x = LCD_X * 0.8; y = LCD_Y * 0.6; // ####
      sprite_init(&bomb, x, y, 6, 4, bombBitmap);
      if (!spriteCollision(bomb)) valid = true;
      valid = true;
    }
  }
  valid = false;
  gen = rand() % 100;
  if (gen <= 29) {
    while (!valid) {
      sprite_init(&shield, x, y, 8, 4, shieldBitmap);
      if (!spriteCollision(shield)) valid = true;
    }
  }
}


// Initialises all the enemy sprites.
void enemyInit(void) { ///### inits broken, fix later
  int x, y;
  // srand(minutes * seed - seconds);
  enemyAm = rand() % 6;
  // enemyAm = 2;
  for (int i = 0; i < enemyAm; i++) {
    bool valid = false;
    sprintf(printArray, "ENEMY no. = %d\r\n", i);
    usb_serial_send(printArray);
    while (!valid) {
      x = randX(); y = randY();
      // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      // x += wallX1; y += wallY1;
      sprite_init(&enemy[i], x, y, EW, EH, enemyBitmap);
      if (!spriteCollision(enemy[i])) valid = true;
      send_str(PSTR("FAILED\r\n"));
    }
  }
}


// Initialises all the treasure sprites.
void treasureInit(void) {
  int x, y;
  // srand(minutes * seed - seconds);
  treasureAm = rand() % 6;
  for (int i = 0; i < treasureAm; i++) {
    bool valid = false;
    while (!valid) {
      sprintf(printArray, "TREASURE no. = %d\r\n", i);
      usb_serial_send(printArray);
      x = randX(); y = randY();
      // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      // x += wallX1; y += wallY1;
      sprite_init(&treasure[i], x, y, 8, 3, treasureBitmap);
      if (!spriteCollision(treasure[i])) valid = true;
      send_str(PSTR("FAILED\r\n"));
    }
  }
}


// Initialises door sprite.
void doorInit(void) {
  int x, y;
  bool valid = false;
  while (!valid) {
    x = randX(); y = randY();
      // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      // x += wallX1; y += wallY1;
    sprite_init(&door, x, y, DW, DH, doorBitmap);
    if (!spriteCollision(door)) break;
    send_str(PSTR("FAILED\r\n"));
  }
}


// Initialises key sprite.
void keyInit(void) {
  int x, y;
  // srand(minutes * seed - seconds);
  bool valid = false;
  while (!valid) {
    x = randX(); y = randY();
    // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
    // x += wallX1; y += wallY1;
    sprite_init(&key, x, y, KW, KH, keyBitmap);
    if (!spriteCollision(key)) break;
    send_str(PSTR("FAILED\r\n"));
  }
}


// Initialises all the sprites on the map.
void mapInit(void) {
  send_str(PSTR("DOOR\r\n"));
  doorInit();
  send_str(PSTR("KEY\r\n"));
  keyInit();
  send_str(PSTR("TREASURE\r\n"));
  treasureInit();
  send_str(PSTR("ENEMY\r\n"));
  enemyInit();
  send_str(PSTR("WALL\r\n"));
	wallInit();
  send_str(PSTR("HERO\r\n"));
	initHero();
  send_str(PSTR("DEFENCE\r\n"));
  defenceInit();
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
		for (int i = 0; i < 5; i++) sprite_draw(&wall[i]); // ###
		for (int i = 0; i < treasureAm; i++) sprite_draw(&treasure[i]);
		sprite_draw(&door); sprite_draw(&key);
    if (!shieldTrailed) sprite_draw(&shield);
    if (!bowTrailed) sprite_draw(&bow);
    if (!bombTrailed) sprite_draw(&bomb);
  }
}


// Enables sprites to trail sprites.
void spriteTrail(Sprite sprite1) {
  int x, y;
  if (sprite1.bitmap == keyBitmap) {
     x = hero.x + HW + 2;  y = hero.y + HH + 3;
  }
  else {
     x = hero.x - sprite1.width - 3;  y = hero.y + HH + 3;
  }
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
	char lev[20]; char liv[20]; char scor[20]; char timer[20];
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
  for (int i = 0; i < 6; i++) {
    if (gapCollision(hero, treasure[i], 1)) {
      spriteMagic(treasure[i]);
      score += 10;
    }
  }
  if (level > 1) {
    if (gapCollision(hero, bomb, 1)) {
      if (!bombTrailed) {
        bombTrailed = true; shieldTrailed = false; bowTrailed = false;
        send_str(PSTR("The hero has located da bomb.\r\n")); //####
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
      initHero(); // ###
      screenX = 0; screenY = 0;
      // mapInitialised = false;
      // destroyGame(); // ### FIX AFTER DEBUGGA
    }
	}
	else if (gapCollision(hero, door, 0)) {
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
  if (keyColl) spriteTrail(key);

	scrollMap();
	staticMap();
  sprite_draw(&hero);
}


// Reset required global variables to restart the game.
void resetVars(void) {
  level = 2, lives = 3, score = 0;
  seconds = 0, minutes = 0;
  screenX = 0, screenY = 0;
  keyColl = false;
  activated = false;
  lvlInit = false;
  wallInitialised = false;
  mapInitialised = false;
  enemyInitialised = false;
  keySpawn = false;
  bombSpawn = false;
  shieldSpawn = false;
  spriteTrailed = false;
  bowTrailed = false;
  bombTrailed = false;
  shieldTrailed = false;
  keyTrailed = false;
  crosshairInit = false;
  resetGame = true;
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



// Serial output for common game stats.
void serialOutput(void) {
  int x = round(hero.x); int y = round(hero.x);
  int min = totalMinutes + minutes;
  int sec = totalSeconds + seconds;
  char printString[40];
  sprintf(printString, "run-time(m:s): %02d:%02d\r\n", min, sec);
  usb_serial_send(printString);
  sprintf(printString, "Score: %d\r\n", score);
  usb_serial_send(printString);
  sprintf(printString, "Level: %d\r\n", level);
  usb_serial_send(printString);
  sprintf(printString, "X,Y Location: %d,%d\r\n", x, y);
  usb_serial_send(printString);
  sprintf(printString, "Remaining lives: %d\r\n\r\n", lives);
  usb_serial_send(printString);

}


// Overflow timer.
ISR(TIMER0_OVF_vect) {
	interval += TIMER_SCALE * PRESCALE / FREQ;
	if ( interval >= 1.0 ) {
		interval = 0;
    seconds++;
		PORTD ^= 1 << 6;
    serialOutput();
	}
  if (interval > 0.495 && interval < 0.505) {
    serialOutput();
  }
  if (seconds == 60) {
  seconds = 0;
  minutes++;
    if (minutes == 100) {
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
  // LEDs.
  SET_BIT(DDRB, 2);
  SET_BIT(DDRB, 3);
}

// Setup (ran on start).
void setup(void) {
  // srand(45387 * seconds * minutes);
  // Set CPU speed.
  set_clock_speed(CPU_8MHz);
  // Set Timer 0 to overflow approx 122 times per second.
  TCCR0B |= 4;
  TIMSK0 = 1;
  // Enable interrupts.
  sei();
  // Initialise different connected devices.
  if (!resetGame){
    usb_init();
    adc_init();
    lcd_init(LCD_DEFAULT_CONTRAST);
    // timer();
    DDRD |= (1<<6);
    PORTD |= (1<<6);
    // Display menu.
    clear_screen();
    draw_string(0, 10, "Connect to a ", FG_COLOUR);
    draw_string(0, 20, "serial terminal", FG_COLOUR);
    draw_string(0, 30, "to continue", FG_COLOUR);
    show_screen();
    // Wait for usb Configuration.
    while(!usb_configured());
    while(!(usb_serial_get_control() & USB_SERIAL_DTR)) usb_serial_flush_input();
    // Turn light off.
    PORTD &= ~(1<<6);
    send_str(PSTR("Welcome to ANSI\r\n"));
    initControls();
  }
	// welcomeScreen();
  clear_screen();
  drawLvl();
  // sprite_draw(&hero);
  show_screen();
}


// Game over menu.
void gameOverScreen(void) {
	clear_screen();
	char lev[50]; char scor[50];
	draw_string(0, 0, "You died in ANZI!", FG_COLOUR);
	sprintf(lev, "level: %d", level); draw_string(0, 10, lev, FG_COLOUR);
	sprintf(scor, "score: %d", score); draw_string(0, 20, scor, FG_COLOUR);
	draw_string(0, 40, "SW2/3 to restart", FG_COLOUR);
	show_screen(); //### Add button press to restart gameplay.
  bool start = false;
  do {
    if (BIT_IS_SET(PINF, 6) || 	BIT_IS_SET(PINF, 5)) start = true;
  } while (!start);
  totalMinutes += minutes; totalSeconds += seconds;
  resetVars();
  setup();
  // setup(); ###
}


// Process (ran every frame).
void process(void) {
	if (lives > 0) {
		clear_screen();
		drawLvl();
    if (bombTrailed || bowTrailed) crosshairMovement();
		moveHero();
		show_screen();
	}
	else {
    send_str(PSTR("Game ova red nova.\n"));
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
