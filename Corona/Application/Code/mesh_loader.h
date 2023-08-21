
#ifndef MESH_LOADER_H_
#define MESH_LOADER_H_

typedef enum _MODEL
{
   MODEL_UNKNOWN,
   MODEL_MD2,
   MODEL_VGD,
   MODEL_COUNT
}
MODEL;

#define TEXTURE_SKY_CUBE   ( 0x1 << 0 )

class Mesh_Loader
{

   public:

            BOOL  Load_Mesh     ( char * filename, Mesh * p_mesh, uint32 texture_flags );
   private:
            MODEL Get_Model_Type( char * filename );

            BOOL  Convert_MD2   ( char * filename, Mesh * p_mesh, uint32 texture_flags );
            BOOL  Convert_VGD   ( char * filename, Mesh * p_mesh, uint32 texture_flags );
};

#endif
