
#include "corona_headers.h"

Vector4  Light::m_ambient_light;

Light::Light( void )
{
}

Light::~Light( )
{
}

void Light::Create( Vector4 * p_world_pos )
{
   m_ambient_light.Set( .50f, .50f, .50f, 0 );

   m_yaw   = 0;
   m_roll  = 0;
   m_pitch = 0;

   m_world_vector = *p_world_pos;
}

void Light::Rotate( float roll, float pitch, float yaw )
{
   m_world_vector.Set( 0, 0, -1.50f, 0 );
   
   Matrix4x4 x_rotation_matrix = Make_X_Rotation_M4( pitch );
   Matrix4x4 y_rotation_matrix = Make_Y_Rotation_M4( yaw );
   Matrix4x4 z_rotation_matrix = Make_Z_Rotation_M4( roll );

   m_world_vector = Multiply_M4( &m_world_vector, &x_rotation_matrix );
   m_world_vector = Multiply_M4( &m_world_vector, &y_rotation_matrix );
   m_world_vector = Multiply_M4( &m_world_vector, &z_rotation_matrix );
}

void Light::Update( uint32 controller_state, POINT * p_mouse_delta )
{
   Update_Rotation( p_mouse_delta );

   Update_Ambient_Light( controller_state );

   Rotate( m_roll, m_pitch, m_yaw );
}

void Light::Update_Rotation( POINT * p_mouse_delta )
{
   float yaw   = .0025f * p_mouse_delta->x;
   float pitch = .0025f * -p_mouse_delta->y;

   m_yaw   += yaw;
   m_pitch += pitch;
}

void Light::Update_Ambient_Light( int controller_state )
{
   if ( controller_state & BUTTON_MINUS )
   {
      m_ambient_light.x = max( 0.00f, m_ambient_light.x - .01f );
      m_ambient_light.y = max( 0.00f, m_ambient_light.y - .01f );
      m_ambient_light.z = max( 0.00f, m_ambient_light.z - .01f );
   }
   else if ( controller_state & BUTTON_PLUS )
   {
      m_ambient_light.x = min( 1.00f, m_ambient_light.x + .01f );
      m_ambient_light.y = min( 1.00f, m_ambient_light.y + .01f );
      m_ambient_light.z = min( 1.00f, m_ambient_light.z + .01f );
   }
}
