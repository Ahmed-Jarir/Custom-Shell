#ifndef DICE_H
#define DICE_H

#include "global.h"

int dice_roll(struct command_t *command) {

    // dice info
    int num_rolls = 1;
    int num_sides;
    char *dice_str = command->args[1];

    // total sum
    int sum = 0;

    char *d_pos = strchr(dice_str, 'd');

    // checks for invalid format
    if (d_pos == NULL) {
        return EXIT;
    }

    // checks if there are only side and saves them
    if (d_pos == dice_str) {
        num_sides = atoi(dice_str + 1);
    
    } 
    // else means there is number of rolls
    else {

        num_rolls = atoi(dice_str);
        
        // checks for invalid format
        if (num_rolls <= 0) {
            return EXIT;
        }

        num_sides = atoi(d_pos + 1);
    }

    // checks for invalid format
    if (num_sides <= 0) {
        return EXIT;
    }

    // ensure seed is always changing for random
    srand(time(NULL));
    
    // saves the rolls and calculates the sum
    int rolls[num_rolls];
    for (int i = 0; i < num_rolls; i++) {
        
        int roll = rand() % num_sides + 1;
        rolls[i] = roll;
        sum += roll;

    }

    // prints the output
    printf("Rolled %d ",sum);
    printf("(");
    for (int i = 0; i < num_rolls; i++) {
        
        printf("%d",rolls[i]);

        if (i < num_rolls - 1) {
            printf(" + ");
        }
    }
    printf(")\n");

    return SUCCESS;

}

#endif