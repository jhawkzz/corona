
#include "corona_headers.h"

Dialog::Dialog( void )
{
}

Dialog::~Dialog( )
{
   Destroy( );
}

BOOL Dialog::Create( void )
{
   BOOL success = FALSE;

   do
   {
      WNDCLASSEX wnd_class_ex = { 0 };

      wnd_class_ex.cbSize        = sizeof( wnd_class_ex );
      wnd_class_ex.hCursor       = LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) );
      wnd_class_ex.hInstance     = GetModuleHandle( NULL );
      wnd_class_ex.lpfnWndProc   = Dialog_Callback;
      wnd_class_ex.lpszClassName = CORONA_DIALOG_CLASS_NAME;

      if ( !RegisterClassEx( &wnd_class_ex ) ) break;

      m_hwnd = CreateWindowEx( 0, CORONA_DIALOG_CLASS_NAME, CORONA_CAPTION, WS_OVERLAPPEDWINDOW  & ~(WS_MAXIMIZEBOX | WS_SIZEBOX), 0, 0, 0, 0, NULL, NULL, GetModuleHandle( NULL ), this );

      //Load the menu
      m_hmenu = LoadMenu( GetModuleHandle( NULL ), MAKEINTRESOURCE( ID_PROGRAM_MENU ) );
      SetMenu( m_hwnd, m_hmenu );

      //Load the accelerator
      m_haccel = LoadAccelerators( GetModuleHandle( NULL ), MAKEINTRESOURCE( ID_PROGRAM_ACCELERATOR ) );

      if ( !m_hwnd ) break;

      success = TRUE;
   }
   while( 0 );

   return success;
}

void Dialog::Destroy( void )
{
   DestroyMenu  ( m_hmenu );
   DestroyWindow( m_hwnd  );

   m_hmenu = NULL;
   m_hwnd  = NULL;
}

LRESULT CALLBACK Dialog::Dialog_Callback( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   Dialog * p_dialog = (Dialog *) GetWindowLong( hwnd, GWL_USERDATA );

   if ( p_dialog || WM_CREATE == uMsg )
   {
      switch( uMsg )
      {
         HANDLE_MSG( hwnd, WM_CREATE   , Dialog::Dialog_WM_CREATE      );
         HANDLE_MSG( hwnd, WM_COMMAND  , p_dialog->Dialog_WM_COMMAND   );
         HANDLE_MSG( hwnd, WM_CLOSE    , p_dialog->Dialog_WM_CLOSE     );
         HANDLE_MSG( hwnd, WM_DESTROY  , p_dialog->Dialog_WM_DESTROY   );

         case WM_OPTIONS_CHANGED: return p_dialog->Dialog_WM_OPTIONS_CHANGED( hwnd, (DWORD)wParam );
      }
   }

   return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

BOOL Dialog::Dialog_WM_CREATE( HWND hwnd, LPCREATESTRUCT lpCreateStruct )
{
   Dialog * p_dialog = (Dialog *)lpCreateStruct->lpCreateParams;

   SetWindowLong( hwnd, GWL_USERDATA, (LONG) p_dialog );

   return p_dialog->Dialog_WM_CREATE( hwnd );
}

BOOL Dialog::Dialog_WM_CREATE( HWND hwnd )
{
   Load_Settings( hwnd );

   //Load the Icon
   HICON icon_handle = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_CORONA_ICON ) );
   SendMessage( hwnd, WM_SETICON, ICON_BIG  , (LPARAM) icon_handle );
   SendMessage( hwnd, WM_SETICON, ICON_SMALL, (LPARAM) icon_handle );

   //Init the options
   m_options_window.Initialize( hwnd );

   ShowWindow( hwnd, SW_SHOWNORMAL );

   return TRUE;
}

BOOL Dialog::Dialog_WM_COMMAND( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
   switch( id )
   {
      case ID_FILE_LOAD_MODEL:
      {
         Mesh_Loader mesh_loader;

         char mesh_name[ MAX_PATH ];
         Open_File_Dialog( "Load Mesh", "All Models\0*.md2;*.vgd\0Quake II Model Format (*.md2)\0*.md2\0Vagabond Model Format (*.vgd)\0*.vgd\0All Files (*.*)\0*.*\0\0", ".md2", mesh_name, MAX_PATH );
         
         if ( mesh_name[ 0 ] )
         {
            mesh_loader.Load_Mesh( mesh_name, &m_p_engine->m_mesh, 0 );
         }
       
         break;
      }
      case ID_FILE_EXIT:
      {
         PostMessage( hwnd, WM_CLOSE, 0, 0 );

         break;
      }

      case ID_OPTIONS_DISPLAY:
      {
         m_options_window.Open( hwnd, OPTIONS_PAGE_DISPLAY );

         break;
      }
   }

   return TRUE;
}

BOOL Dialog::Dialog_WM_OPTIONS_CHANGED( HWND hwnd, DWORD changedOptions )
{
   if ( changedOptions & OPTIONS_RESOLUTION )
   {
      Update_Resolution( );
   }

   return TRUE;
}

BOOL Dialog::Dialog_WM_CLOSE( HWND hwnd )
{
   Destroy( );

   return TRUE;
}  

BOOL Dialog::Dialog_WM_DESTROY( HWND hwnd )
{
   Save_Settings( hwnd );

   PostQuitMessage( 0 );

   return TRUE;
}

BOOL Dialog::Save_Settings( HWND hwnd )
{
   BOOL success = FALSE;

   do
   {
      Registry_Class registry;

      if ( !registry.Open_Key( CORONA_REGISTRY_KEY ) ) break;

      RECT pos_rect;
      GetWindowRect( hwnd, &pos_rect );

      // save the x/y values of the window
      if ( !IsIconic( hwnd ) )
      {
         registry.Set_Value_Data_Int( "X", pos_rect.left );
         registry.Set_Value_Data_Int( "Y", pos_rect.top  );
      }

      m_options_window.Save( );

      success = TRUE;
   }
   while( 0 );

   return success;
}

BOOL Dialog::Load_Settings( HWND hwnd )
{
   BOOL success = FALSE;

   do
   {   
      Registry_Class registry;
      
      if ( !registry.Open_Key( CORONA_REGISTRY_KEY ) ) break;

      // load the x/y values of the window
      uint32 x_pos = registry.Get_Value_Data_Int( "X", 0 );
      uint32 y_pos = registry.Get_Value_Data_Int( "Y", 0 );

      SetWindowPos( hwnd, NULL, x_pos, y_pos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

      m_options_window.Load( );

      success = TRUE;
   }
   while( 0 );

   return success;
}

void Dialog::Open_File_Dialog( char * title, char * filter, char * default_ext, char * buffer, int buffer_length )
{
   buffer[ 0 ] = 0;

   OPENFILENAME open_file_name = { 0 };

   open_file_name.Flags       = OFN_EXPLORER;
   open_file_name.nMaxFile    = buffer_length;
   open_file_name.lpstrFile   = buffer;
   open_file_name.hInstance   = GetModuleHandle( NULL );
   open_file_name.hwndOwner   = m_hwnd;
   open_file_name.lpstrTitle  = title;
   open_file_name.lpstrDefExt = default_ext;
   open_file_name.lpstrFilter = filter;
   open_file_name.lStructSize = sizeof( open_file_name );

   GetOpenFileName( &open_file_name );
}

void Dialog::Get_Non_Client_Dimensions( uint32 * pNonClientWidth, uint32 * pNonClientHeight )
{
   // nonclient border widths / heights
   uint32 border_width   = GetSystemMetrics( SM_CXSIZEFRAME );
   uint32 border_height  = GetSystemMetrics( SM_CYSIZEFRAME );
   uint32 caption_height = GetSystemMetrics( SM_CYCAPTION );
   uint32 menuBarHeight  = GetSystemMetrics( SM_CYMENU );

   MENUBARINFO menuBarInfo;
   menuBarInfo.cbSize = sizeof( MENUBARINFO );

   if ( GetMenuBarInfo( m_hwnd, OBJID_MENU, 0, &menuBarInfo ) )
   {
      menuBarHeight = menuBarInfo.rcBar.bottom - menuBarInfo.rcBar.top; 
   }

   *pNonClientWidth  = border_width * 2;
   *pNonClientHeight = border_height + caption_height + menuBarHeight;
}

void Dialog::Update_Resolution( void )
{
   // resize the window due to non-client dimensions
   uint32 nc_width, nc_height;
   Get_Non_Client_Dimensions( &nc_width, &nc_height );

   SetWindowPos( m_hwnd, NULL, 0, 0, g_options.resolution.width, 
                                     g_options.resolution.height + nc_height, 
                                     SWP_NOMOVE | SWP_NOZORDER );

   // notify the engine of this change
   g_message_stack.Add_Message( "", 0, CM_RESOLUTION_CHANGED );
}
