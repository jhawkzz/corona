
#ifndef GRAPHICS_H_
#define GRAPHICS_H_

class Graphics
{

   public:
                          Graphics          ( void );
                          ~Graphics         ( );

            BOOL          Create               ( HWND hwnd, Resolution * p_resolution );
            
            HDC           Get_Back_Buffer_HDC  ( void );
            uint32    *   Get_Back_Buffer_Bits ( void );

            // for skycube
            HDC           Get_Sky_Buffer_HDC   ( void );
            uint32    *   Get_Sky_Buffer_Bits  ( void );


            uint32        Get_Graphics_Size    ( void );
            void          Destroy              ( void );

   private:

            HDC           m_back_buffer_hdc;
            HBITMAP       m_back_buffer_hbitmap;
            HGDIOBJ       m_back_buffer_hdc_hgdiobj;
            uint32     *  m_back_buffer_bits;

            //for skycube
            HDC           m_sky_buffer_hdc;
            HBITMAP       m_sky_buffer_hbitmap;
            HGDIOBJ       m_sky_buffer_hdc_hgdiobj;
            uint32     *  m_sky_buffer_bits;

            uint32        m_graphics_size;
};

#endif

            