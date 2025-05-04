#include <curses.h>
#include <pthread.h>
#include <stdbool.h>
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

void sleep_until_next_second() {
  struct timespec now, next;
  clock_gettime(CLOCK_REALTIME, &now);
  next.tv_sec = now.tv_sec + 1;
  next.tv_nsec = 0;
  clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL);
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
struct trophy{
	struct box* location;
	int seconds;
	int value;
};

bool is_snake(int x,int y){
	// we want a set here but im not prematurely optimizing
	struct box curr_box = {x,y};
	for (int i = 0; i < snakeSize; i++){
		if (memcmp(&curr_box, &snakeBody[i], sizeof(struct box)) == 0){
			return true;
		}
	}
	return false;
}


void create_trophy(struct trophy** trophy_ptr){
    if (!trophy_ptr) return;  // Guard against NULL pointer
    
    *trophy_ptr = (struct trophy*)malloc(sizeof(struct trophy));
    if (!*trophy_ptr) return;  // Guard against malloc failure
    
    (*trophy_ptr)->location = (struct box*)malloc(sizeof(struct box));
    if (!(*trophy_ptr)->location) {  // Guard against malloc failure
        free(*trophy_ptr);
        *trophy_ptr = NULL;
        return;
    }
    
    int x,y;
    srand(time(NULL));
    do {
        x = rand() % (LINES - 2) + 1;
        y = rand() % (COLS - 2) + 1;
    } while(is_snake(x,y));
    (*trophy_ptr)->location->row = x;
    (*trophy_ptr)->location->col = y;
    (*trophy_ptr)->seconds = (rand() % 8) + 2;  
	(*trophy_ptr)->value = (rand() % 8) + 2;  // Random number between 2 and 9
    move((*trophy_ptr)->location->row, (*trophy_ptr)->location->col);
    addch('0' + (*trophy_ptr)->value);
}

void delete_trophy(struct trophy* trophy) {
	int x = trophy->location->row;
	int y = trophy->location->col;
	move(x,y);
	addch(' ');
	free(trophy->location);
	free(trophy);
}

int main(){
	initscr();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	char* last;
	int seconds = 0;

	initialSnake();
	createWall();
	drawSnake();
	struct trophy* curr_trophy = NULL;  // Initialize to NULL

	// The game will repeat until you press the letter 'e' to exit the game

	while (1) {
		if (!curr_trophy){
			create_trophy(&curr_trophy);
		}
        int ch = getch();        // This will check if a key is being pressed
        if (ch == 'e') {
            if (curr_trophy) {   // Clean up trophy before exit
                delete_trophy(curr_trophy);
            }
            break;               // Exit the game when you press 'e'
        }
		// check for wall collision
		if ((snakeBody[0].row == 0) || (snakeBody[0].row == LINES-1) || snakeBody[0].col == 0 || snakeBody[0].col == COLS-1){
			break;
		} 
        // This will change the direction based on what key arrow you press on
		// we gotta make the opposite direction kill user
        if (ch == KEY_UP) {
			if (strcmp(snakeDirection, "DOWN") == 0){
				break; // snake dies
			}
            strcpy(snakeDirection, "UP");
        }
        else if (ch == KEY_DOWN) {
			if (strcmp(snakeDirection, "UP") == 0){
				break; // snake dies
			}
            strcpy(snakeDirection, "DOWN");
        }
        else if (ch == KEY_LEFT) {
			if (strcmp(snakeDirection, "RIGHT") == 0){
				break; // snake dies
			}
            strcpy(snakeDirection, "LEFT");
        }
        else if (ch == KEY_RIGHT) {
			if (strcmp(snakeDirection, "LEFT") == 0){
				break; // snake dies
			}
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
		sleep_until_next_second(); // sleep until next second 
		if((snakeBody[0].col == curr_trophy->location->col) && (snakeBody[0].row == curr_trophy->location->row)){
			int temp = curr_trophy->value;
			snakeSize += temp;  // Directly add the value instead of using a loop
			delete_trophy(curr_trophy);
			curr_trophy = NULL;  // Set to NULL after deletion
			create_trophy(&curr_trophy);
		}
		if(curr_trophy && curr_trophy->seconds <= seconds){
			delete_trophy(curr_trophy);
			curr_trophy = NULL;  // Set to NULL after deletion
			seconds = 0;
		}
		seconds += 1;


        // The speed will increae as snake gets longer
		/*

        int sleepTime = 160000 - (snakeSize * 1000);
        if (sleepTime < 20000) {
            sleepTime = 20000;     // Will set the minimum speed
        }

        usleep(sleepTime);         // The game will pause for a bit
				*/
        refresh();                 // This will update the screen with changes
    }

    if (curr_trophy) {  // Clean up trophy before exit
        delete_trophy(curr_trophy);
    }
    endwin();                      // Exit curses
    return 0;
}
