CC = c:/tools/tcc/tcc

RAYLIB_DIR = d:/Raylib/raylib_mw
RAYSRC_DIR = $(RAYLIB_DIR)/src

INCLUDES =	-I $(RAYSRC_DIR) -I 3rdParty -I d:\raylib\raygui\src -I $(RAYSRC_DIR)/external/GLFW/include

CFLAGS	= $(INCLUDES) -Wall  -D_WIN32 -Os -s
CFLAGS += -DGRAPHICS_API_OPENGL_33 -DPLATFORM_DESKTOP -DSUPPORT_MODULE_RMODELS -DSUPPORT_FILEFORMAT_TGA -DSUPPORT_FILEFORMAT_JPG -DBUILD_LIBTYPE_SHARED -DGLAD_API_CALL_EXPORT -DGLAD_API_CALL_EXPORT_BUILD -DPHYSFS_SUPPORTS_ZIP

DOBJ = 	main.o zip.o

RAYOBS	= 	$(RAYSRC_DIR)\rcore.o \
			$(RAYSRC_DIR)\rglfw.o \
			$(RAYSRC_DIR)\rtext.o \
			$(RAYSRC_DIR)\rtextures.o \
			$(RAYSRC_DIR)\utils.o \
			$(RAYSRC_DIR)\rshapes.o \
			$(RAYSRC_DIR)\rmodels.o

LDFLAGS = -O4 -s -lgdi32 -lwinmm -luser32 -lshell32 -lkernel32

all : zt.exe raylib.dll 

zt.exe: raylib.dll $(DOBJ) 
	$(CC) $(CFLAGS) $(DOBJ) -o $@ $(LDFLAGS) raylib.dll 

raylib.dll: $(RAYOBS)
	$(CC) $(CFLAGS) $(RAYOBS) -o $@ $(LDFLAGS)  -shared

clean:
	rm -f *.o $(APP_NAME)

.PHONY: all $(PLATS) clean
