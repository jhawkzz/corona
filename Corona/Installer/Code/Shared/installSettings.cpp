
#include "installSettings.h"
#include "registry_class.h"

//We need this so we can map the resources to file names under InstallMap
#include "..\\Installer\\resource.h" 

//SETUP INSTALLER HERE
#include <Setupapi.h>
#include <psapi.h>
#include <Aclapi.h>

// Put the file types that could be registered here
char g_FileTypeList[ ][ MAX_PATH ] = 
{
   ""
};

void InstallSettings::CreateInstallMaps( void )
{
   //SET UNINSTALL DETAILS
   strcpy( m_DisplayIconProgram, "Corona.exe" ); // set the program whose icon you wish to display in the Add/Remove entry
   //END UNINSTALL DETAILS

   // add your mapped install files here.
   strcpy( m_pInstallFileMap[ IDR_RT_RCDATA1 ], "Corona.exe"   );
   strcpy( m_pInstallFileMap[ IDR_RT_RCDATA2 ], "uninstall.exe" );
   strcpy( m_pInstallFileMap[ IDR_RT_RCDATA3 ], "pain.pcx"  );
   strcpy( m_pInstallFileMap[ IDR_RT_RCDATA4 ], "skin.pcx" );
   strcpy( m_pInstallFileMap[ IDR_RT_RCDATA5 ], "tris.md2"   );

   // add your mapped dll files here

   // add your required install packages here

   // add your shortcuts here
   strcpy( m_pShortcutMap[ 0 ].shortcutName  , "Corona"  );
   strcpy( m_pShortcutMap[ 0 ].shortcutTarget, "Corona.exe" );
   strcpy( m_pShortcutMap[ 0 ].shortcutSubDir, "Corona"  );
   m_pShortcutMap[ 0 ].shortcutType = CSIDL_PROGRAMS;

   strcpy( m_pShortcutMap[ 1 ].shortcutName  , "Corona"  );
   strcpy( m_pShortcutMap[ 1 ].shortcutTarget, "Corona.exe" );
   m_pShortcutMap[ 1 ].shortcutType = CSIDL_DESKTOPDIRECTORY;

   memset( m_RegistryValuesByInstaller, 0, sizeof( m_RegistryValuesByInstaller ) );
   memset( m_RegistryKeysToRemove     , 0, sizeof( m_RegistryKeysToRemove      ) );
   memset( m_RegistryValuesToRemove   , 0, sizeof( m_RegistryValuesToRemove    ) );
   memset( m_FileTypeAssociationValue , 0, sizeof( m_FileTypeAssociationValue  ) );
   memset( m_ExtraFilesToRemove       , 0, sizeof( m_ExtraFilesToRemove        ) );

   // add registry values here that the installer should add/remove

   // add registry keys that your app will create, which need to be removed, here

   // add registry values that your app will create, which need to be removed, here

   // put the values used to claim file associations here

   // put the backup value for file type restoration

   // put files your program will create, that need to be cleaned up, right here
}

BOOL InstallSettings::AppSpecificInstall( void )
{
   // if you need to do custom installation stuff after files are installed, do it here.
   return TRUE;
}

void InstallSettings::AppSpecificUninstall( void )
{
}
//END SETUP INSTALLER

InstallSettings::InstallSettings( void )
{
   m_EulaAgreed            = FALSE;
   m_InstallDesktopIcons   = TRUE;
   m_InstallStartMenuItems = TRUE;
   m_IsAppAlreadyInstalled = FALSE;
   m_NeedsRestart          = FALSE;
   m_InstallDir[ 0 ]       = 0;

   // allocate memory for the install and DLL registration maps.
   m_pInstallFileMap      = (char **) new char *[ MAX_MAP_ENTRIES ];
   m_pDLLRegisterMap      = (char **) new char *[ MAX_MAP_ENTRIES ];
   m_pInstallPackageMap   = (char **) new char *[ MAX_MAP_ENTRIES ];
   m_pInstallPackageIDMap = (char **) new char *[ MAX_MAP_ENTRIES ];
   
   m_pInstallFileMapData      = new char[ MAX_PATH * MAX_MAP_ENTRIES ];
   m_pDLLRegisterMapData      = new char[ MAX_PATH * MAX_MAP_ENTRIES ];
   m_pInstallPackageMapData   = new char[ MAX_PATH * MAX_MAP_ENTRIES ];
   m_pInstallPackageIDMapData = new char[ MAX_PATH * MAX_MAP_ENTRIES ];

   memset( m_pInstallFileMapData     , 0, MAX_PATH * MAX_MAP_ENTRIES );
   memset( m_pDLLRegisterMapData     , 0, MAX_PATH * MAX_MAP_ENTRIES );
   memset( m_pInstallPackageMapData  , 0, MAX_PATH * MAX_MAP_ENTRIES );
   memset( m_pInstallPackageIDMapData, 0, MAX_PATH * MAX_MAP_ENTRIES );
   
   char *pCurrFileMapPos      = m_pInstallFileMapData;
   char *pCurrDLLMapPos       = m_pDLLRegisterMapData;
   char *pCurrPackageMapPos   = m_pInstallPackageMapData;
   char *pCurrPackageIDMapPos = m_pInstallPackageIDMapData;

   uint32 i;
   for ( i = 0; i < MAX_MAP_ENTRIES; i++ )
   {
      m_pInstallFileMap[ i ]      = pCurrFileMapPos;
      m_pDLLRegisterMap[ i ]      = pCurrDLLMapPos;
      m_pInstallPackageMap[ i ]   = pCurrPackageMapPos;
      m_pInstallPackageIDMap[ i ] = pCurrPackageIDMapPos;
      
      pCurrFileMapPos      += MAX_PATH;
      pCurrDLLMapPos       += MAX_PATH;
      pCurrPackageMapPos   += MAX_PATH;
      pCurrPackageIDMapPos += MAX_PATH;
   }

   m_pShortcutMap = new ShortcutObject[ MAX_MAP_ENTRIES ];
   memset( m_pShortcutMap, 0, sizeof( ShortcutObject ) * MAX_MAP_ENTRIES );
   
   CreateInstallMaps( );
}

InstallSettings::~InstallSettings( )
{
   delete [] m_pInstallFileMapData;
   delete [] m_pDLLRegisterMapData;
   delete [] m_pInstallPackageMapData;
   delete [] m_pInstallPackageIDMapData;

   delete [] m_pInstallFileMap;
   delete [] m_pDLLRegisterMap;
   delete [] m_pInstallPackageMap;
   delete [] m_pInstallPackageIDMap;

   delete [] m_pShortcutMap;

   Destroy( );
}

BOOL InstallSettings::Create( void )
{
   sprintf( m_UninstallRegPath, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s", IV_UNINSTALL_REG_KEY );

   // get the default install path.
   Registry_Class registry( HKEY_LOCAL_MACHINE );

   // but let's see if they HAVE installed it, and if so, suggest that path.
   if ( registry.Open_Key( m_UninstallRegPath, KEY_ALL_ACCESS, FALSE ) )
   {
      m_IsAppAlreadyInstalled = TRUE;
      m_InstallDesktopIcons   = registry.Get_Value_Data_Int( "InstallDesktopIcons"  , 1 );
      m_InstallStartMenuItems = registry.Get_Value_Data_Int( "InstallStartMenuItems", 1 );
   }

   GetDefaultInstallDir( );

   return TRUE;
}

void InstallSettings::Destroy( void )
{
}

void InstallSettings::GetDefaultInstallDir( void )
{
   // get the default install path.
   Registry_Class registry( HKEY_LOCAL_MACHINE );
   BOOL alreadyInstalled = FALSE; //assume it is NOT installed

   // but let's see if they HAVE installed it, and if so, suggest that path.
   if ( registry.Open_Key( m_UninstallRegPath, KEY_ALL_ACCESS, FALSE ) )
   {
      // if this returns false, then alreadyInstalled remains FALSE, and we'll create a default one below.
      alreadyInstalled = registry.Get_Value_Data_String( "InstallDir", m_InstallDir, MAX_PATH - 1, "" );
   }

   // not installed (or missing the InstallPath reg), so let's suggest their default program files folder.
   if ( !alreadyInstalled )
   {
      char programFilesDir[ MAX_PATH ];

      // get their default program files path.
      SHGetFolderPath( NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, programFilesDir );

      // append as much of the install folder as we can
      uint32 programDirLen = strlen( programFilesDir );
      uint32 availableLen  = MAX_PATH - programDirLen;

      _snprintf( m_InstallDir, MAX_PATH - 1, "%s\\%s", programFilesDir, IV_DEFAULT_PROGRAM_FOLDER );
   }
}

BOOL InstallSettings::RestartSystem( void )
{
   BOOL success        = FALSE;
   HANDLE hToken       = NULL;
   char *pPrevPriv     = NULL;

   // ---- adjust token privileges ----
   do
   {
      // get the token for this process
      if ( !OpenProcessToken( GetCurrentProcess( ), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken ) ) break;

      // find the ownership privilege
      LUID luid;
      if ( !LookupPrivilegeValue(0, SE_SHUTDOWN_NAME, &luid) ) break;

      // adjust our privileges so ownership is enabled
      TOKEN_PRIVILEGES priv;
      priv.PrivilegeCount             = 1;
      priv.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;
      priv.Privileges[ 0 ].Luid       = luid;

      // get the current privilege info so we can restore it. We call twice to first get the buffer size, then the info
      DWORD prevPrivSize;
      AdjustTokenPrivileges( hToken, TokenPrivileges, &priv, 1, (TOKEN_PRIVILEGES *)&prevPrivSize, &prevPrivSize );
      
      DWORD error = GetLastError( );
      if ( error != ERROR_INSUFFICIENT_BUFFER ) break;

      // // adjust our privileges so ownership is enabled, and get the old state
      pPrevPriv = new char[ prevPrivSize ];
      if ( !AdjustTokenPrivileges( hToken, FALSE, &priv, prevPrivSize, (TOKEN_PRIVILEGES *)pPrevPriv, &prevPrivSize ) ) break;

      // ---- perform system restart ----
      InitiateSystemShutdown( NULL, NULL, 0, FALSE, TRUE );

      success = TRUE;
   }
   while( 0 );

   if ( pPrevPriv )
   {
      // restore previous privileges before deleting
      AdjustTokenPrivileges( hToken, FALSE, (TOKEN_PRIVILEGES *)pPrevPriv, 0, NULL, 0 );
      delete [] pPrevPriv;
   }

   return success;
}

BOOL InstallSettings::IsWindowsVersionSupported( void )
{
   BOOL operatingSystemSupported = FALSE;

   do
   {
      // ---- get the windows version info ----
      OSVERSIONINFOEX osVersionInfoEx = { 0 };
      osVersionInfoEx.dwOSVersionInfoSize = sizeof( osVersionInfoEx );

      GetVersionEx( (OSVERSIONINFO *)&osVersionInfoEx );

      /*// Display the OS version info to the user, for debugging
      char sixtyFourBitStr[ MAX_PATH ];
      sprintf( sixtyFourBitStr, "64Bit: %s", IsWindows64Bit( ) ? "Yes" : "No" );

      char versionInfo[ MAX_PATH ];
      sprintf( versionInfo, "dwMajor: %d\n"
                             "dwMinor: %d\n"
                             "dwBuildNumber: %d\n"
                             "dwPlatformId: %d\n"
                             "wServicePackMajor: %d\n"
                             "wServicePackMinor: %d\n"
                             "%s"                     , osVersionInfoEx.dwMajorVersion, 
                                                        osVersionInfoEx.dwMinorVersion, 
                                                        osVersionInfoEx.dwBuildNumber, 
                                                        osVersionInfoEx.dwPlatformId, 
                                                        osVersionInfoEx.wServicePackMajor, 
                                                        osVersionInfoEx.wServicePackMinor,
                                                        sixtyFourBitStr );

      MessageBox( NULL, versionInfo, "Version", MB_OK );*/

      operatingSystemSupported = TRUE;
   }
   while( 0 );

   return operatingSystemSupported;
}

BOOL InstallSettings::IsWindowsXP( void )
{
   // ---- get the windows version info ----
   OSVERSIONINFOEX osVersionInfoEx = { 0 };
   osVersionInfoEx.dwOSVersionInfoSize = sizeof( osVersionInfoEx );

   GetVersionEx( (OSVERSIONINFO *)&osVersionInfoEx );

   if ( 5 == osVersionInfoEx.dwMajorVersion && 1 == osVersionInfoEx.dwMinorVersion )
   {
      return TRUE;
   }

   return FALSE;
}

BOOL InstallSettings::IsWindowsVistaSP0( void )
{
   // ---- get the windows version info ----
   OSVERSIONINFOEX osVersionInfoEx = { 0 };
   osVersionInfoEx.dwOSVersionInfoSize = sizeof( osVersionInfoEx );

   GetVersionEx( (OSVERSIONINFO *)&osVersionInfoEx );

   if ( 6 == osVersionInfoEx.dwMajorVersion && 0 == osVersionInfoEx.dwMinorVersion && 0 == osVersionInfoEx.wServicePackMajor )
   {
      return TRUE;
   }

   return FALSE;
}

BOOL InstallSettings::IsWindowsVistaSP1Plus( void )
{
   // ---- get the windows version info ----
   OSVERSIONINFOEX osVersionInfoEx = { 0 };
   osVersionInfoEx.dwOSVersionInfoSize = sizeof( osVersionInfoEx );

   GetVersionEx( (OSVERSIONINFO *)&osVersionInfoEx );

   if ( 6 == osVersionInfoEx.dwMajorVersion && 0 == osVersionInfoEx.dwMinorVersion && osVersionInfoEx.wServicePackMajor > 0 )
   {
      return TRUE;
   }

   return FALSE;
}

BOOL InstallSettings::IsWindows7( void )
{
   // ---- get the windows version info ----
   OSVERSIONINFOEX osVersionInfoEx = { 0 };
   osVersionInfoEx.dwOSVersionInfoSize = sizeof( osVersionInfoEx );

   GetVersionEx( (OSVERSIONINFO *)&osVersionInfoEx );

   if ( 6 == osVersionInfoEx.dwMajorVersion && 1 == osVersionInfoEx.dwMinorVersion )
   {
      return TRUE;
   }

   return FALSE;
}

BOOL InstallSettings::IsWindows64Bit( void )
{
   BOOL is64BitOS = FALSE;

   // We check if the OS is 64 Bit
   typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

   LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle("kernel32"),"IsWow64Process" );

   if (NULL != fnIsWow64Process)
   {
      fnIsWow64Process( GetCurrentProcess(), &is64BitOS );
   }

   return is64BitOS;
}
