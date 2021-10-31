#include <sega_cdc.h>
#include <sega_gfs.h>

#include "devcart.h"
#include "print.h"
#include "release.h"

// max num files that can be opened simultaneously
#define MAX_OPEN        1

// max number of files per directory
#define MAX_DIR         100

// size of one sector in bytes
#define SECT_SIZE       2048

static Uint32 workArea[GFS_WORK_SIZE(MAX_OPEN) / sizeof(Uint32)]; //library work area

static GfsDirTbl dirTable; //directory info management struct
static GfsDirName dirname[MAX_DIR]; //list of all filenames

void CD_Init(void) {
    if (!DEVCART_LOAD) {
        GFS_DIRTBL_TYPE(&dirTable) = GFS_DIR_NAME;
        GFS_DIRTBL_DIRNAME(&dirTable) = dirname;
        GFS_DIRTBL_NDIR(&dirTable) = MAX_DIR;
        GFS_Init(MAX_OPEN, workArea, &dirTable);
    }
}

void CD_ChangeDir(char *directory) {
    Sint32 id = GFS_NameToId((Sint8 *)directory);
    GFS_DIRTBL_TYPE(&dirTable) = GFS_DIR_NAME;
    GFS_DIRTBL_DIRNAME(&dirTable) = dirname;
    GFS_DIRTBL_NDIR(&dirTable) = MAX_DIR;
    GFS_LoadDir(id, &dirTable);
    GFS_SetDir(&dirTable);
}

/**
 * read data off the cd
 * filename: duh
 * dataBuf: where you want to copy the data to
 * returns number of bytes read
 */
Sint32 CD_Load(char *filename, void *dataBuf) {
    if (DEVCART_LOAD) {
        return Devcart_LoadFile(filename, dataBuf);
    }
    else {
        Sint32 size;

        GfsHn gfs = GFS_Open(GFS_NameToId((Sint8 *)filename));
        GFS_GetFileInfo(gfs, NULL, NULL, &size, NULL);
        //make sure we read at least one sector
        GFS_Fread(gfs, size < SECT_SIZE ? 1 : (size >> 11) + 1, dataBuf, size);
        GFS_Close(gfs);
        return size;
    }
}
