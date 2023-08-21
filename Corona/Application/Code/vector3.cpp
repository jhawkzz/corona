
#include "corona_headers.h"

Vector3::Vector3( void )
{
}

Vector3::Vector3( float new_x, float new_y, float new_z )
{
   x = new_x;
   y = new_y;
   z = new_z;
}

Vector3::~Vector3( )
{
}

void Vector3::Set( float new_x, float new_y, float new_z )
{
   x = new_x;
   y = new_y;
   z = new_z;
}

void Vector3::operator =( const Vector3 & rhs )
{
   x = rhs.x;
   y = rhs.y;
   z = rhs.z;
}

void Vector3::operator =( const float value )
{
   x = value;
   y = value;
   z = value;
}
