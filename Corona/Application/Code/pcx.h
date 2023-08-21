
#ifndef PCX_H_
#define PCX_H_

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include "types.h"

#pragma warning(disable : 4995) //Disable security warnings for stdio functions
#pragma warning(disable : 4996) //Disable security warnings for stdio functions
#define _CRT_SECURE_NO_DEPRECATE //Disable security warnings for stdio functions

//manufacturers
#define PC_PAINTBRUSH_PCX       (10)

//PCX versions
#define PCX_VERSION_2_5         (0)
#define PCX_VERSION_2_8_PALETTE (2)
#define PCX_VERSION_2_8         (3)
#define PCX_VERSION_3_0_PALETTE (5)

//Palette interpretation
#define COLOR_BW                (1)
#define GRAYSCALE               (2)

//Compression Symbols
#define DUPLICATE_FOLLOWING_BYTE (0xC0)
#define DUPLICATE_BYTE_COUNT     (0x3F)

//Palette offsets
#define PALETTE_START            (12)
#define PALETTE_OFFSET_FROM_END  (769)

//128 Bytes
typedef struct _PCX_HEADER
{
   char manufacturer;         //Manufacturer, (should be 10)
   char version;              //Version, see above
   char encoding;             //.PCX Run Length Encoding (should be 1)
   char bit_depth;            //bit depth
   short min_x;               //min x
   short min_y;               //min y
   short max_x;               //max x
   short max_y;               //max y
   short horz_res;            //horizontal resolution of creating device
   short vert_res;            //vertical resolution of creating device
   BYTE  color_map[ 16 * 3 ]; //16 Color EGA Color Palette. We won't really be using it.
   BYTE  reserved;            //Reserved byte set to 0
   BYTE  num_planes;          //Number of color planes (a plane is a collection of one color.)
   short bytes_per_line;      //Number of bytes per scan line per color plane (always even for .PCX files)
   short palette_info;        //How to interpret the palette, see above
   BYTE  padding[ 58 ];       //round out the struct to 128 bytes.   
}
PCX_HEADER;

typedef struct _PALETTE_COLOR
{
   BYTE r;
   BYTE g;
   BYTE b;
}
PALETTE_COLOR;

typedef struct _PCX_FILE
{
   PCX_HEADER    header;
   uint32        width;
   uint32        height;
   BYTE      *   data;
   PALETTE_COLOR palette[ 256 ];
}
PCX_FILE;

typedef struct _BMP_FILE
{
   uint32        width;
   uint32        height;
   uint32        bit_depth;
   BYTE      *   data;
}
BMP_FILE;

BOOL Load_PCX_File( char * filename, PCX_FILE * pcx_file );

BOOL PCX_To_BMP( PCX_FILE * pcx_file, BMP_FILE * bmp_file );

void Destroy_PCX_File( PCX_FILE * pcx_file );
void Destroy_BMP_File( BMP_FILE * bmp_file );

#endif
