void send_str();
void usb_serial_send();
bool spriteCollision();


// Initialise hero.
void initHero(void) {
	int x = LCD_X / 2 - HW / 2;
	int y = LCD_Y / 2 + HH + 3;
  herox = x; heroy = y;
	sprite_init(&hero, x, y, HW, HH, heroBitmap);
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
  srand(seed * (rand() % 6));
  int x = rand() % (wallX2 + (wallX1 + 8 * 1));
  return x;
}

// Produces random y value between game size.
int randY(void) {
  srand(seed * (rand() % 12));
  int y = rand() % (wallY2 + (wallY1 * 1));
  return y;
}


// Initialises defence items.
void defenceInit(void) {
  bool valid = false;
  int gen, x, y;
  // Randomising both the position and chance of spawn.
  // gen = rand() % 100;
  gen = 29;
  if (gen <= 29) {
    while (!valid) {
      // x = randX();
      // y = randY();
      x = LCD_X * 0.8;
      y = LCD_Y * 0.3; // ####
      sprite_init(&bow, x, y, 8, 3, bowBitmap);
      // if (!spriteCollision(bow)) valid = true; #######
      valid = true;
    }
  }
  // srand(seed * gen);
  valid = false;
  gen = rand() % 100;
  if (gen <= 29) {
    while (!valid) {
      // x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      // x += wallX1; y += wallY1;
      x = LCD_X * 0.8;
      y = LCD_Y * 0.6; // ####
      sprite_init(&bomb, x, y, 6, 4, bombBitmap);
      // if (!spriteCollision(bomb)) valid = true;
      valid = true;
    }
  }
  srand(seed * gen * 21);
  valid = false;
  gen = rand() % 100;
  if (gen <= 29) {
    while (!valid) {
      x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      x += wallX1; y += wallY1;
      sprite_init(&shield, x, y, 8, 4, shieldBitmap);
      // if (!spriteCollision(shield)) valid = true;
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
      x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      x += wallX1; y += wallY1;
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
      x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
      x += wallX1; y += wallY1;
      sprite_init(&treasure[i], x, y, 8, 3, treasureBitmap);
      if (!spriteCollision(treasure[i])) valid = true;
      send_str(PSTR("FAILED\r\n"));
    }
  }
}

void doorInit(void) {
  int x, y;
  srand(minutes * seed * seconds);
  bool valid = false;
  while (!valid) {
    x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
    x += wallX1; y += wallY1;
    sprite_init(&door, x, y, DW, DH, doorBitmap);
    if (!spriteCollision(door)) break;
    send_str(PSTR("FAILED\r\n"));
  }
}

void keyInit(void) {
  int x, y;
  srand(minutes * seed - seconds);
  bool valid = false;
  while (!valid) {
    x = rand() % (wallX2 + (wallX1 + 8 * 1)); y = rand() % (wallY2 + (wallY1 * 1));
    x += wallX1; y += wallY1;
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
  // send_str(PSTR("DEFENCE\r\n"));
  // defenceInit();
	mapInitialised = true;
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
