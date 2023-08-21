
#include "corona_headers.h"

void Make_Invert_M4( Matrix4x4 * p_original, Matrix4x4 * p_inverse )
{
   Matrix4x4 temp;

	float fa0 = p_original->m_row[ 0 ].x * p_original->m_row[ 1 ].y - p_original->m_row[ 0 ].y * p_original->m_row[ 1 ].x;
  	float fa1 = p_original->m_row[ 0 ].x * p_original->m_row[ 1 ].z - p_original->m_row[ 0 ].z * p_original->m_row[ 1 ].x;
  	float fa2 = p_original->m_row[ 0 ].x * p_original->m_row[ 1 ].w - p_original->m_row[ 0 ].w * p_original->m_row[ 1 ].x;
  	float fa3 = p_original->m_row[ 0 ].y * p_original->m_row[ 1 ].z - p_original->m_row[ 0 ].z * p_original->m_row[ 1 ].y;
  	float fa4 = p_original->m_row[ 0 ].y * p_original->m_row[ 1 ].w - p_original->m_row[ 0 ].w * p_original->m_row[ 1 ].y;
  	float fa5 = p_original->m_row[ 0 ].z * p_original->m_row[ 1 ].w - p_original->m_row[ 0 ].w * p_original->m_row[ 1 ].z;
  	float fb0 = p_original->m_row[ 2 ].x * p_original->m_row[ 3 ].y - p_original->m_row[ 2 ].y * p_original->m_row[ 3 ].x;
  	float fb1 = p_original->m_row[ 2 ].x * p_original->m_row[ 3 ].z - p_original->m_row[ 2 ].z * p_original->m_row[ 3 ].x;
 	float fb2 = p_original->m_row[ 2 ].x * p_original->m_row[ 3 ].w - p_original->m_row[ 2 ].w * p_original->m_row[ 3 ].x;
 	float fb3 = p_original->m_row[ 2 ].y * p_original->m_row[ 3 ].z - p_original->m_row[ 2 ].z * p_original->m_row[ 3 ].y;
 	float fb4 = p_original->m_row[ 2 ].y * p_original->m_row[ 3 ].w - p_original->m_row[ 2 ].w * p_original->m_row[ 3 ].y;
  	float fb5 = p_original->m_row[ 2 ].z * p_original->m_row[ 3 ].w - p_original->m_row[ 2 ].w * p_original->m_row[ 3 ].z;

	float fdet = fa0 * fb5 - fa1 * fb4 + fa2 * fb3 + fa3 * fb2 - fa4 * fb1 + fa5 * fb0;

	_ASSERTE( fabsf( fdet ) >= .000001f );

	temp.m_row[ 0 ].x =   p_original->m_row[ 1 ].y * fb5 - p_original->m_row[ 1 ].z * fb4 + p_original->m_row[ 1 ].w * fb3;
	temp.m_row[ 1 ].x = - p_original->m_row[ 1 ].x * fb5 + p_original->m_row[ 1 ].z * fb2 - p_original->m_row[ 1 ].w * fb1;
	temp.m_row[ 2 ].x =   p_original->m_row[ 1 ].x * fb4 - p_original->m_row[ 1 ].y * fb2 + p_original->m_row[ 1 ].w * fb0;
	temp.m_row[ 3 ].x = - p_original->m_row[ 1 ].x * fb3 + p_original->m_row[ 1 ].y * fb1 - p_original->m_row[ 1 ].z * fb0;
	temp.m_row[ 0 ].y = - p_original->m_row[ 0 ].y * fb5 + p_original->m_row[ 0 ].z * fb4 - p_original->m_row[ 0 ].w * fb3;
	temp.m_row[ 1 ].y =   p_original->m_row[ 0 ].x * fb5 - p_original->m_row[ 0 ].z * fb2 + p_original->m_row[ 0 ].w * fb1;
	temp.m_row[ 2 ].y = - p_original->m_row[ 0 ].x * fb4 + p_original->m_row[ 0 ].y * fb2 - p_original->m_row[ 0 ].w * fb0;
	temp.m_row[ 3 ].y =   p_original->m_row[ 0 ].x * fb3 - p_original->m_row[ 0 ].y * fb1 + p_original->m_row[ 0 ].z * fb0;
	temp.m_row[ 0 ].z =   p_original->m_row[ 3 ].y * fa5 - p_original->m_row[ 3 ].z * fa4 + p_original->m_row[ 3 ].w * fa3;
	temp.m_row[ 1 ].z = - p_original->m_row[ 3 ].x * fa5 + p_original->m_row[ 3 ].z * fa2 - p_original->m_row[ 3 ].w * fa1;
	temp.m_row[ 2 ].z =   p_original->m_row[ 3 ].x * fa4 - p_original->m_row[ 3 ].y * fa2 + p_original->m_row[ 3 ].w * fa0;
	temp.m_row[ 3 ].z = - p_original->m_row[ 3 ].x * fa3 + p_original->m_row[ 3 ].y * fa1 - p_original->m_row[ 3 ].z * fa0;
	temp.m_row[ 0 ].w = - p_original->m_row[ 2 ].y * fa5 + p_original->m_row[ 2 ].z * fa4 - p_original->m_row[ 2 ].w * fa3;
	temp.m_row[ 1 ].w =   p_original->m_row[ 2 ].x * fa5 - p_original->m_row[ 2 ].z * fa2 + p_original->m_row[ 2 ].w * fa1;
	temp.m_row[ 2 ].w = - p_original->m_row[ 2 ].x * fa4 + p_original->m_row[ 2 ].y * fa2 - p_original->m_row[ 2 ].w * fa0;
	temp.m_row[ 3 ].w =   p_original->m_row[ 2 ].x * fa3 - p_original->m_row[ 2 ].y * fa1 + p_original->m_row[ 2 ].z * fa0;

	float invdet = 1.0f / fdet;

   uint32 i;
	for ( i = 0; i < 4; i++ )
	{
		p_inverse->m_row[ i ].x = temp.m_row[ i ].x * invdet;
		p_inverse->m_row[ i ].y = temp.m_row[ i ].y * invdet;
		p_inverse->m_row[ i ].z = temp.m_row[ i ].z * invdet;
		p_inverse->m_row[ i ].w = temp.m_row[ i ].w * invdet;
	}
}

void Multiply_M4( Matrix4x4 * p_left, Matrix4x4 * p_right, Matrix4x4 * p_result )
{
   // first transpose the matrix since it's orthonormal. Then dot the rows.
   Matrix4x4 trans_right = Transpose_M4( p_right );
   Matrix4x4 result;

   // 0X
   result.m_row[ 0 ].x = Dot_Product_V4( &p_left->m_row[ 0 ], &trans_right.m_row[ 0 ] );

   // 0Y
   result.m_row[ 0 ].y = Dot_Product_V4( &p_left->m_row[ 0 ], &trans_right.m_row[ 1 ] );

   // 0Z
   result.m_row[ 0 ].z = Dot_Product_V4( &p_left->m_row[ 0 ], &trans_right.m_row[ 2 ] );

   // 0W
   result.m_row[ 0 ].w = Dot_Product_V4( &p_left->m_row[ 0 ], &trans_right.m_row[ 3 ] );


   // 1X
   result.m_row[ 1 ].x = Dot_Product_V4( &p_left->m_row[ 1 ], &trans_right.m_row[ 0 ] );

   // 1Y
   result.m_row[ 1 ].y = Dot_Product_V4( &p_left->m_row[ 1 ], &trans_right.m_row[ 1 ] );

   // 1Z
   result.m_row[ 1 ].z = Dot_Product_V4( &p_left->m_row[ 1 ], &trans_right.m_row[ 2 ] );

   // 1W
   result.m_row[ 1 ].w = Dot_Product_V4( &p_left->m_row[ 1 ], &trans_right.m_row[ 3 ] );


   // 2X
   result.m_row[ 2 ].x = Dot_Product_V4( &p_left->m_row[ 2 ], &trans_right.m_row[ 0 ] );

   // 2Y
   result.m_row[ 2 ].y = Dot_Product_V4( &p_left->m_row[ 2 ], &trans_right.m_row[ 1 ] );

   // 2Z
   result.m_row[ 2 ].z = Dot_Product_V4( &p_left->m_row[ 2 ], &trans_right.m_row[ 2 ] );

   // 2W
   result.m_row[ 2 ].w = Dot_Product_V4( &p_left->m_row[ 2 ], &trans_right.m_row[ 3 ] );


   // 3X
   result.m_row[ 3 ].x = Dot_Product_V4( &p_left->m_row[ 3 ], &trans_right.m_row[ 0 ] );

   // 3Y
   result.m_row[ 3 ].y = Dot_Product_V4( &p_left->m_row[ 3 ], &trans_right.m_row[ 1 ] );

   // 3Z
   result.m_row[ 3 ].z = Dot_Product_V4( &p_left->m_row[ 3 ], &trans_right.m_row[ 2 ] );

   // 3W
   result.m_row[ 3 ].w = Dot_Product_V4( &p_left->m_row[ 3 ], &trans_right.m_row[ 3 ] );

   // copy at the end so they can have the same input and output
   *p_result = result;
}

Matrix4x4::Matrix4x4( void )
{
}

Matrix4x4::~Matrix4x4( )
{
}

Matrix4x4::Matrix4x4( Vector4 & x, Vector4 & y, Vector4 & z, Vector4 & w )
{
   m_row[ 0 ] = x;
   m_row[ 1 ] = y;
   m_row[ 2 ] = z;
   m_row[ 3 ] = w;
}

void Matrix4x4::operator =  ( Matrix4x4 & rhs )
{
   m_row[ 0 ] = rhs.m_row[ 0 ];
   m_row[ 1 ] = rhs.m_row[ 1 ];
   m_row[ 2 ] = rhs.m_row[ 2 ];
   m_row[ 3 ] = rhs.m_row[ 3 ];
}

