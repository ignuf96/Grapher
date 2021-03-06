#ifndef DATATYPES_H
#define DATATYPES_H
typedef struct FLOAT_VECTOR2
{
    float x;
    float y;

}fvec2;

typedef struct INT_VECTOR2
{
    int x;
    int y;
}ivec2;

typedef struct INT_VECTOR4 {
    int width;
    int height;
    int x;
    int y;
}ivec4;

enum LINE_FORM {STANDARD, POINT_SLOPE, SLOPE_INTERCEPT};

struct LINE_DATA {
    enum LINE_FORM line_form;
    int slope_rise;
    int slope_run;
    int y_intercept;
    int x_intercept;
};

#endif
