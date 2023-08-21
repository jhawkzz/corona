
#ifndef LIGHT_H_
#define LIGHT_H_

class Light
{

       public:

                Light               ( void );
                ~Light              ( );

       void     Create              ( Vector4 * world_pos );

       void     Rotate              ( float roll, float pitch, float yaw );
       void     Update              ( uint32 controller_state, POINT * p_mouse_delta );
       void     Update_Rotation     ( POINT * p_mouse_delta );
       void     Update_Ambient_Light( int controller_state );

       Vector4  m_world_vector;

       float    m_roll;
       float    m_pitch;
       float    m_yaw;

static Vector4  m_ambient_light;
};

#endif
