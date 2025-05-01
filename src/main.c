#include <curses.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// This will hold the position of the screen (rows and columns)
struct box{

	int row; // Down the screen
	int col; // Across the screen
};

	char snakeDirection[10]; // Direction of the snakes movement
	int snakeSize = 3; // Snake starts with the length of 3
	struct box snakeBody[100]; // Body of snake, which is up to 100 parts




/// This will create the visible border for the snake (The Walls)

void createWall(){

	for (int l = 0 ; l < COLS; l++){
	    move(0,l);
	    addch('-');
	    move(LINES - 1, l);
	    addch('-');
	}
	for (int w = 0; w < LINES; w++){
	    move(w,0);
	    addch('|');
	    move(w, COLS - 1);
	    addch('|');
	}
}

/// This will draw the snake on the screen
void drawSnake(){
	move(snakeBody[0].row, snakeBody[0].col); //Head of the snake
	addch('0');

	for(int i = 1; i < snakeSize; i++){
	    move(snakeBody[i].row, snakeBody[i].col);
	    addch('O');
	}
	
	move(snakeBody[snakeSize].row, snakeBody[snakeSize].col);
	addch(' '); //This will delete the old tail
}

// This will control the snakes head when moving foward and allow move the body back
void moveSnakeHead(int deltarow, int deltacol){
	for (int i = snakeSize; i > 0; i--){
	    snakeBody[i] = snakeBody[i - 1];
	}

	snakeBody[0].row += deltarow;
	snakeBody[0].col += deltacol;

	drawSnake();
}

// This will move the snake a step ahead to where it's currently pointed at in the pit
void stepSnake(int deltaRow, int deltaCol){
	moveSnakeHead(snakeBody[0].row + deltaRow, snakeBody[0].col + deltaCol);
}

// This will set up the snakes position and the direction it will randomly start on
void initialSnake(){
	int startRow = LINES / 2;  // starts in the middle row
	int startCol = COLS / 2;   // starts in the middle column
	srand(time(NULL));         

	int dir = rand() % 4;
	int dRow = 0;
	int dCol = 0;
	
	// This will set the direction name, for when its turning
	switch(dir){
	   case 0:
		strcpy(snakeDirection, "RIGHT");
		dCol = -1;
		break;
	   case 1:
		strcpy(snakeDirection, "LEFT");
		dCol = 1;
		break;
	   case 2:
		strcpy(snakeDirection, "UP");
		dRow = -1;
		break;
	   case 3:
		strcpy(snakeDirection, "DOWN");
		dRow = -1;
		break;
	}
	// This puts the head in the center of pit
	snakeBody[0] = (struct box){startRow, startCol};

	// 2 parts behind the head to follow the direction we choose
	for(int i = 1; i < 3; i++){
		snakeBody[i].row = startRow + i * dRow; // Will move down or up
		snakeBody[i].col = startCol + i * dCol; // Will move left or right
	}
}

int main(){
	initscr();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	initialSnake();
	createWall();
	drawSnake();

	// The game will repeat until you press the letter 'e' to exit the game

	while (1) {
        int ch = getch();        // This will check if a key is being pressed

        if (ch == 'e') {
            break;               // Exit the game when you press 'e'
        }

        // This will change the direction based on what key arrow you press on
        if (ch == KEY_UP && strcmp(snakeDirection, "DOWN") != 0) {
            strcpy(snakeDirection, "UP");
        }
        else if (ch == KEY_DOWN && strcmp(snakeDirection, "UP") != 0) {
            strcpy(snakeDirection, "DOWN");
        }
        else if (ch == KEY_LEFT && strcmp(snakeDirection, "RIGHT") != 0) {
            strcpy(snakeDirection, "LEFT");
        }
        else if (ch == KEY_RIGHT && strcmp(snakeDirection, "LEFT") != 0) {
            strcpy(snakeDirection, "RIGHT");
        }

        // This will move the snake forward one step in the current direction
        int dr = 0, dc = 0;
        if (strcmp(snakeDirection, "UP") == 0) {
            dr = -1;
        }
        else if (strcmp(snakeDirection, "DOWN") == 0) {
            dr = 1;
        }
        else if (strcmp(snakeDirection, "LEFT") == 0) {
            dc = -1;
        }
        else if (strcmp(snakeDirection, "RIGHT") == 0) {
            dc = 1;
        }

        moveSnakeHead(dr, dc);  // Move the head and update the snake

        // The speed will increae as snake gets longer
        int sleepTime = 160000 - (snakeSize * 1000);
        if (sleepTime < 20000) {
            sleepTime = 20000;     // Will set the minimum speed
        }

        usleep(sleepTime);         // The game will pause for a bit
        refresh();                 // This will update the screen with changes
    }

    endwin();                      // Exit curses
    return 0;
}
