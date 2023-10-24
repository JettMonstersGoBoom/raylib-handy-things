//
// LoadPCX as an image, additionally load the palette into a buffer 
//

#include "raylib.h"

#ifdef RAYLIB_PCX_IMPLEMENTATION

#pragma pack(1)
typedef struct 
{
    char manufacturer;
    char version;
    char encoding;
    char bits_per_pixel;
    unsigned short x,y;
    unsigned short width, height;
    unsigned short horv_res;
    unsigned short vert_res;
    char ega_palette[48];
    char reserved;
    char num_color_planes;
    unsigned short bytes_per_line;
    unsigned short palette_type;
    char padding[58];
} pcx_header;

Image LoadImagePCX(char *fileName,unsigned char *palette)
{
char *raw_buffer;
int   raw_length;
pcx_header *header;
unsigned char *ptr;
Image image = {0};
    //  load the raw data and process the header 
    raw_buffer = LoadFileData(fileName,&raw_length);
    if (raw_buffer==NULL)
    {
        TraceLog(LOG_FATAL,TextFormat("File %s failed to load\n",fileName));
        return image;
    }
    header = (pcx_header*)&raw_buffer[0];
    ptr = raw_buffer+128;
    //  width and height + 1 ( 0 means 1 pixel in the header )
    header->width++;
    header->height++;
    //  alloc buffer
    unsigned char *rimage = malloc(header->width * (header->height+64));
    if (rimage==NULL)
    {
        TraceLog(LOG_FATAL,"no memory for image");
        free(raw_buffer);
        exit(0);
    }

    //  start the RLE depacker
    int count = 0;
    unsigned char data = 0;
    int num_bytes =0 ;
    for(int y=0; y < header->height; y++)
	{
        int x=0;
		for(x=0; x < header->width; x++)
		{
            data = *ptr++;
            //  unpack a run
            if ((data>=192) && (data<=255))
            {
                num_bytes = data-192;
                x+=(num_bytes-1);
                data = *ptr++;
                while(num_bytes-- > 0)
                    rimage[count++] = data;
            }
            else 
            {
                //  raw bytes
                rimage[count++] = data;
            }
        }
        //  skip cruft 
        while( x++ < header->bytes_per_line)
			data = *ptr++; 
    }
    data = *ptr++;
    //  check valid palette byte, if not set palette to grayscale
    if (data!=12)
    {
        printf("no palette valid palette?");
        if (palette!=NULL)
        {
            for (int q=0;q<768;q++)
            {
                palette[q] = q/3;
            }
        }
    }
    else 
    {
        if (palette!=NULL)
        {
            for (int q=0;q<768;q++)
            {
                    palette[q] = *ptr++;
            }
        }
    }
    //  return a Raylib Image
    image.data = rimage;
    image.width = header->width;
    image.height = header->height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    //  cleanup
    free(raw_buffer);
    return image;
}

#else 
Image LoadImagePCX(char *fileName,unsigned char *palette);
#endif 
