#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "main.h"

#define MAZE_WIDTH		60
#define MAZE_HEIGHT		40

#define OPEN_TILE   ' '
#define WALL_TILE   '#'
#define FLOOR_TILE  '.'
#define START_TILE  '>'
#define END_TILE    '<'
#define DOOR_TILE   '+'
#define KEY_TILE    'k'
#define SEEN_TILE   0x80

#define ROOM_MAX_WIDTH 	7
#define ROOM_MIN_WIDTH 	3
#define MAX_ROOMS		15

#define ENEMY_GUARD		1
#define ENEMY_CAMERA	2

#define DIR_UP			1
#define DIR_DOWN		2
#define DIR_LEFT		3
#define DIR_RIGHT		4

struct Room {
    uint8_t x,y,h,w;
};

struct Player {
	uint8_t x,y,key;
};

struct Enemy {
	uint8_t type,cycle;
	uint8_t x,y,dir;
	bool alert, pause;
};

struct Room carveRandomRoom(uint8_t width, uint8_t height);
void connectRooms(struct Room r1, struct Room r2);

static uint8_t maze[MAZE_WIDTH][MAZE_HEIGHT];
static struct Player player;
static struct Enemy enemies[MAX_ROOMS];
static uint8_t enemy_count, level, new_game, game_over;

#define setCell(x, y, val) 	(maze[x][y]=val)
#define cell(x, y) (maze[x][y])

int32_t cdcWrite(const char* const buf, const uint16_t length);
void printx(const char* line) {
	cdcWrite(line, strlen(line));
}


void print_maze() {
    char line[80];
	snprintf(line, 80, "\E[H\E[JFloor: %2d\t\t%s\r\n", level, player.key?"\E[32mKey\E[0m":"");
	printx(line);
    for (int j=0; j<MAZE_HEIGHT; ++j) {
        for (int i=0; i<MAZE_WIDTH; ++i) {
        	bool covered = false;
        	char c = maze[i][j];
            bool seen=false;
            if (c & SEEN_TILE) {
                c &=0x7F;
                seen = true;
                printx("\E[30;43m");
            }
            if ((player.x == i) && (player.y==j)) {
                printx("\E[35m@\E[0m");
                continue;
            }
            for (int k=0;k<enemy_count; ++k) {
            	if ((enemies[k].x == i) && (enemies[k].y==j)) {
            		if (enemies[k].type == ENEMY_GUARD)
	                	printx("\E[31mG\E[0m");
	            	else if (enemies[k].type == ENEMY_CAMERA)
	            		printx("\E[31mC\E[0m");
	                covered = true;
	                break;
            	}
            }
            if (covered)
            	continue;
            if (c == KEY_TILE)
                printx("\E[32mk\E[0m");
            else if (c == WALL_TILE)
            	printx("\E[47;90m#\E[0m");
            else if (c == DOOR_TILE)
            	printx("\E[46;90m+\E[0m");
            else {
				char cc[2] = {c,0};
                printx(cc);
			}
            if (seen)
                printx("\E[0m");
        }
        printx("\r\n");
    }
	 printx("\E[0m");
}


void initMaze(int level) {
	uint8_t lwidth=MAZE_WIDTH, lheight=MAZE_HEIGHT;
	struct Room disconnected[MAX_ROOMS], connected[MAX_ROOMS];
	
	int room_count=5+level;
	if (room_count > MAX_ROOMS)
		room_count=MAX_ROOMS;
	int disconnected_size = 0, connected_size=0;
	if (room_count < 10) {
    	lwidth = room_count * 4 + 20;
    	lheight = room_count * 2 + 10;
    }
    
    memset(maze, OPEN_TILE, MAZE_WIDTH * MAZE_HEIGHT);
    for (int i=0; i<room_count; ++i) {
        disconnected[i] = carveRandomRoom(lwidth, lheight);
    }
    disconnected_size = room_count;

    connected[connected_size++] = disconnected[--disconnected_size];

    while (disconnected_size) {
        int best_idx=0, best_jdx=0, best_val=1000;
        for (int i=0; i<disconnected_size; ++i)
        {
            struct Room r1 = disconnected[i];
            for (int j=0; j<connected_size; ++j){
                struct Room r2 = connected[j];
                int dist = abs(r1.x+r1.w/2-(r2.x+r2.w/2)) + abs(r1.y+r1.h/2-(r2.y+r2.h/2));
                if (dist<best_val) {
                    best_val = dist;
                    best_idx = i;
                    best_jdx = j;
                }
            }
        }
        connectRooms(disconnected[best_idx], connected[best_jdx]);
        connected[connected_size++] = disconnected[best_idx];
        memmove(&disconnected[best_idx], &disconnected[best_idx+1], sizeof(struct Room) * (--disconnected_size - best_idx)); //pop room
    }

    struct Room r = connected[0];
    memmove(connected, &connected[1], sizeof(struct Room) * (--connected_size)); //pop front
    setCell(r.x + r.w/2, r.y + r.h/2, START_TILE);
    player.x = r.x + r.w/2;
    player.y = r.y + r.h/2;
	player.key = false;
    r = connected[--connected_size];
    setCell(r.x + r.w/2, r.y + r.h/2, END_TILE);
    //Walk the edges of exit room and place locked doors
    
    //first check corners and only if they are intact then check walls
    if (cell(r.x-1,r.y-1) == FLOOR_TILE) {
        setCell(r.x-1,r.y-1, DOOR_TILE);
    }
    else if (cell(r.x-1,r.y+r.h) == FLOOR_TILE) {
        setCell(r.x-1,r.y+r.h, DOOR_TILE);
    }
    else if (cell(r.x+r.w,r.y-1) == FLOOR_TILE) {
        setCell(r.x+r.w,r.y-1, DOOR_TILE);
    }
    else if (cell(r.x+r.w,r.y+r.h) == FLOOR_TILE) {
        setCell(r.x+r.w,r.y+r.h, DOOR_TILE);
    }
    else {
        for (uint8_t x= r.x; x<r.x+r.w; ++x) {
            if (cell(x,r.y-1) == FLOOR_TILE) {
                setCell(x,r.y-1, DOOR_TILE);
            }
            if (cell(x,r.y+r.h) == FLOOR_TILE) {
                setCell(x,r.y+r.h, DOOR_TILE);
            }
        }
        for (uint8_t y= r.y; y<r.y+r.h; ++y) {
            if (cell(r.x-1,y) == FLOOR_TILE) {
                setCell(r.x-1,y, DOOR_TILE);
            }
            if (cell(r.x+r.w,y) == FLOOR_TILE) {
                setCell(r.x+r.w,y, DOOR_TILE);
            }
        }
    }
    
    //Add Key
    int k = rand_sync_read32(&RAND_0)%connected_size;
    r = connected[k];
    setCell(r.x+rand_sync_read32(&RAND_0)%r.w, r.y+rand_sync_read32(&RAND_0)%r.h, KEY_TILE);

    enemy_count = connected_size;
    
    for (int i=0; i<connected_size; ++i) {
        r = connected[i];
        switch(rand_sync_read32(&RAND_0)%3) {
            case 0:
            case 1:
            	enemies[i] = (struct Enemy){ENEMY_GUARD, 0, r.x+rand_sync_read32(&RAND_0)%r.w, r.y+rand_sync_read32(&RAND_0)%r.h, (rand_sync_read32(&RAND_0)%4)+1,false,false};
                break;
            case 2:
                switch(rand_sync_read32(&RAND_0)%4) {
                    case 0:
                    	enemies[i] = (struct Enemy){ENEMY_CAMERA, 0, r.x, r.y, DIR_DOWN,false,false};
                        break;
                    case 1:
                    	enemies[i] = (struct Enemy){ENEMY_CAMERA, 0, r.x+r.w-1, r.y, DIR_RIGHT,false,false};
                        break;
                    case 2:
                    	enemies[i] = (struct Enemy){ENEMY_CAMERA, 0, r.x+r.w-1,r.y+r.h-1, DIR_UP,false,false};
                        break;
                    case 3:
                    	enemies[i] = (struct Enemy){ENEMY_CAMERA, 0, r.x, r.y+r.h-1, DIR_LEFT,false,false};
                        break;
                }
                break;
        }
    }
	new_game = false;
	game_over=false;
}

bool isValidRoom(struct Room r) {
	for (int x=r.x; x<r.x+r.w; ++x)
        for (int y=r.y; y<r.y+r.h; ++y)
            if (cell(x,y) != OPEN_TILE)
                return false;
    return true;
}

void carveXTunnel(uint8_t x1, uint8_t x2, uint8_t y) {
    if (x1 > x2) {
        uint8_t t = x2;
        x2 = x1; x1 = t;
    }
    for (int x=x1;x<=x2;++x) {
        if (cell(x, y-1)==OPEN_TILE)
            setCell(x, y-1, WALL_TILE);
        if (cell(x, y)!=DOOR_TILE)
            setCell(x, y, FLOOR_TILE);
        if (cell(x, y+1)==OPEN_TILE)
            setCell(x, y+1, WALL_TILE);
    }
    if (cell(x1-1, y-1) == OPEN_TILE) setCell(x1-1, y-1, WALL_TILE);
    if (cell(x1-1, y  )==OPEN_TILE) setCell(x1-1, y  , WALL_TILE);
    if (cell(x1-1, y+1)==OPEN_TILE) setCell(x1-1, y+1, WALL_TILE);
    if (cell(x2+1, y-1)==OPEN_TILE) setCell(x2+1, y-1, WALL_TILE);
    if (cell(x2+1, y  )==OPEN_TILE) setCell(x2+1, y  , WALL_TILE);
    if (cell(x2+1, y+1)==OPEN_TILE) setCell(x2+1, y+1, WALL_TILE);
}

void carveYTunnel(uint8_t y1, uint8_t y2, uint8_t x) {
    if (y1 > y2) {
        uint8_t t = y2;
        y2 = y1; y1 = t;
    }
    for (int y=y1;y<=y2;++y) {
        if (cell(x-1, y)==OPEN_TILE)
            setCell(x-1, y, WALL_TILE);
        if (cell(x, y)!=DOOR_TILE)
            setCell(x, y, FLOOR_TILE);
        if (cell(x+1, y)==OPEN_TILE)
            setCell(x+1, y, WALL_TILE);
    }
    if (cell(x-1, y1-1)==OPEN_TILE) setCell(x-1, y1-1, WALL_TILE);
    if (cell(x  , y1-1)==OPEN_TILE) setCell(x  , y1-1, WALL_TILE);
    if (cell(x+1, y1-1)==OPEN_TILE) setCell(x+1, y1-1, WALL_TILE);
    if (cell(x-1, y1-1)==OPEN_TILE) setCell(x-1, y2+1, WALL_TILE);
    if (cell(x  , y1-1)==OPEN_TILE) setCell(x  , y2+1, WALL_TILE);
    if (cell(x+1, y1-1)==OPEN_TILE) setCell(x+1, y2+1, WALL_TILE);
}

void connectRooms(struct Room r1, struct Room r2) {
    uint8_t x1 = r1.x + (rand_sync_read32(&RAND_0)%2) * (r1.w-1);
    uint8_t y1 = r1.y + (rand_sync_read32(&RAND_0)%2) * (r1.h-1);
    uint8_t x2 = r2.x + (rand_sync_read32(&RAND_0)%2) * (r2.w-1);
    uint8_t y2 = r2.y + (rand_sync_read32(&RAND_0)%2) * (r2.h-1);
    
    uint8_t w = rand_sync_read32(&RAND_0)%2;
    carveXTunnel(x1, x2, w?y1:y2);
    carveYTunnel(y1, y2, w?x2:x1);
}

struct Room carveRandomRoom(uint8_t width, uint8_t height) {
    struct Room r;
    int i=0;
    do
    {
        r.w = rand_sync_read32(&RAND_0)%(ROOM_MAX_WIDTH-ROOM_MIN_WIDTH) + ROOM_MIN_WIDTH;
        r.h = rand_sync_read32(&RAND_0)%(ROOM_MAX_WIDTH-ROOM_MIN_WIDTH) + ROOM_MIN_WIDTH;
        r.x = rand_sync_read32(&RAND_0)%(width-2-r.w)+1;
        r.y = rand_sync_read32(&RAND_0)%(height-2-r.h)+1;
        i++;
    } while(!isValidRoom(r));
        
    for (uint8_t x= r.x-1; x<=r.x+r.w; ++x) {
        setCell(x, r.y-1, WALL_TILE);
        setCell(x, r.y+r.h, WALL_TILE);
    }
    for (uint8_t y= r.y-1; y<=r.y+r.h; ++y) {
        setCell(r.x-1, y, WALL_TILE);
        setCell(r.x+r.w, y, WALL_TILE);
    }
    for (uint8_t x =r.x; x<r.x+r.w; ++x)
        for (uint8_t y=r.y; y<r.y+r.h; ++y)
            setCell(x, y, FLOOR_TILE);
    return r;
}

void clearVison() {
	for (int y=0; y< MAZE_HEIGHT; ++y)
		for (int x=0; x< MAZE_WIDTH; ++x)
			maze[x][y] &= 0x7F;
}

void visionLine(int x1, int y1, int x2, int y2) {
	int dx = abs(x2-x1), sx = x1<x2?1:-1;
    int dy = abs(y2-y1), sy = y1<y2?1:-1;
    
    int err = dx-dy, e2;
    int x=x1, y=y1;
    for(;;){
      if ((maze[x][y] & 0x7F) != FLOOR_TILE)
          break;
      maze[x][y] |= SEEN_TILE;
      if (x==x2 && y==y2) break;
      e2 = 2*err;
      if (e2 >-dy) { err -= dy; x += sx; }
      if (e2 < dx) { err += dx; y += sy; }
    }
}

void calcGaurdVision(struct Enemy e) {
	const int guard_vision[5][2] = {{2,3}, {1,4}, {0,5}, {-1,4}, {-2,3}};
	for (int j=0; j<5; ++j) {
		int x = e.x, y=e.y;
		switch(e.dir) {
			case DIR_DOWN:
				x += guard_vision[j][0];
				y += guard_vision[j][1];
				break;
			case DIR_UP:
				x -= guard_vision[j][0];
				y -= guard_vision[j][1];
				break;
			case DIR_RIGHT:
				x += guard_vision[j][1];
				y += guard_vision[j][0];
				break;
			case DIR_LEFT:
				x -= guard_vision[j][1];
				y -= guard_vision[j][0];
				break;
		}
		visionLine(e.x, e.y, x, y);
	}
}

void calcCameraVision(struct Enemy e) {
	int vis[5][2] = {{4,0}, {3,1}, {2,2}, {1,3}, {0,4}};
	if (e.cycle < 6)
		return;
	else if (e.cycle == 6) {
		visionLine(e.x, e.y, e.x, e.y);
		return;
	}
	else {
		for (int j=0; j<5; ++j) {
			int x = e.x, y=e.y;
			if (e.cycle == 7) {
				vis[j][0] /= 4;
				vis[j][1] /= 4;
			}
			else if (e.cycle == 8) {
				vis[j][0] /= 2;
				vis[j][1] /= 2;
			}
    		else if (e.cycle == 9){
				vis[j][0] *= 3;
				vis[j][1] *= 3;
				vis[j][0] /= 4;
				vis[j][1] /= 4;
			}
			switch(e.dir) {
				case DIR_DOWN:
					x += vis[j][0];
					y += vis[j][1];
					break;
				case DIR_UP:
					x -= vis[j][0];
					y -= vis[j][1];
					break;
				case DIR_RIGHT:
					x -= vis[j][0];
					y += vis[j][1];
					break;
				case DIR_LEFT:
					x += vis[j][0];
					y -= vis[j][1];
					break;
			}
			visionLine(e.x, e.y, x, y);
		}
	}
}

void calcVision() {
	
	for  (int i=0; i<enemy_count; ++i) {
		struct Enemy e = enemies[i];
		if (e.type == ENEMY_GUARD) {
			calcGaurdVision(e);
		}
		else if (e.type == ENEMY_CAMERA) {
			calcCameraVision(e);
		}

	}
}

void guard_think(struct Enemy *e) {
	if (e->pause++) {
        uint8_t moveto[4];
        switch (e->dir) {
            case DIR_UP:
                moveto[0] = DIR_UP;
                moveto[1] = DIR_RIGHT;
                moveto[2] = DIR_LEFT;
                moveto[3] = DIR_DOWN;
                break;
            case DIR_DOWN:
                moveto[0] = DIR_DOWN;
                moveto[1] = DIR_LEFT;
                moveto[2] = DIR_RIGHT;
                moveto[3] = DIR_UP;
                break;
            case DIR_LEFT:
                moveto[0] = DIR_LEFT;
                moveto[1] = DIR_UP;
                moveto[2] = DIR_DOWN;
                moveto[3] = DIR_RIGHT;
                break;
            case DIR_RIGHT:
                moveto[0] = DIR_RIGHT;
                moveto[1] = DIR_DOWN;
                moveto[2] = DIR_UP;
                moveto[3] = DIR_LEFT;
                break;
        }
        e->pause = 0;
        for (int i=0; i<4; ++i) {
            int x=0,y=0;
            switch (moveto[i]) {
                case DIR_UP: x=0; y=-1; break;
                case DIR_DOWN: x=0; y=1; break;
                case DIR_LEFT: x=-1; y=0; break;
                case DIR_RIGHT: x=1; y=0; break;
                default: break;
            }
            if ((maze[e->x+x][e->y+y] & 0x7F)==FLOOR_TILE) {
            	e->x+=x;
            	e->y+=y;
                e->dir = moveto[i];
                return;
            }
        }
    }
}

void enemyThink() {
	for  (int i=0; i<enemy_count; ++i) {
		struct Enemy e = enemies[i];
		if (e.type == ENEMY_GUARD) {
			guard_think(&enemies[i]);
		}
		else if (e.type == ENEMY_CAMERA) {
			enemies[i].cycle = ++e.cycle %12;
		}

	}
}

bool playerInput(uint8_t dir) {
	int x=0,y=0;
	switch (dir) {
		case DIR_UP:y=-1;break;
		case DIR_DOWN:y=1;break;
		case DIR_LEFT:x=-1;break;
		case DIR_RIGHT:x=1;break;
		default:
			return true; //no action
	}
	char c = maze[player.x+x][player.y+y] &0x7F;
	switch (c) {
		case FLOOR_TILE: 
		case END_TILE:
			break;
		case KEY_TILE:
			player.key = true;
			maze[player.x+x][player.y+y] = FLOOR_TILE;
			break;
		case DOOR_TILE:
			if (player.key) {
				player.key = false;
				maze[player.x+x][player.y+y] = FLOOR_TILE;
			}
			else
				return false;
		default:
			return false;
	}
	
	player.x+=x;
	player.y+=y;
	return true;
}

void doStep() {
	clearVison();
	enemyThink();
	calcVision();
	print_maze();
	char c = maze[player.x][player.y];
	if (c == END_TILE) {
		printx("You move up passed this floor.\n");
		level++;
		new_game = true;
	}
	if (c & SEEN_TILE) {
		printx("Game Over! Press R to restart\n");
		if (level >= 5)
			rouge_event = true;
		level = 1;
		game_over = true;
	}
}

void initNewGame() {
	level = 1;
	initMaze(level);
	print_maze();
}

void handleInput(char *c, uint8_t len) {
	if (new_game) {
		initMaze(level);
	}
	
	for (int i=0; i< len; ++i) {
		bool act = false;
		if (c[i] == 'r') {
			initNewGame();
		}
		if (game_over)
			continue;
		if (c[i] == ' ')  { 
			act = true;
		}
		if (c[i] == '\r')  { 
			act = true;
		}
		else if ((c[i] == '\E') && (i+2 < len) && (c[i+1] == '[')) {
			i+=2;
			switch (c[i]) {
				case 'A': 
					act = playerInput(DIR_UP);
					break;
				case 'B':
					act = playerInput(DIR_DOWN);
					break;
				case 'C':
					act = playerInput(DIR_RIGHT);
					break;
				case 'D':
					act = playerInput(DIR_LEFT);
					break;
				default:
					break;
			}
		}
		if (act)
			doStep();
		else
			print_maze();
	}
}