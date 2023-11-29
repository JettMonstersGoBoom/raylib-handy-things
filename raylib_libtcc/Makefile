CC = tcc 

RAYLIB_DIR = raylib
RAYSRC_DIR = $(RAYLIB_DIR)\src

INCLUDES =	-I $(RAYSRC_DIR) -I $(RAYSRC_DIR)/external/GLFW/include
INCLUDES += -I include 

CFLAGS	= $(INCLUDES) -Wall  -D_WIN32 
CFLAGS += -DGRAPHICS_API_OPENGL_33 -DPLATFORM_DESKTOP -DBUILD_LIBTYPE_SHARED 

OBJ	= libtcctest.o

RAYOBS	= 	$(RAYSRC_DIR)\rcore.o \
			$(RAYSRC_DIR)\rglfw.o \
			$(RAYSRC_DIR)\rtext.o \
			$(RAYSRC_DIR)\rtextures.o \
			$(RAYSRC_DIR)\utils.o \
			$(RAYSRC_DIR)\rshapes.o \
			$(RAYSRC_DIR)\rmodels.c

LDFLAGS = -Os -s -lgdi32 -lwinmm -luser32 -lshell32 -lkernel32

all : test.exe raylib.dll 

test.exe: raylib.dll $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) raylib.dll libtcc.dll 

raylib.dll: $(RAYOBS)
	$(CC) $(CFLAGS) $(RAYOBS) -o $@ $(LDFLAGS)  -shared 

clean:
	rm -f *.o $(APP_NAME)

.PHONY: all $(PLATS) clean
