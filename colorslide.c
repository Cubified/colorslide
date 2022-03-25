/*
 * colorslide.c: a terminal-based color editor
 */
#include <math.h>
#include <time.h>
#include <signal.h>

#include "tuibox.h"

#define MAX2(a, b) (a > b ? a : b)
#define MIN2(a, b) (a < b ? a : b)

#define MAX(a, b, c) (MAX2(MAX2(a, b), c))
#define MIN(a, b, c) (MIN2(MIN2(a, b), c))
#define VER "0.0.1"

typedef unsigned char u8;

u8 rgba[4], hsl[3], cmyk[4];
ui_t u;

void recompute(u8 *src){
  if(src == rgba){
    /* rgba to hsl */
    u8 max = MAX(rgba[0], rgba[1], rgba[2]),
       min = MIN(rgba[0], rgba[1], rgba[2]);

    hsl[2] = (max + min) / 2;
    hsl[1] = (hsl[2] >= 128
      ? (max - min) / (max + min)
      : (max - min) / (512 - max - min)
    );
    if(max == rgba[0]){
      hsl[0] = (hsl[1] - hsl[2]) / (max - min);
    } else if(max == rgba[1]){
      hsl[0] = 512 + ((hsl[2] - hsl[0]) / (max - min));
    } else {
      hsl[0] = 1024 + ((hsl[0] - hsl[1]) / (max - min));
    }
    hsl[0] *= 43;

    /* rgba to cmyk */
    cmyk[3] = 255 - max;
    cmyk[0] = max * (255 - rgba[0] - cmyk[0]) / 255;
    cmyk[1] = max * (255 - rgba[1] - cmyk[0]) / 255;
    cmyk[2] = max * (255 - rgba[2] - cmyk[0]) / 255;
  } else if(src == hsl){
    /* hsl to rgba */
  } else if(src == cmyk){
    /* cmyk to rgba */
  }
}

void box(ui_box_t *b, char *out){
  int x, y;
  char tmp[256];

  /* rgba -> rgb with fast bitshift */
  sprintf(
    out, "\x1b[48;2;%i;%i;%im",
    (rgba[3] * rgba[0]) >> 8,
    (rgba[3] * rgba[1]) >> 8,
    (rgba[3] * rgba[2]) >> 8
  );
  for(y=0;y<b->h;y++){
    for(x=0;x<b->w;x++){
      strcat(out, " ");
    }
    strcat(out, "\n");
  }
  strcat(out, "\x1b[0m");
}

void slider(ui_box_t *b, char *out){
  int i;
  u8 *src = (u8*)b->data2;
  strcpy(out, "\x1b[48;2;99;99;99m");
  for(i=0;i<b->w;i++){
    if(src[(int)b->data1] / 8 == i){
      strcat(out, "\x1b[48;2;200;200;200m \x1b[48;2;99;99;99m");
    } else {
      strcat(out, " ");
    }
  }
  strcat(out, "\n\x1b[0m");
}

void click(ui_box_t *b, int x, int y){
  u8 *src = (u8*)b->data2;
  src[(int)b->data1] = (x - b->x) * 8;
  recompute(src);

  ui_draw(&u);
}

void hover(ui_box_t *b, int x, int y, int down){
  if(down) click(b, x, y);
}

void output(ui_box_t *b, char *out){
  sprintf(
    out, "#%X%X%X%X  rgba(%-3i, %-3i, %-3i, %-3i)  hsl(%-3ideg, %-3i%%, %-3i%%)  cmyk(%-3i%%, %-3i%%, %-3i%%, %-3i%%)\n",
    rgba[0], rgba[1], rgba[2], rgba[3],
    rgba[0], rgba[1], rgba[2], rgba[3],
    hsl[0], hsl[1], hsl[2],
    cmyk[0], cmyk[1], cmyk[2], cmyk[3]
  );
}

void stop(){
  ui_free(&u);
  exit(0);
}

int main(int argc, char **argv){
  int i;

  srand(time(NULL));
  rgba[0] = rand() % 255;
  rgba[1] = rand() % 255;
  rgba[2] = rand() % 255;
  rgba[3] = 255;
  recompute(rgba);

  signal(SIGINT,   stop);
  signal(SIGTERM,  stop);
  signal(SIGQUIT,  stop);
  // signal(SIGWINCH, resize);

  ui_new(0, &u);

  ui_text(
    UI_CENTER_X, ui_center_y(30, &u),
    "colorslide v" VER,
    0,
    NULL, NULL,
    &u
  );

  /* Color box */
  ui_add(
    -20 + ui_center_x(54, &u) - 2,
    ui_center_y(26, &u),
    54, 27,
    0,
    NULL, 0,
    box, NULL, NULL,
    rgba, NULL,
    &u
  );
  
  /* Sliders */
  ui_text(
    -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 14 + 2,
    ui_center_y(1, &u) - 13 + 1,
    "RGBA",
    0,
    NULL, NULL,
    &u
  );
  for(i=0;i<4;i++){
    ui_add(
      -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
      ui_center_y(1, &u) - 9 + (2 * i) - 1,
      32, 1,
      0,
      &rgba[i], rgba[i] + 1,
      slider,
      click,
      hover,
      (void*)i,
      (void*)rgba,
      &u
    );
  }

  ui_text(
    -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 14 + 2,
    ui_center_y(1, &u) - 11 + 8 + 1,
    "HSL",
    0,
    NULL, NULL,
    &u
  );
  for(i=0;i<3;i++){
    ui_add(
      -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
      ui_center_y(1, &u) - 11 + 12 + (2 * i) - 1,
      32, 1,
      0,
      &hsl[i], hsl[i] + 1,
      slider,
      click,
      hover,
      (void*)i,
      (void*)hsl,
      &u
    );
  }

  ui_text(
    -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 14 + 2,
    ui_center_y(1, &u) - 11 + 8 + 8 + 1,
    "CMYK",
    0,
    NULL, NULL,
    &u
  );
  for(i=0;i<4;i++){
    ui_add(
      -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
      ui_center_y(1, &u) - 11 + 12 + 8 + (2 * i) - 1,
      32, 1,
      0,
      &cmyk[i], cmyk[i] + 1,
      slider,
      click,
      hover,
      (void*)i,
      (void*)cmyk,
      &u
    );
  }

  ui_add(
    ui_center_x(98, &u),
    ui_center_y(0, &u) + 15,
    100, 1,
    0,
    NULL, 0,
    output,
    NULL,
    NULL,
    NULL,
    NULL,
    &u
  );

  ui_key("q", stop, &u);

  ui_draw(&u);

  ui_loop(&u){
    ui_update(&u);
  }

  return 0;
}
