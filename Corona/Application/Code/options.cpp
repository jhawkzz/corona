
#include "corona_headers.h"

enum TREE_LEAFS
{
   TREE_LEAF_DISPLAY,
   TREE_LEAF_COUNT
};

char * m_tree_leaf_name[ ] = 
{
   "Display",
};

uint32 m_tree_leaf_map[ ] = 
{
   0,
};

char * m_resolution_string[ ] = 
{
   "640x480x16",
   "800x600x16",
   "1024x768x16",
};

Resolution m_resolution[ ] = 
{
   {  640, 480, 16 },
   {  800, 600, 16 },
   { 1024, 768, 16 },
};

// an array of all the items (check boxes, edit fields, etc)
uint32 m_option_ids[ OPTIONS_RESOLUTION_COUNT ] = 
{
   // display
   ID_STATIC_DISPLAY_RESOLUTION, 
   ID_COMBO_DISPLAY_RESOLUTION,
   ID_CHECK_DISPLAY_WIREFRAME,    //3
};

Options_Window::Options_Window( void )
{
   m_hwnd = NULL;

   memset( &g_options, 0, sizeof( g_options ) );
}

Options_Window::~Options_Window( )
{
   Release_Everything( );
}

void Options_Window::Initialize( HWND parent_hwnd )
{
   Load( );

   // Send off an initial change message so the dialog can initialize itself correctly
   DWORD changes = OPTIONS_RESOLUTION;

   PostMessage( parent_hwnd, WM_OPTIONS_CHANGED, changes, 0 );
}

void Options_Window::Open( HWND parent_hwnd, OPTIONS_PAGES option_page )
{
   Release_Everything( );

   m_hwnd = CreateDialogParam( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_PREFERENCES ), parent_hwnd, (DLGPROC) Callback, (LPARAM) this );

   EnableWindow( parent_hwnd, FALSE );

   // go to the current page
   m_current_page = option_page;
   Change_Page( );
}

LRESULT Options_Window::Callback( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   Options_Window * p_options = (Options_Window *)GetWindowLong( hwnd, GWL_USERDATA );

   if ( p_options || WM_INITDIALOG == uMsg )
   {
      switch( uMsg )
      {
         HANDLE_DLGMSG( hwnd, WM_INITDIALOG, Options_Window::Options_WM_INITDIALOG );
         HANDLE_DLGMSG( hwnd, WM_COMMAND   , p_options->Options_WM_COMMAND         );
         HANDLE_DLGMSG( hwnd, WM_CLOSE     , p_options->Options_WM_CLOSE           );
         HANDLE_DLGMSG( hwnd, WM_DESTROY   , p_options->Options_WM_DESTROY         );
         
         case WM_NOTIFY: return p_options->Options_WM_NOTIFY( hwnd, (int)wParam, (LPNMHDR)lParam );
      }
   }

   return FALSE;
}

BOOL Options_Window::Options_WM_INITDIALOG( HWND hwnd, HWND hwndFocus, LPARAM lParam )
{
   Options_Window * p_options = (Options_Window *)lParam;

   SetWindowLong( hwnd, GWL_USERDATA, (LONG) p_options );

   return p_options->Options_WM_INITDIALOG( hwnd );
}

BOOL Options_Window::Options_WM_INITDIALOG( HWND hwnd )
{  
   SetWindowPos( hwnd, NULL, 0, 0, OPTIONS_WIDTH, OPTIONS_HEIGHT, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );

   // initialize everything
   m_hwnd = hwnd; ///Need this so the functions correctly assign stuff.
   Load( );
   Initialize_Menu( );
   Initialize_Controls( );
   Assign_Options_To_Controls( );

   SetFocus( GetDlgItem( hwnd, ID_TREE_OPTIONS_MENU ) );
   ShowWindow( hwnd, SW_SHOWNORMAL );

   return TRUE;
}

BOOL Options_Window::Options_WM_COMMAND( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
   switch( id )
   {
      case IDCANCEL:
      {
         PostMessage( hwnd, WM_CLOSE, 0, 0 );

         break;
      }

      case IDOK:
      {
         Options old_options = g_options;

         Assign_Controls_To_Options( );
         
         // determine which catergories have changed
         DWORD changed_options = Get_Changed_Options( &old_options, &g_options );

         if ( changed_options )
         {
            PostMessage( GetParent( hwnd ), WM_OPTIONS_CHANGED, (WPARAM)changed_options, 0 );
         }

         Save( );

         PostMessage( hwnd, WM_CLOSE, 0, 0 );

         break;
      }

      /*case ID_EDIT_UNDOHISTORY_COUNT:
      {
         if ( EN_UPDATE == HIWORD( wParam ) )
         {
            // don't allow 0
            int undoCount = GetDlgItemInt( hwnd, ID_EDIT_UNDOHISTORY_COUNT, NULL, FALSE );
            
            if ( undoCount <= 0 )
            {
               SetDlgItemInt( hwnd, ID_EDIT_UNDOHISTORY_COUNT, 1, FALSE );
            }

            // update the estimated RAM usage.
            char string[ MAX_PATH ];
            float undoEstRam = options->GetEstUndoRam( );

            sprintf( string, "%.2f", undoEstRam );

            SetDlgItemText( hwnd, ID_EDIT_UNDOHISTORY_EST_RAM, string );
         }

         break;
      }*/
   }

   return TRUE;
}

BOOL Options_Window::Options_WM_NOTIFY( HWND hwnd, int idCtrl, LPNMHDR pnmh )
{
   switch( idCtrl )
   {
      case ID_TREE_OPTIONS_MENU:
      {
         if ( TVN_SELCHANGED == pnmh->code )
         {
            LPNMTREEVIEW notify_message = (LPNMTREEVIEW) pnmh;
            m_current_page              = (OPTIONS_PAGES) notify_message->itemNew.lParam;

            Change_Page( );
         }

         break;
      }
   }

   return TRUE;
}

BOOL Options_Window::Options_WM_CLOSE( HWND hwnd )
{
   Release_Everything( );

   return TRUE;
}

BOOL Options_Window::Options_WM_DESTROY( HWND hwnd )
{
   SetForegroundWindow( GetParent( hwnd ) );

   return TRUE;
}

BOOL Options_Window::Load( void )
{   
   // warning - do not use any Option members in here other than the Options struct, because
   // users can call this function without having init'd options
   Registry_Class registry;

   if ( !registry.Open_Key( CORONA_REGISTRY_KEY ) ) return FALSE;

   // display
   g_options.resolution = m_resolution[ registry.Get_Value_Data_Int( "Resolution", 1 ) ];
   g_options.wire_frame = registry.Get_Value_Data_Int( "Wireframe", 0 );

   return TRUE;
}

BOOL Options_Window::Save( void )
{
   Registry_Class registry;

   if ( !registry.Open_Key( CORONA_REGISTRY_KEY ) ) return FALSE;

   // display
   registry.Set_Value_Data_Int( "Resolution", Get_Resolution_Index( &g_options.resolution ) );
   registry.Set_Value_Data_Int( "Wireframe" , g_options.wire_frame );

   return TRUE;
}

void Options_Window::Release_Everything( void )
{
   if ( m_hwnd )
   {
      EnableWindow( GetParent( m_hwnd ), TRUE );

      DestroyWindow( m_hwnd );

      m_hwnd = NULL;
   }
}

void Options_Window::Initialize_Menu( void )
{
   // create treeview menu
   HWND tree_handle = GetDlgItem( m_hwnd, ID_TREE_OPTIONS_MENU );

   TVINSERTSTRUCT tvInsertItem;
   memset( &tvInsertItem, 0, sizeof( tvInsertItem ) );

   uint32 i;
   for ( i = 0; i < TREE_LEAF_COUNT; i++ )
   {
      if ( !m_tree_leaf_map[ i ] ) tvInsertItem.hParent = TVI_ROOT;
   
      tvInsertItem.hInsertAfter    = TVI_LAST;
      tvInsertItem.item.mask       = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
      tvInsertItem.item.stateMask  = TVIS_EXPANDED;
      tvInsertItem.item.state      = TVIS_EXPANDED;
      tvInsertItem.item.pszText    = (char *)m_tree_leaf_name[ i ];
      tvInsertItem.item.lParam     = i;

      HTREEITEM lastItem = TreeView_InsertItem( tree_handle, &tvInsertItem );

      if ( !m_tree_leaf_map[ i ] ) tvInsertItem.hParent = lastItem;
   }

   m_current_page = OPTIONS_PAGE_DISPLAY;

   Change_Page( );
}

void Options_Window::Initialize_Controls( void )
{
   uint32 i;

   // set up combo boxes
   for ( i = 0; i < OPTIONS_RESOLUTION_COUNT; i++ )
   {      
      SendMessage( GetDlgItem( m_hwnd, ID_COMBO_DISPLAY_RESOLUTION ), CB_ADDSTRING, 0, (LPARAM) m_resolution_string[ i ] );
   }
}

void Options_Window::Change_Page( void )
{
   // hide all options
   uint32 i;
   for ( i = 0; i < OPTIONS_COUNT; i++ )
   {
      ShowWindow( GetDlgItem( m_hwnd, m_option_ids[ i ] ), SW_HIDE );
   }

   switch( m_current_page )
   {
      case OPTIONS_PAGE_DISPLAY:
      {
         SetWindowText( GetDlgItem( m_hwnd, ID_GROUPBOX_OPTIONS_BORDER ), "Display" );
         
         SetWindowPos( GetDlgItem( m_hwnd, ID_STATIC_DISPLAY_RESOLUTION  ), NULL, 130, 50 , 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER );
         SetWindowPos( GetDlgItem( m_hwnd, ID_COMBO_DISPLAY_RESOLUTION   ), NULL, 130, 90 , 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER );

         SetWindowPos( GetDlgItem( m_hwnd, ID_CHECK_DISPLAY_WIREFRAME    ), NULL, 130, 130, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER );

         break;
      }
   }
}

void Options_Window::Assign_Options_To_Controls( void )
{
   // display
   SendMessage( GetDlgItem( m_hwnd, ID_COMBO_DISPLAY_RESOLUTION ), CB_SETCURSEL, Get_Resolution_Index( &g_options.resolution ), 0 );
   CheckDlgButton( m_hwnd, ID_CHECK_DISPLAY_WIREFRAME, g_options.wire_frame );
}

void Options_Window::Assign_Controls_To_Options( void )
{
   // display
   g_options.resolution = m_resolution[ SendMessage( GetDlgItem( m_hwnd, ID_COMBO_DISPLAY_RESOLUTION ), CB_GETCURSEL, 0, 0 ) ];
   g_options.wire_frame = IsDlgButtonChecked( m_hwnd, ID_CHECK_DISPLAY_WIREFRAME );
}

DWORD Options_Window::Get_Changed_Options( Options * old_options, Options * new_options )
{
   DWORD changes = 0;

   // display
   if ( ( old_options->resolution.width  != new_options->resolution.width  ||
          old_options->resolution.height != new_options->resolution.height ||
          old_options->resolution.bits   != new_options->resolution.bits ) )
   {
      changes |= OPTIONS_RESOLUTION;
   }
   // not concerned with wireframe

   return changes;
}

uint32 Options_Window::Get_Resolution_Index( Resolution * p_resolution )
{
   uint32 i;
   for ( i = 0; i < OPTIONS_RESOLUTION_COUNT; i++ )
   {
      if ( ( p_resolution->width  != m_resolution[ i ].width  ||
             p_resolution->height != m_resolution[ i ].height ||
             p_resolution->bits   != m_resolution[ i ].bits ) )
      {
         continue;
      }

      break;
   }

   return i;
}
