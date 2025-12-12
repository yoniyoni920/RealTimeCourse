#ifndef MAKE_TERRAIN_H
#define MAKE_TERRAIN_H

#include <stdlib.h> 
#include <time.h>   


void make_terrain(int terrain[25][80],int score) {
    int i;
    int ascending = 1, curr_y = 24;
    int flat_1_index, flat_2_index;
    int sizeof_ = 8; 
    // Generate two points to put flat surfaces. Could try generalizing it to put more than 2 flat surfaces
    // We put them in two separate halves to separate them
    srand(time(NULL));
    flat_1_index = rand() % 30;
    srand(time(NULL));
    flat_2_index = rand() % 30 + 40;
    if(score>=300){// if score is greater than 300 increase difficulty
        sizeof_ = 4;
    }
    curr_y = 24;
    for (i = 0; i < 80; i++) {
        if ((i >= flat_1_index && i <= flat_1_index+sizeof_) || (i >= flat_2_index && i <= flat_2_index+sizeof_)) {
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



#endif