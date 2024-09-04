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

# raygui treeview 
An example of how to make a tree-view style component 
![](https://github.com/JettMonstersGoBoom/raylib-handy-things/blob/main/rayTreeView/treeview.gif)

# hot_reload 
hot reloading multiple DLL/SO files each DLL can draw what it wants to the camera. and can ( in theory ) be updated on the fly.
only tested on windows , but should work for others. 


# Godot-Simple_Export 
an addon for Godot to export a simple ascii scene representation. DO NOT USE AS IS. it's for learning from. 

# CGLTF direct access test
showing how to get data using the built in implementation of cgltf
![](https://github.com/JettMonstersGoBoom/raylib-handy-things/blob/main/cgltf_direct/screenshot.png)

# Zipfiles 
overload loading binary data with utility functions to load files from zip instead 

# TUI
simple example using a single draw call with a shader for debug text.
![](https://github.com/JettMonstersGoBoom/raylib-handy-things/blob/main/tui_text/demo.gif)
