# colorslide

A fast, terminal-based color picker with mouse support.  Allows for color picking in RGBA, HSL, and CMYK.

![demo.gif](https://github.com/cubified/colorslide/blob/main/demo.gif)

I wrote this because no color picker I could find online had all the features I wanted, in particular the ability to adjust the current color in multiple color spaces at the same time.

Built using my custom terminal UI library [tuibox](https://github.com/Cubified/tuibox).

## Features

 - Fast -- Takes advantage of `tuibox`'s caching features to speed up rendering
 - Completely dependency-free -- Uses pure ANSI escape codes, without `ncurses`
 - Built for mouse -- Color sliders are click-and-drag

## Compiling and Running

To compile and run `colorslide`, run:

     $ make
     $ ./colorslide

To start with a color already selected (as opposed to a random color), run:

     $ ./colorslide [color]

Where `[color]` is a 6-digit or 8-digit hex color code (e.g. "aabbcc" or "aabbccdd").

## To-Do

 - Properly handle resize events (this will require updating `tuibox`)
 - Better keyboard support
