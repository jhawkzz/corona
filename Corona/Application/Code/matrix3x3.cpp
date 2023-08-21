
#include "corona_headers.h"

Matrix3x3::Matrix3x3( void )
{
}

Matrix3x3::~Matrix3x3( )
{
}

void Matrix_Multiply_M3( Matrix3x3 * p_left, Matrix3x3 * p_right, Matrix3x3 * p_result )
{
   // first transpose the matrix since it's orthonormal. Then dot the rows.
   Matrix3x3 trans_right = Matrix_Transpose_M3( p_right );
   Matrix3x3 result;

   // 0X
   result.m_row[ 0 ].x = Dot_Product_V3( &p_left->m_row[ 0 ], &trans_right.m_row[ 0 ] );

   // 0Y
   result.m_row[ 0 ].y = Dot_Product_V3( &p_left->m_row[ 0 ], &trans_right.m_row[ 1 ] );

   // 0Z
   result.m_row[ 0 ].z = Dot_Product_V3( &p_left->m_row[ 0 ], &trans_right.m_row[ 2 ] );


   // 1X
   result.m_row[ 1 ].x = Dot_Product_V3( &p_left->m_row[ 1 ], &trans_right.m_row[ 0 ] );

   // 1Y
   result.m_row[ 1 ].y = Dot_Product_V3( &p_left->m_row[ 1 ], &trans_right.m_row[ 1 ] );

   // 1Z
   result.m_row[ 1 ].z = Dot_Product_V3( &p_left->m_row[ 1 ], &trans_right.m_row[ 2 ] );


   // 2X
   result.m_row[ 2 ].x = Dot_Product_V3( &p_left->m_row[ 2 ], &trans_right.m_row[ 0 ] );

   // 2Y
   result.m_row[ 2 ].y = Dot_Product_V3( &p_left->m_row[ 2 ], &trans_right.m_row[ 1 ] );

   // 2Z
   result.m_row[ 2 ].z = Dot_Product_V3( &p_left->m_row[ 2 ], &trans_right.m_row[ 2 ] );

   // copy at the end so the in and out and be the same.
   *p_result = result;
}

Matrix3x3::Matrix3x3( Vector3 & x, Vector3 & y, Vector3 & z )
{
   m_row[ 0 ] = x;
   m_row[ 1 ] = y;
   m_row[ 2 ] = z;
}

void Matrix3x3::operator =  ( Matrix3x3 & rhs )
{
   m_row[ 0 ] = rhs.m_row[ 0 ];
   m_row[ 1 ] = rhs.m_row[ 1 ];
   m_row[ 2 ] = rhs.m_row[ 2 ];
}
