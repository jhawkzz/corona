
#include "corona_headers.h"

Vertex Mesh::m_world_vertex[ MAX_VERTICES ]; //just so it isn't on the stack

Mesh::Mesh( void )
{
}

Mesh::~Mesh( )
{
}

void Mesh::Create( void )
{
   m_p_frame        = NULL;
   m_p_face         = NULL;

   m_vertex_count   = 0;
   m_face_count     = 0;
   m_frame_count    = 0;

   m_texture_flags  = 0;

   memset( &m_texture, 0, sizeof( m_texture ) );

   // make the world matrix identiy
   m_world_matrix = Make_Identity_M4( );

   m_roll           = 0.00f;
   m_pitch          = 0.00f;
   m_yaw            = 0.00f;
   m_current_frame  = 0;
   m_frame_rate     = 0.00f;
   m_frame_speed    = 0.00f;

   m_animation_paused = FALSE;

   m_now_frame.p_vertex = (Vertex *)_aligned_malloc( sizeof( Vertex ) * MAX_VERTICES, 16 );
}

void Mesh::Destroy( void )
{
   // remove each set of frame verts
   uint32 i;
   for ( i = 0; i < m_frame_count; i++ )
   {
      _aligned_free( m_p_frame[ i ].p_vertex );
   }

   delete [] m_p_frame; //remove the frame list

   delete [] m_p_face;
   delete [] m_texture.p_data;

   m_p_frame        = NULL;
   m_p_face         = NULL;

   m_vertex_count   = 0;
   m_face_count     = 0;
   m_frame_count    = 0;

   memset( &m_texture, 0, sizeof( m_texture ) );

   // current state stuff
   m_roll           = 0.00f;
   m_pitch          = 0.00f;
   m_yaw            = 0.00f;
   m_current_frame  = 0;
   m_frame_rate     = 0.00f;
   m_frame_speed    = 0.00f;

   m_animation_paused = FALSE;

   _aligned_free( m_now_frame.p_vertex );
}

void Mesh::Rotate( float roll, float pitch, float yaw )
{
   // reset the world matrix
   m_world_matrix = Make_Identity_M4( );

   // create a rotation matrix for all 3
   Matrix4x4 x_rotate = Make_X_Rotation_M4( pitch );
   Matrix4x4 y_rotate = Make_Y_Rotation_M4( yaw );
   Matrix4x4 z_rotate = Make_Z_Rotation_M4( roll );
   
   // apply all three rotations to the world matrix
   Multiply_M4( &m_world_matrix, &x_rotate, &m_world_matrix );
   Multiply_M4( &m_world_matrix, &y_rotate, &m_world_matrix );
   Multiply_M4( &m_world_matrix, &z_rotate, &m_world_matrix );
}

void Mesh::Rotate_X( float rotation )
{
   m_world_matrix = Make_Identity_M4( );

   // create a matrix rotated on x
   Matrix4x4 x_rotate = Make_X_Rotation_M4( rotation );

   // apply to the world matrix
   Multiply_M4( &m_world_matrix, &x_rotate, &m_world_matrix );
}

void Mesh::Rotate_Y( float rotation )
{
   m_world_matrix = Make_Identity_M4( );

   // create a matrix rotated on y
   Matrix4x4 y_rotate = Make_Y_Rotation_M4( rotation );

   // apply to the world matrix
   Multiply_M4( &m_world_matrix, &y_rotate, &m_world_matrix );
}

void Mesh::Rotate_Z( float rotation )
{
   m_world_matrix = Make_Identity_M4( );

   // create a matrix rotated on z
   Matrix4x4 z_rotate = Make_Z_Rotation_M4( rotation );

   // apply to the world matrix
   Multiply_M4( &m_world_matrix, &z_rotate, &m_world_matrix );
}

void Mesh::Update( uint32 controller_state )
{
   Update_Rotation( controller_state );

   Update_Keyboard( controller_state );

   Rotate( m_roll, m_pitch, m_yaw );

   if ( m_frame_count > 1 && !m_animation_paused )
   {
      //interpolate for next time.
      if ( m_frame_rate < g_next_frame )
      {
         float interpolation = min( 1.00f, max( 0.00f, ( FRAME_SPEED - ( m_frame_speed - g_next_frame ) ) / FRAME_SPEED ) );
         Interpolate_Frame( &m_p_frame[ m_current_frame ], &m_p_frame[ m_current_frame + 1 ], &m_now_frame, interpolation );

         m_frame_rate = g_next_frame + FRAME_RATE;
      }

      // we completed going from a to b, so reset for the next frame
      if ( m_frame_speed < g_next_frame )
      {
         m_frame_speed = g_next_frame + FRAME_SPEED;

         m_current_frame++;

         if ( m_frame_count - 1 != m_current_frame )
         {
            return;
         }

         m_current_frame = 0;
      }
   }
}

void Mesh::Update_Keyboard( uint32 controller_state )
{
   float old_keydown_timer = m_keydown_timer;

   //Temp! tell the server to start the race as soon as plus is pressed
   if ( controller_state & BUTTON_ENTER )
   {
      if ( m_keydown_timer < g_next_frame )
      {
         m_animation_paused = !m_animation_paused;

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

void Mesh::Update_Rotation( uint32 controller_state )
{
   if ( controller_state & BUTTON_A )
   {
      m_yaw += .03f;
   }
   else if ( controller_state & BUTTON_D )
   {
      m_yaw -= .03f;
   }

   //up/down
   if ( controller_state & BUTTON_W )
   {
      m_pitch += .03f;
   }
   else if ( controller_state & BUTTON_S )
   {
      m_pitch -= .03f;
   }

   if ( controller_state & BUTTON_E )
   {
      m_roll += .03f;
   }
   else if ( controller_state & BUTTON_Q )
   {
      m_roll -= .03f;
   }
}

void Mesh::Interpolate_Frame( Frame * p_cur_frame, Frame * p_next_frame, Frame * p_now_frame, float time )
{
   uint32 i;
   for ( i = 0; i < m_vertex_count; i++ )
   {
      Vertex now_vert;
      now_vert.m_position = Subtract_V4( &p_next_frame->p_vertex[ i ].m_position, &p_cur_frame->p_vertex[ i ].m_position );
      now_vert.m_normal   = Subtract_V3( &p_next_frame->p_vertex[ i ].m_normal  , &p_cur_frame->p_vertex[ i ].m_normal ); //a vec3 operation..sucks and should change
      
      now_vert.m_uv       = p_cur_frame->p_vertex[ i ].m_uv;

      // scale the position
      Vector4 now_pos     = Scale_V4( &now_vert.m_position, time );

      // get the new position
      now_vert.m_position = Add_V4( &p_cur_frame->p_vertex[ i ].m_position, &now_pos );
      
      // scale the vert to the correct position
      Vector3 scaled_norm = Scale_V3( &now_vert.m_normal, time );
      now_vert.m_normal   = Add_V3( &p_cur_frame->p_vertex[ i ].m_normal, &scaled_norm );

      p_now_frame->p_vertex[ i ] = now_vert;
   }
}

void Mesh::Render( Renderer * p_renderer, uint16 * buffer )
{
   uint32 i;
   for ( i = 0; i < m_vertex_count; i++ )
   {
      m_world_vertex[ i ].m_position = Multiply_M4( &m_now_frame.p_vertex[ i ].m_position, &m_world_matrix );
      m_world_vertex[ i ].m_normal   = m_now_frame.p_vertex[ i ].m_normal;
      m_world_vertex[ i ].m_uv       = m_now_frame.p_vertex[ i ].m_uv;
   }

   // assign vertex constants
   p_renderer->m_vertex_constants.p_world_matrix = &m_world_matrix;
   p_renderer->m_vertex_constants.p_texture      = &m_texture;
   p_renderer->m_vertex_constants.texture_flags  = m_texture_flags;
   

   // pass the renderer our mesh
   p_renderer->Render( m_world_vertex, m_vertex_count, m_p_face, m_face_count, buffer );
}
