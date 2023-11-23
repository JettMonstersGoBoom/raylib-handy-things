#CC = c:\tools\tcc\tcc
CC = gcc 
RAYLIB_DIR = d:/Raylib/raylib
RAYSRC_DIR = $(RAYLIB_DIR)/src

INCLUDES =	-I $(RAYSRC_DIR) -I 3rdParty -I d:\raylib\raygui\src -I $(RAYSRC_DIR)/external/GLFW/include

CFLAGS	= $(INCLUDES) -Wall  -D_WIN32 
CFLAGS += -DGRAPHICS_API_OPENGL_33 -DPLATFORM_DESKTOP -DSUPPORT_MODULE_RMODELS -DSUPPORT_FILEFORMAT_TGA -DBUILD_LIBTYPE_SHARED

OBJ	= begin.o plugins.o

RAYOBS	= 	$(RAYSRC_DIR)/rcore.o \
			$(RAYSRC_DIR)/rglfw.o \
			$(RAYSRC_DIR)/rtext.o \
			$(RAYSRC_DIR)/rtextures.o \
			$(RAYSRC_DIR)/utils.o \
			$(RAYSRC_DIR)/rshapes.o \
			$(RAYSRC_DIR)/rmodels.c

LDFLAGS = -Os -s -lgdi32 -lwinmm -luser32 -lshell32 -lkernel32 #-m32 

all : raylib.dll plugins/test_red.dll plugins/test_white_box.dll ps.exe  

plugins/test_red.dll: plugins/test_red.c raylib.dll  
	$(CC) $(CFLAGS) $< -o $@  -shared raylib.dll 

plugins/test_white_box.dll: plugins/test_white_box.c raylib.dll  
	$(CC) $(CFLAGS) $< -o $@  -shared raylib.dll 

ps.exe: $(OBJ) raylib.dll $(PLUGIN_RESULTS)
	echo $(PLUGIN_RESULTS)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) raylib.dll 

raylib.dll: $(RAYOBS)
	$(CC) $(CFLAGS) $(RAYOBS) -o $@ $(LDFLAGS)  -shared #-rdynamic

clean:
	rm -f *.o $(APP_NAME)

.PHONY: all $(PLATS) clean
