
#ifndef REGISTRY_CLASS
#define REGISTRY_CLASS

#include <windows.h>

class Registry_Class
{

   public:

            Registry_Class        ( void );
            ~Registry_Class       ( );
 
      BOOL  Open_Key              ( char * key );
      void  Close_Key             ( void );
      BOOL  Get_Value_Data_String ( char * value, char * data, DWORD buffer_length, char * default_data );
      DWORD Get_Value_Data_Int    ( char * value, uint32 default_data, BOOL * success = NULL );
      BOOL  Set_Value_Data_String ( char * value, const char * data );
      BOOL  Set_Value_Data_Int    ( char * value, DWORD data );
      BOOL  Delete_Value          ( char * value );

   private:
      BOOL  Create_Key            ( char * key );

      HKEY  m_key_handle;

};

#endif
