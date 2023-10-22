# raylib-handy-things
Raylib handy things

# ray_pcx.h 
Single header to unpack PCX image data into a greyscale image, and load the palette into the supplied buffer 

```
unsigned char palettes[768];
  Image PCX_image = LoadImagePCX("draw.pcx",&palettes[0]);
```
