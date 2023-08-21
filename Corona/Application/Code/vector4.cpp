
#include "corona_headers.h"

Vector4::Vector4( void )
{
}

Vector4::Vector4( float new_x, float new_y, float new_z, float new_w )
{
   x = new_x;
   y = new_y;
   z = new_z;
   w = new_w;
}

Vector4::~Vector4( )
{
}

void Vector4::Set( float new_x, float new_y, float new_z, float new_w )
{
   x = new_x;
   y = new_y;
   z = new_z;
   w = new_w;
}

void Vector4::operator =( const Vector4 & rhs )
{
   x = rhs.x;
   y = rhs.y;
   z = rhs.z;
   w = rhs.w;
}

void Vector4::operator =( const float value )
{
   x = value;
   y = value;
   z = value;
   w = value;
}
