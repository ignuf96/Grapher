#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/datatypes.h"

char *end_of_term = "+-/*";
struct LINE_DATA parse_equation(char *equation, int length)
{
    int num_buffer[length];
    int *nbuffer = &num_buffer[0];

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

    int skipped_first_term = 0;
    while((*c) == ' ' || (*c) == '=')
    {
        c++;
        skipped_first_term++;
    }

    int count = 0;

    printf("Number is:  ");

    char first_term_buff[length];
    char *ft_buff = &first_term_buff[0];

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
    //            printf("%d", (*c)-'0');
                *ft_buff = (*c);
                ft_buff++;
                count++;
                break;
            case 'x':
                i=max_tries;
                break;
        }
    }
    printf("\n");

    char *end;
    int slopeconv = strtol(first_term_buff, NULL, 10);

    //c = &equation[count];
    printf("Number contains %d digits\n", count);
    printf("Number is %d\n", slopeconv);

    printf("Skipping ");
    while((*c) == ' ' || (*c) == '=' || (*c) == '+')
    {
        printf("%c", *c);
        c++;
    }
    printf("\n");
    printf("At: %c\n", *c);

    char second_term_buff[length];
    char *st_buff = &second_term_buff[0];

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
    //            printf("%d", (*c)-'0');
                *st_buff = (*c);
                st_buff++;
                count++;
                break;
            case ';':
                i=max_tries;
                break;
        }
    }

   int y_interceptconv = strtol(second_term_buff, NULL, 10);
   printf("converted into: %d\n", y_interceptconv);

   line_data.y_intercept = y_interceptconv;
   line_data.slope_rise = slopeconv;

   printf("Slope is: %d\nY-intercept is: %d\n", line_data.slope_rise, line_data.y_intercept);

   return line_data;
}
