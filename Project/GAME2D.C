/* game.c - xmain, prntr */

#include <stdio.h>
#include <dos.h>
#include <stdio.h>
#include <time.h>

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
int tail = -1;

char ch_arr[ARRSIZE];
int terrain[25][80];
int display_draft[25][80];
int color_draft[25][80]; // Same as display draft but for coloring

unsigned char far *b800h;
int gun_position;           
int no_of_arrows;
int target_disp = 80/TARGET_NUMBER;
char ch;
int no_of_targets;


int front = -1;
int rear = -1;

int point_in_cycle;
int gcycle_length;
int gno_of_pids;

POSITION ship_pos;
POSITION ship_vel;
int fuel = 1000;

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
    exit();
   
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

    if (scan == 1) { // Esc
        my_halt(); // terminate program
    }

    if ((ascii != 0) && (tail < ARRSIZE))
    {
        entered_ascii_codes[++tail] = ascii;
    } // if

Skip1:

} // new_int9

void displayer(void)
{
    int i;
    int row, col;
    char c;

    // for(i=0; i < 2000; i++) {
    //     b800h[2*i+1] = 0x08; // black over white
    // }

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
    static int initial_run = 1;

    if (initial_run == 1) {
        initial_run = 0;
        ship_pos.x = 2;
        ship_pos.y = 0;
        ship_vel.x = 1;
    } // if (initial_run == 1)

    if (flag == 0) {
        ship_pos.x += ship_vel.x;
        ship_pos.x %= 80; // Make the ship go back to left
        
        if (ship_pos.y < 22) {
            ship_pos.y += ship_vel.y;
        }

        if (ship_vel.y < 1) {
            ship_vel.y++;
        }

        if (ship_pos.y+2 > 20) {
            for(i=20; i < 25; i++) {
                for(j=0; j < 80; j++) {
                    if (terrain[i][j] != NULL && j >= ship_pos.x && j <= ship_pos.x+5 && i <= ship_pos.y+2 && i >= ship_pos.y) {
                        ship_vel.x = 0;
                        ship_vel.y = 0;

                         // Game over, ship touched some non flat surface
                         // Not sure for now if this is the best way to check it. It currently checks each point if it's inside the "hitbox" of the ship
                        if (terrain[i][j] != '_') {

                        }
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
void render_col(int row, int col, char c, int attr) {
    display_draft[col][row] = c;
    color_draft[col][row] = attr;
}

void make_terrain() {
    int i;
    int ascending = 1, curr_y = 24;
    int flat_1_index, flat_2_index;

    // Generate two points to put flat surfaces. Could try generalizing it to put more than 2 flat surfaces
    // We put them in two separate halves to separate them
    srand(time(NULL));
    flat_1_index = rand() % 30;
    srand(time(NULL));
    flat_2_index = rand() % 30 + 40;
    
    curr_y = 24;
    for (i = 0; i < 80; i++) {
        if ((i >= flat_1_index && i <= flat_1_index+8) || (i >= flat_2_index && i <= flat_2_index+8)) {
            terrain[ascending ? curr_y : curr_y-1][i] = '_'; // Align the underscore well with desecending mountains
        } else {
            int changed = 0, was_ascending;
            
            if (ascending) {
                terrain[curr_y--][i] = '/';
                if (curr_y <= 20) {
                    changed = 1;
                    ascending = 0;
                }
            } else {
                terrain[curr_y++][i] = '\\';
                if (curr_y >= 24) {
                    ascending = 1;
                    changed = 1;
                }
            }
    
            if (!changed) {
                was_ascending = ascending;
                ascending = rand() % 2;
                changed = was_ascending != ascending;
            }
    
            if (changed) {
                if (ascending) {
                    curr_y--;
                } else {
                    curr_y++;
                }
            }
        }
    }
}

void updater()
{
    int i,j;
    int show_exhaust = 0;
    static int initial_run = 1;
    int x = ship_pos.x, y = ship_pos.y;
    
    char ship[][6] = {
        "  _",
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

    while(front != -1)  {
        ch = ch_arr[front];
        if(front != rear) {
            front++;
        } else {
            front = rear = -1;
        }

        if ((ch == 'w' || ch == 'W') && fuel > 0) {
            if (ship_vel.y > -1) {
                ship_vel.y--;
            }
            show_exhaust = 1;
            fuel--;
        }
    } // while(front != -1)

    for (i = 0; i < 3; i++) {
        char* s = ship[i];
        for (j = 0; j < strlen(s); j++) {
            char c = s[j];
            if (c != ' ') {
                display_draft[y + i][x - 2 + j] = c;
            }
        }
    }

    // Will be the exhaust fire
    if (show_exhaust) {
        render_col(x-1, y+3, '\\', 0x0c);
        render_col(x+1, y+3, '/', 0x0c);
        render_col(x, y+4, 'v', 0x0c);
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

    make_terrain();

    while(1) {
        displayer();
        receiver();
        updater();
   
        delay(1);
        // sleep(1);
    } // while


} // main