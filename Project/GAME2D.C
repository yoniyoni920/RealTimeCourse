/* game.c - xmain, prntr */

#include <stdio.h>
#include <dos.h>

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
int display_draft[25][80][2];
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

// POSITION target_pos[TARGET_NUMBER];
// POSITION arrow_pos[ARROW_NUMBER];
POSITION target_pos[ARRSIZE];
POSITION arrow_pos[ARRSIZE];

POSITION ship_pos;

int flag = 0;
int target_freq = 14;


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
    if (scan == 75) {
        ascii = 'a';
    } else if (scan == 72) {
        ascii = 'w';
    } else if (scan == 77) {
        ascii = 'd';
    }
// if ((scan == 46)&& (ascii == 3)) // Ctrl-C?
    if (scan == 1) { // Esc?
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

    for(i=0; i < 2000; i++) {
        b800h[2*i+1] = 0x08; // black over white
    }

    for(row=0; row < 25; row++) {
        for(col=0; col < 80; col++) {
            i = 2*(row*80 + col);

            b800h[i] = display_draft[row][col][0];
            b800h[i+1] = display_draft[row][col][1];
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
void updateter_targets()
{
    int i,j;
    static int initial_run = 1;
    int no_of_targets;

    if (initial_run == 1)
    {
        initial_run = 0;
        no_of_arrows = 0;

        no_of_targets = 4;
        target_pos[0].x = 3;
        target_pos[0].y = 0; 

        ship_pos.x = 2;
        ship_pos.y = 0;

        for(i=1; i < TARGET_NUMBER; i++)
        {
            target_pos[i].x = i*target_disp;
            target_pos[i].y = 0; 

        } // for
    } // if (initial_run == 1)


//  while(1)
//  {
//   receive();
    if (flag == 0) {
        ship_pos.x++;
        ship_pos.y++;

        for(i=0; i < TARGET_NUMBER; i++) {
            if (target_pos[i].x != -1) {
                if (target_pos[i].y < 22) {
                    target_pos[i].y++;    
                }
            } // if
        }
    }
    flag++;
    flag = flag % target_freq;


  //} // while

} //updateter_targets()

// Helper function to render something at some point in the screen
// Instead of writing two lines
void render_col(int row, int col, char c, int attr) {
    display_draft[col][row][0] = c;
    display_draft[col][row][1] = attr;
}
void render(int row, int col, char c) {
    display_draft[col][row][0] = c;
}

void update_ship() {
    int x = ship_pos.x, y = ship_pos.y;
    render(x, y, '_');

    render(x-1, y+1, '(');
    render(x, y+1, '_');
    render(x+1, y+1, ')');
    
    render(x-1, y+2, '/');
    render(x-2, y+2, '/');
    render(x, y+2, '_');
    render(x+1, y+2, '\\');
    render(x+2, y+2, '\\');

    // Will be the exhaust fire
    // render_col(x-1, y+3, '\\', 0x0c);
    // render_col(x+1, y+3, '/', 0x0c);
    // render_col(x, y+4, 'v', 0x0c);
}

void updater()
{
    int i,j;
    static int initial_run = 1;

    if (initial_run == 1) {
        initial_run = 0;
        no_of_arrows = 0;
        no_of_targets = 4;
        gun_position = 39;

        ship_pos.x = 2;
        ship_pos.y = 0;

        for(i=0; i < ARROW_NUMBER; i++)
            arrow_pos[i].x = arrow_pos[i].y = -1;
    } // if (initial_run

    updateter_targets();

 // while(1)
 // {

  // receive();
    ch = 0;
    for(i=0; i < 25; i++) {
        for(j=0; j < 80; j++) {
            display_draft[i][j][0] = ' ';  // blank
            display_draft[i][j][1] = 0x0f;  // blank
        }
    }

    while(front != -1)  {
        ch = ch_arr[front];
        if(front != rear) {
            front++;
        } else {
            front = rear = -1;
        }

        if ((ch == 'a') || (ch == 'A')) {
            if (gun_position >= 2) {
                gun_position--;
                update_ship();
            }
        } else if ((ch == 'd') || (ch == 'D')) {
            if (gun_position <= 78) {
                gun_position++;
                update_ship();
            }
        } else if ((ch == 'w') || (ch == 'W')) {
            if (no_of_arrows < ARROW_NUMBER) {
                arrow_pos[no_of_arrows].x = gun_position;
                arrow_pos[no_of_arrows].y = 23;
                no_of_arrows++;

            } // if
        }
    } // while(front != -1)

    update_ship();

    for(i=0; i < ARROW_NUMBER; i++) {
        if (arrow_pos[i].x != -1) {
            int j;
            if (arrow_pos[i].y >= 0) {
                arrow_pos[i].y--;
                display_draft[arrow_pos[i].y][arrow_pos[i].x][0] = '^';
                display_draft[arrow_pos[i].y+1][arrow_pos[i].x][0] = '|';
            } // if

            for(j=0; j < TARGET_NUMBER; j++) { /* CHANGE */
                if ((arrow_pos[i].x == target_pos[j].x) && (arrow_pos[i].y <= target_pos[j].y)) {
                    target_flags[j] = 0;
                }
            }

        } // if
    }

    for(i=0; i < TARGET_NUMBER; i++) {
        if (target_flags[i] == 1) { /*CHANGE */
            if (target_pos[i].x != -1)
            {
                // CHANGE TO COMMENT if (target_pos[i].y < 22)
                // CHANGE TO COMMENT      target_pos[i].y++;    
                display_draft[target_pos[i].y][target_pos[i].x][0] = '*';
            } // if
        }
    }
} // updater 



void main() {
    int uppid, dispid, recvpid, targid;
    int i;
 
    for(i=0; i< TARGET_NUMBER; i++)
        target_flags[i] = 1;


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



    while(1)
    {
        displayer();
        receiver();
        updater();
   
        delay(1);
        // sleep(1);

    } // while


} // main