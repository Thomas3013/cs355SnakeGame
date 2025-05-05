#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>


// This will hold the position of the screen (rows and columns) (JERRY)
struct box {
    int row;
    int col;
};

char snakeDirection[10];
int snakeSize = 3;
struct box snakeBody[100];
bool playerWon = false;

struct trophy {
    struct box* location;
    int seconds;
    int value;
    time_t created_at;
};

// With this part, the terminal is restored if the game is forcefully closed (THOMAS)
void handle_exit(int sig) {
    endwin();
    printf("\nGame exited.\n");
    exit(0);
}

// With this part, we draw the walls (the snake pit) (JERRY)
void createWall() {
    for (int l = 0; l < COLS; l++) {
        move(0, l); addch('-');
        move(LINES - 1, l); addch('-');
    }
    for (int w = 0; w < LINES; w++) {
        move(w, 0); addch('|');
        move(w, COLS - 1); addch('|');
    }
}

// With this part, we draw the head and body of the snake (JERRY)
void drawSnake() {
    move(snakeBody[0].row, snakeBody[0].col); addch('0');
    for (int i = 1; i < snakeSize; i++) {
        move(snakeBody[i].row, snakeBody[i].col); addch('O');
    }
    move(snakeBody[snakeSize].row, snakeBody[snakeSize].col); addch(' ');
}

// With this part, we shift the body forward and move the head (JERRY)
void moveSnakeHead(int deltarow, int deltacol) {
    for (int i = snakeSize; i > 0; i--) {
        snakeBody[i] = snakeBody[i - 1];
    }
    snakeBody[0].row += deltarow;
    snakeBody[0].col += deltacol;
    drawSnake();
}

// With this part, the snake moves faster as it gets longer (JERRY)
void sleep_until_next_step() {
    int base_delay = 200000;
    int speedup = snakeSize * 1000;
    int sleepTime = base_delay - speedup;
    if (sleepTime < 50000) sleepTime = 50000;
    usleep(sleepTime);
}

// With this part, we place the snake in the center and randomly assign direction
// Updated initialSnake to properly spawn snake away from itself (JERRY)
void initialSnake() {
    int startRow = LINES / 2;
    int startCol = COLS / 2;
    srand(time(NULL));
    int dir = rand() % 4;
    int dRow = 0, dCol = 0;

    switch (dir) {
        case 0:
	   strcpy(snakeDirection, "RIGHT"); 
	   dCol = 1; 
	   break;
        case 1:
	   strcpy(snakeDirection, "LEFT");  
	   dCol = -1; 
	   break;
        case 2:	
	   strcpy(snakeDirection, "UP");   
	   dRow = -1; 
	   break;
        case 3:
	   strcpy(snakeDirection, "DOWN");  
	   dRow = 1;
	   break;
    }

    snakeBody[0] = (struct box){startRow, startCol};
    // Spawn the tail behind the head based on direction
    for (int i = 1; i < 3; i++) {
        snakeBody[i].row = startRow - i * dRow;
        snakeBody[i].col = startCol - i * dCol;
    }
}

// With this part, we check if a location is part of the snake's body (THOMAS)
bool is_snake(int x, int y) {
    struct box curr_box = {x, y};
    for (int i = 0; i < snakeSize; i++) {
        if (memcmp(&curr_box, &snakeBody[i], sizeof(struct box)) == 0) {
            return true;
        }
    }
    return false;
}

// With this part, we create a trophy in an empty location (THOMAS)
struct trophy* create_trophy(void) {
    struct trophy* new_trophy = malloc(sizeof(struct trophy));
    if (!new_trophy) return NULL;
    new_trophy->location = malloc(sizeof(struct box));
    if (!new_trophy->location) {
        free(new_trophy);
        return NULL;
    }

    int x, y;
    srand(time(NULL));
    do {
        x = rand() % (LINES - 2) + 1;
        y = rand() % (COLS - 2) + 1;
    } while (is_snake(x, y));

    new_trophy->location->row = x;
    new_trophy->location->col = y;
    new_trophy->seconds = (rand() % 9) + 1;
    new_trophy->value = (rand() % 8) + 2;
    new_trophy->created_at = time(NULL);

    move(x, y); addch('0' + new_trophy->value);
    return new_trophy;
}

// With this part, we remove a trophy from the screen and free memory (THOMAS)
void delete_trophy(struct trophy* trophy) {
    if (!trophy) return;
    if (trophy->location) {
        move(trophy->location->row, trophy->location->col);
        addch(' ');
        free(trophy->location);
        trophy->location = NULL;
    }
    free(trophy);
}

// This part of the code will run the gameplay properly. We will see its movement, trophies
// and it will print wether you win or lose the game. (THOMAS AND JERRY)
int main() {
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);

    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    initialSnake();
    createWall();
    drawSnake();
    struct trophy* curr_trophy = NULL;

    while (1) {
        if (!curr_trophy) {
            curr_trophy = create_trophy();
        }

        int ch = getch();

        // With this part, we handle input and prevent reversing direction
        if (ch == KEY_UP) {
            if (strcmp(snakeDirection, "DOWN") == 0) {
                break;
            } else {
                strcpy(snakeDirection, "UP");
            }
        } else if (ch == KEY_DOWN) {
            if (strcmp(snakeDirection, "UP") == 0) {
                break;
            } else {
                strcpy(snakeDirection, "DOWN");
            }
        } else if (ch == KEY_LEFT) {
            if (strcmp(snakeDirection, "RIGHT") == 0) {
                break;
            } else {
                strcpy(snakeDirection, "LEFT");
            }
        } else if (ch == KEY_RIGHT) {
            if (strcmp(snakeDirection, "LEFT") == 0) {
                break;
            } else {
                strcpy(snakeDirection, "RIGHT");
            }
        }

        int dr = 0, dc = 0;
        if (strcmp(snakeDirection, "UP") == 0) {
            dr = -1;
        } else if (strcmp(snakeDirection, "DOWN") == 0) {
            dr = 1;
        } else if (strcmp(snakeDirection, "LEFT") == 0) {
            dc = -1;
        } else if (strcmp(snakeDirection, "RIGHT") == 0) {
            dc = 1;
        }

        int nextRow = snakeBody[0].row + dr;
        int nextCol = snakeBody[0].col + dc;

        // With this part, we check if the snake hits the wall or itself
        if (nextRow == 0 || nextRow == LINES - 1 || nextCol == 0 || nextCol == COLS - 1) break;
        if (is_snake(nextRow, nextCol)) break;

        moveSnakeHead(dr, dc);
        sleep_until_next_step();

        // With this part, we check if the snake eats a trophy
        if (curr_trophy &&
            snakeBody[0].row == curr_trophy->location->row &&
            snakeBody[0].col == curr_trophy->location->col) {
            snakeSize += curr_trophy->value;
            delete_trophy(curr_trophy);
            curr_trophy = NULL;

            if (snakeSize >= (LINES + COLS)) {
                playerWon = true;
                break;
            }
        }

        // With this part, we expire the trophy if time is up (THOMAS)
        if (curr_trophy &&
            difftime(time(NULL), curr_trophy->created_at) >= curr_trophy->seconds) {
            delete_trophy(curr_trophy);
            curr_trophy = NULL;
        }

        refresh();
    }

    if (curr_trophy) {
        delete_trophy(curr_trophy);
        curr_trophy = NULL;
    }

    endwin();

    if (playerWon) {
        printf("YOU WIN!\n");
    } else {
        printf("Game Over\n");
    }

    return 0;
}

