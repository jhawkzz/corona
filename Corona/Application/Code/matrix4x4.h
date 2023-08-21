
#ifndef MATRIX4x4_H_
#define MATRIX4x4_H_

class __declspec(align(16)) Matrix4x4
{
   public:

               Matrix4x4   ( void );
               Matrix4x4   ( Vector4 & x, Vector4 & y, Vector4 & z, Vector4 & w );
               ~Matrix4x4  ( );

       void    operator =  ( Matrix4x4 & rhs );
 
       Vector4 m_row[ 4 ];
};

void Make_Invert_M4( Matrix4x4 * p_original, Matrix4x4 * p_inverse ); //defined in the cpp
void Multiply_M4   ( Matrix4x4 * p_left, Matrix4x4 * p_right, Matrix4x4 * p_result );

static inline Matrix3x3 Extract_Rotation( Matrix4x4 * p_matrix4x4 )
{
   Matrix3x3 rotation;

   //X
   rotation.m_row[ 0 ].x = p_matrix4x4->m_row[ 0 ].x;
   rotation.m_row[ 0 ].y = p_matrix4x4->m_row[ 0 ].y;
   rotation.m_row[ 0 ].z = p_matrix4x4->m_row[ 0 ].z;

   //Y
   rotation.m_row[ 1 ].x = p_matrix4x4->m_row[ 1 ].x;
   rotation.m_row[ 1 ].y = p_matrix4x4->m_row[ 1 ].y;
   rotation.m_row[ 1 ].z = p_matrix4x4->m_row[ 1 ].z;

   //Z
   rotation.m_row[ 2 ].x = p_matrix4x4->m_row[ 2 ].x;
   rotation.m_row[ 2 ].y = p_matrix4x4->m_row[ 2 ].y;
   rotation.m_row[ 2 ].z = p_matrix4x4->m_row[ 2 ].z;

   return rotation;
}

static inline Matrix4x4 Transpose_M4( Matrix4x4 * p_matrix )
{
   Matrix4x4 transposed;

   //X
   transposed.m_row[ 0 ].x = p_matrix->m_row[ 0 ].x;
   transposed.m_row[ 0 ].y = p_matrix->m_row[ 1 ].x;
   transposed.m_row[ 0 ].z = p_matrix->m_row[ 2 ].x;
   transposed.m_row[ 0 ].w = p_matrix->m_row[ 3 ].x;

   //Y
   transposed.m_row[ 1 ].x = p_matrix->m_row[ 0 ].y;
   transposed.m_row[ 1 ].y = p_matrix->m_row[ 1 ].y;
   transposed.m_row[ 1 ].z = p_matrix->m_row[ 2 ].y;
   transposed.m_row[ 1 ].w = p_matrix->m_row[ 3 ].y;

   //Z
   transposed.m_row[ 2 ].x = p_matrix->m_row[ 0 ].z;
   transposed.m_row[ 2 ].y = p_matrix->m_row[ 1 ].z;
   transposed.m_row[ 2 ].z = p_matrix->m_row[ 2 ].z;
   transposed.m_row[ 2 ].w = p_matrix->m_row[ 3 ].z;

   //W
   transposed.m_row[ 3 ].x = p_matrix->m_row[ 0 ].w;
   transposed.m_row[ 3 ].y = p_matrix->m_row[ 1 ].w;
   transposed.m_row[ 3 ].z = p_matrix->m_row[ 2 ].w;
   transposed.m_row[ 3 ].w = p_matrix->m_row[ 3 ].w;

   return transposed;
}

static inline Vector4 Multiply_M4( Vector4 * p_vector, Matrix4x4 * p_matrix )
{
   Vector4 result;
   Matrix4x4 trans_matrix = Transpose_M4( p_matrix );
   
   // 0X
   result.x = Dot_Product_V4( p_vector, &trans_matrix.m_row[ 0 ] );

   // 0Y
   result.y = Dot_Product_V4( p_vector, &trans_matrix.m_row[ 1 ] );

   // 0Z
   result.z = Dot_Product_V4( p_vector, &trans_matrix.m_row[ 2 ] );

   // 0W
   result.w = Dot_Product_V4( p_vector, &trans_matrix.m_row[ 3 ] );

   return result;
}

static inline Matrix4x4 Make_Identity_M4( void )
{
   Matrix4x4 matrix;

   matrix.m_row[ 0 ].Set( 1, 0, 0, 0 );
   matrix.m_row[ 1 ].Set( 0, 1, 0, 0 );
   matrix.m_row[ 2 ].Set( 0, 0, 1, 0 );
   matrix.m_row[ 3 ].Set( 0, 0, 0, 1 );

   return matrix;
}

static inline Matrix4x4 Make_X_Rotation_M4( float rotation )
{
   Matrix4x4 matrix;

   //X
   matrix.m_row[ 0 ].Set( 1, 0, 0 , 0 );
   
   //Y
   matrix.m_row[ 1 ].Set( 0, cosf( rotation ), sinf( rotation ), 0 );
   
   //Z
   matrix.m_row[ 2 ].Set( 0, -sinf( rotation ),  cosf( rotation ), 0 );
   
   //W
   matrix.m_row[ 3 ].Set( 0, 0, 0, 1 );

   return matrix;
}

static inline Matrix4x4 Make_Y_Rotation_M4( float rotation )
{
   Matrix4x4 matrix;

   //X
   matrix.m_row[ 0 ].Set( cosf( rotation ), 0, -sinf( rotation ), 0 );
   
   //Y
   matrix.m_row[ 1 ].Set( 0, 1, 0, 0 );
   
   //Z
   matrix.m_row[ 2 ].Set( sinf( rotation ), 0,  cosf( rotation ), 0 );
   
   //W
   matrix.m_row[ 3 ].Set( 0, 0, 0, 1 );

   return matrix;
}

static inline Matrix4x4 Make_Z_Rotation_M4( float rotation )
{
   Matrix4x4 matrix;

   //X
   matrix.m_row[ 0 ].Set( cosf( rotation ), sinf( rotation ), 0, 0 );
   
   //Y
   matrix.m_row[ 1 ].Set( -sinf( rotation ), cosf( rotation ), 0, 0 );
   
   //Z
   matrix.m_row[ 2 ].Set( 0, 0, 1, 0 );
   
   //W
   matrix.m_row[ 3 ].Set( 0, 0, 0, 1 );

   return matrix;
}

#endif
