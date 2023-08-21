
#include "corona_headers.h"

Corona_Engine::Corona_Engine( void )
{
   m_dialog_ptr       = NULL;
   m_graphics_ptr     = NULL;
   m_clear_backbuffer = TRUE;
   m_paused           = FALSE;

   m_tick_count       = 0;
   m_avg_fps          = 0;
   m_fps              = 0;

   m_keydown_timer    = 0.00f;

   memset( m_debug_info, 0, sizeof( m_debug_info ) );
   memset( &m_last_mouse_pos, 0, sizeof( m_last_mouse_pos ) );
}

Corona_Engine::~Corona_Engine( )
{
}

BOOL Corona_Engine::Create( Dialog * p_dialog, Graphics * p_graphics )
{
   if ( p_dialog && p_graphics )
   {
      m_dialog_ptr   = p_dialog;
      m_graphics_ptr = p_graphics;

      m_camera.Create( 1, 1000, PI / 4 );
      m_renderer.Create( &m_camera );
      g_scene_light.Create( &Vector4( 0, 0, 1, 1 ) );

      //TODO: Make it so this doesn't have to be called
      m_mesh.Create( );
      m_skybox.Create( );

      Mesh_Loader ml;
      ml.Load_Mesh( ROOT_DRIVE"quake 2 models\\monsters\\soldier\\tris.md2", &m_mesh, 0 );
      //ml.Load_Mesh( "E:\\vagabond models\\plane2.vgd", &m_mesh, 0 );
      
      ml.Load_Mesh( ROOT_DRIVE"vagabond models\\skybox.vgd", &m_skybox, TEXTURE_SKY_CUBE );

      return TRUE;
   }

   return FALSE;
}

void Corona_Engine::Destroy( void )
{
   m_mesh.Destroy( );
   m_skybox.Destroy( );
}

BOOL Corona_Engine::Is_Paused( void )
{
   return m_paused;
}

void Corona_Engine::Toggle_Pause( BOOL pause )
{
   m_paused = pause;

   if ( m_paused )
   {
      Timer::Pause( );
   }
   else
   {
      Timer::UnPause( );

      g_next_frame = Timer::m_total;
   }
}

BOOL Corona_Engine::Clear_Backbuffer( void )
{
   return m_clear_backbuffer;
}

void Corona_Engine::Update( void )
{
   Update_Controller_Input( );
   Update_Mouse_Input( );
   Update_Keyboard_Input( );
   Update_Messages( );

   if ( m_paused ) return;

   Update_Camera( );
   Update_Lights( );
   Update_Mesh( );

   m_renderer.Update_Keyboard( m_controller_state );
}

void Corona_Engine::Render( void )
{
   if ( m_paused ) return;

   //Render_Normal( );

   // center the skycube in the camera
   m_skybox.m_world_matrix.m_row[ 3 ] = m_camera.m_world_matrix.m_row[ 3 ];

   m_renderer.Begin( );

   // render skycube to a seperate buffer
   if ( m_skycube_timer < g_next_frame )
   {
      m_skybox.Render( &m_renderer, (uint16 *)m_graphics_ptr->Get_Sky_Buffer_Bits( ) );

      m_skycube_timer = g_next_frame + .033333f;
   }

   // put the skycube on the mesh's buffer
   BitBlt( m_graphics_ptr->Get_Back_Buffer_HDC( ), 0, 0, g_options.resolution.width, g_options.resolution.height, m_graphics_ptr->Get_Sky_Buffer_HDC( ), 0, 0, SRCCOPY );

   m_mesh.Render( &m_renderer, (uint16 *)m_graphics_ptr->Get_Back_Buffer_Bits( ) );

   Render_Debug( );
}

void Corona_Engine::Update_Controller_Input( void )
{
   m_controller_state = 0;

   if ( m_dialog_ptr->Get_HWND( ) == GetForegroundWindow( ) )
   {
      // Directional
      m_controller_state |= (GetAsyncKeyState( VK_LEFT     ) & 0x8000) ? BUTTON_LEFT     : 0;
      m_controller_state |= (GetAsyncKeyState( VK_RIGHT    ) & 0x8000) ? BUTTON_RIGHT    : 0;
      m_controller_state |= (GetAsyncKeyState( VK_UP       ) & 0x8000) ? BUTTON_UP       : 0;
      m_controller_state |= (GetAsyncKeyState( VK_DOWN     ) & 0x8000) ? BUTTON_DOWN     : 0;

      // Numeric
      m_controller_state |= (GetAsyncKeyState( (uint32) '1'   ) & 0x8000) ? BUTTON_1     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '2'   ) & 0x8000) ? BUTTON_2     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '3'   ) & 0x8000) ? BUTTON_3     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '4'   ) & 0x8000) ? BUTTON_4     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '5'   ) & 0x8000) ? BUTTON_5     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '6'   ) & 0x8000) ? BUTTON_6     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '7'   ) & 0x8000) ? BUTTON_7     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '8'   ) & 0x8000) ? BUTTON_8     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '9'   ) & 0x8000) ? BUTTON_9     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) '0'   ) & 0x8000) ? BUTTON_0     : 0;

      // Alpha-Numeric
      m_controller_state |= (GetAsyncKeyState( (uint32) 'W'   ) & 0x8000) ? BUTTON_W     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) 'A'   ) & 0x8000) ? BUTTON_A     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) 'S'   ) & 0x8000) ? BUTTON_S     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) 'D'   ) & 0x8000) ? BUTTON_D     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) 'Q'   ) & 0x8000) ? BUTTON_Q     : 0;
      m_controller_state |= (GetAsyncKeyState( (uint32) 'E'   ) & 0x8000) ? BUTTON_E     : 0;

      // Functional
      m_controller_state |= (GetAsyncKeyState( VK_TAB      ) & 0x8000) ? BUTTON_TAB      : 0;
      m_controller_state |= (GetAsyncKeyState( VK_SPACE    ) & 0x8000) ? BUTTON_SPACE    : 0;
      m_controller_state |= (GetAsyncKeyState( VK_RETURN   ) & 0x8000) ? BUTTON_ENTER    : 0;
      m_controller_state |= (GetAsyncKeyState( VK_BACK     ) & 0x8000) ? BUTTON_BACKSPACE: 0;
      m_controller_state |= (GetAsyncKeyState( 187         ) & 0x8000) ? BUTTON_PLUS     : 0;
      m_controller_state |= (GetAsyncKeyState( 189         ) & 0x8000) ? BUTTON_MINUS    : 0;
   }
}

void Corona_Engine::Update_Mouse_Input( void )
{
   RECT rect;
   GetWindowRect( m_dialog_ptr->Get_HWND( ), &rect );

   POINT point;
   GetCursorPos( &point );

   if ( PtInRect( &rect, point ) )
   {
      m_delta_mouse.x = point.x - m_last_mouse_pos.x;
      m_delta_mouse.y = point.y - m_last_mouse_pos.y;
   }
   else
   {
      m_delta_mouse.x = 0;
      m_delta_mouse.y = 0;
   }

   m_last_mouse_pos = point;
}

void Corona_Engine::Update_Messages( void )
{
   // handle messages
   Message * message = (Message *) g_message_stack.Pop( );

   while( (uint32) message != STACK_EMPTY )
   {
      switch( message->message )
      {
         case CM_LBUTTONDOWN:
         {
            break;
         }

         case CM_RBUTTONDOWN:
         {
            break;
         }

         case CM_PAUSE_ENGINE:
         {
            Toggle_Pause( !m_paused );

            break;
         }

         case CM_RESET_ENGINE:
         {
            Reset( );

            break;
         }

         case CM_TOGGLE_REDRAW_BACKBUFFER:
         {
            m_clear_backbuffer = !m_clear_backbuffer;

            break;
         }

         case CM_RESOLUTION_CHANGED:
         {
            // change gfx resolution
            m_graphics_ptr->Destroy( );
            m_graphics_ptr->Create( m_dialog_ptr->Get_HWND( ), &g_options.resolution );
            break;
         }
      }

      g_message_stack.Remove_Message( message );

      message = (Message *) g_message_stack.Pop( );
   }
}

void Corona_Engine::Update_Camera( void )
{
   POINT point = { 0, 0 };

   if ( ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) )
   {
      point = m_delta_mouse;
   }

   m_camera.Update( m_controller_state, &point );
}

void Corona_Engine::Update_Lights( void )
{
   POINT point = { 0, 0 };

   if ( ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 ) )
   {
      point = m_delta_mouse;
   }

   g_scene_light.Update( m_controller_state, &point );
}

void Corona_Engine::Update_Mesh( void )
{
   m_mesh.Update( m_controller_state );
}

void Corona_Engine::Render_Normal( void )
{
   // clear the back buffer
   if ( m_clear_backbuffer )
   {
      memset( m_graphics_ptr->Get_Back_Buffer_Bits( ), 0xaa, m_graphics_ptr->Get_Graphics_Size( ) );
   }
}

void Corona_Engine::Flip( void )
{
   HWND hwnd = m_dialog_ptr->Get_HWND( );

   HDC dc = GetDC( hwnd );

   HDC back_buffer_hdc = m_graphics_ptr->Get_Back_Buffer_HDC( );

   BitBlt( dc, 0, 0, g_options.resolution.width, g_options.resolution.height, back_buffer_hdc, 0, 0, SRCCOPY );

   ReleaseDC( hwnd, dc );
}

void Corona_Engine::Render_Debug( void )
{
   if ( !m_clear_backbuffer )
   {
      return;
   }

   m_debug_info[ 0 ] = 0;

   //Render FPS
   if ( m_tick_count > CORONA_AVERAGE_FPS_TICKS )
   {
      m_fps        = m_avg_fps / CORONA_AVERAGE_FPS_TICKS;
      m_tick_count = 0;
      m_avg_fps    = 0;
   }
   else
   {
      m_avg_fps += Timer::m_rate;
      m_tick_count++;
   }

   // save to use the x for ambient light, because it should be uniform across all
   char specularity_str[ MAX_PATH ];
   char filtering_str[ MAX_PATH ];

   switch( m_renderer.m_texture_filter )
   {
      case FILTER_NONE:        strcpy( filtering_str, "Nearest Neighbor" ); break;
      case FILTER_BILINEAR_16: strcpy( filtering_str, "Bilinear 16-Bit" );  break;
      case FILTER_BILINEAR_32: strcpy( filtering_str, "Bilinear 32-Bit" );  break;
   }

   if ( m_renderer.m_specularity )
   {
      strcpy( specularity_str, "Yes" );
   }
   else
   {
      strcpy( specularity_str, "No" );
   }

   sprintf( m_debug_info, "[ FPS: %.2f | Ambient Lighting: %.2f | Specularity: %s | Texture Filtering: %s ]", m_fps, Light::m_ambient_light.x, specularity_str, filtering_str );
   
   SetTextColor( m_graphics_ptr->Get_Back_Buffer_HDC( ), RGB( 255, 255, 255 ) );
   SetBkMode   ( m_graphics_ptr->Get_Back_Buffer_HDC( ), TRANSPARENT );

   TextOut( m_graphics_ptr->Get_Back_Buffer_HDC( ), 0, 20, m_debug_info, strlen( m_debug_info ) );
}

void Corona_Engine::Update_Keyboard_Input( void )
{
   float old_keydown_timer = m_keydown_timer;

   //Temp! tell the server to start the race as soon as plus is pressed
   if ( m_controller_state & BUTTON_PLUS )
   {
      if ( m_keydown_timer < g_next_frame )
      {
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

void Corona_Engine::Reset( void )
{
}
