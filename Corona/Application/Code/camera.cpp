
#include "corona_headers.h"

Camera::Camera( void )
{
}

Camera::~Camera( )
{
}

void Camera::Create( float near_clip_plane, float far_clip_plane, float field_of_view )
{
   // camera setup

   // create our projection matrix
   float q = far_clip_plane / ( far_clip_plane - near_clip_plane );
   float h = 1.0f / tanf( field_of_view / 2 );
   float w = h / ( g_options.resolution.width / g_options.resolution.height );

   memset( &m_projection_matrix, 0, sizeof( m_projection_matrix ) );

   m_projection_matrix.m_row[ 0 ].x = w;
   m_projection_matrix.m_row[ 1 ].y = h;
   m_projection_matrix.m_row[ 2 ].z = q;
   m_projection_matrix.m_row[ 2 ].w = 1;
   m_projection_matrix.m_row[ 3 ].z = -q * near_clip_plane;

   // setup the world matrix
   m_world_matrix = Make_Identity_M4( );

   m_x_position = 0;
   m_y_position = 0;
   m_z_position = -70;

   // create a view projection matrix
   Prepare_Camera( );

   // set our rotations to 0
   m_roll  = 0.00f;
   m_pitch = 0.00f;
   m_yaw   = 0.00f;
}

void Camera::Rotate( float roll, float pitch, float yaw )
{
   m_world_matrix = Make_Identity_M4( );

   m_world_matrix.m_row[ 3 ].x = m_x_position;
   m_world_matrix.m_row[ 3 ].y = m_y_position;
   m_world_matrix.m_row[ 3 ].z = m_z_position;

   Matrix4x4 x_rotation = Make_X_Rotation_M4( pitch );
   Matrix4x4 y_rotation = Make_Y_Rotation_M4( yaw );
   Matrix4x4 z_rotation = Make_Z_Rotation_M4( roll );

   // apply the rotation to the world matrix 
   Multiply_M4( &m_world_matrix, &x_rotation, &m_world_matrix );
   Multiply_M4( &m_world_matrix, &y_rotation, &m_world_matrix );
   Multiply_M4( &m_world_matrix, &z_rotation, &m_world_matrix );

   // recreate the perspective matrix
   Prepare_Camera( );
}

void Camera::Update( uint32 controller_state, POINT * p_point )
{
   Update_Rotation( p_point );
   Update_Movement( controller_state );

   Rotate( m_roll, m_pitch, m_yaw );
}

void Camera::Update_Rotation( POINT * p_mouse_delta )
{
   float yaw   = .005f * p_mouse_delta->x;
   float pitch = .005f * p_mouse_delta->y;

   m_yaw   += yaw;
   m_pitch += pitch;
}

void Camera::Update_Movement( uint32 controller_state )
{
   if ( !controller_state ) return;

   float movement = !(GetAsyncKeyState( VK_SHIFT ) & 0x8000) ? 1.00f : .05f;

   if ( controller_state & BUTTON_RIGHT )
   {
      m_x_position += movement;
   }
   else if ( controller_state & BUTTON_LEFT )
   {
      m_x_position -= movement;
   }

   //up/down
   if ( controller_state & BUTTON_DOWN )
   {
      m_z_position -= movement;
   }
   else if ( controller_state & BUTTON_UP )
   {
      m_z_position += movement;
   }
}

void Camera::Prepare_Camera( void )
{
   Matrix4x4 view_matrix;
   Make_Invert_M4( &m_world_matrix, &view_matrix );

   Multiply_M4( &view_matrix, &m_projection_matrix, &m_view_projection_matrix );
}
