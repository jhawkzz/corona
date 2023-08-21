
#include "corona_headers.h"

static char m_model_ext[ MODEL_COUNT ][ 4 ] =
{
   "",
   "MD2",
   "VGD",
};

BOOL Mesh_Loader::Load_Mesh( char * filename, Mesh * p_mesh, uint32 texture_flags )
{
   BOOL success = FALSE;

   MODEL model = Get_Model_Type( filename );

   switch( model )
   {
      case MODEL_MD2: success = Convert_MD2( filename, p_mesh, texture_flags ); break;
      case MODEL_VGD: success = Convert_VGD( filename, p_mesh, texture_flags ); break;
   }

   return success;
}

MODEL Mesh_Loader::Get_Model_Type( char * filename )
{
   MODEL model = MODEL_UNKNOWN;

   do
   {
      char * ext = strrchr( filename, '.' );

      if ( !ext ) break;

      ext++;

      uint32 i;
      for ( i = 0; i < MODEL_COUNT; i++ )
      {
         if ( !stricmp( ext, m_model_ext[ i ] ) )
         {
            model = (MODEL) i;
            break;
         }
      }
   }
   while( 0 );

   return model;
}

BOOL Mesh_Loader::Convert_MD2( char * filename, Mesh * p_mesh, uint32 texture_flags )
{
   MD2 md2_loader;

   if ( !md2_loader.Load( filename ) ) return FALSE;

   // clear out the old mesh.
   p_mesh->Destroy( );
   p_mesh->Create( );

   // set counts
   p_mesh->m_vertex_count = md2_loader.m_model.header.num_vertices;
   p_mesh->m_frame_count  = md2_loader.m_model.header.num_frames;

   // allocate frames
   p_mesh->m_p_frame = new Frame[ p_mesh->m_frame_count ];

   // allocate verts and translate all md2 vert data
   uint32 i;
   for ( i = 0; i < p_mesh->m_frame_count; i++ )
   {
      // use max_vertices so that there is room for creating new verts so nothing shares
      p_mesh->m_p_frame[ i ].p_vertex = (Vertex *)_aligned_malloc( sizeof( Vertex ) * MAX_VERTICES, 16 );

      uint32 c;
      for ( c = 0; c < p_mesh->m_vertex_count; c++ )
      {
         md2_frame_t * p_md2_frame = &md2_loader.m_model.frames[ i ];

         p_mesh->m_p_frame[ i ].p_vertex[ c ].m_position.x = ( p_md2_frame->scale[ 0 ] * p_md2_frame->verts[ c ].v[ 0 ] ) + p_md2_frame->translate[ 0 ];
         p_mesh->m_p_frame[ i ].p_vertex[ c ].m_position.z = ( p_md2_frame->scale[ 1 ] * p_md2_frame->verts[ c ].v[ 1 ] ) + p_md2_frame->translate[ 1 ]; //swap y and z
         p_mesh->m_p_frame[ i ].p_vertex[ c ].m_position.y = ( p_md2_frame->scale[ 2 ] * p_md2_frame->verts[ c ].v[ 2 ] ) + p_md2_frame->translate[ 2 ];
         p_mesh->m_p_frame[ i ].p_vertex[ c ].m_position.w = 1;

         p_mesh->m_p_frame[ i ].p_vertex[ c ].m_normal.x = -normal_lookup[ p_md2_frame->verts[ c ].normalIndex ][ 0 ];
         p_mesh->m_p_frame[ i ].p_vertex[ c ].m_normal.z = -normal_lookup[ p_md2_frame->verts[ c ].normalIndex ][ 1 ]; //swap y and z
         p_mesh->m_p_frame[ i ].p_vertex[ c ].m_normal.y = -normal_lookup[ p_md2_frame->verts[ c ].normalIndex ][ 2 ];
      }
   }

   // read the faces
   p_mesh->m_face_count = md2_loader.m_model.header.num_tris;
   
   p_mesh->m_p_face     = new Face_Map[ p_mesh->m_face_count ];

   // correct any faces that want to share verts but not UVs.
   // for those cases, create new verts.
   for ( i = 0; i < p_mesh->m_face_count; i++ )
   {
      uint32 c;
      for ( c = 0; c < 3; c++ )
      {
         // if this is true we need to create and use a new vertex
         if ( md2_loader.m_model.triangles[ i ].vertex[ c ] != md2_loader.m_model.triangles[ i ].st[ c ] )
         {
            // make a new copy of this vert for each frame
            uint32 d;
            for ( d = 0; d < p_mesh->m_frame_count; d++ )
            {
               p_mesh->m_p_frame[ d ].p_vertex[ p_mesh->m_vertex_count ] = p_mesh->m_p_frame[ d ].p_vertex[ md2_loader.m_model.triangles[ i ].vertex[ c ] ];
            }
            
            // change the index in the md2 and mesh
            md2_loader.m_model.triangles[ i ].vertex[ c ] = p_mesh->m_vertex_count;

            p_mesh->m_vertex_count++;
         }
      }
   }

   // assign face vertex indices
   for ( i = 0; i < p_mesh->m_face_count; i++ )
   {
      // MD2 triangles go clock-wise, mine go counter clockwise.
      p_mesh->m_p_face[ i ].vertex_list[ 0 ] = md2_loader.m_model.triangles[ i ].vertex[ 0 ];
      p_mesh->m_p_face[ i ].vertex_list[ 1 ] = md2_loader.m_model.triangles[ i ].vertex[ 1 ];
      p_mesh->m_p_face[ i ].vertex_list[ 2 ] = md2_loader.m_model.triangles[ i ].vertex[ 2 ];

      Face_Map       * p_face     = &p_mesh->m_p_face[ i ];
      md2_triangle_t * p_md2_face = &md2_loader.m_model.triangles[ i ];

      // translate/assign uvs, which will need to be done for every frame
      uint32 c;
      for ( c = 0; c < p_mesh->m_frame_count; c++ )
      {
         //MD2 uvs are stored like this:
         //Each triangle holds the 3 verts and 3 UVs, one for each vert. It actually holds the indices for each vert that are then within a texcoords lookup table. 
         //Then, to get the final result, we must divide by the corresponding skin's width/height.
         p_mesh->m_p_frame[ c ].p_vertex[ p_face->vertex_list[ 0 ] ].m_uv.u = (float) md2_loader.m_model.texcoords[ p_md2_face->st[ 0 ] ].s / md2_loader.m_model.header.skinwidth;
         p_mesh->m_p_frame[ c ].p_vertex[ p_face->vertex_list[ 0 ] ].m_uv.v = (float) md2_loader.m_model.texcoords[ p_md2_face->st[ 0 ] ].t / md2_loader.m_model.header.skinheight;

         p_mesh->m_p_frame[ c ].p_vertex[ p_face->vertex_list[ 1 ] ].m_uv.u = (float) md2_loader.m_model.texcoords[ p_md2_face->st[ 1 ] ].s / md2_loader.m_model.header.skinwidth;
         p_mesh->m_p_frame[ c ].p_vertex[ p_face->vertex_list[ 1 ] ].m_uv.v = (float) md2_loader.m_model.texcoords[ p_md2_face->st[ 1 ] ].t / md2_loader.m_model.header.skinheight;

         p_mesh->m_p_frame[ c ].p_vertex[ p_face->vertex_list[ 2 ] ].m_uv.u = (float) md2_loader.m_model.texcoords[ p_md2_face->st[ 2 ] ].s / md2_loader.m_model.header.skinwidth;
         p_mesh->m_p_frame[ c ].p_vertex[ p_face->vertex_list[ 2 ] ].m_uv.v = (float) md2_loader.m_model.texcoords[ p_md2_face->st[ 2 ] ].t / md2_loader.m_model.header.skinheight;
      }
   }

   if ( 1 == p_mesh->m_frame_count )
   {
      // copy the frame into nowframe
      for ( i = 0; i < p_mesh->m_vertex_count; i++ )
      {
         p_mesh->m_now_frame.p_vertex[ i ] = p_mesh->m_p_frame->p_vertex[ i ];
      }
   }

   // load the texture
   p_mesh->m_texture.width    = md2_loader.m_model.header.skinwidth;
   p_mesh->m_texture.height   = md2_loader.m_model.header.skinheight;
   p_mesh->m_texture.tex_size = md2_loader.m_skin_data.tex_size;
   p_mesh->m_texture_flags    = texture_flags; //eventually this should be taken from the model file

   p_mesh->m_texture.p_data   = (short *)new BYTE[ md2_loader.m_skin_data.tex_size ];

   if ( md2_loader.m_skin_data.p_tex[ 0 ] )
   {
      memcpy( p_mesh->m_texture.p_data, md2_loader.m_skin_data.p_tex[ 0 ], p_mesh->m_texture.tex_size );
   }
   else
   {
      memset( p_mesh->m_texture.p_data, 0xFF, p_mesh->m_texture.tex_size );
   }

   // free the md2 file.
   md2_loader.UnLoad( );

   return TRUE;
}

BOOL Mesh_Loader::Convert_VGD( char * filename, Mesh * p_mesh, uint32 texture_flags )
{
   VGD vgd_loader;

   if ( !vgd_loader.Load( filename ) ) return FALSE;

   p_mesh->Destroy( );
   p_mesh->Create( );

   p_mesh->m_vertex_count      = vgd_loader.m_model.header.vertices;
   p_mesh->m_frame_count       = 1; //no frames for now
   
   p_mesh->m_p_frame           = new Frame;
   p_mesh->m_p_frame->p_vertex = (Vertex *)_aligned_malloc( sizeof( Vertex ) * p_mesh->m_vertex_count, 16 );

   uint32 i;
   for ( i = 0; i < p_mesh->m_vertex_count; i++ )
   {
      //pos
      p_mesh->m_p_frame->p_vertex[ i ].m_position.x = vgd_loader.m_model.p_vert[ i ].pos[ 0 ];
      p_mesh->m_p_frame->p_vertex[ i ].m_position.y = vgd_loader.m_model.p_vert[ i ].pos[ 1 ];
      p_mesh->m_p_frame->p_vertex[ i ].m_position.z = vgd_loader.m_model.p_vert[ i ].pos[ 2 ];
      p_mesh->m_p_frame->p_vertex[ i ].m_position.w = 1;

      //uvs
      p_mesh->m_p_frame->p_vertex[ i ].m_uv.u = vgd_loader.m_model.p_vert[ i ].uv[ 0 ];
      p_mesh->m_p_frame->p_vertex[ i ].m_uv.v = vgd_loader.m_model.p_vert[ i ].uv[ 1 ];

      //normals
      p_mesh->m_p_frame->p_vertex[ i ].m_normal.x = vgd_loader.m_model.p_vert[ i ].normal[ 0 ];
      p_mesh->m_p_frame->p_vertex[ i ].m_normal.y = vgd_loader.m_model.p_vert[ i ].normal[ 1 ];
      p_mesh->m_p_frame->p_vertex[ i ].m_normal.z = vgd_loader.m_model.p_vert[ i ].normal[ 2 ];

      // there appears to be a bug in the exporting of VGD formats, so we need to normalize the normals.
      Normalize_V3( &p_mesh->m_p_frame->p_vertex[ i ].m_normal, &p_mesh->m_p_frame->p_vertex[ i ].m_normal );
   }

   p_mesh->m_face_count = vgd_loader.m_model.header.faces;
   
   p_mesh->m_p_face     = new Face_Map[ p_mesh->m_face_count ];

   for ( i = 0; i < p_mesh->m_face_count; i++ )
   {
      p_mesh->m_p_face[ i ].vertex_list[ 0 ] = vgd_loader.m_model.p_face[ i ].vertex_index[ 0 ];
      p_mesh->m_p_face[ i ].vertex_list[ 1 ] = vgd_loader.m_model.p_face[ i ].vertex_index[ 1 ];
      p_mesh->m_p_face[ i ].vertex_list[ 2 ] = vgd_loader.m_model.p_face[ i ].vertex_index[ 2 ];
   }


   if ( 1 == p_mesh->m_frame_count )
   {
      // copy the frame into nowframe
      for ( i = 0; i < p_mesh->m_vertex_count; i++ )
      {
         p_mesh->m_now_frame.p_vertex[ i ] = p_mesh->m_p_frame->p_vertex[ i ];
      }
   }

   // load the texture
   p_mesh->m_texture.width    = vgd_loader.m_model.texture.width;
   p_mesh->m_texture.height   = vgd_loader.m_model.texture.height;
   p_mesh->m_texture.tex_size = vgd_loader.m_model.texture.tex_size;
   p_mesh->m_texture_flags    = texture_flags; //eventually this should be taken from the model file

   p_mesh->m_texture.p_data   = (short *)new BYTE[ p_mesh->m_texture.tex_size ];

   memcpy( p_mesh->m_texture.p_data, vgd_loader.m_model.texture.p_data, p_mesh->m_texture.tex_size );

   vgd_loader.UnLoad( );

   return TRUE;
}
