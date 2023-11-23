#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

#define CHECK_COOLDOWN 2.5f

//  depending on platform, define the loaders and ext name

#ifdef _WIN32
    __declspec(dllimport) void* LoadLibraryA(const char* FileName);
    __declspec(dllimport) void FreeLibrary(void *handle);
    __declspec(dllimport) void* GetProcAddress(void *handle,const char *name);
	#define PLUG_EXT ".dll"
	#define ray_dlopen(name) LoadLibraryA(name)
	#define ray_dlclose(handle) FreeLibrary((void*) handle)
	#define ray_dlsym(handle, name) GetProcAddress((void*) handle, name)
#else 
	#include <dlfcn.h>
#ifdef __APPLE__	
	#define PLUG_EXT ".dylib"
#else
	#define PLUG_EXT ".so"
#endif
	#define ray_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
	#define ray_dlclose(handle) dlclose(handle)
	#define ray_dlsym(handle, name) dlsym(handle, name)
#endif

//  function prototypes we grab from the .so/.dll

typedef void (*onLoadProc_t)(void); 
typedef void (*onUnloadProc_t)(void); 
typedef void (*onUpdate2DProc_t)(Camera2D *cam); 
typedef void (*onUpdate3DProc_t)(Camera3D *cam); 

//  the main plugin struct 

typedef struct _plugin_t_ 
{
    char                *filepath;
    onLoadProc_t        onLoad;
    onUnloadProc_t      onUnload;
    onUpdate3DProc_t    onUpdate3D;
    onUpdate2DProc_t    onUpdate2D;
    long                last_file_modification_time;
    void                *shared_library;
    struct _plugin_t_   *next;
} _plugin_t_;

//  our list of plugins
_plugin_t_ *plugin_list;
static float checkTimer;

static void CopyFile(const char *fname,const char *outname)
{
int len;
    unsigned char *buffer = LoadFileData(fname,&len);
    SaveFileData(outname,buffer,len);
    FILE *fp = fopen(outname,"rb");
    fflush(fp);
    fclose(fp);
    free(buffer);
}

//  initalize the system and store the names of each plugin

void PluginsInit(const char *basepath)
{
    plugin_list = NULL;
    //  scan the plugins folder    
    FilePathList flist = LoadDirectoryFiles(basepath);

    for (int q=0;q<flist.count;q++)
    {
        char *str = flist.paths[q];
        //  check for our extension
        if (stricmp(GetFileExtension(str),PLUG_EXT)==0)
        {
            //  NOTE: we don't load here 
            _plugin_t_ *addon = RL_CALLOC(1,sizeof(_plugin_t_));
            addon->filepath = strdup(str);
            addon->last_file_modification_time = 0;
            addon->next = plugin_list;
            plugin_list = addon;
        }
    }
    checkTimer = CHECK_COOLDOWN;
}

//  check for any updated plugins, and reload them if needed 

void PluginsCheckForUpdates()
{
    checkTimer+=GetFrameTime();
    if (checkTimer<CHECK_COOLDOWN)
        return;
    printf("Check %f\n",GetTime());
    //  don't check every frame 
    checkTimer = 0.0f;
    _plugin_t_ *plug = plugin_list;
    while(plug!=NULL)
    {
        //  checking last mod time 
        long md = GetFileModTime(plug->filepath);
        if (md!=plug->last_file_modification_time)
        {
            //  ok we're new or we haven't loaded yet 
            if (plug->shared_library!=NULL)
            {
                //  unload 
                if (plug->onUnload!=NULL)
                    plug->onUnload();
                //  close it
                ray_dlclose(plug->shared_library);
            }
            //  copy the plugin to a backup
            //  we use the backup file, this is so we can live update the plugin and cause less issues 
            const char *out = TextFormat("%s_tmp",plug->filepath);
            CopyFile(plug->filepath,out);
            //  now we open it
            plug->shared_library = ray_dlopen(out);
            if (plug->shared_library!=NULL)
            {
                plug->last_file_modification_time = md;
                plug->onLoad = (onLoadProc_t)ray_dlsym(plug->shared_library, "OnLoad");
                plug->onUpdate3D = (onUpdate3DProc_t)ray_dlsym(plug->shared_library, "OnUpdate3D");
                plug->onUpdate2D = (onUpdate2DProc_t)ray_dlsym(plug->shared_library, "OnUpdate2D");
                plug->onUnload = (onUnloadProc_t)ray_dlsym(plug->shared_library, "OnUnload");

                //  we don't check for found symbols because we can support plugins without some of the functions 

                if (plug->onLoad!=NULL)
                    plug->onLoad();
            }
            else 
            {
                TraceLog(LOG_ERROR,"Plugin %s failed to load\n");
            }
        }
        plug = plug->next;
    }
}

void PluginsUpdate3D(Camera3D *cam)
{
    //  for 3D rendering efforts 
    //  run through the list and call onUpdate3D if available
    _plugin_t_ *plug = plugin_list;
    while(plug!=NULL)
    {
        if (plug->onUpdate3D!=NULL)
            plug->onUpdate3D(cam);
        plug = plug->next;
    }
}

void PluginsUpdate2D(Camera2D *cam)
{
    //  for 2D rendering efforts 
    //  run through the list and call onUpdate2D if available
    _plugin_t_ *plug = plugin_list;
    while(plug!=NULL)
    {
        if (plug->onUpdate2D!=NULL)
            plug->onUpdate2D(cam);
        plug = plug->next;
    }
}

//  force trigger a reload of all plugins 
void PluginsForceReLoad()
{
    _plugin_t_ *plug = plugin_list;
    while(plug!=NULL)
    {
        plug->last_file_modification_time = 0;
        plug = plug->next;
    }
}

//  unload and free memory 
void PluginsUnLoad()
{
    _plugin_t_ *plug = plugin_list;
    while(plug!=NULL)
    {
        _plugin_t_ *next = plug->next;
        if (plug->shared_library!=NULL)
        {
            if (plug->onUnload!=NULL)
                plug->onUnload();
            ray_dlclose(plug->shared_library);
        }
        if (plug->filepath)
            RL_FREE(plug->filepath);
        RL_FREE(plug);
        plug = next;
    }
}
