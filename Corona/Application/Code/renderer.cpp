
#include "corona_headers.h"

Span_Buffer   Renderer::m_span_buffer;
float         Renderer::m_z_buffer[ CORONA_MAX_SCREEN_WIDTH * CORONA_MAX_SCREEN_HEIGHT ];
Cached_Vertex Renderer::m_vertex_cache[ MAX_VERTICES ];
Face          Renderer::m_face_clip[ MAX_FACES * 2 ];
Vertex_Const  Renderer::m_vertex_constants;

static uint32 m_red_up_lookup[ 32 ];
static uint32 m_green_up_lookup[ 32 ];

static uint32 m_red_down_lookup[ 31745 ];
static uint32 m_green_down_lookup[ 993 ];

static float m_specular_power_lookup[ 100 ];

Renderer::Renderer( void )
{
   //create lookup tables for 16bit RGB values
   uint16 i;
   for ( i = 0; i < 32; i++ )
   {
      // give us the shifted value in the original position. (1 << 10 = 1024)
      m_red_up_lookup[ i ]   = ( i << 10 );
      m_green_up_lookup[ i ] = ( i << 5 );

      // give us the original value in the shifted position. :)
      m_red_down_lookup[ m_red_up_lookup[ i ] ]     = i;
      m_green_down_lookup[ m_green_up_lookup[ i ] ] = i;
   }

   // calculate specularity lookup
   float value = 0.00f;

   for ( i = 0; i < 100; i++ )
   {
      m_specular_power_lookup[ i ] = pow( value, SPECULAR_INTENSITY );

      value += .01f;
   }
}

Renderer::~Renderer( )
{
}

void Renderer::Create( Camera * p_camera )
{
   // save the camera
   m_p_camera = p_camera;

   Toggle_Specularity( FALSE );
   Toggle_Texture_Filtering( FILTER_NONE );
}

void Renderer::Destroy( void )
{
}

void Renderer::Update_Keyboard( int controller_state )
{
   float old_keydown_timer = m_keydown_timer;

   //Temp! tell the server to start the race as soon as plus is pressed
   if ( controller_state & BUTTON_1 )
   {
      if ( m_keydown_timer < g_next_frame )
      {
         Toggle_Texture_Filtering( FILTER_NONE );

         // do suh-in
         m_keydown_timer = g_next_frame + .05f;
      }
   }
   else if ( controller_state & BUTTON_2 )
   {
      if ( m_keydown_timer < g_next_frame )
      {
         Toggle_Texture_Filtering( FILTER_BILINEAR_16 );

         // do suh-in
         m_keydown_timer = g_next_frame + .05f;
      }
   }
   else if ( controller_state & BUTTON_3 )
   {
      if ( m_keydown_timer < g_next_frame )
      {
         Toggle_Texture_Filtering( FILTER_BILINEAR_32 );

         // do suh-in
         m_keydown_timer = g_next_frame + .05f;
      }
   }
   else if ( controller_state & BUTTON_SPACE )
   {
      if ( m_keydown_timer < g_next_frame )
      {
         // take the opposite for specularity
         Toggle_Specularity( !m_specularity );

         // do suh-in
         m_keydown_timer = g_next_frame + .05f;
      }
   }
   else
   {
      m_keydown_timer = 0.00f;
   }

   //This can only be 0 if this is the first time we've pressed a key since releasing one.
   //If that is the case, override the normal keydown timer and force a 1 second delay.
   if ( !old_keydown_timer && m_keydown_timer )
   {
      m_keydown_timer = g_next_frame + 1.00f;
   }
}

void Renderer::Toggle_Texture_Filtering( TEXTURE_FILTER filter )
{
   switch( filter )
   {
      case FILTER_NONE:        Filter_Function   = &Nearest_Neighbor_Pixel;   break;
      case FILTER_BILINEAR_16: Filter_Function   = &Bilinear_Filter_16_Pixel; break;
      case FILTER_BILINEAR_32: Filter_Function   = &Bilinear_Filter_32_Pixel; break;
   }

   m_texture_filter = filter;
}

void Renderer::Toggle_Specularity( BOOL specularity )
{
   if ( !specularity )
   {
      Specular_Function = &Calculate_No_Specularity;
   }
   else
   {
      Specular_Function = &Calculate_Specularity;
   }

   m_specularity = specularity;
}

void Renderer::Render( Vertex * p_vertex, uint32 vertex_count, Face_Map * p_face, uint32 face_count, uint16 * buffer )
{
   _ASSERTE( vertex_count < MAX_VERTICES );

   // cull faces
   uint32 i;
   for ( i = 0; i < face_count; i++ )
   {
      Vector4 side_a, side_c;

      // subtract to get the edges
      side_a = Subtract_V4( &p_vertex[ p_face[ i ].vertex_list[ 0 ] ].m_position, &p_vertex[ p_face[ i ].vertex_list[ 1 ] ].m_position );
      side_c = Subtract_V4( &p_vertex[ p_face[ i ].vertex_list[ 0 ] ].m_position, &p_vertex[ p_face[ i ].vertex_list[ 2 ] ].m_position );

      // calc the face normal 
      Vector4 face_normal = Cross_Product_V4( &side_a, &side_c );
      Normalize_V4( &face_normal, &face_normal );

      // calc the camera look
      Vector4 camera_look;
      Normalize_V4( &m_p_camera->m_world_matrix.m_row[ 2 ], &camera_look );

      // see if they are away from each other. if so, don't render this face.
      if ( Dot_Product_V4( &camera_look, &face_normal ) > 0 )
      {
         p_face[ i ].culled = TRUE;
      }
      else
      {
         p_face[ i ].culled = FALSE;
      }
   }

   // translate mesh vertices and put them in the vertex cache
   //Light_Verts      ( p_vertex, vertex_count );
   Fill_Vertex_Cache( p_vertex, vertex_count );

   // clip the verts, filling the clipped vertex/face cache
   Clip_Mesh( m_vertex_cache, p_face, face_count );

   if ( !(m_vertex_constants.texture_flags & TEXTURE_SKY_CUBE) )
   { 
      Fill_Span_Buffer_PC( m_face_clip, m_face_clip_count, buffer );
   }
   else
   {
      Fill_Span_Buffer_Sky_Cube( m_face_clip, m_face_clip_count, buffer );
   }
}

/*void Renderer::Light_Verts( Vertex * p_vertex, uint32 vertex_count )
{
   // transform the camera to object space
   Matrix3x3 mesh_rotation = Extract_Rotation( m_vertex_constants.p_world_matrix );

   // transposing and inverting an orthonormal matrix are the same thing..
   mesh_rotation = Matrix_Transpose_M3( &mesh_rotation );

   // put the light's direction in object space
   Vector3 light_look = Vector_Matrix_Multiply_M3( (Vector3 *)&g_scene_light.m_world_vector, &mesh_rotation );

   // we want the reverse DP because for us, when two vetors point at each other that is full intensity.
   light_look *= -1;

   // translate the vertices
   uint32 i;
   for ( i = 0; i < vertex_count; i++ )
   {
      // clamp to 0
      m_vertex_cache[ i ].m_light_amount = max( 0, Dot_Product_V3( &light_look, &p_vertex[ i ].m_normal ) );
   }
}*/

void Renderer::Fill_Vertex_Cache( Vertex * p_vertex, uint32 vertex_count )
{
   // translate the vertices
   uint32 i;
   for ( i = 0; i < vertex_count; i++ )
   {
      m_vertex_cache[ i ].m_position = Multiply_M4( &p_vertex[ i ].m_position, &m_p_camera->m_view_projection_matrix );
      m_vertex_cache[ i ].m_uv       = p_vertex[ i ].m_uv;
      m_vertex_cache[ i ].m_normal   = p_vertex[ i ].m_normal;

      m_vertex_cache[ i ].m_position.w = fabsf( m_vertex_cache[ i ].m_position.w );

      float w_reciprocal = 1 / m_vertex_cache[ i ].m_position.w;

      // scale down by w (divide by w)
      m_vertex_cache[ i ].m_position = Scale_V4( &m_vertex_cache[ i ].m_position, w_reciprocal );
   }
}

void Renderer::Clip_Mesh( Cached_Vertex * p_vertex, Face_Map * p_face, uint32 face_count )
{
   Cached_Vertex clipped_vert_a[ 32 ];
   Cached_Vertex clipped_vert_b[ 32 ];

   uint32 clipped_vert_a_count;
   uint32 clipped_vert_b_count;
   uint32 clipped_face_count;

   m_face_clip_count = 0;

   uint32 i;
   for ( i = 0; i < face_count; i++ )
   {
      if ( FALSE == p_face[ i ].culled )
      {
         // copy the 3 verts of this triangle into the list.
         clipped_vert_a[ 0 ] = p_vertex[ p_face[ i ].vertex_list[ 0 ] ];
         clipped_vert_a[ 1 ] = p_vertex[ p_face[ i ].vertex_list[ 1 ] ];
         clipped_vert_a[ 2 ] = p_vertex[ p_face[ i ].vertex_list[ 2 ] ];

         // Clip X
         Clip_Left( clipped_vert_a, 3, clipped_vert_b, &clipped_vert_b_count );
         
         Clip_Right( clipped_vert_b, clipped_vert_b_count, clipped_vert_a, &clipped_vert_a_count );

         // Clip Y
         Clip_Top( clipped_vert_a, clipped_vert_a_count, clipped_vert_b, &clipped_vert_b_count );

         Clip_Bottom( clipped_vert_b, clipped_vert_b_count, clipped_vert_a, &clipped_vert_a_count );

         // Clip Z
         Clip_Far( clipped_vert_a, clipped_vert_a_count, clipped_vert_b, &clipped_vert_b_count );
         
         Clip_Near( clipped_vert_b, clipped_vert_b_count, clipped_vert_a, &clipped_vert_a_count );

         // create triangles
         Create_Triangle_Fan( clipped_vert_a, (sint32) clipped_vert_a_count, m_face_clip + m_face_clip_count, &clipped_face_count );

         m_face_clip_count += clipped_face_count;
      }
   }
}

void Renderer::Create_Triangle_Fan( Cached_Vertex * p_vertex, sint32 vertex_count, Face * p_face, uint32 * p_face_count )
{
   *p_face_count = 0;

   sint32 i;
   for ( i = 0; i < vertex_count - 2; i++ )
   {
      p_face[ i ].vertex[ 0 ] = p_vertex[ 0 ];
      p_face[ i ].vertex[ 1 ] = p_vertex[ i + 1 ];
      p_face[ i ].vertex[ 2 ] = p_vertex[ i + 2 ];

      (*p_face_count)++;
   }
}

void Renderer::Clip_Left( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count )
{
   // create a list of verts so we can wrap back to 0.
   Cached_Vertex * p_vertex_list[ 32 ];
   Create_Vertex_List( p_vertex_list, p_vertex_in, vertex_in_count );

   Cached_Vertex clipped_vert;
   *p_clipped_vert_count = 0;

   uint32 i;
   for ( i = 0; i < vertex_in_count; i++ )
   {
      Cached_Vertex * vert_a = p_vertex_list[ i ];
      Cached_Vertex * vert_b = p_vertex_list[ i + 1 ];

      // Go through each edge of the poly

      // Are both verts in screen? 
      if ( vert_a->m_position.x > -1 && vert_b->m_position.x > -1 )
      {
         //add only the first. The next one will be added next loop.
         p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
      }
      else if ( vert_a->m_position.x > -1 || vert_b->m_position.x > -1 )
      {
         // vert a is out of the screen
         if ( vert_a->m_position.x < -1 )
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( -1 - vert_a->m_position.x ) / ( vert_b->m_position.x - vert_a->m_position.x );

            // clip and add vert a. Verb will be added next loop.
            Clip_Edge( vert_a, vert_b, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
         else
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( -1 - vert_b->m_position.x ) / ( vert_a->m_position.x - vert_b->m_position.x );

            // Vert b is out..add vert a (since we won't see it again) and the clipped vert b
            Clip_Edge( vert_b, vert_a, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
      }
   }
}

void Renderer::Clip_Right( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count )
{
   // create a list of verts so we can wrap back to 0.
   Cached_Vertex * p_vertex_list[ 32 ];
   Create_Vertex_List( p_vertex_list, p_vertex_in, vertex_in_count );

   Cached_Vertex clipped_vert;
   *p_clipped_vert_count = 0;

   uint32 i;
   for ( i = 0; i < vertex_in_count; i++ )
   {
      Cached_Vertex * vert_a = p_vertex_list[ i ];
      Cached_Vertex * vert_b = p_vertex_list[ i + 1 ];

      // Go through each edge of the poly

      // Are both verts in screen? 
      if ( vert_a->m_position.x < 1 && vert_b->m_position.x < 1 )
      {
         //add only the first. The next one will be added next loop.
         p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
      }
      else if ( vert_a->m_position.x < 1 || vert_b->m_position.x < 1 )
      {
         // vert a is out of the screen
         if ( vert_a->m_position.x > 1 )
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( 1 - vert_a->m_position.x ) / ( vert_b->m_position.x - vert_a->m_position.x );

            // clip and add vert a. Verb will be added next loop.
            Clip_Edge( vert_a, vert_b, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
         else
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( 1 - vert_b->m_position.x ) / ( vert_a->m_position.x - vert_b->m_position.x );

            // Vert b is out..add vert a (since we won't see it again) and the clipped vert b
            Clip_Edge( vert_b, vert_a, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
      }
   }
}

void Renderer::Clip_Top( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count )
{
   // create a list of verts so we can wrap back to 0.
   Cached_Vertex * p_vertex_list[ 32 ];
   Create_Vertex_List( p_vertex_list, p_vertex_in, vertex_in_count );

   Cached_Vertex clipped_vert;
   *p_clipped_vert_count = 0;

   uint32 i;
   for ( i = 0; i < vertex_in_count; i++ )
   {
      Cached_Vertex * vert_a = p_vertex_list[ i ];
      Cached_Vertex * vert_b = p_vertex_list[ i + 1 ];

      // Go through each edge of the poly

      // Are both verts in screen? 
      if ( vert_a->m_position.y > -1 && vert_b->m_position.y > -1 )
      {
         //add only the first. The next one will be added next loop.
         p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
      }
      else if ( vert_a->m_position.y > -1 || vert_b->m_position.y > -1 )
      {
         // vert a is out of the screen
         if ( vert_a->m_position.y < -1 )
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( -1 - vert_a->m_position.y ) / ( vert_b->m_position.y - vert_a->m_position.y );

            // clip and add vert a. Verb will be added next loop.
            Clip_Edge( vert_a, vert_b, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
         else
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( -1 - vert_b->m_position.y ) / ( vert_a->m_position.y - vert_b->m_position.y );

            // Vert b is out..add vert a (since we won't see it again) and the clipped vert b
            Clip_Edge( vert_b, vert_a, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
      }
   }
}

void Renderer::Clip_Bottom( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count )
{
   // create a list of verts so we can wrap back to 0.
   Cached_Vertex * p_vertex_list[ 32 ];
   Create_Vertex_List( p_vertex_list, p_vertex_in, vertex_in_count );

   Cached_Vertex clipped_vert;
   *p_clipped_vert_count = 0;

   uint32 i;
   for ( i = 0; i < vertex_in_count; i++ )
   {
      Cached_Vertex * vert_a = p_vertex_list[ i ];
      Cached_Vertex * vert_b = p_vertex_list[ i + 1 ];

      // Go through each edge of the poly

      // Are both verts in screen? 
      if ( vert_a->m_position.y < 1 && vert_b->m_position.y < 1 )
      {
         //add only the first. The next one will be added next loop.
         p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
      }
      else if ( vert_a->m_position.y < 1 || vert_b->m_position.y < 1 )
      {
         // vert a is out of the screen
         if ( vert_a->m_position.y > 1 )
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( 1 - vert_a->m_position.y ) / ( vert_b->m_position.y - vert_a->m_position.y );

            // clip and add vert a. Verb will be added next loop.
            Clip_Edge( vert_a, vert_b, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
         else
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( 1 - vert_b->m_position.y ) / ( vert_a->m_position.y - vert_b->m_position.y );

            // Vert b is out..add vert a (since we won't see it again) and the clipped vert b
            Clip_Edge( vert_b, vert_a, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
      }
   }
}

void Renderer::Clip_Near( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count )
{
   // create a list of verts so we can wrap back to 0.
   Cached_Vertex * p_vertex_list[ 32 ];
   Create_Vertex_List( p_vertex_list, p_vertex_in, vertex_in_count );

   Cached_Vertex clipped_vert;
   *p_clipped_vert_count = 0;

   uint32 i;
   for ( i = 0; i < vertex_in_count; i++ )
   {
      Cached_Vertex * vert_a = p_vertex_list[ i ];
      Cached_Vertex * vert_b = p_vertex_list[ i + 1 ];

      // Go through each edge of the poly

      // Are both verts in screen? 
      if ( vert_a->m_position.z > 0 && vert_b->m_position.z > 0 )
      {
         //add only the first. The next one will be added next loop.
         p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
      }
      else if ( vert_a->m_position.z > 0 || vert_b->m_position.z > 0 )
      {
         // vert a is out of the screen
         if ( vert_a->m_position.z < 0 )
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( -vert_a->m_position.z ) / ( vert_b->m_position.z - vert_a->m_position.z );

            // clip and add vert a. Verb will be added next loop.
            Clip_Edge( vert_a, vert_b, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
         else
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( -vert_b->m_position.z ) / ( vert_a->m_position.z - vert_b->m_position.z );

            // Vert b is out..add vert a (since we won't see it again) and the clipped vert b
            Clip_Edge( vert_b, vert_a, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
      }
   }
}

void Renderer::Clip_Far( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count )
{
   // create a list of verts so we can wrap back to 0.
   Cached_Vertex * p_vertex_list[ 32 ];
   Create_Vertex_List( p_vertex_list, p_vertex_in, vertex_in_count );

   Cached_Vertex clipped_vert;
   *p_clipped_vert_count = 0;

   uint32 i;
   for ( i = 0; i < vertex_in_count; i++ )
   {
      Cached_Vertex * vert_a = p_vertex_list[ i ];
      Cached_Vertex * vert_b = p_vertex_list[ i + 1 ];

      // Go through each edge of the poly

      // Are both verts in screen? 
      if ( vert_a->m_position.z < 1 && vert_b->m_position.z < 1 )
      {
         //add only the first. The next one will be added next loop.
         p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
      }
      else if ( vert_a->m_position.z < 1 || vert_b->m_position.z < 1 )
      {
         // vert a is out of the screen
         if ( vert_a->m_position.z > 1 )
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( 1 - vert_a->m_position.z ) / ( vert_b->m_position.z - vert_a->m_position.z );

            // clip and add vert a. Verb will be added next loop.
            Clip_Edge( vert_a, vert_b, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
         else
         {
            // time is the clip pos in vector space / delta AXIS
            float time = ( 1 - vert_b->m_position.z ) / ( vert_a->m_position.z - vert_b->m_position.z );

            // Vert b is out..add vert a (since we won't see it again) and the clipped vert b
            Clip_Edge( vert_b, vert_a, &clipped_vert, time );

            p_clipped_vert[ (*p_clipped_vert_count)++ ] = *vert_a;
            p_clipped_vert[ (*p_clipped_vert_count)++ ] = clipped_vert;
         }
      }
   }
}

void Renderer::Clip_Edge( Cached_Vertex * p_vertex_a, Cached_Vertex * p_vertex_b, Cached_Vertex * p_clipped, float time )
{
   // scale the origin to the new place
   Vector4 delta_position = Subtract_W_V4( &p_vertex_b->m_position, &p_vertex_a->m_position );
   Vector4 scaled_delta   = Scale_W_V4( &delta_position, time );
   p_clipped->m_position  = Add_W_V4( &p_vertex_a->m_position, &scaled_delta );
   
   // scale the normal origin to the new place
   Vector3 delta_normal   = Subtract_V3( &p_vertex_b->m_normal, &p_vertex_a->m_normal );
   Vector3 scaled_normal  = Scale_V3( &delta_normal, time );
   p_clipped->m_normal    = Add_V3( &p_vertex_a->m_normal, &scaled_normal );


   float recip_a_w = 1 / p_vertex_a->m_position.w;
   float recip_b_w = 1 / p_vertex_b->m_position.w;

   //uvs
   float u_start = p_vertex_a->m_uv.u * recip_a_w;
   float v_start = p_vertex_a->m_uv.v * recip_a_w;

   float u_end   = p_vertex_b->m_uv.u * recip_b_w;
   float v_end   = p_vertex_b->m_uv.v * recip_b_w;

   float delta_u = u_end - u_start;
   float delta_v = v_end - v_start;

   //r
   float r_start = 1.0f * recip_a_w;
   float r_end   = 1.0f * recip_b_w;
   float delta_r = r_end - r_start;
   float recip_r = 1 / ( r_start + ( delta_r * time ) );

   p_clipped->m_uv.u = ( u_start + ( delta_u * time ) ) * recip_r;
   p_clipped->m_uv.v = ( v_start + ( delta_v * time ) ) * recip_r;

   p_clipped->m_position.w = recip_r; //store a perspective correct interpolated w

   // and light
   //float delta_light = p_vertex_b->m_light_amount - p_vertex_a->m_light_amount;
   
   //p_clipped->m_light_amount = p_vertex_a->m_light_amount + ( delta_light * time );
}

void Renderer::Fill_Span_Buffer_Sky_Cube( Face * p_face, uint32 face_count, uint16 * buffer )
{
   // Convert to screen space
   Screen_Position screen_pos[ 3 ];

   uint32 i;
   for ( i = 0; i < face_count; i++ )
   {
      // convert cached vertices to screen space
      Screen_Position * p_screen_pos[ 3 ];
      To_Screen_Space( screen_pos, &p_face[ i ] );

      // create and sort pointers to speed up swapping.
      p_screen_pos[ 0 ] = &screen_pos[ 0 ];
      p_screen_pos[ 1 ] = &screen_pos[ 1 ];
      p_screen_pos[ 2 ] = &screen_pos[ 2 ];

      Sort_Positions_By_Y( p_screen_pos );

      // determine which edge to draw (who knows where it came from)
	   float edge_result = ( ( (float)p_screen_pos[ 1 ]->x - (float)p_screen_pos[ 0 ]->x ) * ( (float)p_screen_pos[ 2 ]->y - (float)p_screen_pos[ 0 ]->y ) ) - 
                          ( ( (float)p_screen_pos[ 1 ]->y - (float)p_screen_pos[ 0 ]->y ) * ( (float)p_screen_pos[ 2 ]->x - (float)p_screen_pos[ 0 ]->x ) );

	   uint32 edge = edge_result > 0 ? 1 : 0;

      // now trace each side of the triangle
      Trace_Edge_Sky_Cube( p_screen_pos[ 0 ], p_screen_pos[ 1 ], (SPAN_EDGE) edge, FALSE ); //( Va to Vb )
      Trace_Edge_Sky_Cube( p_screen_pos[ 1 ], p_screen_pos[ 2 ], (SPAN_EDGE) edge, FALSE ); //( Vb to Vc )
      Trace_Edge_Sky_Cube( p_screen_pos[ 0 ], p_screen_pos[ 2 ], (SPAN_EDGE) !edge, TRUE ); //( Vc to Va ) (For this, reverse the edge of course)

      
      Render_Span_Buffer_Sky_Cube( buffer, p_screen_pos[ 0 ]->y, p_screen_pos[ 2 ]->y );
   }
}

void Renderer::Fill_Span_Buffer_PC( Face * p_face, uint32 face_count, uint16 * buffer )
{ 
   // Convert to screen space
   Screen_Position screen_pos[ 3 ];

   uint32 i;
   for ( i = 0; i < face_count; i++ )
   {
      // convert cached vertices to screen space
      Screen_Position * p_screen_pos[ 3 ];
      To_Screen_Space( screen_pos, &p_face[ i ] );

      // create and sort pointers to speed up swapping.
      p_screen_pos[ 0 ] = &screen_pos[ 0 ];
      p_screen_pos[ 1 ] = &screen_pos[ 1 ];
      p_screen_pos[ 2 ] = &screen_pos[ 2 ];

      Sort_Positions_By_Y( p_screen_pos );

      // determine which edge to draw (who knows where it came from)
	   float edge_result = ( ( (float)p_screen_pos[ 1 ]->x - (float)p_screen_pos[ 0 ]->x ) * ( (float)p_screen_pos[ 2 ]->y - (float)p_screen_pos[ 0 ]->y ) ) - 
                          ( ( (float)p_screen_pos[ 1 ]->y - (float)p_screen_pos[ 0 ]->y ) * ( (float)p_screen_pos[ 2 ]->x - (float)p_screen_pos[ 0 ]->x ) );

	   uint32 edge = edge_result > 0 ? 1 : 0;

      // now trace each side of the triangle
      Trace_Edge_PC( p_screen_pos[ 0 ], p_screen_pos[ 1 ], (SPAN_EDGE) edge, FALSE ); //( Va to Vb )
      Trace_Edge_PC( p_screen_pos[ 1 ], p_screen_pos[ 2 ], (SPAN_EDGE) edge, FALSE ); //( Vb to Vc )
      Trace_Edge_PC( p_screen_pos[ 0 ], p_screen_pos[ 2 ], (SPAN_EDGE) !edge, TRUE ); //( Vc to Va ) (For this, reverse the edge of course)

      
      Render_Span_Buffer_PC( buffer, p_screen_pos[ 0 ]->y, p_screen_pos[ 2 ]->y );
   }
}

void Renderer::To_Screen_Space( Screen_Position * p_screen_pos, Face * p_face )
{
   uint32 half_width  = g_options.resolution.width  / 2;
   uint32 half_height = g_options.resolution.height / 2;

   // copy triangle into screen pos
   //Pos 1
   p_screen_pos[ 0 ].x            = ftoi( ( ( p_face->vertex[ 0 ].m_position.x * half_width  ) + half_width ) );
   p_screen_pos[ 0 ].y            = ftoi( ( ( p_face->vertex[ 0 ].m_position.y * half_height ) + half_height ) );
   p_screen_pos[ 0 ].z            = p_face->vertex[ 0 ].m_position.z;
   p_screen_pos[ 0 ].world_z      = p_face->vertex[ 0 ].m_position.w; //W is the world_z.
//   p_screen_pos[ 0 ].light_amount = p_face->vertex[ 0 ].m_light_amount;
   p_screen_pos[ 0 ].uv           = p_face->vertex[ 0 ].m_uv;
   p_screen_pos[ 0 ].normal       = p_face->vertex[ 0 ].m_normal;
   p_screen_pos[ 0 ].position.Set( p_face->vertex[ 0 ].m_position.x, p_face->vertex[ 0 ].m_position.y, p_face->vertex[ 0 ].m_position.z );

   // clamp screen coords
   _ASSERTE( p_screen_pos[ 0 ].x >= 0 && p_screen_pos[ 0 ].x <= g_options.resolution.width );
   _ASSERTE( p_screen_pos[ 0 ].y >= 0 && p_screen_pos[ 0 ].y <= g_options.resolution.height );


   //Pos 2
   p_screen_pos[ 1 ].x            = ftoi( ( ( p_face->vertex[ 1 ].m_position.x * half_width  ) + half_width ) );
   p_screen_pos[ 1 ].y            = ftoi( ( ( p_face->vertex[ 1 ].m_position.y * half_height ) + half_height ) );
   p_screen_pos[ 1 ].z            = p_face->vertex[ 1 ].m_position.z;
   p_screen_pos[ 1 ].world_z      = p_face->vertex[ 1 ].m_position.w; //W is the world_z.
 //  p_screen_pos[ 1 ].light_amount = p_face->vertex[ 1 ].m_light_amount;
   p_screen_pos[ 1 ].uv           = p_face->vertex[ 1 ].m_uv;
   p_screen_pos[ 1 ].normal       = p_face->vertex[ 1 ].m_normal;
   p_screen_pos[ 1 ].position.Set( p_face->vertex[ 1 ].m_position.x, p_face->vertex[ 1 ].m_position.y, p_face->vertex[ 1 ].m_position.z );

   // clamp screen coords TODO: In certain cases the vertex position can be garbage, causing this to assert. Generally a plane on it's back causes it.
   _ASSERTE( p_screen_pos[ 1 ].x >= 0 && p_screen_pos[ 1 ].x <= g_options.resolution.width );
   _ASSERTE( p_screen_pos[ 1 ].y >= 0 && p_screen_pos[ 1 ].y <= g_options.resolution.height );


   //Pos 3
   p_screen_pos[ 2 ].x            = ftoi( ( ( p_face->vertex[ 2 ].m_position.x * half_width  ) + half_width ) );
   p_screen_pos[ 2 ].y            = ftoi( ( ( p_face->vertex[ 2 ].m_position.y * half_height ) + half_height ) );
   p_screen_pos[ 2 ].z            = p_face->vertex[ 2 ].m_position.z;
   p_screen_pos[ 2 ].world_z      = p_face->vertex[ 2 ].m_position.w; //W is the world_z.
 //  p_screen_pos[ 2 ].light_amount = p_face->vertex[ 2 ].m_light_amount;
   p_screen_pos[ 2 ].uv           = p_face->vertex[ 2 ].m_uv;
   p_screen_pos[ 2 ].normal       = p_face->vertex[ 2 ].m_normal;
   p_screen_pos[ 2 ].position.Set( p_face->vertex[ 2 ].m_position.x, p_face->vertex[ 2 ].m_position.y, p_face->vertex[ 2 ].m_position.z );

   // clamp screen coords
   _ASSERTE( p_screen_pos[ 2 ].x >= 0 && p_screen_pos[ 2 ].x <= g_options.resolution.width );
   _ASSERTE( p_screen_pos[ 2 ].y >= 0 && p_screen_pos[ 2 ].y <= g_options.resolution.height );
   
}

void Renderer::Sort_Positions_By_Y( Screen_Position ** p_screen_pos )
{
   //sort the three point pointers.
   
   // First, don't bother checking to see if 0 is the least. If it is the below check will fail

   // If 1 is least, put it in 0's spot
   if ( p_screen_pos[ 1 ]->y <= p_screen_pos[ 0 ]->y && p_screen_pos[ 1 ]->y <= p_screen_pos[ 2 ]->y )
   {
      // swap 1 and 0
      Screen_Position * temp_pos = p_screen_pos[ 0 ];
      p_screen_pos[ 0 ]          = p_screen_pos[ 1 ];
      p_screen_pos[ 1 ]          = temp_pos;
   }
   //If 2 is least, put it in 0's spot
   else if ( p_screen_pos[ 2 ]->y <= p_screen_pos[ 0 ]->y && p_screen_pos[ 2 ]->y <= p_screen_pos[ 1 ]->y )
   {
      // swap 2 and 0
      Screen_Position * temp_pos = p_screen_pos[ 0 ];
      p_screen_pos[ 0 ]          = p_screen_pos[ 2 ];
      p_screen_pos[ 2 ]          = temp_pos;
   }

   // Now we are guarunteed that 0 and 1 are sorted. Now sort 1 and 2.
   if ( p_screen_pos[ 1 ]->y > p_screen_pos[ 2 ]->y )
   {
      Screen_Position * temp_pos = p_screen_pos[ 2 ];
      p_screen_pos[ 2 ]          = p_screen_pos[ 1 ];
      p_screen_pos[ 1 ]          = temp_pos;
   }
}

void Renderer::Trace_Edge_Sky_Cube( Screen_Position * p_start, Screen_Position * p_end, SPAN_EDGE edge, BOOL last_edge )
{
   float y_range = 1 / ( (float) p_end->y - (float) p_start->y );

    //slope of x/z in relation to y
   float m        = ( (float) p_end->x - (float) p_start->x ) * y_range;
   float z_slope  = ( (float) p_end->z - (float) p_start->z ) * y_range;

   float x        = (float) p_start->x;
   float z        = (float) p_start->z;
   
   // uv interpolation - use perspective correct texturing.
   float recip_start_w = 1 / p_start->world_z;
   float recip_end_w   = 1 / p_end->world_z;

   float u_start = p_start->uv.u * recip_start_w;
   float v_start = p_start->uv.v * recip_start_w;

   UV    uv      = { u_start, v_start };

   float u_end = p_end->uv.u * recip_end_w;
   float v_end = p_end->uv.v * recip_end_w;

   float u_step  = ( u_end - u_start ) * y_range;
   float v_step  = ( v_end - v_start ) * y_range;

   float r_start = 1.0f * recip_start_w;
   float r_end   = 1.0f * recip_end_w;
   float r_step  = ( r_end - r_start ) * y_range;
   float r       = r_start;

   float world_z_step = ( p_end->world_z - p_start->world_z ) * y_range;

   // don't limit the perspective texturing here, as it causes too much loss
   uint32 y;
   for ( y = p_start->y; y < p_end->y; y++ )
   {
      float world_z = 1 / r;

      UV tang_uv = { uv.u * world_z, uv.v * world_z };

      m_span_buffer.span_row[ y ].x[ edge ]            = ftoi( x );
      m_span_buffer.span_row[ y ].z[ edge ]            = z;
      m_span_buffer.span_row[ y ].uv[ edge ]           = tang_uv;
      m_span_buffer.span_row[ y ].world_z[ edge ]      = world_z; //here store world z

      x += m;
      z += z_slope;

      // handle persp texture UVs
      uv.u += u_step;
      uv.v += v_step;

      world_z += world_z_step;

      r += r_step; //here linearlly interpolate r
   }
}

void Renderer::Render_Span_Buffer_Sky_Cube( uint16 * buffer, uint32 start_y, uint32 end_y )
{
   uint16 * p_buffer = buffer + ( start_y * g_options.resolution.width );
   uint32 pixel_row  = ( start_y * g_options.resolution.width );

   uint32 y;
   for ( y = start_y; y < end_y; y++ )
   {
      // get the x range TODO: Sometimes this is 0, causing #-1INF000 and a release build crash
      float x_range    = 1 / (float) ( m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ] - m_span_buffer.span_row[ y ].x[ LEFT_EDGE ] );

      // get the steps for interpolating z, and UV
      float z_step     = ( m_span_buffer.span_row[ y ].z[ RIGHT_EDGE ] - m_span_buffer.span_row[ y ].z[ LEFT_EDGE ] ) * x_range;
      float z          = m_span_buffer.span_row[ y ].z[ LEFT_EDGE ];
      
      uint32 x_step    = g_options.wire_frame ? m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ] : 1;

      // perspective correct UV
      float recip_left_z  = 1 / m_span_buffer.span_row[ y ].world_z[ LEFT_EDGE ];
      float recip_right_z = 1 / m_span_buffer.span_row[ y ].world_z[ RIGHT_EDGE ];

      float u_start = m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].u * recip_left_z;
      float v_start = m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].v * recip_left_z;

      float u_end = m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].u * recip_right_z;
      float v_end = m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].v * recip_right_z;

      float u_step = ( u_end - u_start ) * x_range;
      float v_step = ( v_end - v_start ) * x_range;

      UV uv = { u_start, v_start };

      // tangent uvs
      float tang_u_step = ( m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].u - m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].u ) * x_range;
      float tang_v_step = ( m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].v - m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].v ) * x_range;

      // R
      float r_start = recip_left_z;
      float r_end   = recip_right_z;

      float r_step  = ( r_end - r_start ) * x_range;
      float r       = r_start;

      // interpolate colors
      uint32 x;
      for ( x = m_span_buffer.span_row[ y ].x[ LEFT_EDGE ]; x < m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ]; x += x_step )
      {  
         uint32 next_x = x + PERSPECTIVE_CORRECTION_RATE;
         uint32 stop_x = next_x < m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ] ? next_x : m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ];

         float world_z = 1 / r;

         // put uvs back in tangent space
         UV tang_uv = { uv.u * world_z, uv.v * world_z };

         // begin linear interpolation for up to PERSPECTIVE_CORRECTION_RATE amount of pixels.
         for ( ; x < stop_x; x++ )
         {
            if ( z < m_z_buffer[ pixel_row + x ] )
            {
               // test the z buffer, and then draw. The compiler will optimize this to use no branching
               uint32 row        = ( (int) ( m_vertex_constants.p_texture->height * tang_uv.v )/* & ( m_vertex_constants.p_texture->height - 1 )*/ );
               uint32 row_offset = ( (int) ( m_vertex_constants.p_texture->width  * tang_uv.u )/* & ( m_vertex_constants.p_texture->width - 1 )*/ );

               row = min( row, m_vertex_constants.p_texture->height - 1 );

               short pixel = m_vertex_constants.p_texture->p_data[ ( m_vertex_constants.p_texture->width * row ) + row_offset ];

               p_buffer[ x ] = pixel;

               m_z_buffer[ pixel_row + x ] = z;
            }

            //TrapperM's idea, here we can reset the z buffer cuz the skycube is the last to render.
            //m_z_buffer[ pixel_row + x ] = 1.00f;

            // interpolate all our values.
            uv.u += u_step;
            uv.v += v_step;

            tang_uv.u += tang_u_step;
            tang_uv.v += tang_v_step;

            r += r_step;
            z += z_step;
         }
         
         // pull x back for processing.
         x--;
      }

      pixel_row += g_options.resolution.width;
      p_buffer  += g_options.resolution.width;
   }
}

//void Renderer::Trace_Edge_PC( Screen_Position * p_start, Screen_Position * p_end, SPAN_EDGE edge, BOOL last_edge )
//{
//   float y_range = 1 / ( (float) p_end->y - (float) p_start->y );
//
//    //slope of x/z in relation to y
//   float m        = ( (float) p_end->x - (float) p_start->x ) * y_range;
//   float z_slope  = ( (float) p_end->z - (float) p_start->z ) * y_range;
//
//   float x        = (float) p_start->x;
//   float z        = (float) p_start->z;
//   //float light    = p_start->light_amount;
//   
//   // uv interpolation - use perspective correct texturing.
//   float recip_start_w = 1 / p_start->world_z;
//   float recip_end_w   = 1 / p_end->world_z;
//
//   float u_start = p_start->uv.u * recip_start_w;
//   float v_start = p_start->uv.v * recip_start_w;
//
//   UV    uv      = { u_start, v_start };
//
//   float u_end = p_end->uv.u * recip_end_w;
//   float v_end = p_end->uv.v * recip_end_w;
//
//   float u_step  = ( u_end - u_start ) * y_range;
//   float v_step  = ( v_end - v_start ) * y_range;
//
//   float r_start = 1.0f * recip_start_w;
//   float r_end   = 1.0f * recip_end_w;
//   float r_step  = ( r_end - r_start ) * y_range;
//   float r       = r_start;
//
//   float world_z_step = ( p_end->world_z - p_start->world_z ) * y_range;
//
//   // light interpolation
//   //float light_step = ( p_end->light_amount - p_start->light_amount ) * y_range;
//
//   // normal interpolation for phong shading
//   Vector3 normal;
//   normal.Set( p_start->normal.x, p_start->normal.y, p_start->normal.z );
//
//   float normal_x_step = ( p_end->normal.x - p_start->normal.x ) * y_range;
//   float normal_y_step = ( p_end->normal.y - p_start->normal.y ) * y_range;
//   float normal_z_step = ( p_end->normal.z - p_start->normal.z ) * y_range;
//
//
//   // position interpolation for specularity
//   Vector3 position;
//   position.Set( p_start->position.x, p_start->position.y, p_start->position.z );
//
//   float pos_x_step = ( p_end->position.x - p_start->position.x ) * y_range;
//   float pos_y_step = ( p_end->position.y - p_start->position.y ) * y_range;
//   float pos_z_step = ( p_end->position.z - p_start->position.z ) * y_range;
//
//   // don't limit the perspective texturing here, as it causes too much loss
//   uint32 y;
//   for ( y = p_start->y; y < p_end->y; y++ )
//   {
//      float world_z = 1 / r;
//
//      UV tang_uv = { uv.u * world_z, uv.v * world_z };
//
//      m_span_buffer.span_row[ y ].x[ edge ]            = ftoi( x );
//      m_span_buffer.span_row[ y ].z[ edge ]            = z;
//      m_span_buffer.span_row[ y ].uv[ edge ]           = tang_uv;
//      //m_span_buffer.span_row[ y ].light_amount[ edge ] = light;
//      m_span_buffer.span_row[ y ].world_z[ edge ]      = world_z; //here store world z
//
//      m_span_buffer.span_row[ y ].normal[ edge ]       = normal;
//
//      m_span_buffer.span_row[ y ].position[ edge ]     = position;
//
//      normal.x += normal_x_step;
//      normal.y += normal_y_step;
//      normal.z += normal_z_step;
//
//      position.x += pos_x_step;
//      position.y += pos_y_step;
//      position.z += pos_z_step;
//
//      //light += light_step;
//
//      x += m;
//      z += z_slope;
//
//      // handle persp texture UVs
//      uv.u += u_step;
//      uv.v += v_step;
//
//      world_z += world_z_step;
//
//      r += r_step; //here linearlly interpolate r
//   }
//}

void Renderer::Trace_Edge_PC( Screen_Position * p_start, Screen_Position * p_end, SPAN_EDGE edge, BOOL last_edge )
{
   // BRESENHAM
   sint32 x = p_start->x;
   sint32 y = p_start->y;

   sint32 delta_x = abs( (sint32) ( p_end->x - p_start->x ) );
   sint32 delta_y = abs( (sint32) ( p_end->y - p_start->y ) );

   sint32 x_slope_inc;   //used if x is calculated via the slope
   sint32 x_iterate_inc; //used if we're iterating across x

   sint32 y_slope_inc;   //used if y is calculated via the slope
   sint32 y_iterate_inc; //used if we're iterating across y

   sint32 numerator;
   sint32 denominator;
   sint32 numerator_add;
   sint32 pixel_count;

   // see which direction x goes
   if ( p_end->x >= p_start->x )
   {
      x_slope_inc    = 1;  
      x_iterate_inc  = 1;
   }
   else
   {
      x_slope_inc    = -1;
      x_iterate_inc  = -1;
   }

   // see which direction y goes
   if ( p_end->y >= p_start->y )
   {
      y_slope_inc    = 1;  
      y_iterate_inc  = 1;
   }
   else
   {
      y_slope_inc    = -1;
      y_iterate_inc  = -1;
   }

   // now determine how our loop will proceed
   if ( delta_x >= delta_y ) //we want to iterate x
   {
      x_slope_inc   = 0;   // we are iterating x, so we don't want this
      y_iterate_inc = 0;   // we are NOT iterating y, so we don't want this either

      pixel_count   = delta_x;     // we want to iterate across x, so delta_x

      numerator     = delta_x / 2;  // our fraction is ( delta_x / 2 ) / delta_x
      denominator   = delta_x;

      numerator_add = delta_y; // we add to the numerator delta_y
   }
   else
   {
      x_iterate_inc = 0;  // we are NOT iterating x, so don't want this
      y_slope_inc   = 0;  // we are iterating y, so we don't want this.

      pixel_count   = delta_y;   // want to iterate across y, so delta_y

      numerator     = delta_y / 2; // our fraction is ( delta_y / 2 ) / delta_y
      denominator   = delta_y;

      numerator_add = delta_x; // we add to the numerator delta_x
   }

   // calculate all the steps
   float y_range = 1 / (float) denominator;

    //slope of z in relation to y
   float z_slope  = ( (float) p_end->z - (float) p_start->z ) * y_range;

   float z        = (float) p_start->z;
   
   // uv interpolation - use perspective correct texturing.
   float recip_start_w = 1 / p_start->world_z;
   float recip_end_w   = 1 / p_end->world_z;

   float u_start = p_start->uv.u * recip_start_w;
   float v_start = p_start->uv.v * recip_start_w;

   UV    uv      = { u_start, v_start };

   float u_end = p_end->uv.u * recip_end_w;
   float v_end = p_end->uv.v * recip_end_w;

   float u_step  = ( u_end - u_start ) * y_range;
   float v_step  = ( v_end - v_start ) * y_range;

   float r_start = 1.0f * recip_start_w;
   float r_end   = 1.0f * recip_end_w;
   float r_step  = ( r_end - r_start ) * y_range;
   float r       = r_start;

   float world_z_step = ( p_end->world_z - p_start->world_z ) * y_range;

   // normal interpolation for phong shading
   Vector3 normal, normal_step;
   normal.Set( p_start->normal.x, p_start->normal.y, p_start->normal.z );
   normal_step = Scale_V3( &Subtract_V3( &p_end->normal, &p_start->normal ), y_range );

   // position interpolation for specularity
   Vector3 position, position_step;
   position.Set( p_start->position.x, p_start->position.y, p_start->position.z );
   position_step = Scale_V3( &Subtract_V3( &p_end->position, &p_start->position ), y_range );

   
   // trace the edge
   sint32 i;
   for ( i = 0; i <= pixel_count; i++ )
   {
      float world_z = 1 / r;

      UV tang_uv = { uv.u * world_z, uv.v * world_z };

      m_span_buffer.span_row[ y ].x[ edge ]            = x;
      m_span_buffer.span_row[ y ].z[ edge ]            = z;
      m_span_buffer.span_row[ y ].uv[ edge ]           = tang_uv;
      m_span_buffer.span_row[ y ].world_z[ edge ]      = world_z; //here store world z

      m_span_buffer.span_row[ y ].normal[ edge ]       = normal;

      m_span_buffer.span_row[ y ].position[ edge ]     = position;

      // interpolate the normal and position
      normal   = Add_V3( &normal  , &normal_step );
      position = Add_V3( &position, &position_step );

      z += z_slope;

      // handle persp texture UVs
      uv.u += u_step;
      uv.v += v_step;

      world_z += world_z_step;

      r += r_step; //here linearlly interpolate r


      // add the numerator
      numerator += numerator_add;

      // if it has gained a whole number increase x/y by the slope and remove the whole number from the fraction
      if ( numerator >= denominator )
      {
         numerator -= denominator;

         x += x_slope_inc;
         y += y_slope_inc;
      }

      x += x_iterate_inc;
      y += y_iterate_inc;
   }
   // BRESENHAM
}

uint32 Color_16_To_32( short color )
{
   uint16 pixel_red     = m_red_down_lookup[ ( color & 0x7C00 ) ]; 
   uint16 pixel_green   = m_green_down_lookup[ ( color & 0x03E0 ) ];
   uint16 pixel_blue  = ( color & 0x001F );

   uint32 red   = (uint32) ( ( (float) pixel_red   / 31 ) * 255 );
   uint32 green = (uint32) ( ( (float) pixel_green / 31 ) * 255 );
   uint32 blue  = (uint32) ( ( (float) pixel_blue  / 31 ) * 255 );

   return red << 16 | green << 8 | blue;
}

short Color_32_To_16( uint32 color )
{
   uint32 pixel_red   = ( color & 0x00FF0000 ) >> 16;
   uint32 pixel_green = ( color & 0x0000FF00 ) >> 8;
   uint32 pixel_blue  = ( color & 0x000000FF );

   uint16 red   = (uint16) ( ( (float) pixel_red   / 255 ) * 31 );
   uint16 green = (uint16) ( ( (float) pixel_green / 255 ) * 31 );
   uint16 blue  = (uint16) ( ( (float) pixel_blue  / 255 ) * 31 );

   return red << 10 | green << 5 | blue;
}

//void Renderer::Render_Span_Buffer_PC( uint16 * buffer, uint32 start_y, uint32 end_y )
//{
//   uint16 * p_buffer = buffer + ( ( g_options.resolution.height / 2 ) * g_options.resolution.width );
//
//   POINT point_a = { 10, 10 };
//   POINT point_b = { 1000, 15 };
//
//   sint32 x = point_a.x;
//   sint32 y = point_a.y;
//
//   sint32 delta_x = abs( point_b.x - point_a.x );
//   sint32 delta_y = abs( point_b.y - point_a.y );
//
//   sint32 x_slope_inc;   //used if x is calculated via the slope
//   sint32 x_iterate_inc; //used if we're iterating across x
//
//   sint32 y_slope_inc;   //used if y is calculated via the slope
//   sint32 y_iterate_inc; //used if we're iterating across y
//
//
//   // see which direction x goes
//   if ( point_b.x >= point_a.x )
//   {
//      x_slope_inc    = 1;  
//      x_iterate_inc  = 1;
//   }
//   else
//   {
//      x_slope_inc    = -1;
//      x_iterate_inc  = -1;
//   }
//
//   // see which direction y goes
//   if ( point_b.y >= point_a.y )
//   {
//      y_slope_inc    = 1;  
//      y_iterate_inc  = 1;
//   }
//   else
//   {
//      y_slope_inc    = -1;
//      y_iterate_inc  = -1;
//   }
//
//   sint32 numerator;
//   sint32 denominator;
//   sint32 numerator_add;
//   sint32 pixel_count;
//
//   // now determine how our loop will proceed
//   if ( delta_x >= delta_y ) //we want to iterate x
//   {
//      x_slope_inc   = 0;   // we are iterating x, so we don't want this
//      y_iterate_inc = 0;   // we are NOT iterating y, so we don't want this either
//
//      pixel_count   = delta_x;     // we want to iterate across x, so delta_x
//
//      numerator     = delta_x / 2;  // our fraction is ( delta_x / 2 ) / delta_x
//      denominator   = delta_x;
//
//      numerator_add = delta_y; // we add to the numerator delta_y
//   }
//   else
//   {
//      x_iterate_inc = 0;  // we are NOT iterating x, so don't want this
//      y_slope_inc   = 0;  // we are iterating y, so we don't want this.
//
//      pixel_count   = delta_y;   // want to iterate across y, so delta_y
//
//      numerator     = delta_y / 2; // our fraction is ( delta_y / 2 ) / delta_y
//      denominator   = delta_y;
//
//      numerator_add = delta_x; // we add to the numerator delta_x
//   }
//
//   sint32 i;
//   for ( i = 0; i < pixel_count; i++ )
//   {
//      // calculate the correct row and draw the pixel
//      uint32 row = y * g_options.resolution.width;
//
//      p_buffer[ row + x ] = 0xffff;
//
//      // add the numerator
//      numerator += numerator_add;
//
//      // if it has gained a whole number increase x/y by the slope and remove the whole number from the fraction
//      if ( numerator >= denominator )
//      {
//         numerator -= denominator;
//
//         x += x_slope_inc;
//         y += y_slope_inc;
//      }
//
//      x += x_iterate_inc;
//      y += y_iterate_inc;
//   }
//}

void Renderer::Render_Span_Buffer_PC( uint16 * buffer, uint32 start_y, uint32 end_y )
{
   uint16 * p_buffer = buffer + ( start_y * g_options.resolution.width );
   uint32 pixel_row  = ( start_y * g_options.resolution.width );

   // transform the camera to object space
   Matrix3x3 mesh_rotation = Extract_Rotation( m_vertex_constants.p_world_matrix );

   // transposing and inverting an orthonormal matrix are the same thing..
   mesh_rotation = Matrix_Transpose_M3( &mesh_rotation );
   Vector3 light_look = Vector_Matrix_Multiply_M3( (Vector3 *)&g_scene_light.m_world_vector, &mesh_rotation );

   // we want the reverse DP because for us, when two vetors point at each other that is full intensity.
   light_look = Scale_V3( &light_look, -1 );

   uint32 y;
   for ( y = start_y; y < end_y; y ++ )
   {
      // get the x range TODO: Sometimes this is 0, causing #-1INF000 and a release build crash
      float x_range    = 1 / (float) ( m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ] - m_span_buffer.span_row[ y ].x[ LEFT_EDGE ] );

      // get the steps for interpolating z, light and UV
      float z_step     = ( m_span_buffer.span_row[ y ].z[ RIGHT_EDGE ] - m_span_buffer.span_row[ y ].z[ LEFT_EDGE ] ) * x_range;
      float z          = m_span_buffer.span_row[ y ].z[ LEFT_EDGE ];

      /*float light_step = ( m_span_buffer.span_row[ y ].light_amount[ RIGHT_EDGE ] - m_span_buffer.span_row[ y ].light_amount[ LEFT_EDGE ] ) * x_range;
      float light      = m_span_buffer.span_row[ y ].light_amount[ LEFT_EDGE ];*/
      
      uint32 x_step    = g_options.wire_frame ? m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ] : 1;

      // perspective correct UV
      float recip_left_z  = 1 / m_span_buffer.span_row[ y ].world_z[ LEFT_EDGE ];
      float recip_right_z = 1 / m_span_buffer.span_row[ y ].world_z[ RIGHT_EDGE ];

      float u_start = m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].u * recip_left_z;
      float v_start = m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].v * recip_left_z;

      float u_end = m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].u * recip_right_z;
      float v_end = m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].v * recip_right_z;

      float u_step = ( u_end - u_start ) * x_range;
      float v_step = ( v_end - v_start ) * x_range;

      UV uv = { u_start, v_start };

      // tangent uvs
      float tang_u_step = ( m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].u - m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].u ) * x_range;
      float tang_v_step = ( m_span_buffer.span_row[ y ].uv[ RIGHT_EDGE ].v - m_span_buffer.span_row[ y ].uv[ LEFT_EDGE ].v ) * x_range;

      // R
      float r_start = recip_left_z;
      float r_end   = recip_right_z;

      float r_step  = ( r_end - r_start ) * x_range;
      float r       = r_start;

      // phong shading normal
      Vector3 pixel_normal = m_span_buffer.span_row[ y ].normal[ LEFT_EDGE ];
      Vector3 normal_step  = Scale_V3( &Subtract_V3( &m_span_buffer.span_row[ y ].normal[ RIGHT_EDGE ], &m_span_buffer.span_row[ y ].normal[ LEFT_EDGE ] ), x_range );

      // phong lighting interpolation, cuz we can speed this up a lot
      float diffuse_light_start = max( 0.00f, Dot_Product_V3( &m_span_buffer.span_row[ y ].normal[ LEFT_EDGE ] , &light_look ) );
      float diffuse_light_end   = max( 0.00f, Dot_Product_V3( &m_span_buffer.span_row[ y ].normal[ RIGHT_EDGE ], &light_look ) );

      float diffuse_step        = ( diffuse_light_end - diffuse_light_start ) * x_range;
      
      //specularity
      Vector3 pixel_position = m_span_buffer.span_row[ y ].position[ LEFT_EDGE ];
      Vector3 pixel_step     = Scale_V3( &Subtract_V3( &m_span_buffer.span_row[ y ].position[ RIGHT_EDGE ], &m_span_buffer.span_row[ y ].position[ LEFT_EDGE ] ), x_range );

      Vector3 camera_position;
      camera_position.Set( m_p_camera->m_world_matrix.m_row[ 3 ].x, m_p_camera->m_world_matrix.m_row[ 3 ].y, m_p_camera->m_world_matrix.m_row[ 3 ].z );

      Vector3 light_position;
      light_position.Set( g_scene_light.m_world_vector.x, g_scene_light.m_world_vector.y, g_scene_light.m_world_vector.z );


      // interpolate colors
      uint32 x;
      for ( x = m_span_buffer.span_row[ y ].x[ LEFT_EDGE ]; x < m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ]; x += x_step )
      {  
         uint32 next_x = x + PERSPECTIVE_CORRECTION_RATE;
         uint32 stop_x = next_x < m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ] ? next_x : m_span_buffer.span_row[ y ].x[ RIGHT_EDGE ];

         float world_z = 1 / r;

         // put uvs back in tangent space
         UV tang_uv = { uv.u * world_z, uv.v * world_z };

         // calculate the phong shading
         float diffuse_light = max( 0.00f, Dot_Product_V3( &pixel_normal, &light_look ) );

         // begin linear interpolation for up to PERSPECTIVE_CORRECTION_RATE amount of pixels.
         for ( ; x < stop_x; x += x_step )
         {
            if ( z < m_z_buffer[ pixel_row + x ] )
            {
               short pixel = Filter_Function( &tang_uv, &m_vertex_constants );

               //use these lookup tables to avoid shifting. (saves over 5fps)
               uint16 pixel_red   = m_red_down_lookup[ ( pixel & 0x7C00 ) ]; 
               uint16 pixel_green = m_green_down_lookup[ ( pixel & 0x03E0 ) ];
               uint16 pixel_blue  = ( pixel & 0x001F );

               // calculate specularity
               float specular_intensity = Specular_Function( &pixel_normal, &pixel_position, &light_position, &camera_position );

               // apply ambient light to the phone light
               float total_red_light   = Light::m_ambient_light.x + diffuse_light;
               float total_green_light = Light::m_ambient_light.y + diffuse_light;
               float total_blue_light  = Light::m_ambient_light.z + diffuse_light;

               
               sint16 final_red   = (sint16) ( pixel_red   * total_red_light  );
               sint16 final_green = (sint16) ( pixel_green * total_green_light );
               sint16 final_blue  = (sint16) ( pixel_blue  * total_blue_light );

               // now scale the specularity
               sint16 specular_red   = (sint16) ( ( specular_intensity * total_red_light   ) * 31 );
               sint16 specular_green = (sint16) ( ( specular_intensity * total_green_light ) * 31 );
               sint16 specular_blue  = (sint16) ( ( specular_intensity * total_blue_light  ) * 31 );
             
               // create our color by setting the bytes so we avoid bit shifting.
               uint16 lit_pixel = (uint16) ( m_red_up_lookup[ min( final_red + specular_red, 31 ) ] | m_green_up_lookup[ min( final_green + specular_green, 31 ) ] | min( final_blue + specular_blue, 31 ) );

               p_buffer[ x ] = lit_pixel;

               // update the z buffer
               m_z_buffer[ pixel_row + x ] = z;
            }

            // interpolate all our values.
            uv.u += u_step;
            uv.v += v_step;

            tang_uv.u += tang_u_step;
            tang_uv.v += tang_v_step;

            r += r_step;
            z += z_step;

            diffuse_light += diffuse_step;

            // interpolate the normals for phong shading
            pixel_normal = Add_V3( &pixel_normal, &normal_step );

            pixel_position = Add_V3( &pixel_position, &pixel_step );
         }
         
         // pull x back for processing.
         x--;
      }

      pixel_row += g_options.resolution.width;
      p_buffer  += g_options.resolution.width;
   }
}

float Renderer::Calculate_Specularity( Vector3 * pixel_normal, Vector3 * pixel_position, Vector3 * light_position, Vector3 * camera_position )
{
   // calculate specularity
   Vector3 light_dir, pixel_to_camera, half_way;

   // let's do both of these at once
   Vector3 light_to_pixel  = Subtract_V3( pixel_position, light_position );
   Vector3 camera_to_pixel = Subtract_V3( pixel_position, camera_position );

   Normalize_V3( &light_to_pixel , &light_dir );
   Normalize_V3( &camera_to_pixel, &pixel_to_camera );

   Normalize_V3( &Add_V3( &light_dir, &pixel_to_camera ), &half_way );

   float specular_light     = max( 0.00f, Dot_Product_V3( pixel_normal, &half_way ) );
   return m_specular_power_lookup[ (uint32) ( specular_light * 100 ) ];//pow( specular_light, 12.00f );
}

float Renderer::Calculate_No_Specularity( Vector3 * pixel_normal, Vector3 * pixel_position, Vector3 * light_pos, Vector3 * camera_pos )
{
   return 0.00f;
}

short Renderer::Nearest_Neighbor_Pixel( UV * p_uv, Vertex_Const * p_vertex_constants )
{
   uint32 row        = ( (int) ( p_vertex_constants->p_texture->height * p_uv->v ) );
   uint32 row_offset = ( (int) ( p_vertex_constants->p_texture->width  * p_uv->u ) );

   row = min( row, p_vertex_constants->p_texture->height - 1 );

   short pixel = p_vertex_constants->p_texture->p_data[ ( p_vertex_constants->p_texture->width * row ) + row_offset ];

   return pixel;
}

short Renderer::Bilinear_Filter_16_Pixel( UV * p_uv, Vertex_Const * p_vertex_constants )
{
   float pixel_u = p_uv->u * p_vertex_constants->p_texture->width;
   float pixel_v = p_uv->v * p_vertex_constants->p_texture->height;

   pixel_v = min( pixel_v, p_vertex_constants->p_texture->height - 1 );


   // get the 4 texels we need
   uint32 x_texel_left = (uint32) pixel_u;
   uint32 y_texel_top  = (uint32) pixel_v * p_vertex_constants->p_texture->width;

   uint32 x_texel_right  = x_texel_left + 1;
   uint32 y_texel_bottom = y_texel_top + p_vertex_constants->p_texture->width; // go down a row. CAUTION! This could theoretically go one past the edge of the texture.

   float u_decimal = pixel_u - (uint32) pixel_u;
   float v_decimal = pixel_v - (uint32) pixel_v;

   uint16 top_left_texel     = p_vertex_constants->p_texture->p_data[ x_texel_left  + y_texel_top ];
   uint32 top_right_texel    = p_vertex_constants->p_texture->p_data[ x_texel_right + y_texel_top ];
   uint32 bottom_left_texel  = p_vertex_constants->p_texture->p_data[ x_texel_left  + y_texel_bottom ];
   uint32 bottom_right_texel = p_vertex_constants->p_texture->p_data[ x_texel_right + y_texel_bottom ];

   float inv_u_dec = ( 1 - u_decimal );
   float inv_v_dec = ( 1 - v_decimal );

   // scale top_left
   uint16 red   = m_red_down_lookup[ ( top_left_texel & 0x7C00 ) ]; 
   uint16 green = m_green_down_lookup[ ( top_left_texel & 0x03E0 ) ];
   uint16 blue  = ( top_left_texel & 0x001F );

   red   = (uint16) ( red   * inv_u_dec * inv_v_dec );
   green = (uint16) ( green * inv_u_dec * inv_v_dec );
   blue  = (uint16) ( blue  * inv_u_dec * inv_v_dec );

   top_left_texel = m_red_up_lookup[ red ] | m_green_up_lookup[ green ] | blue;


   // scale top_right
   red   = m_red_down_lookup[ ( top_right_texel & 0x7C00 ) ]; 
   green = m_green_down_lookup[ ( top_right_texel & 0x03E0 ) ];
   blue  = ( top_right_texel & 0x001F );

   red   = (uint16) ( red   * u_decimal * inv_v_dec );
   green = (uint16) ( green * u_decimal * inv_v_dec );
   blue  = (uint16) ( blue  * u_decimal * inv_v_dec );

   top_right_texel = m_red_up_lookup[ red ] | m_green_up_lookup[ green ] | blue;


   // scale bottom_left
   red   = m_red_down_lookup[ ( bottom_left_texel & 0x7C00 ) ]; 
   green = m_green_down_lookup[ ( bottom_left_texel & 0x03E0 ) ];
   blue  = ( bottom_left_texel & 0x001F );

   red   = (uint16) ( red   * inv_u_dec * v_decimal );
   green = (uint16) ( green * inv_u_dec * v_decimal );
   blue  = (uint16) ( blue  * inv_u_dec * v_decimal );

   bottom_left_texel = m_red_up_lookup[ red ] | m_green_up_lookup[ green ] | blue;

   // scale bottom_right_texel
   red   = m_red_down_lookup[ ( bottom_right_texel & 0x7C00 ) ]; 
   green = m_green_down_lookup[ ( bottom_right_texel & 0x03E0 ) ];
   blue  = ( bottom_right_texel & 0x001F );

   red   = (uint16) ( red   * u_decimal * v_decimal );
   green = (uint16) ( green * u_decimal * v_decimal );
   blue  = (uint16) ( blue  * u_decimal * v_decimal );

   bottom_right_texel = m_red_up_lookup[ red ] | m_green_up_lookup[ green ] | blue;

   // add together the final weighted colors.
   short pixel = top_left_texel + top_right_texel + bottom_left_texel + bottom_right_texel;

   return pixel;
}

short Renderer::Bilinear_Filter_32_Pixel( UV * p_uv, Vertex_Const * p_vertex_constants )
{
   float pixel_u = p_uv->u * p_vertex_constants->p_texture->width;
   float pixel_v = p_uv->v * p_vertex_constants->p_texture->height;

   pixel_v = min( pixel_v, p_vertex_constants->p_texture->height - 1 );

   // get the 4 texels we need
   uint32 x_texel_left = (uint32) pixel_u;
   uint32 y_texel_top  = (uint32) pixel_v * p_vertex_constants->p_texture->width;

   uint32 x_texel_right  = x_texel_left + 1;
   uint32 y_texel_bottom = y_texel_top + p_vertex_constants->p_texture->width; // go down a row. CAUTION! This could theoretically go one past the edge of the texture.
  

   // checking
   float u_decimal = pixel_u - (uint32) pixel_u;
   float v_decimal = pixel_v - (uint32) pixel_v;

   short * t = p_vertex_constants->p_texture->p_data;

   uint32 top_left_texel = t[ x_texel_left  + y_texel_top ];
   top_left_texel = Color_16_To_32( (short) top_left_texel );

   uint32 top_right_texel    = t[ x_texel_right + y_texel_top ];
   top_right_texel = Color_16_To_32( (short) top_right_texel );


   uint32 bottom_left_texel  = t[ x_texel_left  + y_texel_bottom ];
   bottom_left_texel = Color_16_To_32( (short) bottom_left_texel );


   uint32 bottom_right_texel = t[ x_texel_right + y_texel_bottom ];
   bottom_right_texel = Color_16_To_32( (short) bottom_right_texel );

   float inv_u_dec = ( 1 - u_decimal );
   float inv_v_dec = ( 1 - v_decimal );

   // scale top_left
   uint32 red   = ( top_left_texel & 0x00FF0000 ) >> 16;
   uint32 green = ( top_left_texel & 0x0000FF00 ) >> 8;
   uint32 blue  = ( top_left_texel & 0x000000FF );

   red   = (uint32) ( red   * inv_u_dec * inv_v_dec );
   green = (uint32) ( green * inv_u_dec * inv_v_dec );
   blue  = (uint32) ( blue  * inv_u_dec * inv_v_dec );

   top_left_texel = red << 16 | green << 8 | blue;


   // scale top_right
   red   = ( top_right_texel & 0x00FF0000 ) >> 16;
   green = ( top_right_texel & 0x0000FF00 ) >> 8;
   blue  = ( top_right_texel & 0x000000FF );

   red   = (uint32) ( red   * u_decimal * inv_v_dec );
   green = (uint32) ( green * u_decimal * inv_v_dec );
   blue  = (uint32) ( blue  * u_decimal * inv_v_dec );

   top_right_texel = red << 16 | green << 8 | blue;


   // scale bottom_left
   red   = ( bottom_left_texel & 0x00FF0000 ) >> 16;
   green = ( bottom_left_texel & 0x0000FF00 ) >> 8;
   blue  = ( bottom_left_texel & 0x000000FF );

   red   = (uint32) ( red   * inv_u_dec * v_decimal );
   green = (uint32) ( green * inv_u_dec * v_decimal );
   blue  = (uint32) ( blue  * inv_u_dec * v_decimal );

   bottom_left_texel = red << 16 | green << 8 | blue;

   // scale bottom_right_texel
   red   = ( bottom_right_texel & 0x00FF0000 ) >> 16;
   green = ( bottom_right_texel & 0x0000FF00 ) >> 8;
   blue  = ( bottom_right_texel & 0x000000FF );

   red   = (uint32) ( red   * u_decimal * v_decimal );
   green = (uint32) ( green * u_decimal * v_decimal );
   blue  = (uint32) ( blue  * u_decimal * v_decimal );

   bottom_right_texel = red << 16 | green << 8 | blue;

   uint32 pixel_32 = top_left_texel + top_right_texel + bottom_left_texel + bottom_right_texel;

   return Color_32_To_16( pixel_32 );
}

void Renderer::Begin( void )
{
   //clear the z buffer
   uint32 i;
   for ( i = 0; i < g_options.resolution.width * g_options.resolution.height; i++ )
   {
      m_z_buffer[ i ] = 1.00f;
   }
}
