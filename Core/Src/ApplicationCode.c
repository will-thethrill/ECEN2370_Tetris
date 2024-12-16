/*
 * ApplicationCode.h
 *
 *  Created on: Dec 1, 2024
 *      Author: Will Fraser
 */

#include "ApplicationCode.h"

// define block struct to store color and shape info for each block
typedef struct {
	uint8_t shape[BLOCK_SIZE][BLOCK_SIZE]; // 4x4 matrix for block shape
	uint16_t color;                        // color of the block
} TetrisBlock;

// define standard tetris blocks
const TetrisBlock tetrisBlocks[7] = { { { { 0, 0, 0, 0 }, { 1, 1, 1, 1 }, { 0,
		0, 0, 0 }, { 0, 0, 0, 0 } }, I_BLOCK_COLOR }, // I Block
		{ { { 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 0, 1, 1, 0 }, { 0, 0, 0, 0 } },
		O_BLOCK_COLOR }, // O Block
		{ { { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 0, 0, 1, 0 }, { 0, 0, 0, 0 } },
		T_BLOCK_COLOR }, // T Block
		{ { { 0, 0, 1, 1 }, { 0, 1, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
		S_BLOCK_COLOR }, // S Block
		{ { { 0, 1, 1, 0 }, { 0, 0, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
		Z_BLOCK_COLOR }, // Z Block
		{ { { 0, 0, 0, 0 }, { 0, 1, 1, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 0 } },
		J_BLOCK_COLOR }, // J Block
		{ { { 0, 0, 0, 1 }, { 0, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
		L_BLOCK_COLOR }  // L Block
};

// Static variables
static bool menuTouched;
static uint8_t score[4];

// Function prototypes
extern void initialise_monitor_handles(void);
void LCDTouchScreenInterruptGPIOInit(void);

// Game variables
static uint16_t gameGrid[GRID_HEIGHT][GRID_WIDTH]; 		// game state array
static uint16_t currentBlock[GRID_HEIGHT][GRID_WIDTH]; 	// current block array
static uint8_t currentBlockX;
static uint8_t currentBlockY;

// Touch variables
static STMPE811_TouchData StaticTouchData;
static EXTI_HandleTypeDef LCDTouchIRQ;
volatile bool userButtonPressed = false;

void ApplicationInit(void) {
	initPeripherals();					// Initializes all peripherals
	addSchedulerEvent(MAIN_MENU);		// Starts game in main menu first
}

// Initialize peripherals
void initPeripherals(void) {
	initialise_monitor_handles(); // Allows printf functionality

	// Initialize LCD, RNG, Timer, Button, and Touch
	LCD_Init();
	RNG_Init();
	TIMER_Init();
	BUTTON_Init();
	InitializeLCDTouch();
	LCDTouchScreenInterruptGPIOInit();
	menuTouched = false;

	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;
}

// Displays main menu
void displayMainMenu(void) {
	if (menuTouched == false) {	// checks menuTouched flag to not display menu more than once
		InitGameGrid();

		arrangeBlocks();			// positions blocks in starting position

		DrawGameGrid();				// show grid

		LCD_Draw_Rectangle_Fill(20, 200, 219, 299, LCD_COLOR_BLACK);// button box

		// Title
		LCD_SetFont(&Font16x24);
		LCD_SetTextColor(LCD_COLOR_WHITE);
		LCD_DisplayChar(62, 21, 'T');
		LCD_DisplayChar(82, 21, 'E');
		LCD_DisplayChar(102, 21, 'T');
		LCD_DisplayChar(122, 21, 'R');
		LCD_DisplayChar(142, 21, 'I');
		LCD_DisplayChar(162, 21, 'S');

		// Start button
		LCD_SetTextColor(LCD_COLOR_GREEN);
		LCD_DisplayChar(102, 241, 'G');
		LCD_DisplayChar(122, 241, 'O');

		menuTouched = true;							// sets flag

	}
}

// Timer variables
static uint32_t lastUpdate;
static uint32_t startTime;

// Start the game
void startGame(void) {
	InitGameGrid();

	GenerateBlock(RANDOM_BLOCK);
	DrawGameGrid();

	lastUpdate = HAL_GetTick(); // Get the current tick count at startup
	startTime = lastUpdate;

}

// Update game screen
void updateGameScreen(void) {

	uint32_t currentTime = HAL_GetTick();

	// Only move block once per second
	if ((currentTime - lastUpdate >= FRAMERATE) || (userButtonPressed)) {
		if (MoveCurrentBlock(MOVE_DOWN)) {
			// If move down returns true, place block
			printf("\nPLACE BLOCK");
			PlaceCurrentBlock();
			GenerateBlock(RANDOM_BLOCK);
		}
		// update screen
		DrawGameGrid();

		// update time
		lastUpdate = currentTime;
	}
}

// Display results screen
void displayResultsScreen(void) {
	// calculate total game time
	lastUpdate = HAL_GetTick() - startTime;

	// create buffer and store char version of game length
	char buffer[11];
	snprintf(buffer, sizeof(buffer), "%lu", lastUpdate);

	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);

	LCD_DisplayChar(72, 61, 'T');
	LCD_DisplayChar(92, 61, 'O');
	LCD_DisplayChar(112, 61, 'T');
	LCD_DisplayChar(132, 61, 'A');
	LCD_DisplayChar(152, 61, 'L');

	LCD_DisplayChar(52, 81, 'T');
	LCD_DisplayChar(72, 81, 'I');
	LCD_DisplayChar(92, 81, 'M');
	LCD_DisplayChar(112, 81, 'E');
	LCD_DisplayChar(132, 81, ' ');
	LCD_DisplayChar(152, 81, 'M');
	LCD_DisplayChar(172, 81, 'S');

	uint16_t current_x = 80;

	// Step through each char and display
	for (int i = 0; buffer[i] != '\0'; i++) {
		LCD_DisplayChar(current_x, 121, buffer[i]); // display the current char
		current_x += 20; // Move to the next character position
	}

	LCD_DisplayChar(32, 181, 'S');
	LCD_DisplayChar(52, 181, 'I');
	LCD_DisplayChar(72, 181, 'N');
	LCD_DisplayChar(92, 181, 'G');
	LCD_DisplayChar(112, 181, 'L');
	LCD_DisplayChar(132, 181, 'E');
	LCD_DisplayChar(152, 181, 'S');

	LCD_DisplayChar(192, 181, score[0] + '0');

	LCD_DisplayChar(32, 201, 'D');
	LCD_DisplayChar(52, 201, 'O');
	LCD_DisplayChar(72, 201, 'U');
	LCD_DisplayChar(92, 201, 'B');
	LCD_DisplayChar(112, 201, 'L');
	LCD_DisplayChar(132, 201, 'E');
	LCD_DisplayChar(152, 201, 'S');

	LCD_DisplayChar(192, 201, score[1] + '0');

	LCD_DisplayChar(32, 221, 'T');
	LCD_DisplayChar(52, 221, 'R');
	LCD_DisplayChar(72, 221, 'I');
	LCD_DisplayChar(92, 221, 'P');
	LCD_DisplayChar(112, 221, 'L');
	LCD_DisplayChar(132, 221, 'E');
	LCD_DisplayChar(152, 221, 'S');

	LCD_DisplayChar(192, 221, score[2] + '0');

	LCD_DisplayChar(32, 241, 'T');
	LCD_DisplayChar(52, 241, 'E');
	LCD_DisplayChar(72, 241, 'T');
	LCD_DisplayChar(92, 241, 'R');
	LCD_DisplayChar(112, 241, 'I');
	LCD_DisplayChar(132, 241, 'S');
	LCD_DisplayChar(152, 241, '!');

	LCD_DisplayChar(192, 241, score[3] + '0');

	removeSchedulerEvent(RESULTS);
}

void InitGameGrid(void) {
	for (uint16_t y = 0; y < GRID_HEIGHT; y++) {
		for (uint16_t x = 0; x < GRID_WIDTH; x++) {
			gameGrid[y][x] = EMPTY_CELL; // initialize all cells as empty
		}
	}
}

// iterates through each grid position in array and displays it
void DrawGameGrid(void) {
	for (uint16_t y = 0; y < GRID_HEIGHT; y++) {
		for (uint16_t x = 0; x < GRID_WIDTH; x++) {
			uint16_t color = gameGrid[y][x];
			if (color == EMPTY_CELL) {
				color = currentBlock[y][x];
			}
			LCD_Draw_Rectangle_Fill(x * CELL_SIZE, y * CELL_SIZE,
					(x + 1) * CELL_SIZE - 1, (y + 1) * CELL_SIZE - 1, color);
		}
	}
}

// Create block in currentBlock array
void GenerateBlock(uint8_t blockIndex) {

	// check if user wants random block
	if (blockIndex == RANDOM_BLOCK) {
		blockIndex = RandomNumbersGeneration() % 7;
	} else {
		blockIndex--;
	}

	// set block starting position
	currentBlockY = 0;
	currentBlockX = 4;

	// copy block data
	for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
		for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
			if (tetrisBlocks[blockIndex].shape[y][x] == 1) {
				currentBlock[y][x + 4] = tetrisBlocks[blockIndex].color;
			}
		}
	}
}

bool MoveCurrentBlock(uint8_t direction) {
	uint16_t tempBlock[GRID_HEIGHT][GRID_WIDTH] = { 0 }; // temp buffer to store the shifted block

	switch (direction) {
	case MOVE_DOWN:
		for (uint16_t y = GRID_HEIGHT - 1; y > 0; y--) {
			for (uint16_t x = 0; x < GRID_WIDTH; x++) {
				if ((currentBlock[y - 1][x] != EMPTY_CELL)
						&& (gameGrid[y][x] != EMPTY_CELL)) {
					return true;
				} else if ((currentBlock[y][x] != EMPTY_CELL)
						&& (y == GRID_HEIGHT - 1)) {
					return true;
				}
				// check if block can be moved and shift tempBlock
				tempBlock[y][x] = currentBlock[y - 1][x];
			}
		}
		currentBlockY++;
		break;

	case MOVE_UP:
		for (uint16_t y = 0; y < GRID_HEIGHT - 1; y++) {
			for (uint16_t x = 0; x < GRID_WIDTH; x++) {
				if ((currentBlock[y + 1][x] != EMPTY_CELL)
						&& (gameGrid[y][x] != EMPTY_CELL)) {
					return true;
				}
				// check if block can be moved and shift tempBlock
				tempBlock[y][x] = currentBlock[y + 1][x];

			}
		}
		currentBlockY--;
		break;

	case MOVE_LEFT:
		for (uint16_t y = 0; y < GRID_HEIGHT; y++) {
			for (uint16_t x = 0; x < GRID_WIDTH - 1; x++) {
				if ((currentBlock[y][x] != EMPTY_CELL) && (x == 0)) {
					return false;
				} else if ((currentBlock[y][x + 1] != EMPTY_CELL)
						&& (gameGrid[y][x] != EMPTY_CELL)) {
					return false;
				}
				// check if block can be rotated and shift tempBlock
				tempBlock[y][x] = currentBlock[y][x + 1];

			}
		}
		currentBlockX--;
		break;

	case MOVE_RIGHT:
		for (uint16_t y = 0; y < GRID_HEIGHT; y++) {
			for (uint16_t x = GRID_WIDTH - 1; x > 0; x--) {
				if ((currentBlock[y][x] != EMPTY_CELL)
						&& (x == GRID_WIDTH - 1)) {
					return false;
				} else if ((currentBlock[y][x - 1] != EMPTY_CELL)
						&& (gameGrid[y][x] != EMPTY_CELL)) {
					return false;
				}
				// check if block can be rotated and shift tempBlock
				tempBlock[y][x] = currentBlock[y][x - 1];

			}
		}
		currentBlockX++;
		break;

	default:
		return false;
	}

	// Copy tempBlock back into currentBlock
	for (uint16_t y = 0; y < GRID_HEIGHT; y++) {
		for (uint16_t x = 0; x < GRID_WIDTH; x++) {
			currentBlock[y][x] = tempBlock[y][x];
		}
	}

	return false; // move successful
}

bool RotateCurrentBlock(uint8_t direction) {
	// temp storage for the 4x4 block
	uint16_t temp[BLOCK_SIZE][BLOCK_SIZE] = { 0 };

	// extract the 4x4 section
	for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
		for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
			temp[y][x] = currentBlock[currentBlockY + y][currentBlockX + x];
		}
	}

	//rotate into a new temporary array
	uint16_t rotated[BLOCK_SIZE][BLOCK_SIZE] = { 0 };

	if (direction == ROTATE_RIGHT) {
		// rotate right: new[x][3 - y] = old[y][x]
		for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
			for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
				rotated[x][BLOCK_SIZE - 1 - y] = temp[y][x];
			}
		}
	} else {
		// rotate left: new[3 - x][y] = old[y][x]
		for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
			for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
				rotated[BLOCK_SIZE - 1 - x][y] = temp[y][x];
			}
		}
	}

	// check if rotated block can fit (add later)

	// if valid, copy rotated shape back into currentBlock
	for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
		for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
			currentBlock[currentBlockY + y][currentBlockX + x] = rotated[y][x];
		}
	}

	return false;
}

void PlaceCurrentBlock(void) {
	for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
		for (uint8_t x = 0; x < GRID_WIDTH; x++) {
			if (currentBlock[y][x] != EMPTY_CELL) {
				gameGrid[y][x] = currentBlock[y][x]; // copy currentBlock to gameGrid
				currentBlock[y][x] = 0;					// erase currentBlock
			}
		}
	}
	// update score and clear lines
	uint8_t linesCleared = ClearCompleteLines();

	// update score based on lines cleared
	switch (linesCleared) {
	case 0:
		score[0] += 0; // add 0 to score as a consolation prize
		break;
	case 1:
		score[0]++;
		break;
	case 2:
		score[1]++;
		break;
	case 3:
		score[2]++;
		break;
	case 4:
		score[3]++;
		break;
	default:
		score[3]++;
	}

	// reset flag to prevent next block from being placed
	userButtonPressed = false;

	CheckGameEnd();
}

uint8_t ClearCompleteLines(void) {
	uint8_t linesCleared = 0;

	// iterate over each row from bottom to top
	for (int16_t y = GRID_HEIGHT - 1; y >= 0; y--) {
		bool isLineComplete = true;

		// check if the current row is complete (no EMPTY_CELL)
		for (uint16_t x = 0; x < GRID_WIDTH; x++) {
			if (gameGrid[y][x] == EMPTY_CELL) {
				isLineComplete = false;
				break;
			}
		}

		// if line is complete, clear it and shift down the above lines
		if (isLineComplete) {
			linesCleared++; // increment the count of cleared lines

			// shift all rows above the cleared line down by one
			for (int16_t ty = y; ty > 0; ty--) {
				for (uint16_t x = 0; x < GRID_WIDTH; x++) {
					gameGrid[ty][x] = gameGrid[ty - 1][x];
				}
			}

			// clear the topmost row as it has been shifted down
			for (uint16_t x = 0; x < GRID_WIDTH; x++) {
				gameGrid[0][x] = EMPTY_CELL;
			}

			y++;
		}
	}

	return linesCleared;
}

void CheckGameEnd(void) {
	// iterate through the top four rows
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < GRID_WIDTH; x++) {
			if (gameGrid[y][x] != EMPTY_CELL) {

				// update the game state to RESULTS to trigger game over
				addSchedulerEvent(RESULTS);
				removeSchedulerEvent(GAME);
				return;
			}
		}
	}
}

void arrangeBlocks(void) {
	// Positions T block
	GenerateBlock(T_BLOCK);
	RotateCurrentBlock(ROTATE_LEFT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	PlaceCurrentBlock();

	// Positions S block
	GenerateBlock(S_BLOCK);
	RotateCurrentBlock(ROTATE_RIGHT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	PlaceCurrentBlock();

	// Positions L block
	GenerateBlock(L_BLOCK);
	RotateCurrentBlock(ROTATE_RIGHT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	PlaceCurrentBlock();

	// Positions O block
	GenerateBlock(O_BLOCK);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	PlaceCurrentBlock();

	// Positions J block
	GenerateBlock(J_BLOCK);
	RotateCurrentBlock(ROTATE_RIGHT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_RIGHT);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	PlaceCurrentBlock();

	// Positions Z block
	GenerateBlock(Z_BLOCK);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_LEFT);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	PlaceCurrentBlock();

	// Positions I block
	GenerateBlock(I_BLOCK);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_DOWN);
	MoveCurrentBlock(MOVE_RIGHT);
	PlaceCurrentBlock();
}

void HandleTouch(void) {
	// determine what to do about touch based on current game state
	uint32_t gameState = getScheduledEvents();

	// if in menu, start game
	if (gameState & MAIN_MENU) {
		if ((StaticTouchData.x >= 20) && (StaticTouchData.x <= 219)
				&& (StaticTouchData.y >= 20) && (StaticTouchData.y <= 139)) {
			printf("GAME STARTED");

			removeSchedulerEvent(MAIN_MENU);
			addSchedulerEvent(GAME);
			startGame();

		}

		// if in game, move block
	} else if (gameState & GAME) {
		if (StaticTouchData.x <= 119) {
			if (StaticTouchData.y > 159) {
				printf("\nRotate LEFT");
				RotateCurrentBlock(ROTATE_LEFT);
				DrawGameGrid();
			} else {
				printf("\nMove LEFT");
				if (MoveCurrentBlock(MOVE_LEFT)) {
					printf("PLACE\n\n");
					PlaceCurrentBlock();
					GenerateBlock(RANDOM_BLOCK);
				}
				DrawGameGrid();
			}
		} else {
			if (StaticTouchData.y > 159) {
				printf("\nRotate RIGHT");
				RotateCurrentBlock(ROTATE_RIGHT);
				DrawGameGrid();
			} else {
				printf("\nMove RIGHT");
				if (MoveCurrentBlock(MOVE_RIGHT)) {
					printf("PLACE\n\n");
					PlaceCurrentBlock();
					GenerateBlock(RANDOM_BLOCK);
				}
				DrawGameGrid();
			}
		}
	}
}

void EXTI0_IRQHandler(void) {

	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	GPIO_PinState pinState = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

	// check if the interrupt was triggered by the User Button GPIO Pin
	if (__HAL_GPIO_EXTI_GET_IT(BUTTON_PIN) != RESET) {
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_PIN);
		if (pinState == PRESSED) {
			userButtonPressed = false; // set flag false
		} else {
			userButtonPressed = true;  // set flag true
		}
	}

	// clear the interrupt flag
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

// TouchScreen Interrupt
void LCDTouchScreenInterruptGPIOInit(void) {
	GPIO_InitTypeDef LCDConfig = { 0 };
	LCDConfig.Pin = GPIO_PIN_15;
	LCDConfig.Mode = GPIO_MODE_IT_RISING_FALLING;
	LCDConfig.Pull = GPIO_NOPULL;
	LCDConfig.Speed = GPIO_SPEED_FREQ_HIGH;

	// Clock enable
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// GPIO Init
	HAL_GPIO_Init(GPIOA, &LCDConfig);

	// Interrupt Configuration
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	LCDTouchIRQ.Line = EXTI_LINE_15;
}

#define TOUCH_DETECTED_IRQ_STATUS_BIT   (1 << 0)  // Touchscreen detected bitmask

static uint8_t statusFlag;

void EXTI15_10_IRQHandler() {
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); // May consider making this a universial interrupt guard
	bool isTouchDetected = false;

	static uint32_t count;
	static uint32_t timeout;

	count = 0;
	timeout = 0;

	while ((count == 0) && (timeout < 10)) {
		count = STMPE811_Read(STMPE811_FIFO_SIZE);
		timeout++;
	}

	// Disable touch interrupt bit on the STMPE811
	uint8_t currentIRQEnables = ReadRegisterFromTouchModule(STMPE811_INT_EN);
	WriteDataToTouchModule(STMPE811_INT_EN, 0x00);

	// Clear the interrupt bit in the STMPE811
	statusFlag = ReadRegisterFromTouchModule(STMPE811_INT_STA);
	uint8_t clearIRQData = (statusFlag | TOUCH_DETECTED_IRQ_STATUS_BIT); // Write one to clear bit
	WriteDataToTouchModule(STMPE811_INT_STA, clearIRQData);

	uint8_t ctrlReg = ReadRegisterFromTouchModule(STMPE811_TSC_CTRL);
	if (ctrlReg & 0x80) {
		isTouchDetected = true;
	}

	// Determine if it is pressed or unpressed
	if (isTouchDetected) // Touch has been detected
	{
		DetermineTouchPosition(&StaticTouchData);
		HandleTouch();
		/* Touch valid */
	}

	STMPE811_Write(STMPE811_FIFO_STA, 0x01);
	STMPE811_Write(STMPE811_FIFO_STA, 0x00);

	// Re-enable IRQs
	WriteDataToTouchModule(STMPE811_INT_EN, currentIRQEnables);
	HAL_EXTI_ClearPending(&LCDTouchIRQ, EXTI_TRIGGER_RISING_FALLING);

	HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	//Potential ERRATA? Clearing IRQ bit again due to an IRQ being triggered DURING the handling of this IRQ..
	WriteDataToTouchModule(STMPE811_INT_STA, clearIRQData);

}
