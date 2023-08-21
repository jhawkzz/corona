
#ifndef VECTOR4_H_
#define VECTOR4_H_

class __declspec(align(16)) Vector4
{

   public:

            Vector4    ( void );
            Vector4    ( float new_x, float new_y, float new_z, float new_w );
            ~Vector4   ( );

   void     Set        ( float new_x, float new_y, float new_z, float new_w );
   
   void     operator = ( const Vector4 & rhs );
   void     operator = ( const float value );

   union
   {
      struct
      {
         float x;
         float y;
         float z;
         float w;
      };

      struct
      {
         float a;
         float r;
         float g;
         float b;
      };

      #ifdef USE_SSE

         __m128 m128;

      #else

         float f[ 4 ];

      #endif
   };
};

//ADD
inline Vector4 Add_V4( const Vector4 * a, const Vector4 * b )
{
   // w is ignored here, so if it's different we assert
   _ASSERTE( a->w == b->w );

   Vector4 result;

   #ifdef USE_SSE

      result.m128 = _mm_add_ps( a->m128, b->m128 );

   #else
      
      result.x = a->x + b->x;
      result.y = a->y + b->y;
      result.z = a->z + b->z;
 
   #endif

   result.w = a->w;

   return result;
}

inline Vector4 Add_W_V4( const Vector4 * a, const Vector4 * b )
{
   Vector4 result;

   #ifdef USE_SSE

      result.m128 = _mm_add_ps( a->m128, b->m128 );

   #else
      
      result.x = a->x + b->x;
      result.y = a->y + b->y;
      result.z = a->z + b->z;
      result.w = a->w + b->w;
 
   #endif

   return result;
}

//SUBTRACT
inline Vector4 Subtract_V4( const Vector4 * a, const Vector4 * b )
{
   // w is ignored here, so if it's different we assert
   _ASSERTE( a->w == b->w );

   Vector4 result;

   #ifdef USE_SSE

      result.m128 = _mm_sub_ps( a->m128, b->m128 );

   #else
      
      result.x = a->x - b->x;
      result.y = a->y - b->y;
      result.z = a->z - b->z;
 
   #endif

   result.w = a->w;

   return result;
}

inline Vector4 Subtract_W_V4( const Vector4 * a, const Vector4 * b )
{
   Vector4 result;

   #ifdef USE_SSE

      result.m128 = _mm_sub_ps( a->m128, b->m128 );

   #else
      
      result.x = a->x - b->x;
      result.y = a->y - b->y;
      result.z = a->z - b->z;
      result.w = a->w - b->w;
 
   #endif

   return result;
}

// SCALE
inline Vector4 Scale_V4( const Vector4 * a, const float scalar )
{
   Vector4 result;

   #ifdef USE_SSE

      __m128 scalar_128 = _mm_load_ps1( &scalar );

      result.m128 = _mm_mul_ps( a->m128, scalar_128 );
   
   #else

      result.x = a->x * scalar;
      result.y = a->y * scalar;
      result.z = a->z * scalar;

   #endif

   result.w = a->w;

   return result;
}

inline Vector4 Scale_W_V4( const Vector4 * a, const float scalar )
{
   Vector4 result;

   #ifdef USE_SSE

      __m128 scalar_128 = _mm_load_ps1( &scalar );

      result.m128 = _mm_mul_ps( a->m128, scalar_128 );
   
   #else

      result.x = a->x * scalar;
      result.y = a->y * scalar;
      result.z = a->z * scalar;
      result.w = a->w * scalar;

   #endif

   return result;
}

//DP
inline float Dot_Product_V4( const Vector4 * a, const Vector4 * b )
{
   #ifdef USE_SSE

      Vector4 result;
      
      result.m128 = _mm_mul_ps( a->m128, b->m128 );

      return result.x + result.y + result.z + result.w;
   
   #else

      return ( ( a->x * b->x ) + ( a->y * b->y ) + ( a->z * b->z ) + ( a->w * b->w ) );
   
   #endif
}

//CP
inline Vector4 Cross_Product_V4( const Vector4 * u, const Vector4 * v )
{
   _ASSERTE( u->w == v->w );

   Vector4 cross_product;

   cross_product.x =  ( ( u->y * v->z ) - ( u->z * v->y ) );
   cross_product.y = -( ( u->x * v->z ) - ( u->z * v->x ) );
   cross_product.z =  ( ( u->x * v->y ) - ( u->y * v->x ) );
   cross_product.w = u->w;

   return cross_product;
}

//MAGNITUDE
inline float Magnitude_V4( const Vector4 * a )
{
   Vector4 result;

   _asm
   {
      mov   eax , a

      movss xmm0, [eax]Vector4.x
      movss xmm1, [eax]Vector4.y
      movss xmm2, [eax]Vector4.z

      mulss xmm0, xmm0 //x * x
      mulss xmm1, xmm1 //y * y
      mulss xmm2, xmm2 //z * z

      addss xmm0, xmm1 //x + y
      addss xmm0, xmm2 //x/y + z

      sqrtss xmm0, xmm0

      movss result, xmm0
   }

   _ASSERTE( result.x > 0.0001f );

   return result.x;
}

//NORMALIZE
inline void Normalize_V4( const Vector4 * a, Vector4 * vec_out )
{
   __asm 
   {
      mov   eax,  a
      mov   ebx,  vec_out

      // move the vector's components into sse registers
      movss xmm0, [eax]Vector4.x
      movss xmm1, [eax]Vector4.y
      movss xmm2, [eax]Vector4.z
      movss xmm3, [eax]Vector4.w

      mulss xmm0, xmm0 //square x
      mulss xmm1, xmm1 //square y
      mulss xmm2, xmm2 //square z

      addss xmm0, xmm1 //x + y
      addss xmm2, xmm0 //x/y + z

      // move the vector's into the second set of sse registers
      movss xmm4, [eax]Vector4.x 
      movss xmm5, [eax]Vector4.y
      movss xmm6, [eax]Vector4.z

      rsqrtss xmm2, xmm2 //xmm2 contains the dot product, now square root it and do 1 /
   
      mulss  xmm4, xmm2 //xmm2 now has 1 / sqrt( dp )
      mulss  xmm5, xmm2
      mulss  xmm6, xmm2

      movss  [ebx]Vector4.x, xmm4
      movss  [ebx]Vector4.y, xmm5
      movss  [ebx]Vector4.z, xmm6
      movss  [ebx]Vector4.w, xmm3 //xmm3 stored w the whole time
   }

   /*float magnitude = Magnitude_V4( a );

   _ASSERTE( magnitude );

   float inverse_magnitude = 1 / magnitude;

   result.x = a->x * inverse_magnitude;
   result.y = a->y * inverse_magnitude;
   result.z = a->z * inverse_magnitude;
   result.w = a->w;*/
}

inline void Normalize_W_V4( const Vector4 * a, Vector4 * vec_out )
{
   __asm 
   {
      mov   eax,  a
      mov   ebx,  vec_out

      // move the vector's components into sse registers
      movss xmm0, [eax]Vector4.x
      movss xmm1, [eax]Vector4.y
      movss xmm2, [eax]Vector4.z
      movss xmm3, [eax]Vector4.w

      mulss xmm0, xmm0 //square x
      mulss xmm1, xmm1 //square y
      mulss xmm2, xmm2 //square z
      mulss xmm3, xmm3 //square w

      addss xmm0, xmm1 //x + y
      addss xmm2, xmm0 //x/y + z
      addss xmm2, xmm3 //x/y/z + w

      // move the vector's into the second set of sse registers
      movss xmm4, [eax]Vector4.x 
      movss xmm5, [eax]Vector4.y
      movss xmm6, [eax]Vector4.z
      movss xmm7, [eax]Vector4.w

      rsqrtss xmm2, xmm2 //xmm2 contains the dot product, now square root it and do 1 /
   
      mulss  xmm4, xmm2 //xmm2 now has 1 / sqrt( dp )
      mulss  xmm5, xmm2
      mulss  xmm6, xmm2
      mulss  xmm7, xmm2

      movss  [ebx]Vector4.x, xmm4
      movss  [ebx]Vector4.y, xmm5
      movss  [ebx]Vector4.z, xmm6
      movss  [ebx]Vector4.w, xmm7
   }

   /*float magnitude = Magnitude_V4( a );

   _ASSERTE( magnitude );

   float inverse_magnitude = 1 / magnitude;

   result.x = a->x * inverse_magnitude;
   result.y = a->y * inverse_magnitude;
   result.z = a->z * inverse_magnitude;
   result.w = a->w * inverse_magnitude;

   return result;*/
}

#endif
