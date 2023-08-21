
#ifndef RENDERER_H_
#define RENDERER_H_

#define MAX_VERTICES                (4096)
#define MAX_FACES                   (4096)
#define SPECULAR_INTENSITY          (12.00f)
#define PERSPECTIVE_CORRECTION_RATE (16)

enum TEXTURE_FILTER
{
   FILTER_NONE,
   FILTER_BILINEAR_16,
   FILTER_BILINEAR_32,
};

enum SPAN_EDGE
{
   LEFT_EDGE  = 0,
   RIGHT_EDGE = 1
};

typedef struct _Span
{
   uint32  x[ 2 ];
   float   z[ 2 ];
   float   world_z[ 2 ];

   UV      uv[ 2 ];
   Vector3 normal[ 2 ];
   Vector3 position[ 2 ]; //for specularity
   //float   light_amount[ 2 ];
}
Span;

typedef struct _Span_Buffer
{
	Span span_row[ CORONA_MAX_SCREEN_HEIGHT ];
}
Span_Buffer;

typedef struct _Screen_Position
{
   uint32  x;
   uint32  y;
   float   z;
   float   world_z;

   UV      uv;
   Vector3 normal;
   Vector3 position; //for specularity
   //float   light_amount;
}
Screen_Position;

typedef struct _Face_Map
{
   uint32 vertex_list[ 3 ];
   BOOL   culled;
}
Face_Map;

typedef struct __declspec(align(16)) _Face
{
   Cached_Vertex vertex[ 3 ];
}
Face;

typedef struct _Texture
{
   short * p_data;
   uint32  tex_size;
   uint32  width;
   uint32  height;
}
Texture;

typedef struct _Vertex_Const
{
   Matrix4x4 * p_world_matrix;
   Texture   * p_texture;
   uint32      texture_flags;
}
Vertex_Const;

class Renderer
{

   public:

                                 Renderer                   ( void );
                                 ~Renderer                  ( );

                  void           Create                     ( Camera * p_camera );
                  void           Destroy                    ( void );

                  void           Update_Keyboard            ( int controller_state );
                  void           Toggle_Specularity         ( BOOL specularity );
                  void           Toggle_Texture_Filtering   ( TEXTURE_FILTER filter );

                  void           Render                     ( Vertex * p_vertex, uint32 vertex_count, Face_Map * p_face, uint32 face_count, uint16 * buffer );
 
                  void           Begin                      ( void );
                  
static            Vertex_Const   m_vertex_constants;

   private:
      
                  // span buffer functions
                  void           Light_Verts                ( Vertex * p_vertex, uint32 vertex_count );
                  void           Fill_Vertex_Cache          ( Vertex * p_vertex, uint32 vertex_count );
                  
                  void           To_Screen_Space            ( Screen_Position * p_screen_pos, Face * p_face );
                  void           Sort_Positions_By_Y        ( Screen_Position ** p_screen_pos );

                  void           Fill_Span_Buffer_Sky_Cube  ( Face * p_face, uint32 face_count, uint16 * buffer );
                  void           Trace_Edge_Sky_Cube        ( Screen_Position * p_start, Screen_Position * p_end, SPAN_EDGE edge, BOOL last_edge );
                  void           Render_Span_Buffer_Sky_Cube( uint16 * buffer, uint32 start_y, uint32 end_y );

                  void           Fill_Span_Buffer_PC        ( Face * p_face, uint32 face_count, uint16 * buffer );
                  void           Trace_Edge_PC              ( Screen_Position * p_start, Screen_Position * p_end, SPAN_EDGE edge, BOOL last_edge );
                  void           Render_Span_Buffer_PC      ( uint16 * buffer, uint32 start_y, uint32 end_y );
         
         static   short          Nearest_Neighbor_Pixel     ( UV * p_uv, Vertex_Const * p_vertex_constants );
         static   short          Bilinear_Filter_16_Pixel   ( UV * p_uv, Vertex_Const * p_vertex_constants );
         static   short          Bilinear_Filter_32_Pixel   ( UV * p_uv, Vertex_Const * p_vertex_constants );

         static   float          Calculate_Specularity      ( Vector3 * pixel_normal, Vector3 * pixel_position, Vector3 * light_position, Vector3 * camera_position );
         static   float          Calculate_No_Specularity   ( Vector3 * pixel_normal, Vector3 * pixel_position, Vector3 * light_position, Vector3 * camera_position );

                  void           Clip_Mesh                  ( Cached_Vertex * p_vertex, Face_Map * p_face, uint32 face_count );
                  void           Clip_Left                  ( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count );
                  void           Clip_Right                 ( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count );
                  void           Clip_Top                   ( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count );
                  void           Clip_Bottom                ( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count );
                  void           Clip_Near                  ( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count );
                  void           Clip_Far                   ( Cached_Vertex * p_vertex_in, uint32 vertex_in_count, Cached_Vertex * p_clipped_vert, uint32 * p_clipped_vert_count );
                  void           Clip_Edge                  ( Cached_Vertex * p_vertex_a, Cached_Vertex * p_vertex_b, Cached_Vertex * p_clipped, float time );
                  void           Create_Triangle_Fan        ( Cached_Vertex * p_vertex, sint32 vertex_count, Face * p_face, uint32 * p_face_count );
         
         inline   void           Create_Vertex_List         ( Cached_Vertex ** p_vertex_list, Cached_Vertex * p_vertex, uint32 vert_count )
                  {
                     uint32 c;
                     for ( c = 0; c < vert_count; c++ )
                     {
                        p_vertex_list[ c ] = &p_vertex[ c ];
                     }

                     p_vertex_list[ c ] = &p_vertex[ 0 ];
                  }

static   inline   uint32      ftoi                ( float f )
                  {
	                  f = f;

	                  uint32 i;

	                  _asm
	                  {
		                  fld   dword ptr [ f ]
		                  fistp dword ptr [ i ]
	                  };

	                  return i;
                  }

                  Camera   *     m_p_camera;
static            float          m_z_buffer[ CORONA_MAX_SCREEN_WIDTH * CORONA_MAX_SCREEN_HEIGHT ];
static            Span_Buffer    m_span_buffer;
static            Cached_Vertex  m_vertex_cache[ MAX_VERTICES ];
static            Face           m_face_clip[ MAX_FACES * 2 ];
                  uint32         m_face_clip_count;
                  float          m_keydown_timer;

                  // function pointers for renderings
                  short         (*Filter_Function)   ( UV * p_uv, Vertex_Const * p_vertex_constants );
                  float         (*Specular_Function) ( Vector3 * pixel_normal, Vector3 * pixel_position, Vector3 * light_position, Vector3 * camera_position );

public:
                  BOOL           m_specularity;
                  TEXTURE_FILTER m_texture_filter;
};

#endif
