
#ifndef OPTIONS_WINDOW_H_
#define OPTIONS_WINDOW_H_

#define WM_OPTIONS_CHANGED          (WM_APP + 1)

#define OPTIONS_COUNT                ( 3) // amount of check boxes, edit fields, etc
#define OPTIONS_WIDTH               (440)
#define OPTIONS_HEIGHT              (265)

#define OPTIONS_RESOLUTION          (0x00000001)

#define OPTIONS_RESOLUTION_COUNT    (3)

enum OPTIONS_PAGES
{
   OPTIONS_PAGE_DISPLAY,
   OPTIONS_PAGE_COUNT
};

typedef struct _Resolution
{
   uint32 width;
   uint32 height;
   uint32 bits;
}
Resolution;

typedef struct _Options
{
   // display
   Resolution resolution;
   BOOL       wire_frame;
}
Options;

class Options_Window
{

   public:

                       Options_Window            ( void );
                       ~Options_Window           ( );

      void             Initialize                ( HWND parent_hwnd );
      void             Open                      ( HWND parent_hwnd, OPTIONS_PAGES option_page );
      BOOL             Load                      ( void );
      BOOL             Save                      ( void );
   
   private:

static   LRESULT       Callback                  ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static   BOOL          Options_WM_INITDIALOG     ( HWND hwnd, HWND hwndFocus, LPARAM lParam );
         BOOL          Options_WM_INITDIALOG     ( HWND hwnd );
         BOOL          Options_WM_COMMAND        ( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
         BOOL          Options_WM_NOTIFY         ( HWND hwnd, int idCtrl, LPNMHDR pnmh );
         BOOL          Options_WM_CLOSE          ( HWND hwnd );
         BOOL          Options_WM_DESTROY        ( HWND hwnd );
         

         void          Release_Everything        ( void );
         void          Change_Page               ( void );
         void          Initialize_Menu           ( void );
         void          Initialize_Controls       ( void );
         void          Assign_Options_To_Controls( void );
         void          Assign_Controls_To_Options( void );
         DWORD         Get_Changed_Options       ( Options * old_options, Options * new_options );

         uint32        Get_Resolution_Index      ( Resolution * p_resolution );

         HWND          m_hwnd;
         OPTIONS_PAGES m_current_page;
};

#endif
