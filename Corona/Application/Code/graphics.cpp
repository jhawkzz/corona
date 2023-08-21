
#include "corona_headers.h"

Graphics::Graphics( void )
{
   m_back_buffer_hdc          = NULL;
   m_back_buffer_hbitmap      = NULL;
   m_back_buffer_hdc_hgdiobj  = NULL;
   m_back_buffer_bits         = NULL;
   m_graphics_size            = 0;
}

Graphics::~Graphics( )
{
}

BOOL Graphics::Create( HWND hwnd, Resolution * p_resolution )
{
   BOOL success = FALSE;

   do
   {
      // get hdcs for the back buffer
      HDC main_hdc = GetDC( hwnd );

      m_back_buffer_hdc = CreateCompatibleDC( main_hdc );
      if ( !m_back_buffer_hdc ) break;

      m_sky_buffer_hdc = CreateCompatibleDC( main_hdc );
      if ( !m_sky_buffer_hdc ) break;


      // create back buffer
      BITMAPINFO bitmap_info;

      memset( &bitmap_info, 0, sizeof ( bitmap_info ) );

      bitmap_info.bmiHeader.biSize      = sizeof( bitmap_info );
      bitmap_info.bmiHeader.biPlanes    = 1;
      bitmap_info.bmiHeader.biWidth     = p_resolution->width;
      bitmap_info.bmiHeader.biHeight    = p_resolution->height;
      bitmap_info.bmiHeader.biBitCount  = p_resolution->bits;
      bitmap_info.bmiHeader.biSizeImage = ( ( p_resolution->width * ( p_resolution->bits / 8 ) ) * p_resolution->height );

      m_graphics_size = bitmap_info.bmiHeader.biSizeImage;

      m_back_buffer_hbitmap = CreateDIBSection( m_back_buffer_hdc, &bitmap_info, DIB_RGB_COLORS, ( void **) &m_back_buffer_bits, NULL, NULL );
      if ( !m_back_buffer_hbitmap ) break;

      m_back_buffer_hdc_hgdiobj = (HGDIOBJ) SelectObject( m_back_buffer_hdc, m_back_buffer_hbitmap );


      // create sky buffer (use the same bitmap_info)
      m_sky_buffer_hbitmap = CreateDIBSection( m_sky_buffer_hdc, &bitmap_info, DIB_RGB_COLORS, ( void **) &m_sky_buffer_bits, NULL, NULL );
      if ( !m_sky_buffer_hbitmap ) break;

      m_sky_buffer_hdc_hgdiobj = (HGDIOBJ) SelectObject( m_sky_buffer_hdc, m_sky_buffer_hbitmap );


      ReleaseDC( hwnd, main_hdc );

      success = TRUE;
   }
   while( 0 );

   return success;
}

HDC Graphics::Get_Back_Buffer_HDC( void )
{
   return m_back_buffer_hdc;
}

uint32 * Graphics::Get_Back_Buffer_Bits( void )
{
   return m_back_buffer_bits;
}

HDC Graphics::Get_Sky_Buffer_HDC( void )
{
   return m_sky_buffer_hdc;
}

uint32 * Graphics::Get_Sky_Buffer_Bits( void )
{
   return m_sky_buffer_bits;
}

uint32 Graphics::Get_Graphics_Size( void )
{
   return m_graphics_size;
}

void Graphics::Destroy( void )
{
   // clean up everything
   if ( m_back_buffer_hdc )
   {
      // restore original object in the dc
      SelectObject( m_back_buffer_hdc, m_back_buffer_hdc_hgdiobj );

      DeleteDC( m_back_buffer_hdc );

      m_back_buffer_hdc         = NULL;
      m_back_buffer_hdc_hgdiobj = NULL;
   }

   if ( m_back_buffer_hbitmap )
   {
      DeleteObject( m_back_buffer_hbitmap );

      m_back_buffer_hbitmap = NULL;
   }

   // clean up skycube
   if ( m_sky_buffer_hdc )
   {
      // restore original object in the dc
      SelectObject( m_sky_buffer_hdc, m_sky_buffer_hdc_hgdiobj );

      DeleteDC( m_sky_buffer_hdc );

      m_sky_buffer_hdc         = NULL;
      m_sky_buffer_hdc_hgdiobj = NULL;
   }

   if ( m_sky_buffer_hbitmap )
   {
      DeleteObject( m_sky_buffer_hbitmap );

      m_sky_buffer_hbitmap = NULL;
   }
}
