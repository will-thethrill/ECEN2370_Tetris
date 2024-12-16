/*
 * ApplicationCode.h
 *
 *  Created on: Dec 1, 2024
 *      Author: Will Fraser
 */

#include "stm32f4xx_hal.h"

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "Scheduler.h"
#include "RNG.h"
#include "LCD_Driver.h"
#include "Timer.h"

#define FRAMERATE 1000 // 1 FPS

// defne grid
#define GRID_WIDTH  12
#define GRID_HEIGHT 16
#define CELL_SIZE	20
#define BLOCK_SIZE 4 	// Blocks are 4x4 matrices

#define EMPTY_CELL  0 // Represent an empty cell with 0

#define RANDOM_BLOCK 0
#define I_BLOCK		1
#define O_BLOCK		2
#define T_BLOCK		3
#define S_BLOCK		4
#define Z_BLOCK		5
#define J_BLOCK		6
#define L_BLOCK		7

#define MOVE_DOWN  0
#define MOVE_LEFT  1
#define MOVE_RIGHT 2
#define MOVE_UP    3

#define ROTATE_LEFT  0
#define ROTATE_RIGHT 1

#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

void ApplicationInit(void);
void initPeripherals(void);

void displayMainMenu(void);
void startGame(void);
void updateGameScreen(void);
void displayResultsScreen(void);

void InitGameGrid(void);
void DrawGameGrid(void);

void GenerateBlock(uint8_t blockNum);
bool MoveCurrentBlock(uint8_t direction);
bool RotateCurrentBlock(uint8_t direction);

void PlaceCurrentBlock(void);
uint8_t ClearCompleteLines(void);
void CheckGameEnd(void);

void arrangeBlocks(void);
void HandleTouch(void);

#endif /* INC_APPLICATIONCODE_H_ */
