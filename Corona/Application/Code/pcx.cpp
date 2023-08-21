
#include "pcx.h"

BOOL Load_PCX_File( char * filename, PCX_FILE * pcx_file )
{
   BOOL   success        = FALSE;
   FILE * file           = NULL;
   BYTE * file_data      = NULL;
   BYTE * cur_pos        = NULL;
   BYTE * image_pos      = NULL;
   uint32 scanline_length;
   BYTE   run_count;
   uint32 y;
   uint32 x;
   uint32 i;
   long   file_length;

   do
   {
      file = fopen( filename, "rb" );

      if ( !file ) break;

      // get the file length
      fseek( file, 0, SEEK_SET );
      fseek( file, 0, SEEK_END );

      file_length = ftell( file );

      fseek( file, 0, SEEK_SET );

      // allocate enough memory for the whole file.
      file_data = (BYTE *)malloc( file_length );

      if ( !file_data ) break;

      // read the entire file into memory, because we'll need to do a lot of jumping around.
      fread( file_data, 1, file_length, file );

      // our working pointer.
      cur_pos = file_data;

      // read the pcx header
      memcpy( &pcx_file->header, cur_pos, sizeof( pcx_file->header ) );
      cur_pos += sizeof( pcx_file->header );

      // validate the header
      if ( pcx_file->header.manufacturer != PC_PAINTBRUSH_PCX ) break;
      if ( pcx_file->header.encoding     != 1                 ) break;

      // get image dimensions
      pcx_file->width  = ( pcx_file->header.max_x - pcx_file->header.min_x ) + 1;
      pcx_file->height = ( pcx_file->header.max_y - pcx_file->header.min_y ) + 1;

      //this is the length of one DECODED scan line.
      scanline_length = pcx_file->header.bytes_per_line * pcx_file->header.num_planes;

      // allocate memory for the image
      pcx_file->data = (BYTE *)malloc( scanline_length * pcx_file->height );
      image_pos      = pcx_file->data;

      // begin decoding scan lines
      for ( y = 0; y < pcx_file->height; y++ )
      {
         //we begin at 0 and want to stop when we have uncompressed "pcx_file->width" bytes. That signifies the end of this span.
         for ( x = 0; x < pcx_file->width; )
         {
            // is this byte data or a compression flag?
            if ( DUPLICATE_FOLLOWING_BYTE == ( *cur_pos & DUPLICATE_FOLLOWING_BYTE ) )
            {
               // it is!!! oh happy day. Let's find out how many bytes will follow.
               run_count = (*cur_pos & DUPLICATE_BYTE_COUNT);

               // move on to the next byte and begin reading.
               cur_pos++;
            }
            else
            {
               // read one byte, and it'll be this byte.
               run_count = 1;
            }

            // read the chunk
            for ( i = 0; i < run_count; i++, x++ )
            {
               *image_pos = *cur_pos;

               // advance data.
               image_pos++;
               cur_pos++;
            }
         }
         
         // there can be extra bytes at the end of a file. here we want to remove them.
         for ( ; x < (uint32)pcx_file->header.bytes_per_line; x++ )
         {
            cur_pos++;
         }
      }

      // we should have the complete decompressed file in "pcx_file->data"
      cur_pos = ( file_data + file_length ) - PALETTE_OFFSET_FROM_END;

      //256 color palette found?
      if ( *cur_pos != PALETTE_START ) break;

      cur_pos++;

      // read the palette
      for ( i = 0; i < 256; i++ )
      {
         memcpy( &pcx_file->palette[ i ], cur_pos, sizeof( PALETTE_COLOR ) );

         cur_pos += sizeof( PALETTE_COLOR );
      }

      success = TRUE;

   }
   while( 0 );

   if ( file )
   {
      fclose( file );
   }

   free( file_data );

   return success;
}

BOOL PCX_To_BMP( PCX_FILE * pcx_file, BMP_FILE * bmp_file )
{
   BOOL     success       = FALSE;
   BYTE   * p_pcx_data    = NULL;
   short  * p_bmp_data_16 = NULL;
   uint32 * p_bmp_data_32 = NULL;
   BYTE   * p_bmp_data_8  = NULL;
   short    r;
   short    b;
   short    g;
   uint32   y, x; 

   do
   { 
      if ( 16 != bmp_file->bit_depth && 32 != bmp_file->bit_depth ) break;

      bmp_file->width  = pcx_file->width;
      bmp_file->height = pcx_file->height;
      bmp_file->data   = (BYTE *)malloc( bmp_file->width * bmp_file->height * ( bmp_file->bit_depth / 8 ) );

      p_pcx_data = pcx_file->data;

      if ( 16 == bmp_file->bit_depth )
      {
         p_bmp_data_16 = (short *)bmp_file->data;

         for ( y = 0; y < pcx_file->height; y++ )
         {
            for ( x = 0; x < pcx_file->width; x++ )
            {
               r = (short) ( ( pcx_file->palette[ p_pcx_data[ x ] ].r / 255.0f ) * 31.0f );
               g = (short) ( ( pcx_file->palette[ p_pcx_data[ x ] ].g / 255.0f ) * 31.0f );
               b = (short) ( ( pcx_file->palette[ p_pcx_data[ x ] ].b / 255.0f ) * 31.0f );

               p_bmp_data_16[ x ] = ( r << 10 ) | ( g << 5 ) | b;
            }
            
            p_pcx_data    += pcx_file->width;
            p_bmp_data_16 += bmp_file->width;
         }
      }
      else
      {
         p_bmp_data_32 = (uint32 *)bmp_file->data;

         for ( x = 0; x < pcx_file->width * pcx_file->height; x++ )
         {
            // use a byte to avoid shifting.          
            p_bmp_data_8 = (BYTE *)p_bmp_data_32;

            *p_bmp_data_8       = pcx_file->palette[ *p_pcx_data ].b;
            *(p_bmp_data_8 + 1 )= pcx_file->palette[ *p_pcx_data ].g;
            *(p_bmp_data_8 + 2 )= pcx_file->palette[ *p_pcx_data ].r;

            p_pcx_data++;
            p_bmp_data_32++;
         }
      }

      success = TRUE;
   }
   while( 0 );

   return success;
}

void Destroy_PCX_File( PCX_FILE * pcx_file )
{
   //clean up the PCX file.
   free( pcx_file->data );

   memset( &pcx_file, 0, sizeof( pcx_file ) );
}

void Destroy_BMP_File( BMP_FILE * bmp_file )
{
   free( bmp_file->data );

   memset( &bmp_file, 0, sizeof( bmp_file ) );
}
