
#ifndef Vector3_H_
#define Vector3_H_

class __declspec(align(16)) Vector3
{

   public:

            Vector3    ( void );
            Vector3    ( float new_x, float new_y, float new_z );
            ~Vector3   ( );

   void     Set        ( float new_x, float new_y, float new_z);
   
   void     operator = ( const Vector3 & rhs );
   void     operator = ( const float value );

   union
   {
      struct
      {
         float x;
         float y;
         float z;
      };

      struct
      {
         float r;
         float g;
         float b;
      };

     // #ifdef USE_SSE

         __m128 m128;

      //#else

        // float f[ 4 ];

      //#endif
   };
};

//ADD
inline Vector3 Add_V3( const Vector3 * a, const Vector3 * b )
{
   Vector3 result;

   #ifdef USE_SSE

      result.m128 = _mm_add_ps( a->m128, b->m128 );

   #else
      
      result.x = a->x + b->x;
      result.y = a->y + b->y;
      result.z = a->z + b->z;
 
   #endif

   return result;
}

//SUBTRACT
inline Vector3 Subtract_V3( const Vector3 * a, const Vector3 * b )
{
   Vector3 result;

   #ifdef USE_SSE

      result.m128 = _mm_sub_ps( a->m128, b->m128 );

   #else
      
      result.x = a->x - b->x;
      result.y = a->y - b->y;
      result.z = a->z - b->z;
 
   #endif

   return result;
}

// SCALE
inline Vector3 Scale_V3( const Vector3 * a, const float scalar )
{
   Vector3 result;

   #ifdef USE_SSE

      __m128 scalar_128 = _mm_load_ps1( &scalar );

      result.m128 = _mm_mul_ps( a->m128, scalar_128 );
   
   #else

      result.x = a->x * scalar;
      result.y = a->y * scalar;
      result.z = a->z * scalar;

   #endif

   return result;
}

//DP
inline float Dot_Product_V3( const Vector3 * a, const Vector3 * b )
{
   Vector3 result;

   _asm
   {
      mov   eax, a
      mov   ecx, b

      movss   xmm0, [eax]Vector3.x
      movss   xmm1, [eax]Vector3.y
      movss   xmm2, [eax]Vector3.z

      movss   xmm4, [ecx]Vector3.x
      movss   xmm5, [ecx]Vector3.y
      movss   xmm6, [ecx]Vector3.z

      mulss   xmm0, xmm4 //x*x
      mulss   xmm1, xmm5 //y*y
      mulss   xmm2, xmm6 //z*z

      addss   xmm0, xmm1 //x + y
      addss   xmm0, xmm2 //x/y + z
      
      movss   result, xmm0
   }

   return result.x;

   //return ( ( a->x * b->x ) + ( a->y * b->y ) + ( a->z * b->z ) );
}

//CP
inline Vector3 Cross_Product_V3( const Vector3 * u, const Vector3 * v )
{
   Vector3 cross_product;

   cross_product.x =  ( ( u->y * v->z ) - ( u->z * v->y ) );
   cross_product.y = -( ( u->x * v->z ) - ( u->z * v->x ) );
   cross_product.z =  ( ( u->x * v->y ) - ( u->y * v->x ) );

   return cross_product;
}

//MAGNITUDE
inline float Magnitude_V3( const Vector3 * a )
{
   Vector3 result;

   _asm
   {
      mov   eax , a

      movss xmm0, [eax]Vector3.x
      movss xmm1, [eax]Vector3.y
      movss xmm2, [eax]Vector3.z

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

  /* float magnitude = Dot_Product_V3( a, a );

   magnitude = sqrt( magnitude );
   

   if ( fabsf( magnitude ) < 0.0001f )
   {
      return 1.00f;
   }

   return magnitude;*/
}

//NORMALIZE
inline void Normalize_V3( const Vector3 * a, Vector3 * b )
{
   __asm {
      mov   eax,  a
      mov   ebx,  b

      // move the vector's components into sse registers
      movss xmm0, [eax]Vector3.x
      movss xmm1, [eax]Vector3.y
      movss xmm2, [eax]Vector3.z

      mulss xmm0, xmm0 //square x
      mulss xmm1, xmm1 //square y
      mulss xmm2, xmm2 //square z

      addss xmm0, xmm1 //x + y
      addss xmm2, xmm0 //x/y + z

      // move the vector's into the second set of sse registers
      movss xmm4, [eax]Vector3.x 
      movss xmm5, [eax]Vector3.y
      movss xmm6, [eax]Vector3.z

      rsqrtss xmm2, xmm2 //xmm2 contains the dot product, now square root it and do 1 /
   
      mulss  xmm4, xmm2 //xmm2 now has 1 / sqrt( dp )
      mulss  xmm5, xmm2
      mulss  xmm6, xmm2

      movss  [ebx]Vector3.x, xmm4
      movss  [ebx]Vector3.y, xmm5
      movss  [ebx]Vector3.z, xmm6
   }

   /*float magnitude = Magnitude_V3( a );

   _ASSERTE( magnitude );

   float inverse_magnitude = 1 / magnitude;

   b->x = a->x * inverse_magnitude;
   b->y = a->y * inverse_magnitude;
   b->z = a->z * inverse_magnitude;*/
}

inline void Normalize_Quad_V3( const Vector3 * aligned_x, const Vector3 * aligned_y, const Vector3 * aligned_z, Vector3 * out_x, Vector3 * out_y, Vector3 * out_z )
{
   __asm {
      mov   eax,  aligned_x
      mov   ebx,  aligned_y
      mov   ecx,  aligned_z

      // move the vector's components into sse registers
      movaps xmm0, [eax]Vector3.x
      movaps xmm1, [ebx]Vector3.x
      movaps xmm2, [ecx]Vector3.x

      mulps xmm0, xmm0 //square all xs
      mulps xmm1, xmm1 //square all ys
      mulps xmm2, xmm2 //square all zs

      addps xmm0, xmm1 //add all xs + ys
      addps xmm2, xmm0 //xs/ys + zs

      // move the vector's into the second set of sse registers
      movaps xmm4, [eax]Vector3.x
      movaps xmm5, [ebx]Vector3.x
      movaps xmm6, [ecx]Vector3.x

      rsqrtps xmm2, xmm2 //xmm2 contains the dot product, now square root it and do 1 /
   
      mulps  xmm4, xmm2 //xmm2 now has 1 / sqrt( dp )
      mulps  xmm5, xmm2
      mulps  xmm6, xmm2

      mov    eax, out_x
      mov    ebx, out_y
      mov    ecx, out_z

      movaps  [eax]Vector3.x, xmm4
      movaps  [ebx]Vector3.x, xmm5
      movaps  [ecx]Vector3.x, xmm6
   }

   /*float magnitude = Magnitude_V3( a );

   _ASSERTE( magnitude );

   float inverse_magnitude = 1 / magnitude;

   b->x = a->x * inverse_magnitude;
   b->y = a->y * inverse_magnitude;
   b->z = a->z * inverse_magnitude;*/
}

#endif
