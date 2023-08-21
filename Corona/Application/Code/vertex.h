
#ifndef VERTEX_H_
#define VERTEX_H_

typedef struct _Color
{
   float r;
   float g;
   float b;

   inline void Set( float red, float green, float blue ) { r = red; g = green; b = blue; }
}
Color;

typedef struct _UV
{
   float u;
   float v;
}
UV;

class __declspec(align(16)) Vertex
{
   public:

      Vector4  m_position;
      Vector3  m_normal;
      UV       m_uv;
};

class __declspec(align(16)) Cached_Vertex
{
   public:

      Vector4 m_position;
      Vector3 m_normal;
      UV      m_uv;
      float   m_world_z;
      //float   m_light_amount;
};

#endif
