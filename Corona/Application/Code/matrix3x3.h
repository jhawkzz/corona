
#ifndef MATRIX3x3_H_
#define MATRIX3x3_H_

class Matrix3x3
{
   public:

               Matrix3x3   ( void );
               Matrix3x3   ( Vector3 & x, Vector3 & y, Vector3 & z );
               ~Matrix3x3  ( );

       void    operator =  ( Matrix3x3 & rhs );
 
       Vector3 m_row[ 3 ];
};

void Matrix_Multiply_M3( Matrix3x3 * p_left, Matrix3x3 * p_right, Matrix3x3 * p_result );

static inline Matrix3x3 Matrix_Transpose_M3( Matrix3x3 * p_matrix )
{
   Matrix3x3 transposed;

   //X
   transposed.m_row[ 0 ].x = p_matrix->m_row[ 0 ].x;
   transposed.m_row[ 0 ].y = p_matrix->m_row[ 1 ].x;
   transposed.m_row[ 0 ].z = p_matrix->m_row[ 2 ].x;

   //Y
   transposed.m_row[ 1 ].x = p_matrix->m_row[ 0 ].y;
   transposed.m_row[ 1 ].y = p_matrix->m_row[ 1 ].y;
   transposed.m_row[ 1 ].z = p_matrix->m_row[ 2 ].y;

   //Z
   transposed.m_row[ 2 ].x = p_matrix->m_row[ 0 ].z;
   transposed.m_row[ 2 ].y = p_matrix->m_row[ 1 ].z;
   transposed.m_row[ 2 ].z = p_matrix->m_row[ 2 ].z;

   return transposed;
}

static inline Vector3 Vector_Matrix_Multiply_M3( Vector3 * p_vector, Matrix3x3 * p_matrix )
{
   Vector3 result;
   
   Matrix3x3 trans_matrix = Matrix_Transpose_M3( p_matrix );

   //X
   result.x = Dot_Product_V3( p_vector, &trans_matrix.m_row[ 0 ] );

   //Y
   result.y = Dot_Product_V3( p_vector, &trans_matrix.m_row[ 1 ] );

   //Z
   result.z = Dot_Product_V3( p_vector, &trans_matrix.m_row[ 2 ] );

   return result;
}

static inline Matrix3x3 Make_Identity_M3( void )
{
   Matrix3x3 matrix;

   matrix.m_row[ 0 ].Set( 1, 0, 0 );
   matrix.m_row[ 1 ].Set( 0, 1, 0 );
   matrix.m_row[ 2 ].Set( 0, 0, 1 );

   return matrix;
}

static inline Matrix3x3 Make_X_Rotation_M3( float rotation )
{
   Matrix3x3 matrix;

   //X
   matrix.m_row[ 0 ].Set( 1, 0, 0 );
   
   //Y
   matrix.m_row[ 1 ].Set( 0, cosf( rotation ), sinf( rotation ) );
   
   //Z
   matrix.m_row[ 2 ].Set( 0, -sinf( rotation ),  cosf( rotation ) );

   return matrix;
}

static inline Matrix3x3 Make_Y_Rotation_M3( float rotation )
{
   Matrix3x3 matrix;

   //X
   matrix.m_row[ 0 ].Set( cosf( rotation ), 0, -sinf( rotation ) );
   
   //Y
   matrix.m_row[ 1 ].Set( 0, 1, 0 );
   
   //Z
   matrix.m_row[ 2 ].Set( sinf( rotation ), 0,  cosf( rotation ) );

   return matrix;
}

static inline Matrix3x3 Make_Z_Rotation_M3( float rotation )
{
   Matrix3x3 matrix;

   //X
   matrix.m_row[ 0 ].Set( cosf( rotation ), sinf( rotation ), 0 );
   
   //Y
   matrix.m_row[ 1 ].Set( -sinf( rotation ), cosf( rotation ), 0 );
   
   //Z
   matrix.m_row[ 2 ].Set( 0, 0, 1 );

   return matrix;
}

#endif
