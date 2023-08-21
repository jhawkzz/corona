
#ifndef CORONA_ENGINE_H_
#define CORONA_ENGINE_H_

class Dialog;

class Corona_Engine
{
   public:
                           Corona_Engine                   ( void );
                           ~Corona_Engine                  ( );

            BOOL           Create                          ( Dialog * dialog, Graphics * graphics );
            void           Destroy                         ( void );

            BOOL           Is_Paused                       ( void );
            void           Toggle_Pause                    ( BOOL pause );

            BOOL           Clear_Backbuffer                ( void );

            void           Update                          ( void );
            void           Render                          ( void );
            void           Flip                            ( void );
            
            Mesh           m_mesh;

   private:
            void           Update_Controller_Input         ( void );
            void           Update_Mouse_Input              ( void );
            void           Update_Messages                 ( void );
            void           Update_Keyboard_Input           ( void );
            void           Update_Camera                   ( void );
            void           Update_Lights                   ( void );
            void           Update_Mesh                     ( void );
            

            void           Render_Normal                   ( void );

            void           Render_Debug                    ( void );

            void           Reset                           ( void );

            Dialog      *  m_dialog_ptr;
            Graphics    *  m_graphics_ptr;

            Renderer       m_renderer;
            Camera         m_camera;

            Mesh           m_skybox;

            uint32         m_tick_count;
            float          m_avg_fps;
            float          m_fps;
            
            uint32         m_controller_state;
            POINT          m_last_mouse_pos;
            POINT          m_delta_mouse;
            BOOL           m_clear_backbuffer;
            BOOL           m_paused;

            float          m_keydown_timer;
            char           m_debug_info[ MAX_PATH ];

            float          m_skycube_timer;
};

#endif
