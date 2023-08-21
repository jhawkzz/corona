
#ifndef VGD_H_
#define VGD_H_

//Vagabond importing stuff
typedef struct _VGDMeshHeader
{
   union
   {  
      struct
      {
         uint32 flags;
	      uint32 vertices;
	      uint32 materials;
         uint32 textures;
	      uint32 faces;
         uint32 collisionObjects;
                
         uint32 boundingVolumeOffset;
         uint32 verticesOffset;
         uint32 indicesOffset;
         uint32 bumpVerticesOffset;
         uint32 textureOffset;
         uint32 materialOffset;
         uint32 physicsOffset;
         uint32 vertexWeightOffset;
         uint32 collisionOffset;
         uint32 ecoWeightOffset;
      };

      BYTE force128[ 128 ];
   };

}VGDMeshHeader;


typedef struct _Vag_Vertex
{
	float pos[ 4 ];
   float normal[ 4 ];
	float uv[ 4 ];

}Vag_Vertex;

typedef struct _Vag_Face
{
   int vertex_index[ 3 ];
}
Vag_Face;

typedef struct _Vag_Texture
{
   short * p_data;
   int     tex_size;
   int     width;
   int     height;
}
Vag_Texture;

typedef struct _Vag_Model
{
   VGDMeshHeader  header;
   Vag_Vertex   * p_vert;
   Vag_Face     * p_face;
   Vag_Texture    texture;
}
Vag_Model;

class VGD
{
   public:

      BOOL      Load  ( char * filename );
      void      UnLoad( void );
      
      Vag_Model m_model;

   private:
      BOOL      Load_Texture( char * texture_name );
};

#endif
