
#ifndef DIALOG_H_
#define DIALOG_H_

#define CORONA_DIALOG_CLASS_NAME ("Corona Dialog")

class Corona_Engine;

class Dialog
{

   public:

                           Dialog                   ( void );
                          ~Dialog                   ( );

         BOOL             Create                    ( void );
         void             Destroy                   ( void );

         void             Set_Engine_Pointer        ( Corona_Engine * p_engine ) { m_p_engine = p_engine; }

inline   HWND             Get_HWND                  ( void ) { return m_hwnd;   }
inline   HACCEL           Get_HACCEL                ( void ) { return m_haccel; }

   private:
static   LRESULT CALLBACK Dialog_Callback           ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static   BOOL             Dialog_WM_CREATE          ( HWND hwnd, LPCREATESTRUCT lpCreateStruct );
         BOOL             Dialog_WM_CREATE          ( HWND hwnd );
         BOOL             Dialog_WM_COMMAND         ( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
         BOOL             Dialog_WM_OPTIONS_CHANGED ( HWND hwnd, DWORD changedOptions );
         BOOL             Dialog_WM_CLOSE           ( HWND hwnd );
         BOOL             Dialog_WM_DESTROY         ( HWND hwnd );

         BOOL             Save_Settings             ( HWND hwnd );
         BOOL             Load_Settings             ( HWND hwnd );
         void             Open_File_Dialog          ( char * title, char * filter, char * default_ext, char * buffer, int buffer_length );
         void             Get_Non_Client_Dimensions ( uint32 * pNonClientWidth, uint32 * pNonClientHeight );
         void             Update_Resolution         ( void );

         HWND             m_hwnd;
         HMENU            m_hmenu;
         HACCEL           m_haccel;
         Options_Window   m_options_window;
         Corona_Engine *  m_p_engine;
};

#endif
