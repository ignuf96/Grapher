#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/datatypes.h"

#define LENGTH_OF_SEGMENTS 20
#define NUMBER_OF_SEGMENTS 3

bool y_present;
bool x_present;

char segments[NUMBER_OF_SEGMENTS][LENGTH_OF_SEGMENTS];

void skip_white_space(char *str)
{
    while(*str == ' ')
        str++;
}

void check_variables(char *str)
{
    while(*str)
    {
        skip_white_space(str);
        switch (*str)
        {
           case 'y':
               y_present = true;
               break;
           case 'x':
               x_present = true;
               break;
        }
        str++;
    }
    printf("End check\n");
    str = &str[0];
}

void discard(char *str)
{
    while(*str)
    {
        skip_white_space(str);
        switch (*str)
        {
            case 'y':
                *str = ' ';
               break;
            case '=':
                *str = ' ';
               break;
        }
        str++;
    }
    skip_white_space(str);
    printf("End check\n");
    str = &str[0];
}
#include <unistd.h>
#include <stdlib.h>
struct LINE_DATA parse_equation(char *equation, int length)
{
    struct LINE_DATA line_data;
    char str_buff[length];

    strncpy(str_buff, equation, length);

    //printf("String copied from equation: %s\n", str_buff);
    //exit(1);

   #define MAX_LENGTH_BUFFERS 8
    char *starting_pos = &str_buff[0];
    char *xpos = NULL;
    char *divpos = NULL;
    char *operatorpos = NULL;
    char *semicolonpos = NULL;
    char *str = str_buff;

    discard(str);
    // find division sign and variable x position
    // from there, we have all the necessary information
    // to calculate the slope
    skip_white_space(str);
    str = &str[0];
    printf("STR AT THE START: %s\n", str);
    while(*str)
    {
        if(*str == 'x')
        {
            xpos = str;
        }
        if(*str == '/')
        {
            divpos = str;
        }
        if(*str == '+' || *str == '-')
        {
            operatorpos = str;
        }
        if(*str == ';')
        {
            semicolonpos = str;
        }
        printf("This is *str: %c\n", *str);
        str++;
    }

    str = starting_pos;
    skip_white_space(str);
    char buff[MAX_LENGTH_BUFFERS];
    char *buffp = &buff[0];
    int rise, run, y_intercept;
    if(divpos)
    {
        memset(buff, '\0', MAX_LENGTH_BUFFERS);
        // get rise
        do {
            *buffp = *str;
            buffp++;
            str++;
        }while ((str) != (divpos));
        *buffp = '\0';
        // memory leak? no check on length
        rise = strtol(&buff[0], NULL, 10);

        memset(buff, '\0', MAX_LENGTH_BUFFERS);
        // get run
        str = divpos+1;
        buffp = buff;
        do {
            *buffp = *str;
            printf("Moving char str: %c to buffp: %c\n", *str, *buffp);
            buffp++;
            str++;
        }while ((str) != (xpos));
        *buffp = '\0';
        // memory leak? no check on length
        run = strtol(&buff[0], NULL, 10);
    } else
    {
        // No division sign means our run is 1
        run = 1;

        memset(buff, '\0', MAX_LENGTH_BUFFERS);
        // get run
        str = starting_pos;
        buffp = buff;
        do {
            *buffp = *str;
            printf("Moving char str: %c to buffp: %c\n", *str, *buffp);
            buffp++;
            str++;
        }while ((str) != (xpos));
        *buffp = '\0';
        // memory leak? no check on length
        rise = strtol(&buff[0], NULL, 10);
    }

    bool is_y = false;
    str = xpos+1;
    while(*str)
    {
       if(*str == '+' || *str == '-')
       {
           is_y = true;

           // we skip the plus sign so we can convert it without issues
           if(*str == '+')
               operatorpos = str+1;
           // needs no skipping as negative sign is part of number in this instance
          if(*str == '-')
               operatorpos = str;
        }
        if(*str == ';')
        {
            semicolonpos = str;
        }
        str++;
    }
    memset(buff, '\0', MAX_LENGTH_BUFFERS);
    str = operatorpos;
    buffp = buff;
    // get y-intercept
    do {
        skip_white_space(str);
        *buffp = *str;
        printf("Copying y...*str: %c to *buffp: %c\n", *str, *buffp);
        buffp++;
        str++;
    }while ((str) != (semicolonpos));
    *buffp = '\0';
    // memory leak? no check on length
    y_intercept = strtol(&buff[0], NULL, 10);


    if(!is_y) y_intercept = 0;

    printf("Rise/Run (+-) y-intercept is: %d/%d  (y): %d \n", rise, run, y_intercept);

    line_data.slope_rise = rise;
    line_data.slope_run = run;
    line_data.y_intercept = y_intercept;

   return line_data;
}
