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

//char entered_ascii_codes[ARRSIZE];
int game_over = 0;
int tail = -1;
char next_cm = 0;
char ch_arr[ARRSIZE];
int terrain[25][80];
int display_draft[25][80];
int color_draft[25][80]; // Same as display draft but for coloring
int game_running = 1;
unsigned char far *b800h;
int gun_position;           
int no_of_arrows;
int target_disp = 80/TARGET_NUMBER;
char ch;
int no_of_targets;
int score = 0;
int lives = 3;
int front = -1;
int rear = -1;

int point_in_cycle;
int gcycle_length;
int gno_of_pids;

POSITION ship_pos;
POSITION ship_vel;
int fuel = 10;

int flag = 0;
int target_freq = 4;


/*CHANGE */
int target_flags[TARGET_NUMBER];
void interrupt (*old_int9)(void);

void my_halt()
{
    setvect(9, old_int9);
    asm {
        CLI         // Clear Interrupt Flag, disabling maskable interrupts
    }
    exit(0);
   
} // my_halt()


void interrupt new_int9(void)
{
    char result = 0;
    int scan = 0;
    int ascii = 0;

    (*old_int9)(); // Call the original INT 9 (Hardware Keyboard) handler


    asm {
        MOV AH,1    // Function 1: Check if a key is waiting in the keyboard buffer
        INT 16h     // BIOS Keyboard Interrupt
        JZ Skip1    // Jump if Zero (ZF is set, meaning no key is available)
        MOV AH,0    // Function 0: Read key from keyboard buffer and wait if necessary
        INT 16h     // BIOS Keyboard Interrupt
        MOV BYTE PTR scan,AH // AH contains the scan code
        MOV BYTE PTR ascii,AL// AL contains the ASCII code
    } //asm

    ascii = 0;
    if (scan == 0x48) { // Up arrow or W
        ascii = 'w';
    }
    else if (scan == 0x4B) { // Left Arrow
        ascii = 'a';
    }
    else if (scan == 0x4D) { // Right Arrow
        ascii = 'd';
    }
    if (scan == 1) { // Esc
        game_running = 0; // terminate program
    }
    next_cm = ascii;
    // if ((ascii != 0) && (tail < ARRSIZE))
    // {
    //     entered_ascii_codes[++tail] = ascii;
    // } // if

Skip1:

} // new_int9

void displayer(void)
{
    int i;
    int row, col;
    char c;
    
    char hud_text[81];
    char msg_score[40];
    char msg_gameover[] = "GAME OVER";
    // for(i=0; i < 2000; i++) {
    //     b800h[2*i+1] = 0x08; // black over white
    // }
    if(!game_over){
        for(row=0; row < 25; row++) {
            for(col=0; col < 80; col++) {
                i = 2*(row*80 + col);

                b800h[i] = display_draft[row][col];
                b800h[i+1] = color_draft[row][col];
            } // for
        }
        sprintf(hud_text, "Fuel:%4d   Altitude:%2d   X.speed:%2d   Y.Speed:%2d Score:%4d  Lives:%2d", 
                fuel, (20 - ship_pos.y), ship_vel.x, ship_vel.y,score,lives);
                for(i = 0; i < strlen(hud_text); i++) {
                    b800h[2*i] = hud_text[i];
                    b800h[2*i+1] = 0x1F;
                }
        }else{
            for(i=0; i < 2000; i++) {
            b800h[2*i] = ' '; 
            b800h[2*i+1] = 0x4F;
        }
        for(i=0; i < strlen(msg_gameover); i++) {
            int offset = 2 * (10 * 80 + 35 + i); 
            b800h[offset] = msg_gameover[i];
            b800h[offset+1] = 0x4E; // Yellow on Red
        }
        sprintf(msg_score, "FINAL SCORE: %d", score);
        for(i=0; i < strlen(msg_score); i++) {
            int offset = 2 * (12 * 80 + 35 + i); 
            b800h[offset] = msg_score[i];
            b800h[offset+1] = 0x4F;
        }

        }

        } // displayer


// void receiver()
// {
//     // char temp;
//     // while(tail > -1)
//     // {
//     //     temp = entered_ascii_codes[tail];
//     //     rear++;
//     //     tail--;
//     //     // if (rear < ARRSIZE) {
//     //     //     ch_arr[rear] = temp;
//     //     // }
//     //     // if (front == -1) {
//     //     //     front = 0;
//     //     // }
//     // } // while

// } // receiver

// CHANGE
void update_ship_pos()
{
    int i,j;
    static int initial_run = 1;

    if (initial_run == 1) {
        initial_run = 0;
        ship_pos.x = 2;
        ship_pos.y = 0;
        ship_vel.x = 1;
    } // if (initial_run == 1)
    if (game_over) return;
    if (flag == 0) {
        ship_pos.x += ship_vel.x;
        ship_pos.x %= 80; // Make the ship go back to left
        
        if (ship_pos.y < 21) {
            ship_pos.y += ship_vel.y;
        }

        if (ship_vel.y < 1) {
            ship_vel.y++;
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
                            score+=100;
                            fuel += 10;
                            if(score >= 300){
                                target_freq = 2; // increase game speed
                            }
                            if (score >= 600) {
                                game_over = 1;
                            } else {
                                
                                memset(terrain, 0, sizeof(terrain));
                                make_terrain(terrain,score);
                                ship_pos.x = 2;
                                ship_pos.y = 0;
                                ship_vel.x = 1;
                                ship_vel.y = 0;
                        }
                        }else{
                            // ship did not land on flat surface - hit obstacle
                            //lose 5 fuel minus 50 points
                                lives --;
                                score-=50;
                                if(score <= 300){
                                target_freq = 4; // decrease  game speed
                            }
                            if(fuel <= 0 || lives <= 0){
                                game_over = 1; 
                            } else {
                                // Reset for retry
                                memset(terrain, 0, sizeof(terrain));
                                make_terrain(terrain,score);
                                ship_pos.x = 2;
                                ship_pos.y = 0;
                                ship_vel.x = 1;
                                ship_vel.y = 0;
                            }
                           
                        }
                        return;
                    }
                }
            }

        }
    }
    flag++;
    flag = flag % target_freq;


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
    int input_w = 0;
    int input_d = 0;
    int input_a = 0;
    int i,j;
    int show_exhaust = 0;
    static int initial_run = 1;
    int x = ship_pos.x, y = ship_pos.y;
    
char ship[][6] = {
        "  _",
        " | |",
        " (_)",
        "//_\\\\"
    };


    if (initial_run == 1) {
        initial_run = 0;
        ship_pos.x = 2;
        ship_pos.y = 0;
    }

    update_ship_pos();

    ch = 0;
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

   // while(front != -1)  {//Keep looping as long as the queue is not empty
        // ch = ch_arr[front];//take the first char
        // if(front != 0) {
        //     front++;
        // } else {
        //     front = rear = -1;
        // }

        if (next_cm == 'w' || next_cm == 'W') input_w = 1;
        if (next_cm == 'd' || next_cm == 'D') input_d = 1;
        if (next_cm == 'a' || next_cm == 'A') input_a = 1;
    //}// while(front != -1)
        if ((input_w) && fuel > 0 ) {
            if (ship_vel.y > -3 ) {
                // for some reason the speed here is backwards meaning minus speed means going up- 
                //in the diplayer i made it so positive velocity meanst up
                ship_vel.y--;
                show_exhaust = 1;
                fuel--;
                
            }
           
            
        }
        if ((input_d) && fuel > 0 && ship_vel.x <=2) {
            ship_vel.x++;
            show_exhaust = 3;
            fuel--;
            
        }
        if ((input_a) && fuel > 0 && ship_vel.x >=-2) {
            ship_vel.x--;
            show_exhaust = 2;
            fuel--;
            
        }
        input_w = 0;
        input_d = 0;
        input_a = 0;
        next_cm = 0;

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
   if (show_exhaust == 1) {
        render_col(x-1, y+4, '\\', 0x0c);
        render_col(x+1, y+4, '/', 0x0c);
        render_col(x, y+5, 'v', 0x0c);
    }
    // 2. Left exhaust
    else if (show_exhaust == 2) {
        render_col(x+4, y+1, '\\', 0x0c); 
        render_col(x+4, y+2, '=', 0x0e);  
        render_col(x+5, y+2, '>', 0x0c);  
        render_col(x+4, y+3, '/', 0x0c); 
    }
    
    // 3. Right  exhaust
    else if (show_exhaust == 3) {
        render_col(x-3, y+1, '/', 0x0c);  
        render_col(x-3, y+2, '=', 0x0e);  
        render_col(x-4, y+2, '<', 0x0c);  
        render_col(x-3, y+3, '\\', 0x0c); 
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

    make_terrain(terrain,score);

    while(game_running) {
        displayer();
        // receiver();
        updater();
   
        delay(300);
        // sleep(1);
    } // while
    setvect(9, old_int9);
    
    asm {
        MOV AX, 3
        INT 10h
    }
    exit(0);
} // main