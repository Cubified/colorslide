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

#define UNWRAP(l, v, d) (((double)((l >> (v - d)) & 0xff)) / 255.0f)

#define VER "0.0.1"

double rgba[4], hsl[3], cmyk[4];
ui_t u;

/**
 * COLOR SPACE CONVERSIONS
 */

/* https://stackoverflow.com/questions/2353211/hsl-to-rgb-color-conversion */
void rgba_to_hsl(){
  double max = MAX(rgba[0], rgba[1], rgba[2]),
         min = MIN(rgba[0], rgba[1], rgba[2]),
         d = max - min;
  
  hsl[2] = (max + min) / 2.0f;
  if(d == 0.0f){
    hsl[0] = hsl[1] = 0.0f;
  } else {
    hsl[1] = (hsl[2] > 0.5) ? d / (2.0f - max - min) : d / (max + min);
    if(max == rgba[0]){
        hsl[0] = (rgba[1] - rgba[2]) / d + (rgba[1] < rgba[2] ? 6.0f : 0.0f);
    } else if(max == rgba[1]){
        hsl[0] = (rgba[2] - rgba[0]) / d + 2.0f;
    } else {
        hsl[0] = (rgba[0] - rgba[1]) / d + 4.0f;
    }
    hsl[0] /= 6.0f;
  }
}
double hue_to_rgb(double p, double q, double t){
  if(t < 0.0f) t += 1.0f;
  if(t > 1.0f) t -= 1.0f;
  if(t < (1.0f / 6.0f)) return p + (q - p) * 6.0f * t;
  if(t < 0.5f) return q;
  if(t < (2.0f / 3.0f)) return p + (q - p) * ((2.0f / 3.0f) - t) * 6.0f;
  return p;
}
void hsl_to_rgba(){
  double p, q;

  rgba[3] = 1.0f;

  if(hsl[1] == 0.0f){
    rgba[0] = rgba[1] = rgba[2] = hsl[2];
  } else {
    q = (hsl[2] < 0.5 ? hsl[2] * (1.0f + hsl[1]) : (hsl[2] + hsl[1] - (hsl[2] * hsl[1])));
    p = (2.0f * hsl[2]) - q;
    rgba[0] = hue_to_rgb(p, q, hsl[0] + (1.0f / 3.0f));
    rgba[1] = hue_to_rgb(p, q, hsl[0]);
    rgba[2] = hue_to_rgb(p, q, hsl[0] - (1.0f / 3.0f));
  }
}

/* https://stackoverflow.com/questions/2426432/convert-rgb-color-to-cmyk */
void rgba_to_cmyk(){
  double max = MAX(rgba[0], rgba[1], rgba[2]);

  cmyk[3] = 1.0f - max;
  cmyk[0] = (max - rgba[0]) / max;
  cmyk[1] = (max - rgba[1]) / max;
  cmyk[2] = (max - rgba[2]) / max;
}
void cmyk_to_rgba(){
  double max = 1.0f - cmyk[3];

  rgba[0] = max * (1.0f - cmyk[0]);
  rgba[1] = max * (1.0f - cmyk[1]);
  rgba[2] = max * (1.0f - cmyk[2]);
  rgba[3] = 1.0f;
}

/**
 * UI EVENTS
 */

void recompute(double *src){
  if(src == rgba){
    rgba_to_hsl();
    rgba_to_cmyk();
  } else if(src == hsl){
    hsl_to_rgba();
    rgba_to_cmyk();
  } else if(src == cmyk){
    cmyk_to_rgba();
    rgba_to_hsl();
  }
}

void box(ui_box_t *b, char *out){
  int x, y;
  char tmp[256];

  sprintf(
    out, "\x1b[48;2;%i;%i;%im",
    (int)floor(255.0f * rgba[3] * rgba[0]),
    (int)floor(255.0f * rgba[3] * rgba[1]),
    (int)floor(255.0f * rgba[3] * rgba[2])
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
  double *src = (double*)b->data2;
  strcpy(out, "\x1b[48;2;99;99;99m");
  for(i=0;i<=b->w;i++){
    if((int)floor(src[(int)b->data1] * (double)b->w) == i){
      strcat(out, "\x1b[48;2;200;200;200m \x1b[48;2;99;99;99m");
    } else {
      strcat(out, " ");
    }
  }
  strcat(out, "\n\x1b[0m");
}

void click(ui_box_t *b, int x, int y){
  double *src = (double*)b->data2;
  src[(int)b->data1] = ((double)(x - b->x)) / (double)b->w;
  recompute(src);

  ui_draw(&u);
}

void hover(ui_box_t *b, int x, int y, int down){
  if(down) click(b, x, y);
}

void output(ui_box_t *b, char *out){
  if(b->data1 == rgba){
    sprintf(
      out, "RGBA: rgba(%i, %i, %i, %.2f)\n",
      (int)(255.0f * rgba[0]),
      (int)(255.0f * rgba[1]),
      (int)(255.0f * rgba[2]),
      rgba[3]
    );
  } else if(b->data1 == hsl){
    sprintf(
      out, "HSL: hsl(%ideg, %i%%, %i%%)\n",
      (int)(360.0f * hsl[0]),
      (int)(100.0f * hsl[1]),
      (int)(100.0f * hsl[2])
    );
  } else if(b->data1 == cmyk){
    sprintf(
      out, "CMYK: cmyk(%i%%, %i%%, %i%%, %i%%)\n",
      (int)(100.0f * cmyk[0]),
      (int)(100.0f * cmyk[1]),
      (int)(100.0f * cmyk[2]),
      (int)(100.0f * cmyk[3])
    );
  } else {
    sprintf(
      out, "HEX: #%.2X%.2X%.2X%.2X\n",
      (int)(255.0f * rgba[0]),
      (int)(255.0f * rgba[1]),
      (int)(255.0f * rgba[2]),
      (int)(255.0f * rgba[3])
    );
  }
}

/**
 * UTIL AND MAIN
 */

void usage(){
  printf("Usage: colorslide [color]\n  Where [color] is in the form RRGGBB or RRGGBBAA\n");
  exit(0);
}

void stop(){
  ui_free(&u);
  exit(0);
}

int main(int argc, char **argv){
  int i;
  char *end;
  long int l, v;

  if(argc > 1){
    l = strtol(argv[1], &end, 16);
    v = (end - argv[1]) * 4;

    if(v == 24 || v == 32){
      rgba[0] = UNWRAP(l, v, 8);
      rgba[1] = UNWRAP(l, v, 16);
      rgba[2] = UNWRAP(l, v, 24);
      rgba[3] = v == 24 ? 1.0f : UNWRAP(l, v, 32);
    } else usage();
  } else {
    srand(time(NULL));
    rgba[0] = ((double)(rand() % 255)) / 255.0f;
    rgba[1] = ((double)(rand() % 255)) / 255.0f;
    rgba[2] = ((double)(rand() % 255)) / 255.0f;
    rgba[3] = 1.0f;
  }
  recompute(rgba);

  signal(SIGINT,   stop);
  signal(SIGTERM,  stop);
  signal(SIGQUIT,  stop);
  /* signal(SIGWINCH, resize); */

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
  ui_add(
    -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
    ui_center_y(1, &u) - 13 + 1,
    16, 1,
    0,
    NULL, 0,
    output,
    NULL,
    NULL,
    (void*)rgba,
    NULL,
    &u
  );
  for(i=0;i<4;i++){
    ui_add(
      -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
      ui_center_y(1, &u) - 9 + (2 * i) - 1,
      32, 1,
      0,
      NULL, 0,
      slider,
      click,
      hover,
      (void*)i,
      (void*)rgba,
      &u
    );
  }

  ui_add(
    -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
    ui_center_y(1, &u) - 11 + 8 + 1,
    16, 1,
    0,
    NULL, 0,
    output,
    NULL,
    NULL,
    (void*)hsl,
    NULL,
    &u
  );
  for(i=0;i<3;i++){
    ui_add(
      -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
      ui_center_y(1, &u) - 11 + 12 + (2 * i) - 1,
      32, 1,
      0,
      NULL, 0,
      slider,
      click,
      hover,
      (void*)i,
      (void*)hsl,
      &u
    );
  }

  ui_add(
    -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
    ui_center_y(1, &u) - 11 + 8 + 8 + 1,
    16, 1,
    0,
    NULL, 0,
    output,
    NULL,
    NULL,
    (void*)cmyk,
    NULL,
    &u
  );
  for(i=0;i<4;i++){
    ui_add(
      -20 + 2 + ui_center_x(32, &u) + 24 + 16 + 2,
      ui_center_y(1, &u) - 11 + 12 + 8 + (2 * i) - 1,
      32, 1,
      0,
      NULL, 0,
      slider,
      click,
      hover,
      (void*)i,
      (void*)cmyk,
      &u
    );
  }

  ui_add(
    ui_center_x(56, &u),
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
