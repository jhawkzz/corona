
#ifndef MESH_H_
#define MESH_H_

class Mesh_Loader;

typedef struct _Frame
{
   Vertex * p_vertex;
}
Frame;

#define FRAME_SPEED (.15f)    //smoothness from frame a to b. The lower the time, the larger the range of interpolation
#define FRAME_RATE  (.03333f) //amount of times per second the frame will be interpolated.

class Mesh
{
   friend Mesh_Loader;

   public:

                        Mesh             ( void );
                        ~Mesh            ( );

         void           Create           ( void );
         void           Destroy          ( void );

         void           Rotate           ( float x, float y, float z );
         void           Rotate_X         ( float rotation );
         void           Rotate_Y         ( float rotation );
         void           Rotate_Z         ( float rotation );

         void           Render           ( Renderer * p_renderer, uint16 * buffer );

         void           Update           ( uint32 controller_state );
         void           Update_Rotation  ( uint32 controller_state );

         void           Update_Keyboard  ( uint32 controller_state );

         void           Interpolate_Frame( Frame * p_cur_frame, Frame * p_next_frame, Frame * p_now_frame, float time );

   public:
      
         Frame        * m_p_frame;
         Face_Map     * m_p_face;
         Texture        m_texture;

         uint32         m_vertex_count;
         uint32         m_frame_count;
         uint32         m_face_count;

         uint32         m_texture_flags;

         // current state
         Matrix4x4      m_world_matrix;
         float          m_roll;
         float          m_pitch;
         float          m_yaw;

         Frame          m_now_frame;
         sint32         m_current_frame;
         float          m_frame_rate;
         float          m_frame_speed;

         float          m_keydown_timer;

         BOOL           m_animation_paused;
         
static   Vertex         m_world_vertex[ MAX_VERTICES ]; //just so it isn't on the stack
};

#endif
