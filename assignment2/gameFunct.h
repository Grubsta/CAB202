void send_str();
void spriteMagic();
bool gapCollision();
void initHero();
void loadingScreen();
void scrollMap();
void staticMap();
void level1Init();
void enemyMovement();
void displayMenu();
void initControls();
void mapInit();
// void map
// Enables sprites to trail sprites.
void spriteTrail(Sprite sprite1) {
  int x, y;
  if (sprite1.bitmap == keyBitmap) {
     x = hero.x + HW + 2;  y = hero.y + HH + 3;
  }
  else {
     x = hero.x - 3;  y = hero.y + HH + 3;
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
  // if (!crosshairInit){
  //   sprite_init(&crosshair, LCD_X * 0.5, LCD_Y * 0.5, 3, 3, crosshairBitmap);
  //   crosshairInit = true;
  // }
  crosshair.x = (double) left_adc * (LCD_X - crosshair.width) / 1024;
  crosshair.y = (double) right_adc * (LCD_Y - crosshair.height) / 1024;
  sprite_draw(&crosshair);
  sendIt();
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
        // send_str(PSTR("The hero has located da bomb.\r\n")); //####
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
        // send_str(PSTR("The hero has picked up a shield. +1 protection.\r\n"));
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

// Initialise Timer.
void timer(void) {
	timeCounter++;
	if (timeCounter == 10) {
	seconds++;
	timeCounter = 0;
		if (seconds == 60) {
		seconds = 0;
		minutes++;
			if (minutes == 100) {
			}
		}
	}
}

double interval = 0;
// Overflow timer.
ISR(TIMER0_OVF_vect) {
	interval += TIMER_SCALE * PRESCALE / FREQ;
	if ( interval >= 1.0 ) {
		interval = 0;
		PORTD ^= 1 << 6;
	}
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
