# raylib-handy-things
Raylib handy things

# ray_pcx.h 
Single header to unpack PCX image data into a greyscale image, and load the palette into the supplied buffer.

Intended for using a palettized shader with 8-bit data.

```
unsigned char palettes[768];
  Image PCX_image = LoadImagePCX("draw.pcx",&palettes[0]);
```
In the sample provided I make 7 variants of the input palette and cycle them.  

![](https://github.com/JettMonstersGoBoom/raylib-handy-things/blob/main/RayPCX/pcx_test.gif)

NOTE: the FPS variations is caused by recording, not in app.

The example image is (C) Mark J. Ferrari. used here as reference.

# raygui_menubar.h 
A RayGUI component to add a menubar to the top of the screen. 

![](https://github.com/JettMonstersGoBoom/raylib-handy-things/blob/main/RayMenuBar/menu.gif)
