
#include "corona_headers.h"

BOOL MD2::Load( char * filename )
{
   BOOL success = FALSE;

   FILE * file = fopen( filename, "rb" );

   do
   {
      if ( !file ) break;

      // read the header
      fread( &m_model.header, sizeof( m_model.header ), 1, file );

      // verify version
      if ( m_model.header.ident != MODEL_IDENT ||m_model.header.version != MODEL_VERSION ) break;

      // allocate memory
      m_model.skins     = new md2_skin_t[ m_model.header.num_skins ]; //skins
      m_model.texcoords = new md2_texCoord_t[ m_model.header.num_st ]; //texture coords
      m_model.triangles = new md2_triangle_t[ m_model.header.num_tris ]; //triangles
      m_model.frames    = new md2_frame_t[ m_model.header.num_frames ]; //frames
      m_model.glcmds    = new int[ m_model.header.num_glcmds ]; //opengl commands

      // read model data
      fseek( file, m_model.header.offset_skins, SEEK_SET );
      fread( m_model.skins, m_model.header.num_skins * sizeof( md2_skin_t ), 1, file );

      fseek( file, m_model.header.offset_st, SEEK_SET );
      fread( m_model.texcoords, m_model.header.num_st * sizeof( md2_texCoord_t ), 1, file );

      fseek( file, m_model.header.offset_tris, SEEK_SET );
      fread( m_model.triangles, m_model.header.num_tris * sizeof( md2_triangle_t ), 1, file );

      fseek( file, m_model.header.offset_glcmds, SEEK_SET );
      fread( m_model.glcmds, m_model.header.num_glcmds * sizeof( int ), 1, file );

      // read frames
      fseek( file, m_model.header.offset_frames, SEEK_SET );

      uint32 i;
      for ( i = 0; i < (uint32) m_model.header.num_frames; i++ )
      {
         // allocate verts for this frame
         m_model.frames[ i ].verts = new md2_vertex_t[ m_model.header.num_vertices ];
         
         // read frame data
         fread( m_model.frames[ i ].scale    , sizeof( vec3_t ), 1, file );
         fread( m_model.frames[ i ].translate, sizeof( vec3_t ), 1, file );
         fread( m_model.frames[ i ].name     , sizeof( char )  ,16, file );
         fread( m_model.frames[ i ].verts    , sizeof( md2_vertex_t ), m_model.header.num_vertices, file );
      }
      
      // get the folder that the file lives in
      char modelDir[ MAX_PATH ] = { 0 };
      const char *pDirEnd = strrchr( filename, '\\' );

      if ( pDirEnd )
      {
         pDirEnd++;
         strncpy( modelDir, filename, pDirEnd - filename );
      }

      Load_Textures( modelDir );

      success = TRUE;
   }
   while( 0 );

   if ( file )
   {
      fclose( file );
   }

   return success;
}

BOOL MD2::Load_Textures( char *pTextureDir )
{
   BOOL success    = FALSE;
   FILE * bmp_file = NULL;

   do
   {
      PCX_FILE pcx_file;
      BMP_FILE bmp_file;

      bmp_file.bit_depth   = 16; //request

      m_skin_data.tex_size = m_model.header.skinwidth * m_model.header.skinheight * sizeof( short );
      m_skin_data.p_tex    = (short **)new short *[ m_model.header.num_skins ]; //allocate enough pointers for all our skins

      memset( m_skin_data.p_tex, 0, sizeof( short * ) * m_model.header.num_skins );

      //char skin_path[ MAX_PATH ] = { ROOT_DRIVE"quake 2 " };
      char skin_name[ MAX_PATH ];

      uint32 i;
      for ( i = 0; i < (uint32)m_model.header.num_skins; i++ )
      {         
         // the names contain a relative path, which we don't need. skip it.
         // for safety, if there is no path for some reason, just default to the full name.
         char *pSkinName = strrchr( m_model.skins[ i ].name, '/' );
         if ( !pSkinName )
         {
            pSkinName = m_model.skins[ i ].name;
         }

         sprintf( skin_name, "%s%s", pTextureDir, pSkinName );

         if ( !Load_PCX_File( skin_name, &pcx_file ) ) break;
         if ( !PCX_To_BMP( &pcx_file, &bmp_file ) )    break;

         // allocate memory for the skin
         m_skin_data.p_tex[ i ] = (short *)new BYTE[ m_skin_data.tex_size ];

         memcpy( m_skin_data.p_tex[ i ], bmp_file.data, m_skin_data.tex_size );

         Destroy_PCX_File( &pcx_file );
         Destroy_BMP_File( &bmp_file );
      }

      // verify all the skins were loaded.
      if ( i == m_model.header.num_skins )
      {
         success = TRUE;
      }
   }
   while( 0 );

   return success;
}

void MD2::UnLoad( void )
{
   uint32 i;

   //remove verts 
   for ( i = 0; i < (uint32) m_model.header.num_frames; i++ )
   {
      delete [] m_model.frames[ i ].verts;
   }

   // remove the skins
   for ( i = 0; i < (uint32) m_model.header.num_skins; i++ )
   {
      delete [] m_skin_data.p_tex[ i ];
   }

   // remove the pointer array
   delete [] m_skin_data.p_tex;

   delete [] m_model.glcmds;
   delete [] m_model.frames;
   delete [] m_model.triangles;
   delete [] m_model.texcoords;
   delete [] m_model.skins;

   memset( &m_model, 0, sizeof( m_model ) );
}
