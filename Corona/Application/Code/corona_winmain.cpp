
#include "corona_headers.h"

float         g_next_frame;
Message_Stack g_message_stack;
Options       g_options;
Light         g_scene_light;

int WINAPI WinMain( HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpszcmdline, int ncmdshow )
{
   // watch for memory leaks
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );

   //Shift the PC into 24bit mode, reduces floating point accuracy, drops cpu usage by 4-5%.
	//_control87 ( _PC_24 , _MCW_PC );

   // seed randomization
   srand( GetTickCount( ) );

   Dialog dialog;
   Graphics graphics;
   Corona_Engine engine;

   // initialize timer
   Timer::Create( );

   g_next_frame = Timer::m_total;

   // initialize dialog
   if ( !dialog.Create( ) )
   {
      MessageBox( NULL, "Failed to initialize window", "Error", MB_ICONERROR | MB_OK );

      return 0;
   }

   if ( !graphics.Create( dialog.Get_HWND( ), &g_options.resolution ) )
   {
      MessageBox( NULL, "Failed to initialize engine", "Error", MB_ICONERROR | MB_OK );

      return 0;
   }

   if ( !engine.Create( &dialog, &graphics ) )
   {
      MessageBox( NULL, "Failed to initialize engine", "Error", MB_ICONERROR | MB_OK );

      return 0;
   }

   dialog.Set_Engine_Pointer( &engine );

   MSG msg;

   for ( ; ; )
   {
      while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
      {
         if ( !TranslateAccelerator( dialog.Get_HWND( ), dialog.Get_HACCEL( ), &msg ) )
         {
            TranslateMessage ( &msg );
            DispatchMessage  ( &msg );
         }
         
         if ( WM_QUIT == msg.message )
         {
            engine.Destroy( );

            return (uint32) msg.wParam;
         }
      }
      
      Timer::Update( );
      
      // update game tick
      float old_frame = g_next_frame;

      while( g_next_frame < Timer::m_total )
      {
         // update
         engine.Update( );

         g_next_frame += CORONA_DELTA_UPDATE_FRAMES;
      }

      // don't render if we didn't update.
      if ( old_frame != g_next_frame )
      {
         // render
         engine.Render( );
         engine.Flip( );
      }
   }

   return 0;
}
