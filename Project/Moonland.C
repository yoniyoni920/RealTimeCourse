/* game.c - xmain, prntr */

#include <stdio.h>
#include <dos.h>
#include <time.h>
#include "M_t.h"
#include <string.h>

#define ARROW_NUMBER 80
#define TARGET_NUMBER 4
#define ARRSIZE 1000


typedef struct position
{
    int x;
    int y;

} POSITION;

void interrupt (*old_int9)(void);

char entered_ascii_codes[ARRSIZE];
int front = -1;
int rear = -1;

int game_over = 0;
int tail = -1;
char ch_arr[ARRSIZE];
int terrain[25][80];
int display_draft[25][80];
unsigned char far *b800h;
int color_draft[25][80]; // Same as display draft but for coloring
POSITION ship_pos;
POSITION ship_vel;

int passes = 0;
int score = 0;
int lives = 3;
int fuel = 10;
int diff = 1;
int active_thrust;

int flag = 0;
int velocity_flag = 0;

const char ship[][6] = {
    "  _",
    " | |",
    " (_)",
    "//_\\\\"
};

/*CHANGE */
int target_flags[TARGET_NUMBER];
void interrupt (*old_int9)(void);

void quit_game()
{
    outport(0x20, 0x20);
    setvect(9, old_int9);
    asm {
        CLI         // Clear Interrupt Flag, disabling maskable interrupts
    }
    exit(0);
} // quit_game()

void start_game() {
    memset(terrain, 0, sizeof(terrain));
    make_terrain(terrain, diff);
    ship_pos.x = 1;
    ship_pos.y = 0;
    ship_vel.x = 1;
    ship_vel.y = 1;
}

void reset_game() {
    diff = 1;
    score = 0;
    lives = 3;
    fuel = 10;
    game_over = 0;
    passes = 0;
    start_game();
}

void interrupt new_int9(void)
{
    unsigned char scan;

    scan = inport(0x60);   // Read raw scan code

    if (scan & 0x80) {
        // KEY RELEASE
        scan &= 0x7F;   // remove release bit

        if (scan == 75) { entered_ascii_codes[++tail] = 'a'; }
        else if (scan == 72) { entered_ascii_codes[++tail] = 'w'; }
        else if (scan == 77) { entered_ascii_codes[++tail] = 'd'; }
        else if (scan == 0x1C) { 
            if (game_over) {
                reset_game();
            }   
        }
        else if (scan == 1) quit_game();
    }

    outport(0x20, 0x20); // End Of Interrupt (EOI)
}

void displayer(void)
{
    int i;
    int row, col;
    char c;
    
    char hud_text[81];
    char msg_score[40];
    char msg_gameover[] = "GAME OVER";
    char press_enter[] = "PRESS ENTER TO CONTINUE";
    // for(i=0; i < 2000; i++) {
    //     b800h[2*i+1] = 0x08; // black over white
    // }
    if(!game_over){
        int x_pos = 5;
        sprintf(hud_text, "Fuel:%4d   Altitude:%2d   X.speed:%2d   Y.Speed:%2d Score:%4d  Lives:%2d", 
                fuel, (20 - ship_pos.y), ship_vel.x * diff, ship_vel.y * diff,score,lives);
        for(i = 0; i < strlen(hud_text); i++) {
            display_draft[1][x_pos+i] = hud_text[i];
            color_draft[1][x_pos+i] = 0x1F;
        }


    } else {
        int x_pos = 30;
        for(row=0; row < 25; row++) {
            for(col=0; col < 80; col++) {
                color_draft[row][col] = 0x4E; // Yellow on Red
            }
        }

        for(i=0; i < strlen(msg_gameover); i++) {
            display_draft[12][x_pos + i] = msg_gameover[i];
        }
        sprintf(msg_score, "FINAL SCORE: %10d", score);
        for(i=0; i < strlen(msg_score); i++) {
            display_draft[13][x_pos + i] = msg_score[i];
        }
        for(i=0; i < strlen(press_enter); i++) {
            display_draft[14][x_pos + i] = press_enter[i];
        }
    }

    for(row=0; row < 25; row++) {
        for(col=0; col < 80; col++) {
            i = 2*(row*80 + col);

            b800h[i] = display_draft[row][col];
            b800h[i+1] = color_draft[row][col];
        } // for
    }

} // displayer


void receiver()
{
    char temp;
    while(tail > -1)
    {
        temp = entered_ascii_codes[tail];
        rear++;
        tail--;
        if (rear < ARRSIZE) {
            ch_arr[rear] = temp;
        }
        if (front == -1) {
            front = 0;
        }
    } // while

} // receiver

// CHANGE
void update_ship_pos()
{
    int i,j;

    if (flag == 0) {
        ship_pos.x += ship_vel.x;
        
        // Fix X position
        if (ship_pos.x < 0) {
            ship_pos.x += 80;
        }
        ship_pos.x %= 80;

        if (active_thrust > 0) {
            ship_vel.y--;
            active_thrust--;
        } else if (ship_vel.y < 1) {
            ship_vel.y++;
        }
        
        if ((ship_vel.y > 0 && ship_pos.y < 21) || (ship_vel.y < 0 && ship_pos.y > 2)) { // Prevent going above/below the screen
            ship_pos.y += ship_vel.y;
        }

        if (ship_pos.y+3 > 20) {
            for(i=20; i < 25; i++) {
                for(j=0; j < 80; j++) {
                    if (terrain[i][j] != NULL && j >= ship_pos.x && j <= ship_pos.x+5 && i <= ship_pos.y+3 && i >= ship_pos.y) {
                        //NULL is 0 which is nothing (air), so if we are not in air and - 
                        //Is point j between the left and right walls of the ship?
                        //Is point i between the roof and the floor of the ship?
                        //Together we get answer to the question is there something inside the ship? - did we hit anything?
                        ship_vel.x = 0;// stop the ship
                        ship_vel.y = 0;

                    
                        if (terrain[i][j] == '_') {// landed saftly
                            //print you landed saftly got 100 points and 10 fuel
                            //if you get too 300 points you win message end game
                            passes++;
                            score+=100;
                            fuel += 5;
                            if (passes == 3) {
                                diff++; // Increase game's difficulty
                            }
                            start_game();
                        }else{
                            // ship did not land on flat surface - hit obstacle
                            // minus 50 points
                            lives--;
                            score-=50;

                            if (fuel <= 0 || lives <= 0) {
                                game_over = 1; 
                            } else {
                                passes = 0;
                                if (diff > 1) {
                                    diff--;
                                }
                                // Retry
                                start_game();
                            }
                           
                        }
                        return;
                    }
                }
            }

        }
    }

    flag++;
    flag %= (4 - diff);
    velocity_flag++;
    velocity_flag %= 4;


  //} // while

} //update_ship_pos()

// Helper function to render something at some point in the screen with color
void render_col(int col, int row, char c, int attr) {
    if (row >= 0 && row < 25 && col >= 0 && col < 80) {
        display_draft[row][col] = c;  
        color_draft[row][col] = attr;
    }
}

void updater()
{
    int input_w = 0, input_d = 0, input_a = 0;
    int i,j;
    int show_exhaust = 0;
    int x = ship_pos.x, y = ship_pos.y;

    if (game_over) {
        return;
    }

    update_ship_pos();

    for(i=0; i < 25; i++) {
        for(j=0; j < 80; j++) {
            char c = terrain[i][j];
            if (c != NULL) {
                display_draft[i][j] = c;
            } else {
                display_draft[i][j] = ' ';  // blank
            }

            color_draft[i][j] = 0x0f;  // blank
        }
    }

   while(front != -1)  {//Keep looping as long as the queue is not empty
        char ch = ch_arr[front];//take the first char
        if(front != 0) {
            front++;
        } else {
            front = rear = -1;
        }

        if (ch == 'w' || ch == 'W') input_w = 1;
        if (ch == 'd' || ch == 'D') input_d = 1;
        if (ch == 'a' || ch == 'A') input_a = 1;
    }// while(front != -1)

    if ((input_w) && fuel > 0) {
        if (ship_vel.y > -3 ) {
            active_thrust = 3;
            fuel--;
        }
    }
    if ((input_d) && fuel > 0 && ship_vel.x <=2 ) {
        ship_vel.x++;
        show_exhaust |= 1;
        fuel--;
        
    }
    if ((input_a) && fuel > 0 && ship_vel.x >=-2 ) {
        ship_vel.x--;
        show_exhaust |= 2;
        fuel--;
        
    }

    for (i = 0; i < 4; i++) {
        char* s = ship[i];
        for (j = 0; j < strlen(s); j++) {
            char c = s[j];
            if (c != ' ') {
                display_draft[y + i][x - 2 + j] = c;
            }
        }
    }

    // Will be the exhaust fire
   if (active_thrust > 0) {
        render_col(x-1, y+4, '\\', 0x0c);
        render_col(x+1, y+4, '/', 0x0c);
        render_col(x, y+5, 'v', 0x0c);
    }
    // 2. Left exhaust
    else if (show_exhaust & 1) {
        render_col(x-3, y+1, '/', 0x0c);  
        render_col(x-3, y+2, '=', 0x0e);  
        render_col(x-4, y+2, '<', 0x0c);  
        render_col(x-3, y+3, '\\', 0x0c); 
    }
    
    // 3. Right  exhaust
    else if (show_exhaust & 2) {
        render_col(x+4, y+1, '\\', 0x0c); 
        render_col(x+4, y+2, '=', 0x0e);  
        render_col(x+5, y+2, '>', 0x0c);  
        render_col(x+4, y+3, '/', 0x0c); 
    }
} // updater 



void main() {
    int uppid, dispid, recvpid, targid;
    int i;
 
    for(i=0; i< TARGET_NUMBER; i++) {
        target_flags[i] = 1;
    }


    asm {
        // Assume b800h is a pointer to the start of video memory
        MOV WORD PTR b800h[2],0b800h // Set the segment part (at offset +2) to B800h (CGA/VGA text mode segment)
        MOV WORD PTR b800h, 0        // Set the offset part (at offset +0) to 0
    }

    asm {
        MOV AX,3    // AX = 0003h (Set video mode to 80x25 color text)
        INT 10h     // BIOS Video Interrupt
    }

    old_int9 = getvect(9);
    setvect(9, new_int9);

    make_terrain(terrain, diff);

    reset_game();

    while(1) {
        displayer();
        receiver();
        updater();
   
        delay(150);
        // sleep(1);
    } // while
    setvect(9, old_int9);
    
    asm {
        MOV AX, 3
        INT 10h
    }
    exit(0);
} // main