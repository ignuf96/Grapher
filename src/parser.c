#include <stdio.h>
#include "../include/datatypes.h"

struct LINE_DATA parse_equation(char *equation, int length)
{
    struct LINE_DATA line_data;

    char *c = equation;

    int max_tries = 20;

    int i = 0;

    int slope_buffer[20];
    int *slopeb = &slope_buffer[0];

    // First pass checks what form equation is in
    if((*c) == 'y')
    {
        printf("Slope-intercept form\n");
    }

    while((*c) == ' ' || (*c) == '=')
        c++;

    int count = 0;

    printf("Number is:  ");
    for(int i=0; i < max_tries; i++, c++)
    {
        switch (*c)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            printf("%d", (*c)-'0');
            count++;
            break;
            case 'x':
            i=max_tries;
            break;
        }
    }
    printf("\n");

    c = &equation[0];

    for(int i=0; i < max_tries; i++, c++)
    {
        switch (*c)
        {
            case 'x':
                line_data.slope_rise = *(c-1) - '0';
                break;
            case '+': case'-':
                line_data.y_intercept = (*(c+2)) - '0';
            case ' ':
                continue;
        }
    }

    printf("Slope is: %d\nY-intercept is: %d\n", line_data.slope_rise, line_data.y_intercept);

    return line_data;
}
