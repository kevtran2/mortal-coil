#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// gcc -o mortal mortal.c -lcurl
// compiler, output flag, output file name, file name, -l for library + lib name
// Milestone 1 
// Get board from API
// print board

// Milestone 2
// get dfs working

// Milestone 3
// print solve times
// use a set to keep track of available squares(allows unsolvable check later)
struct board {
    int **grid;
    int row;
    int col;
    char *path;
    int step; //a path is made of many steps. the step var keeps track of which index of the path to update
    int bufmax;
    bool complete;
    int remainingSquares;
};

struct board extractInfo(char* ptr) {
    //Get Level
    char *levelStart = strstr(ptr, "Level:") + 7;
    char *levelEnd = strstr(levelStart, "<");
    int levelStrLen = levelEnd - levelStart;
    char level[levelStrLen + 1];
    memcpy(level, levelStart, levelStrLen);
    strcpy(level + levelStrLen, "\0");
    printf("Level: %s\n", level);

    //Get Width
    char *widthStart = strstr(levelEnd, "width = ") + 8;
    char *widthEnd = strstr(widthStart, ";");
    int widthStrLen = widthEnd - widthStart;
    char width[widthStrLen + 1];
    memcpy(width, widthStart, widthStrLen);
    strcpy(width + widthStrLen, "\0");
    printf("Width: %s\n", width);

    //Get Height
    char *heightStart = widthEnd + 15;
    char *heightEnd = strstr(heightStart, ";");
    int heightStrLen = heightEnd - heightStart;
    char height[heightStrLen + 1];
    memcpy(height, heightStart, heightStrLen);
    strcpy(height + heightStrLen, "\0");
    printf("Height: %s\n", height);

    //Get Board
    char *boardStart = heightEnd + 18;
    char *boardEnd = strstr(boardStart, "\"");
    int boardStrLen = boardEnd - boardStart;
    char board[boardStrLen + 1];
    memcpy(board, boardStart, boardStrLen);
    strcpy(board + boardStrLen, "\0");
    //printf("Board: %s\n", board);

    int col = atoi(width);
    int row = atoi(height);
    int **grid = (int **)malloc(row * sizeof(int *));
    for (int i = 0; i < row; i++) {
        grid[i] = (int *)malloc(col * sizeof(int));
    }

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            int index = (i * col) + j;
            if (board[index] == 'X') {
                grid[i][j] = 1;
            } else {
                grid[i][j] = 0;
            }
        }
    }
    struct board b = {.grid = grid, .row = row, .col = col, .path = malloc(1000), .step = 0, .bufmax = 1000, .complete = false, .remainingSquares = 0};
    return b;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    //printf("\nwriteback called\n");
    size_t data_size = size * nmemb;
    //printf("\nSize of user data: %zu\n", data_size);
    char ** data = (char **) userdata;
    if (*data == NULL) {
        //printf("Allocating memory first time\n");
        *data = malloc(data_size + 1);
        if (*data == NULL) {
            fprintf(stderr, "out of memory\n");
            return 0;
        }
        memcpy(*data, ptr, data_size);
        (*data)[data_size] = '\0';
    } else {
        //printf("Reallocating memory\n");
        int part1_size = strlen(*data);
        //printf("size of part 1 is: %d\n", part1_size);
        *data = realloc(*data, strlen(*data) + data_size + 1);
        if (*data == NULL) {
            fprintf(stderr, "out of memory\n");
            return 0;
        }
        memcpy(*data + part1_size, ptr, data_size);
        (*data)[strlen(*data) + data_size] = '\0';
    }
    return size * nmemb;
}

char* getAPIBoardData() {
    CURL *curl;
    CURLcode res;
    char* data = NULL;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.hacker.org/coil/index.php/?name=KAYTEE&password=pokemon123");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);

    }
    return data;
}

void printGrid(struct board *grid) {
    for (int i = 0; i < grid->row; i++) {
        for (int j = 0; j < grid->col; j++) {
            printf("%d", grid->grid[i][j]);
        }
        printf("\n");
    }
}

struct point {
    int i;
    int j;
};

struct point goUp(struct board *grid, int i, int j) {
    int i2 = i;
    int j2 = j;
    while (i2 - 1 >= 0 && grid->grid[i2 - 1][j2] == 0) {
        i2--;
        grid->grid[i2][j2] = 1;
        grid->remainingSquares -= 1;
    }
    struct point p = {i2, j2};
    return p;
}

struct point goDown(struct board *grid, int i, int j) {
    int i2 = i;
    int j2 = j;
    while (i2 + 1 < grid->row && grid->grid[i2 + 1][j2] == 0) {
        i2++;
        grid->grid[i2][j2] = 1;
        grid->remainingSquares -= 1;
    }
    struct point p = {i2, j2};
    return p;
}

struct point goLeft(struct board *grid, int i, int j) {
    int i2 = i;
    int j2 = j;
    while (j2 - 1 >= 0 && grid->grid[i2][j2 - 1] == 0) {
        j2--;
        grid->grid[i2][j2] = 1;
        grid->remainingSquares -= 1;
    }
    struct point p = {i2, j2};
    return p;
}

struct point goRight(struct board *grid, int i, int j) {
    int i2 = i;
    int j2 = j;
    while (j2 + 1 < grid->col && grid->grid[i2][j2 + 1] == 0) {
        j2++;
        grid->grid[i2][j2] = 1;
        grid->remainingSquares -= 1;
    }
    struct point p = {i2, j2};
    return p;
}

//Undo position at (i2, j2) back to original (i, j). (i,j) remains blocked.
void undoGoUp(struct board *grid, int i, int j, int i2, int j2) {
    while (i2 < i) {
        grid->grid[i2][j2] = 0;
        grid->remainingSquares += 1;
        i2++;
    }
}

void undoGoDown(struct board *grid, int i, int j , int i2, int j2) {
    while (i2 > i) {
        grid->grid[i2][j2] = 0;
        grid->remainingSquares += 1;
        i2--;
    }
}

void undoGoLeft(struct board *grid, int i, int j , int i2, int j2) {
    while (j2 < j) {
        grid->grid[i2][j2] = 0;
        grid->remainingSquares += 1;
        j2++;
    }
}

void undoGoRight(struct board *grid, int i, int j, int i2, int j2) {
    while (j2 > j) {
        grid->grid[i2][j2] = 0;
        grid->remainingSquares += 1;
        j2--;
    }
}

int dfs(struct board *grid, int i, int j) {
    if (grid->remainingSquares == 0) {
        printf("Solved\n");
        //printGrid(grid);
        return 1;
    }
    if (grid->step >= grid->bufmax) {
        printf("bufmax reached. Reallocating more data to path buffer...\n");
        grid->path = realloc(grid->path, grid->bufmax + 1000);
        grid->bufmax += 1000;
    }
    
    //check up
    if (i != 0 && grid->grid[i - 1][j] == 0) {
        grid->path[grid->step] = 'U';
        grid->step += 1;
        //printf("Start going up\n");
        struct point endPoint = goUp(grid, i, j);
        if (dfs(grid, endPoint.i, endPoint.j) == 1) return 1;
        grid->step -= 1;
        //printf("Undo up\n");
        undoGoUp(grid, i, j, endPoint.i, endPoint.j);
    } 
    //check down
    
    if (i + 1 < grid->row && grid->grid[i + 1][j] == 0) {
        grid->path[grid->step] = 'D';
        grid->step += 1;
       // printf("Start going down\n");
        struct point endPoint = goDown(grid, i, j);
        if (dfs(grid, endPoint.i, endPoint.j) == 1) return 1;
        grid->step -= 1;
       // printf("Undo down\n");
        undoGoDown(grid, i, j, endPoint.i, endPoint.j);
    } 
    
    //check left
    if (j != 0 && grid->grid[i][j - 1] == 0) {
        grid->path[grid->step] = 'L';
        grid->step += 1;
       // printf("Start going left\n");
        struct point endPoint = goLeft(grid, i, j);
        if (dfs(grid, endPoint.i, endPoint.j) == 1) return 1;
        grid->step -= 1;
       // printf("Undo left\n");
        undoGoLeft(grid, i, j, endPoint.i, endPoint.j);
    }
    //check right
    
    if (j + 1 < grid->col && grid->grid[i][j + 1] == 0) {
        grid->path[grid->step] = 'R';
        grid->step += 1;
       // printf("Start going right\n");
        struct point endPoint = goRight(grid, i, j);
        if (dfs(grid, endPoint.i, endPoint.j) == 1) return 1;
        grid->step -= 1;
       // printf("Undo right\n");
        undoGoRight(grid, i, j, endPoint.i, endPoint.j);
    }
    return 0;
}


struct point mortalSolve(struct board *grid) {
    //iterate through grid, for each empty spot, call dfs
    for (int i = 0; i < grid->row; i++) {
        for (int j = 0; j < grid->col; j++) {
            if (grid->grid[i][j] == 0) {
                grid->remainingSquares += 1;
            } 
        }
    }
    //printf("Total empty squares: %d\n", grid->remainingSquares);
    for (int i = 0; i < grid->row; i++) {
        for (int j = 0; j < grid->col; j++) {
            if (grid->grid[i][j] == 0) { //if space is empty, then it is a valid start point
                grid->grid[i][j] = 1;
                grid->remainingSquares -= 1;
                int solutionFound = dfs(grid, i, j);
                struct point p = {i, j};
                if (solutionFound == 1) {
                    return p;
                }
                grid->grid[i][j] = 0;
                grid->remainingSquares += 1;
            }
        }
    }
    struct point p = {-1, -1};
    return p;
}

size_t handle_response(char *ptr, size_t size, size_t nmemb, void *userdata) {
    // Do nothing. We just return the number of bytes processed.
    return size * nmemb;
}

void submitAPIBoardSolution(char *path, int i, int j) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        char query[1000];
        sprintf(query, "http://www.hacker.org/coil/index.php/?name=KAYTEE&password=pokemon123&path=%s&x=%d&y=%d", path, j, i);
        curl_easy_setopt(curl, CURLOPT_URL, query);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
}

int main() {
    while (true) {
        // start timer
        time_t start_time;
        time(&start_time);

        // get problem, solve, submit solution
        char* data = getAPIBoardData(); //don't forget to free data
        struct board grid = extractInfo(data);
        free(data);
        struct point startPosition = mortalSolve(&grid);
        printf("Starting position: (%d, %d)\n", startPosition.i, startPosition.j);
        grid.path[grid.step ] = '\0';
        submitAPIBoardSolution(grid.path, startPosition.i, startPosition.j);

        // solve time calc and printing
        time_t end_time;
        time(&end_time);
        int time = ((int) (end_time - start_time));
        printf("Solve time: %d seconds\n\n", time);

        //need to free grid and each row within grid
        for (int i = 0; i < grid.row; i++) {
            free(grid.grid[i]);
        }
        free(grid.grid);
        free(grid.path);
    }      
    return 0;
}