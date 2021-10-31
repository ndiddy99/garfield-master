#ifndef CD_H
#define CD_H

#define LWRAM	(0x200000)

//init cd stuff
void CD_Init(void);

// change the current directory
void CD_ChangeDir(char *directory);

/**
 * read data off the cd's root directory
 * filename: duh
 * dataBuf: where to copy the data to
 * returns the loaded file's size
 */
Sint32 CD_Load(char *filename, void *dataBuf);
#endif
