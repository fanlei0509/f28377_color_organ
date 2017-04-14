/* This file defines an 8 row by 6 column font


 */
#ifndef FONT_H
#define FONT_H

#include "globals.h"

#ifdef  LARGE_ARRAY
#define FONT_HEIGHT                    8
#define FONT_WIDTH                     6
#define FONT_YOFF                      0
#else
#define FONT_HEIGHT                    6
#define FONT_WIDTH                     4
#define FONT_YOFF                      2
#endif

#define NUM_FONT_CHAR                  75
#define FONT_MAX_CHAR                  30
#define FONT_SPACE_IDX                 10

typedef struct
{
  uint16_t asc:8;  //ASCII code for the character
  uint16_t wid:8;  //number of columns with pixels on.
  #ifdef  LARGE_ARRAY
  uint16_t pix[3]; //byte 0 contains column zero, top down. For the 8x6 font.
  #else
  uint16_t pix[2]; //byte 0 contains column zero, top down. For the 6x4 font.
  #endif
} font_t;


extern const font_t fontchar[NUM_FONT_CHAR];


uint16_t font_str_len(char *str);
uint16_t font_pix_len(char *str, uint16_t *str_idx);
uint16_t font_get_idx(uint16_t code);

#endif
