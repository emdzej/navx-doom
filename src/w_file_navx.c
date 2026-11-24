//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 navx-doom
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// DESCRIPTION:
//	WAD I/O: doomgeneric's w_file_stdc.c (built instead of it), plus
//	`-mmap`: the whole WAD read into memory in one go. One sequential
//	read instead of a seek for every lump: on a CD or DVD drive that's
//	seconds instead of minutes at start-up. Lumps are then used in
//	place (wad.mapped), so the zone holds no copies. Without enough
//	memory it says so and reads from the file as before.
//	(doomgeneric's own -mmap path needs HAVE_MMAP, which its config.h
//	turns off.)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m_argv.h"
#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    FILE *fstream;
} stdc_wad_file_t;

extern wad_file_class_t stdc_wad_file;

// The whole file in memory, or NULL.
static byte *W_ReadAll(FILE *f, unsigned int size, char *path)
{
    byte *data = size ? malloc(size) : NULL;
    size_t got = 0, n;

    if (data == NULL)
    {
        printf("W_OpenFile: no memory for %s (%u bytes): reading from the file\n", path, size);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    while (got < size && (n = fread(data + got, 1, size - got > 0x40000 ? 0x40000 : size - got, f)) > 0)
    {
        got += n;
    }
    if (got != size)
    {
        free(data);
        return NULL;
    }
    printf("W_OpenFile: %s in memory (%u bytes)\n", path, size);
    return data;
}

static wad_file_t *W_StdC_OpenFile(char *path)
{
    stdc_wad_file_t *result;
    FILE *fstream;

    fstream = fopen(path, "rb");

    if (fstream == NULL)
    {
        return NULL;
    }

    result = Z_Malloc(sizeof(stdc_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &stdc_wad_file;
    result->wad.mapped = NULL;
    result->wad.length = M_FileLength(fstream);
    result->fstream = fstream;

    if (M_CheckParm("-mmap"))
    {
        result->wad.mapped = W_ReadAll(fstream, result->wad.length, path);
    }

    return &result->wad;
}

static void W_StdC_CloseFile(wad_file_t *wad)
{
    stdc_wad_file_t *stdc_wad = (stdc_wad_file_t *) wad;

    free(wad->mapped);
    fclose(stdc_wad->fstream);
    Z_Free(stdc_wad);
}

size_t W_StdC_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    stdc_wad_file_t *stdc_wad = (stdc_wad_file_t *) wad;

    if (wad->mapped != NULL)
    {
        if (offset >= wad->length)
        {
            return 0;
        }
        if (buffer_len > wad->length - offset)
        {
            buffer_len = wad->length - offset;
        }
        memcpy(buffer, wad->mapped + offset, buffer_len);
        return buffer_len;
    }

    fseek(stdc_wad->fstream, offset, SEEK_SET);
    return fread(buffer, 1, buffer_len, stdc_wad->fstream);
}

wad_file_class_t stdc_wad_file =
{
    W_StdC_OpenFile,
    W_StdC_CloseFile,
    W_StdC_Read,
};
