# raylib-handy-things
Raylib handy things

# ray_pcx.h 
Single header to unpack PCX image data into a greyscale image, and load the palette into the supplied buffer.

Intended for using a palettized shader with 8-bit data.

```
unsigned char palettes[768];
  Image PCX_image = LoadImagePCX("draw.pcx",&palettes[0]);
```

![](https://github.com/JettMonstersGoBoom/raylib-handy-things/blob/main/RayPCX/pcx_test.gif)

# raygui_menubar.h 
A RayGUI component to add a menubar to the top of the screen. 

![](https://github.com/JettMonstersGoBoom/raylib-handy-things/blob/main/RayMenuBar/menu.gif)
