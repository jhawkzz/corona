
//#include "registry_class.h"
#include "corona_headers.h"

Registry_Class::Registry_Class( void )
{
   m_key_handle = NULL;
}

Registry_Class::~Registry_Class( )
{
   Close_Key( );
}

BOOL Registry_Class::Open_Key( char * key )
{
   BOOL success = TRUE;

   Close_Key( );

   if ( RegOpenKeyEx( HKEY_CURRENT_USER, key, 0, KEY_ALL_ACCESS, &m_key_handle ) != ERROR_SUCCESS )
   {
      // attemp to create the key
      if ( !Create_Key( key ) )
      {
         success = FALSE;
      }
   }

   return success;
}

void Registry_Class::Close_Key( void )
{
   if ( m_key_handle )
   {
      RegCloseKey( m_key_handle );

      m_key_handle = NULL;
   }
}

BOOL Registry_Class::Get_Value_Data_String( char * value, char * data, DWORD buffer_length, char * default_data )
{
   BOOL success = FALSE;

   do
   {
      if ( !m_key_handle ) break;

      DWORD type = REG_SZ;

      if ( RegQueryValueEx( m_key_handle, value, 0, &type, (BYTE *)data, &buffer_length ) != ERROR_SUCCESS ) break;

      success = TRUE;
   }
   while( 0 );

   if ( !success )
   {
      strcpy( data, default_data );
   }

   return success;
}

DWORD Registry_Class::Get_Value_Data_Int( char * value, uint32 default_data, BOOL * success )
{
   BOOL result = FALSE;

   DWORD data = default_data;

   do
   {
      if ( !m_key_handle ) break;

      DWORD type = REG_DWORD;
      DWORD buffer_size = sizeof( data );

      if ( RegQueryValueEx( m_key_handle, value, 0, &type, (BYTE *)&data, &buffer_size ) != ERROR_SUCCESS ) break;

      result = TRUE;
   }
   while( 0 );

   if ( !result && success )
   {
      *success = result;
   }

   return data;
}

BOOL Registry_Class::Set_Value_Data_String( char * value, const char * data )
{
   BOOL success = FALSE;

   do
   {
      if ( !m_key_handle ) break;

      DWORD type = REG_SZ;
      if ( RegSetValueEx( m_key_handle, value, 0, REG_SZ, (BYTE *) data, (DWORD) strlen( data ) ) != ERROR_SUCCESS ) break;

      success = TRUE;
   }
   while( 0 );

   return success;
}

BOOL Registry_Class::Set_Value_Data_Int( char * value, DWORD data )
{
   BOOL success = FALSE;

   do
   {
      if ( !m_key_handle ) break;

      if ( RegSetValueEx( m_key_handle, value, 0, REG_DWORD, (BYTE *) &data, (DWORD) sizeof( data ) ) != ERROR_SUCCESS ) break;

      success = TRUE;
   }
   while( 0 );

   return success;
}

BOOL Registry_Class::Delete_Value( char * value )
{
   BOOL success = FALSE;

   do
   {
      if ( !m_key_handle ) break;

      if ( RegDeleteValue( m_key_handle, value ) != ERROR_SUCCESS ) break;
   }
   while( 0 );

   success = TRUE;

   return success;
}

BOOL Registry_Class::Create_Key( char * key )
{
   if ( RegCreateKeyEx( HKEY_CURRENT_USER, key, 0, NULL, REG_OPTION_NON_VOLATILE, NULL, NULL, &m_key_handle, NULL ) != ERROR_SUCCESS ) 
   {
      return FALSE;
   }

   return TRUE;
}