
#include "corona_headers.h"

BOOL VGD::Load( char * filename )
{
   char texture_name[ MAX_PATH ];

   BOOL success    = FALSE;
   FILE * file     = fopen( filename, "rb" );
   FILE * bmp_file = NULL;

   do
   {
      if ( !file ) break;

      // get the texture now so we can bail if it doesn't exist.
      strcpy( texture_name, filename );
      char * ext = strrchr( texture_name, '.' );
      
      if ( !ext ) break;

      // change to bmp
      strcpy( ext, ".bmp" );

      // read the texture
      if ( !Load_Texture( texture_name ) ) break;

      // read the header
      fread( &m_model.header, 1, sizeof( m_model.header ), file );

      // read verts
      fseek( file, m_model.header.verticesOffset, SEEK_SET );

      m_model.p_vert = new Vag_Vertex[ m_model.header.vertices ];

      uint32 i;
      for ( i = 0; i < m_model.header.vertices; i++ )
      {
         fread( &m_model.p_vert[ i ], 1, sizeof( Vag_Vertex ), file );
      }

      // read face indices
      fseek( file, m_model.header.indicesOffset, SEEK_SET );
      
      m_model.p_face = new Vag_Face[ m_model.header.faces ];

      for ( i = 0; i < m_model.header.faces; i++ )
      {
         // read 3 face indices at once.
         fread( &m_model.p_face[ i ].vertex_index, 1, sizeof( int ) * 3, file );
      }

      success = TRUE;
   }
   while( 0 );

   if ( file )
   {
      fclose( file );
   }

   return success;
}

BOOL VGD::Load_Texture( char * texture_name )
{
   BOOL success    = FALSE;
   FILE * bmp_file = NULL;

   do
   {
      BITMAPFILEHEADER fh;
	   BITMAPINFOHEADER bmih;

      bmp_file = fopen( texture_name, "rb" );

      if ( bmp_file )
      {
         //Read the file header
	      fread( &fh, sizeof(BITMAPFILEHEADER), 1, bmp_file );

	      //read the bmih
	      fread( &bmih, sizeof( bmih ), 1, bmp_file );

	      //Correct the filesize for the info header, by taking the bit offset after the file header.
	      if ( !bmih.biSizeImage ) 
	      {
		      bmih.biSizeImage = fh.bfSize - fh.bfOffBits;
	      }

	      m_model.texture.width    = bmih.biWidth;
	      m_model.texture.height   = bmih.biHeight;
         m_model.texture.tex_size = bmih.biSizeImage;

         m_model.texture.p_data = (short *)new BYTE[ m_model.texture.tex_size ];

         fread( m_model.texture.p_data, m_model.texture.tex_size, 1, bmp_file );
      }
      else
      {
         // generate a fake texture
         m_model.texture.width    = 2; 
	      m_model.texture.height   = 2;
         m_model.texture.tex_size = 8;//8 = short * 2x2.

         m_model.texture.p_data = (short *)new BYTE[ m_model.texture.tex_size ];

         memset( m_model.texture.p_data, 0xFF, m_model.texture.tex_size );
      }

      success = TRUE;
   }
   while( 0 );

   if ( bmp_file )
   {
      fclose( bmp_file );
   }

   return success;
}

void VGD::UnLoad( void )
{
   //remove verts 
   delete [] m_model.texture.p_data;

   delete [] m_model.p_face;
   delete [] m_model.p_vert;

   memset( &m_model, 0, sizeof( m_model ) );
}
