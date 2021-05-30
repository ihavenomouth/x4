# x4

A draw-on-screen tool for teachers using Linux.


![Demo](./img/demo.gif)

# How to compile

## Dependencies

    sudo apt install build-essential libx11-dev

## Compile

    gcc x4.c -o x4 -lX11 -lm

# How to use it

Create a keyboard shortcut to execute x4 (it depends on your window manager). I use Meta+4 as the shortcut.

Press c or control to draw a rectangle  
Press a or shift to draw an arrow  
Press b to change to color blue  
Press g to change to color green  
Press r to change to color red  
Press y to change to color yellow  
Press w to change to color white  


# TODO

- Cleaner code
- Better comments
- Use a Linter
- Change the predraw color (currently bluish)
- Allow to change the border size
- Read configuration from files
- What about some screenshot?
- A screnshot in the Readme file
- Learn markdown?

