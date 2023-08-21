
#ifndef CAMERA_H_
#define CAMERA_H_

class Camera
{

      public:

                     Camera         ( void );
                     ~Camera        ( );

         void        Create         ( float near_clip_plane, float far_clip_plane, float field_of_view );
         void        Rotate         ( float roll, float pitch, float yaw );

         void        Update         ( uint32 controller_state, POINT * p_point );
         void        Update_Rotation( POINT * p_mouse_delta );
         void        Update_Movement( uint32 controller_state );

         Matrix4x4   m_world_matrix;
         Matrix4x4   m_projection_matrix;
         Matrix4x4   m_view_projection_matrix;

      private:

         void        Prepare_Camera ( void );

         float       m_roll;
         float       m_pitch;
         float       m_yaw;
         float       m_x_position;
         float       m_y_position;
         float       m_z_position;
};

#endif
