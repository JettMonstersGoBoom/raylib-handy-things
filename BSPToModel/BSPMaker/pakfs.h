#ifdef PAK_FS_IMPL

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "qstrcmp.h"

//  adapted from here 
//  https://quakewiki.org/wiki/.pak#:~:text=pak%20is%20Quake's%20container%20file,many%20tools%20for%20working%20with%20.

typedef struct 
{
    uint32_t ID;
    uint32_t offset;
    uint32_t size;
} pak_header_t;

typedef struct 
{
    uint8_t Name[56];
    uint32_t offset;
    uint32_t size;
} pak_entry_t;

static struct 
{
    FILE *fp;
    int         nFiles;
    pak_entry_t *Files;
} pak;

// Load data from file into a buffer
static unsigned char *OS_LoadFileData(const char *fileName, int *dataSize)
{
    unsigned char *data = NULL;
    *dataSize = 0;

    if (fileName != NULL)
    {
        FILE *file = fopen(fileName, "rb");

        if (file != NULL)
        {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);     // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                data = (unsigned char *)RL_MALLOC(size*sizeof(unsigned char));

                if (data != NULL)
                {
                    // NOTE: fread() returns number of read elements instead of bytes, so we read [1 byte, size elements]
                    size_t Count = fread(data, sizeof(unsigned char), size, file);

                    // WARNING: fread() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
                    // dataSize is unified along raylib as a 'int' type, so, for file-sizes > INT_MAX (2147483647 bytes) we have a limitation
                    if (Count > 2147483647)
                    {
                        TRACELOG(LOG_WARNING, "FILEIO: [%s] File is bigger than 2147483647 bytes, avoid using LoadFileData()", fileName);

                        RL_FREE(data);
                        data = NULL;
                    }
                    else
                    {
                        *dataSize = (int)Count;

                        if ((*dataSize) != size) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially loaded (%i bytes out of %i)", fileName, dataSize, Count);
                        else TRACELOG(LOG_INFO, "FILEIO: [%s] File loaded successfully", fileName);
                    }
                }
                else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
            }
            else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return data;
}


void *PAKFS_loadfile(const char *pak_filename, const char *filename, int *out_filesize)
{
    FILE *fp;
    pak_header_t pak_header;
    int num_files;
    int i;
    pak_entry_t pak_file;
    void *buffer;

    fp = fopen(pak_filename, "rb");
    if (!fp)
    {
        TraceLog(LOG_INFO,"File %s not opened",pak_filename);
        return NULL;
    }

    if (!fread(&pak_header, sizeof(pak_header_t), 1, fp))
        goto pak_error;

    if (pak_header.ID!=0x4b434150)
        goto pak_error;

    num_files = pak_header.size / sizeof(pak_entry_t);

    if (fseek(fp, pak_header.offset, SEEK_SET) != 0)
        goto pak_error;

    for (i = 0; i < num_files; i++)
    {
        if (!fread(&pak_file, sizeof(pak_entry_t), 1, fp))
            goto pak_error;

        if (!qstricmp((const char*)pak_file.Name, filename))
        {
            if (fseek(fp, pak_file.offset, SEEK_SET) != 0)
                goto pak_error;

            buffer = RL_CALLOC(pak_file.size,1);
            if (!buffer)
                goto pak_error;

            if (!fread(buffer, pak_file.size, 1, fp))
            {
                RL_FREE(buffer);
                goto pak_error;
            }

            TraceLog(LOG_INFO,"File %s",filename);

            if (out_filesize)
                *out_filesize = pak_file.size;
            return buffer;
        }
    }
pak_error:
    TraceLog(LOG_INFO,"failed %s",filename);
    fclose(fp);
    return NULL;
}

static const char *pakPath=".";
static unsigned char* PAK_LoadFileData(const char* path, int* len)
{
    FilePathList paks = LoadDirectoryFilesEx(pakPath,".pak",false);
    for (int q=0;q<paks.count;q++)
    {
        unsigned char *ret = PAKFS_loadfile(paks.paths[q],path,len);
        if (ret!=NULL)
        {
            UnloadDirectoryFiles(paks);
            return ret;
        }
    }
    UnloadDirectoryFiles(paks);
    return (OS_LoadFileData(path,len));
}

void PAKFS_Mount(const char *_pakPath)
{
    pakPath = _pakPath;
    SetLoadFileDataCallback(PAK_LoadFileData);
}

void PAKFS_Unmount()
{
    SetLoadFileDataCallback(NULL);
}
#else 

void PAKFS_Mount(const char *_pakPath);
void PAKFS_Unmount();

#endif