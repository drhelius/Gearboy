/*
Oolong Engine for the iPhone / iPod touch
Copyright (c) 2007-2008 Wolfgang Engel  http://code.google.com/p/oolongengine/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#ifndef VECTOR_H_
#define VECTOR_H_

#include <OpenGLES/ES1/gl.h>
#include <math.h>

//#include "Matrix.h"


#define VERTTYPE GLfloat
#define VERTTYPEENUM GL_FLOAT

// Floating-point operations
#define VERTTYPEMUL(a,b)			( (VERTTYPE)((a)*(b)) )
#define VERTTYPEDIV(a,b)			( (VERTTYPE)((a)/(b)) )
#define VERTTYPEABS(a)				( (VERTTYPE)(fabs(a)) )

#define f2vt(x)						(x)
#define vt2f(x)						(x)

#define PIOVERTWO				PIOVERTWOf
#define PI						PIf
#define TWOPI					TWOPIf
#define ONE						ONEf

#define X2F(x)		((float)(x)/65536.0f)
#define XMUL(a,b)	( (int)( ((INT64BIT)(a)*(b)) / 65536 ) )
#define XDIV(a,b)	( (int)( (((INT64BIT)(a))<<16)/(b) ) )
#define _ABS(a)		((a) <= 0 ? -(a) : (a) )


// Define a 64-bit type for various platforms
#if defined(__int64) || defined(WIN32)
#define INT64BIT __int64
#elif defined(TInt64)
#define INT64BIT TInt64
#else
#define INT64BIT long long int
#endif

typedef struct _LARGE_INTEGER
	{
		union
		{
			struct
			{
				unsigned long LowPart;
				long HighPart;
			};
			INT64BIT QuadPart;
		};
	} LARGE_INTEGER, *PLARGE_INTEGER;

/****************************************************************************
 ** Typedefs
 ****************************************************************************/
/*!***************************************************************************
 2D floating point vector
 *****************************************************************************/
typedef struct
	{
		float x;	/*!< x coordinate */
		float y;	/*!< y coordinate */
	} VECTOR2;


/*!***************************************************************************
 3D floating point vector
 *****************************************************************************/
typedef struct
	{
		float x;	/*!< x coordinate */
		float y;	/*!< y coordinate */
		float z;	/*!< z coordinate */
	} VECTOR3;


/*!***************************************************************************
 4D floating point vector
 *****************************************************************************/
typedef struct
	{
		float x;	/*!< x coordinate */
		float y;	/*!< y coordinate */
		float z;	/*!< z coordinate */
		float w;	/*!< w coordinate */
	} VECTOR4;

/*!***************************************************************************
** Forward Declarations for vector and matrix structs
****************************************************************************/
//struct VECTOR4;
//struct VECTOR3;

/*!***************************************************************************
	** Vec2 2 component vector
	****************************************************************************/
struct Vec2
{
	VERTTYPE x, y;
	/*!***************************************************************************
		** Constructors
		****************************************************************************/
	/*!***************************************************************************
		@Function			Vec2
		@Description		Blank constructor.
		*****************************************************************************/
	Vec2() {}
	/*!***************************************************************************
		@Function			Vec2
		@Input				fX	X component of vector
		@Input				fY	Y component of vector
		@Description		Simple constructor from 2 values.
		*****************************************************************************/
	Vec2(VERTTYPE fX, VERTTYPE fY) : x(fX), y(fY) {}
	/*!***************************************************************************
		@Function			Vec2
		@Input				fValue	a component value
		@Description		Constructor from a single value.
		*****************************************************************************/
	Vec2(VERTTYPE fValue) : x(fValue), y(fValue) {}
	/*!***************************************************************************
		@Function			Vec2
		@Input				pVec	an array
		@Description		Constructor from an array
		*****************************************************************************/
	Vec2(const VERTTYPE* pVec) : x(pVec[0]), y(pVec[1]) {}
	/*!***************************************************************************
		@Function			Vec2
		@Input				v3Vec a Vec3
		@Description		Constructor from a Vec3
		*****************************************************************************/
//	Vec2(const Vec3& v3Vec);
	/*!***************************************************************************
		** Operators
		****************************************************************************/
	/*!***************************************************************************
		@Function			+
		@Input				rhs another Vec2
		@Returns			result of addition
		@Description		componentwise addition operator for two Vec2s
		*****************************************************************************/
	Vec2 operator+(const Vec2& rhs) const
	{
		Vec2 out(*this);
		return out += rhs;
	}
	/*!***************************************************************************
		@Function			-
		@Input				rhs another Vec2
		@Returns			result of subtraction
		@Description		componentwise subtraction operator for two Vec2s
		****************************************************************************/
	Vec2 operator-(const Vec2& rhs) const
	{
		Vec2 out(*this);
		return out -= rhs;
	}
	
	/*!***************************************************************************
		@Function			+=
		@Input				rhs another Vec2
		@Returns			result of addition
		@Description		componentwise addition and assignment operator for two Vec2s
		****************************************************************************/
	Vec2& operator+=(const Vec2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	
	/*!***************************************************************************
		@Function			-=
		@Input				rhs another Vec2
		@Returns			result of subtraction
		@Description		componentwise subtraction and assignment operator for two Vec2s
		****************************************************************************/
	Vec2& operator-=(const Vec2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	
	/*!***************************************************************************
		@Function			-
		@Input				rhs another Vec2
		@Returns			result of negation
		@Description		negation operator for a Vec2
		****************************************************************************/
	friend Vec2 operator- (const Vec2& rhs) { return Vec2(-rhs.x, -rhs.y); }
	
	/*!***************************************************************************
		@Function			*
		@Input				lhs scalar
		@Input				rhs a Vec2
		@Returns			result of negation
		@Description		negation operator for a Vec2
		****************************************************************************/
	friend Vec2 operator*(const VERTTYPE lhs, const Vec2&  rhs)
	{
		Vec2 out(lhs);
		return out *= rhs;
	}
	
	/*!***************************************************************************
		@Function			/
		@Input				lhs scalar
		@Input				rhs a Vec2
		@Returns			result of division
		@Description		division operator for scalar and Vec2
		****************************************************************************/
	friend Vec2 operator/(const VERTTYPE lhs, const Vec2&  rhs)
	{
		Vec2 out(lhs);
		return out /= rhs;
	}
	
	/*!***************************************************************************
		@Function			*
		@Input				rhs a scalar
		@Returns			result of multiplication
		@Description		componentwise multiplication by scalar for Vec2
		****************************************************************************/
	Vec2 operator*(const VERTTYPE& rhs) const
	{
		Vec2 out(*this);
		return out *= rhs;
	}
	
	/*!***************************************************************************
		@Function			*=
		@Input				rhs a scalar
		@Returns			result of multiplication and assignment
		@Description		componentwise multiplication and assignment by scalar for Vec2
		****************************************************************************/
	Vec2& operator*=(const VERTTYPE& rhs)
	{
		x = VERTTYPEMUL(x, rhs);
		y = VERTTYPEMUL(y, rhs);
		return *this;
	}

	/*!***************************************************************************
		@Function			*=
		@Input				rhs a Vec2
		@Returns			result of multiplication and assignment
		@Description		componentwise multiplication and assignment by Vec2 for Vec2
		****************************************************************************/
	Vec2& operator*=(const Vec2& rhs)
	{
		x = VERTTYPEMUL(x, rhs.x);
		y = VERTTYPEMUL(y, rhs.y);
		return *this;
	}

	/*!***************************************************************************
		@Function			/
		@Input				rhs a scalar
		@Returns			result of division
		@Description		componentwise division by scalar for Vec2
		****************************************************************************/
	Vec2 operator/(const VERTTYPE& rhs) const
	{
		Vec2 out(*this);
		return out /= rhs;
	}
	
	/*!***************************************************************************
		@Function			/=
		@Input				rhs a scalar
		@Returns			result of division and assignment
		@Description		componentwise division and assignment by scalar for Vec2
		****************************************************************************/
	Vec2& operator/=(const VERTTYPE& rhs)
	{
		x = VERTTYPEDIV(x, rhs);
		y = VERTTYPEDIV(y, rhs);
		return *this;
	}
	
	/*!***************************************************************************
		@Function			/=
		@Input				rhs a Vec2
		@Returns			result of division and assignment
		@Description		componentwise division and assignment by Vec2 for Vec2
		****************************************************************************/
	Vec2& operator/=(const Vec2& rhs)
	{
		x = VERTTYPEDIV(x, rhs.x);
		y = VERTTYPEDIV(y, rhs.y);
		return *this;
	}	
	// FUNCTIONS
	/*!***************************************************************************
		@Function			lenSqr
		@Returns			the square of the magnitude of the vector
		@Description		calculates the square of the magnitude of the vector
		****************************************************************************/
	VERTTYPE lenSqr() const
	{
		return VERTTYPEMUL(x,x)+VERTTYPEMUL(y,y);
	}
	
	/*!***************************************************************************
		@Function			length
		@Returns			the of the magnitude of the vector
		@Description		calculates the magnitude of the vector
		****************************************************************************/
	VERTTYPE length() const
	{
		return (VERTTYPE) f2vt(sqrt(vt2f(x)*vt2f(x) + vt2f(y)*vt2f(y)));
	}
	
	/*!***************************************************************************
		@Function			normalize
		@Returns			the normalized value of the vector
		@Description		normalizes the vector
		****************************************************************************/
	Vec2 normalize()
	{
		return *this /= length();
	}
	
	/*!***************************************************************************
		@Function			normalized
		@Returns			returns the normalized value of the vector
		@Description		returns a normalized vector of the same direction as this
		vector
		****************************************************************************/
	Vec2 normalized() const
	{
		Vec2 out(*this);
		return out.normalize();
	}

	/*!***************************************************************************
		@Function			rotated90
		@Returns			returns the vector rotated 90ﾰ
		@Description		returns the vector rotated 90ﾰ
		****************************************************************************/
	Vec2 rotated90() const
	{
		return Vec2(-y, x);
	}

	/*!***************************************************************************
		@Function			dot
		@Input				rhs a single value
		@Returns			scalar product
		@Description		calculate the scalar product of two Vec3s
		****************************************************************************/
	VERTTYPE dot(const Vec2& rhs)
	{
		return VERTTYPEMUL(x, rhs.x) + VERTTYPEMUL(y, rhs.y);
	}
			
	/*!***************************************************************************
		@Function			ptr
		@Returns			pointer
		@Description		returns a pointer to memory containing the values of the
		Vec3
		****************************************************************************/
	VERTTYPE *ptr() { return (VERTTYPE*)this; }
};

/*!***************************************************************************
** Vec3 3 component vector
****************************************************************************/
struct Vec3 : public VECTOR3
{
/*!***************************************************************************
** Constructors
****************************************************************************/
/*!***************************************************************************
 @Function			Vec3
 @Description		Blank constructor.
*****************************************************************************/
	Vec3(){}
/*!***************************************************************************
 @Function			Vec3
 @Input				fX	X component of vector
 @Input				fY	Y component of vector
 @Input				fZ	Z component of vector
 @Description		Simple constructor from 3 values.
*****************************************************************************/
	Vec3(VERTTYPE fX, VERTTYPE fY, VERTTYPE fZ)
	{
		x = fX;	y = fY;	z = fZ;
	}
/*!***************************************************************************
 @Function			Vec3
 @Input				fValue	a component value
 @Description		Constructor from a single value.
*****************************************************************************/
	Vec3(const VERTTYPE fValue)
	{
		x = fValue; y = fValue; z = fValue;
	}
/*!***************************************************************************
 @Function			Vec3
 @Input				pVec	an array
 @Description		Constructor from an array
*****************************************************************************/
	Vec3(const VERTTYPE* pVec)
	{
		x = (*pVec++); y = (*pVec++); z = *pVec;
	}
/*!***************************************************************************
 @Function			Vec3
 @Input				v4Vec a Vec4
 @Description		Constructor from a Vec4
*****************************************************************************/
//	Vec3(const Vec4& v4Vec);
/*!***************************************************************************
** Operators
****************************************************************************/
/*!***************************************************************************
 @Function			+
 @Input				rhs another Vec3
 @Returns			result of addition
 @Description		componentwise addition operator for two VECTOR3s
*****************************************************************************/
	Vec3 operator+(const Vec3& rhs) const
	{
		Vec3 out;
		out.x = x+rhs.x;
		out.y = y+rhs.y;
		out.z = z+rhs.z;
		return out;
	}
/*!***************************************************************************
 @Function			-
 @Input				rhs another Vec3
 @Returns			result of subtraction
 @Description		componentwise subtraction operator for two VECTOR3s
****************************************************************************/
	Vec3 operator-(const Vec3& rhs) const
	{
		Vec3 out;
		out.x = x-rhs.x;
		out.y = y-rhs.y;
		out.z = z-rhs.z;
		return out;
	}

/*!***************************************************************************
 @Function			+=
 @Input				rhs another Vec3
 @Returns			result of addition
 @Description		componentwise addition and assignement operator for two VECTOR3s
****************************************************************************/
	Vec3& operator+=(const Vec3& rhs)
	{
		x +=rhs.x;
		y +=rhs.y;
		z +=rhs.z;
		return *this;
	}

/*!***************************************************************************
 @Function			-=
 @Input				rhs another Vec3
 @Returns			result of subtraction
 @Description		componentwise subtraction and assignement operator for two VECTOR3s
****************************************************************************/
	Vec3& operator-=(const Vec3& rhs)
	{
		x -=rhs.x;
		y -=rhs.y;
		z -=rhs.z;
		return *this;
	}

/*!***************************************************************************
 @Function			-
 @Input				rhs another Vec3
 @Returns			result of negation
 @Description		negation operator for a Vec3
****************************************************************************/
	friend Vec3 operator - (const Vec3& rhs) { return Vec3(rhs) *= f2vt(-1); }

/*!***************************************************************************
 @Function			*
 @Input				lhs single value
 @Input				rhs a Vec3
 @Returns			result of negation
 @Description		negation operator for a Vec3
****************************************************************************/
	friend Vec3 operator*(const VERTTYPE lhs, const Vec3&  rhs)
	{
		Vec3 out;
		out.x = VERTTYPEMUL(lhs,rhs.x);
		out.y = VERTTYPEMUL(lhs,rhs.y);
		out.z = VERTTYPEMUL(lhs,rhs.z);
		return out;
	}

/*!***************************************************************************
 @Function			*
 @Input				lhs single value
 @Input				rhs a Vec3
 @Returns			result of negation
 @Description		negation operator for a Vec3
****************************************************************************/
	friend Vec3 operator/(const VERTTYPE lhs, const Vec3&  rhs)
	{
		Vec3 out;
		out.x = VERTTYPEDIV(lhs,rhs.x);
		out.y = VERTTYPEDIV(lhs,rhs.y);
		out.z = VERTTYPEDIV(lhs,rhs.z);
		return out;
	}

/*!***************************************************************************
 @Function			*
 @Input				rhs a PVRTMat3
 @Returns			result of multiplication
 @Description		matrix multiplication operator Vec3 and PVRTMat3
****************************************************************************/
//	Vec3 operator*(const PVRTMat3& rhs) const;

/*!***************************************************************************
 @Function			*=
 @Input				rhs a PVRTMat3
 @Returns			result of multiplication and assignment
 @Description		matrix multiplication and assignment operator for Vec3 and PVRTMat3
****************************************************************************/
//	Vec3& operator*=(const PVRTMat3& rhs);

/*!***************************************************************************
 @Function			*
 @Input				rhs a single value
 @Returns			result of multiplication
 @Description		componentwise multiplication by single dimension value for Vec3
****************************************************************************/
	Vec3 operator*(const VERTTYPE& rhs) const
	{
		Vec3 out;
		out.x = VERTTYPEMUL(x,rhs);
		out.y = VERTTYPEMUL(y,rhs);
		out.z = VERTTYPEMUL(z,rhs);
		return out;
	}

/*!***************************************************************************
 @Function			*
 @Input				rhs a single value
 @Returns			result of multiplication and assignment
 @Description		componentwise multiplication and assignement by single
					dimension value	for Vec3
****************************************************************************/
	Vec3& operator*=(const VERTTYPE& rhs)
	{
		x = VERTTYPEMUL(x,rhs);
		y = VERTTYPEMUL(y,rhs);
		z = VERTTYPEMUL(z,rhs);
		return *this;
	}

/*!***************************************************************************
 @Function			/
 @Input				rhs a single value
 @Returns			result of division
 @Description		componentwise division by single
					dimension value	for Vec3
****************************************************************************/
	Vec3 operator/(const VERTTYPE& rhs) const
	{
		Vec3 out;
		out.x = VERTTYPEDIV(x,rhs);
		out.y = VERTTYPEDIV(y,rhs);
		out.z = VERTTYPEDIV(z,rhs);
		return out;
	}

/*!***************************************************************************
 @Function			/=
 @Input				rhs a single value
 @Returns			result of division and assignment
 @Description		componentwise division and assignement by single
					dimension value	for Vec3
****************************************************************************/
	Vec3& operator/=(const VERTTYPE& rhs)
	{
		x = VERTTYPEDIV(x,rhs);
		y = VERTTYPEDIV(y,rhs);
		z = VERTTYPEDIV(z,rhs);
		return *this;
	}

	// FUNCTIONS
/*!***************************************************************************
 @Function			lenSqr
 @Returns			the square of the magnitude of the vector
 @Description		calculates the square of the magnitude of the vector
****************************************************************************/
	VERTTYPE lenSqr() const
	{
		return VERTTYPEMUL(x,x)+VERTTYPEMUL(y,y)+VERTTYPEMUL(z,z);
	}

/*!***************************************************************************
 @Function			length
 @Returns			the of the magnitude of the vector
 @Description		calculates the magnitude of the vector
****************************************************************************/
	VERTTYPE length() const
	{
		return (VERTTYPE) f2vt(sqrt(vt2f(x)*vt2f(x) + vt2f(y)*vt2f(y) + vt2f(z)*vt2f(z)));
	}

/*!***************************************************************************
 @Function			normalize
 @Returns			the normalized value of the vector
 @Description		normalizes the vector
****************************************************************************/
	Vec3 normalize()
	{
#if defined(PVRT_FIXED_POINT_ENABLE)
		// Scale vector by uniform value
		int n = PVRTABS(x) + PVRTABS(y) + PVRTABS(z);
		x = VERTTYPEDIV(x, n);
		y = VERTTYPEDIV(y, n);
		z = VERTTYPEDIV(z, n);

		// Calculate x2+y2+z2/sqrt(x2+y2+z2)
		int f = dot(*this);
		f = VERTTYPEDIV(PVRTF2X(1.0f), PVRTF2X(sqrt(PVRTX2F(f))));

		// Multiply vector components by f
		x = PVRTXMUL(x, f);
		y = PVRTXMUL(y, f);
		z = PVRTXMUL(z, f);
#else
		VERTTYPE len = length();
		x =VERTTYPEDIV(x,len);
		y =VERTTYPEDIV(y,len);
		z =VERTTYPEDIV(z,len);
#endif
		return *this;
	}

/*!***************************************************************************
 @Function			normalized
 @Returns			returns the normalized value of the vector
 @Description		returns a normalized vector of the same direction as this
					vector
****************************************************************************/
	Vec3 normalized() const
	{
		Vec3 out;
#if defined(PVRT_FIXED_POINT_ENABLE)
		// Scale vector by uniform value
		int n = PVRTABS(x) + PVRTABS(y) + PVRTABS(z);
		out.x = VERTTYPEDIV(x, n);
		out.y = VERTTYPEDIV(y, n);
		out.z = VERTTYPEDIV(z, n);

		// Calculate x2+y2+z2/sqrt(x2+y2+z2)
		int f = out.dot(out);
		f = VERTTYPEDIV(PVRTF2X(1.0f), PVRTF2X(sqrt(PVRTX2F(f))));

		// Multiply vector components by f
		out.x = PVRTXMUL(out.x, f);
		out.y = PVRTXMUL(out.y, f);
		out.z = PVRTXMUL(out.z, f);
#else
		VERTTYPE len = length();
		out.x =VERTTYPEDIV(x,len);
		out.y =VERTTYPEDIV(y,len);
		out.z =VERTTYPEDIV(z,len);
#endif
		return out;
	}

/*!***************************************************************************
 @Function			dot
 @Input				rhs a single value
 @Returns			scalar product
 @Description		calculate the scalar product of two VECTOR3s
****************************************************************************/
	VERTTYPE dot(const Vec3& rhs)
	{
		return VERTTYPEMUL(x,rhs.x)+VERTTYPEMUL(y,rhs.y)+VERTTYPEMUL(z,rhs.z);
	}

/*!***************************************************************************
 @Function			dot
 @Returns			scalar product
 @Description		calculate the scalar product of two VECTOR3s
****************************************************************************/
	Vec3 cross(const Vec3& rhs)
	{
		Vec3 out;
		out.x = VERTTYPEMUL(y,rhs.z)-VERTTYPEMUL(z,rhs.y);
		out.y = VERTTYPEMUL(z,rhs.x)-VERTTYPEMUL(x,rhs.z);
		out.z = VERTTYPEMUL(x,rhs.y)-VERTTYPEMUL(y,rhs.x);
		return out;
	}

/*!***************************************************************************
 @Function			ptr
 @Returns			pointer
 @Description		returns a pointer to memory containing the values of the
					Vec3
****************************************************************************/
	VERTTYPE *ptr() { return (VERTTYPE*)this; }
};

/*!***************************************************************************
** Vec4 4 component vector
****************************************************************************/
struct Vec4 : public VECTOR4
{
/*!***************************************************************************
** Constructors
****************************************************************************/
/*!***************************************************************************
 @Function			Vec4
 @Description		Blank constructor.
*****************************************************************************/
	Vec4(){}

/*!***************************************************************************
 @Function			Vec3
 @Description		Blank constructor.
*****************************************************************************/
	Vec4(const VERTTYPE vec)
	{
		x = vec; y = vec; z = vec; w = vec;
	}

/*!***************************************************************************
 @Function			multiple value constructor
 @Input				fX value of x component
 @Input				fY value of y component
 @Input				fZ value of z component
 @Input				fW value of w component
 @Description		Constructs a Vec4 from 4 separate values
****************************************************************************/
	Vec4(VERTTYPE fX, VERTTYPE fY, VERTTYPE fZ, VERTTYPE fW)
	{
		x = fX;	y = fY;	z = fZ;	w = fW;
	}

/*!***************************************************************************
 @Function			constructor from Vec3
 @Input				vec3 a Vec3
 @Input				fW value of w component
 @Description		Constructs a Vec4 from a vec3 and a w component
****************************************************************************/
	Vec4(const Vec3& vec3, VERTTYPE fW)
	{
		x = vec3.x;	y = vec3.y;	z = vec3.z;	w = fW;
	}

/*!***************************************************************************
 @Function			constructor from Vec3
 @Input				fX value of x component
 @Input				vec3 a Vec3
 @Description		Constructs a vec4 from a vec3 and a w component
****************************************************************************/
	Vec4(VERTTYPE fX, const Vec3& vec3)
	{
		x = fX;	y = vec3.x;	z = vec3.y;	w = vec3.z;
	}

/*!***************************************************************************
 @Function			constructor from array
 @Input				pVec a pointer to an array of four values
 @Description		Constructs a Vec4 from a pointer to an array of four values
****************************************************************************/
	Vec4(const VERTTYPE* pVec)
	{
		x = (*pVec++); y = (*pVec++); z= (*pVec++); w = *pVec;
	}

/*!***************************************************************************
** Vec4 Operators
****************************************************************************/
/*!***************************************************************************
 @Function			+
 @Input				rhs another Vec4
 @Returns			result of addition
 @Description		Addition operator for Vec4
****************************************************************************/
	Vec4 operator+(const Vec4& rhs) const
	{
		Vec4 out;
		out.x = x+rhs.x;
		out.y = y+rhs.y;
		out.z = z+rhs.z;
		out.w = w+rhs.w;
		return out;
	}

/*!***************************************************************************
 @Function			-
 @Input				rhs another Vec4
 @Returns			result of subtraction
 @Description		Subtraction operator for Vec4
****************************************************************************/
	Vec4 operator-(const Vec4& rhs) const
	{
		Vec4 out;
		out.x = x-rhs.x;
		out.y = y-rhs.y;
		out.z = z-rhs.z;
		out.w = w-rhs.w;
		return out;
	}

/*!***************************************************************************
 @Function			+=
 @Input				rhs another Vec4
 @Returns			result of addition and assignment
 @Description		Addition and assignment operator for Vec4
****************************************************************************/
	Vec4& operator+=(const Vec4& rhs)
	{
		x +=rhs.x;
		y +=rhs.y;
		z +=rhs.z;
		w +=rhs.w;
		return *this;
	}

/*!***************************************************************************
 @Function			-=
 @Input				rhs another Vec4
 @Returns			result of subtraction and assignment
 @Description		Subtraction and assignment operator for Vec4
****************************************************************************/
	Vec4& operator-=(const Vec4& rhs)
	{
		x -=rhs.x;
		y -=rhs.y;
		z -=rhs.z;
		w -=rhs.w;
		return *this;
	}

/*!***************************************************************************
 @Function			*
 @Input				rhs a PVRTMat4
 @Returns			result of multiplication
 @Description		matrix multiplication for Vec4 and PVRTMat4
****************************************************************************/
//	Vec4 operator*(const MATRIX& rhs) const;

/*!***************************************************************************
 @Function			*=
 @Input				rhs a PVRTMat4
 @Returns			result of multiplication and assignement
 @Description		matrix multiplication and assignment for Vec4 and PVRTMat4
****************************************************************************/
//	Vec4& operator*=(const MATRIX& rhs);

/*!***************************************************************************
 @Function			*
 @Input				rhs a single dimension value
 @Returns			result of multiplication
 @Description		componentwise multiplication of a Vec4 by a single value
****************************************************************************/
	Vec4 operator*(const VERTTYPE& rhs) const
	{
		Vec4 out;
		out.x = VERTTYPEMUL(x,rhs);
		out.y = VERTTYPEMUL(y,rhs);
		out.z = VERTTYPEMUL(z,rhs);
		out.w = VERTTYPEMUL(w,rhs);
		return out;
	}

/*!***************************************************************************
 @Function			*=
 @Input				rhs a single dimension value
 @Returns			result of multiplication and assignment
 @Description		componentwise multiplication and assignment of a Vec4 by
				a single value
****************************************************************************/
	Vec4& operator*=(const VERTTYPE& rhs)
	{
		x = VERTTYPEMUL(x,rhs);
		y = VERTTYPEMUL(y,rhs);
		z = VERTTYPEMUL(z,rhs);
		w = VERTTYPEMUL(w,rhs);
		return *this;
	}

/*!***************************************************************************
 @Function			/
 @Input				rhs a single dimension value
 @Returns			result of division
 @Description		componentwise division of a Vec4 by a single value
****************************************************************************/
	Vec4 operator/(const VERTTYPE& rhs) const
	{
		Vec4 out;
		out.x = VERTTYPEDIV(x,rhs);
		out.y = VERTTYPEDIV(y,rhs);
		out.z = VERTTYPEDIV(z,rhs);
		out.w = VERTTYPEDIV(w,rhs);
		return out;
	}

/*!***************************************************************************
 @Function			/=
 @Input				rhs a single dimension value
 @Returns			result of division and assignment
 @Description		componentwise division and assignment of a Vec4 by
					a single value
****************************************************************************/
	Vec4& operator/=(const VERTTYPE& rhs)
	{
		x = VERTTYPEDIV(x,rhs);
		y = VERTTYPEDIV(y,rhs);
		z = VERTTYPEDIV(z,rhs);
		w = VERTTYPEDIV(w,rhs);
		return *this;
	}

/*!***************************************************************************
 @Function			*
 @Input				lhs a single dimension value
 @Input				rhs a Vec4
 @Returns			result of muliplication
 @Description		componentwise multiplication of a Vec4 by
					a single value
****************************************************************************/
friend Vec4 operator*(const VERTTYPE lhs, const Vec4&  rhs)
{
	Vec4 out;
	out.x = VERTTYPEMUL(lhs,rhs.x);
	out.y = VERTTYPEMUL(lhs,rhs.y);
	out.z = VERTTYPEMUL(lhs,rhs.z);
	out.w = VERTTYPEMUL(lhs,rhs.w);
	return out;
}

/*!***************************************************************************
** Functions
****************************************************************************/
/*!***************************************************************************
 @Function			lenSqr
 @Returns			square of the magnitude of the vector
 @Description		calculates the square of the magnitude of the vector
****************************************************************************/
	VERTTYPE lenSqr() const
	{
		return VERTTYPEMUL(x,x)+VERTTYPEMUL(y,y)+VERTTYPEMUL(z,z)+VERTTYPEMUL(w,w);
	}

/*!***************************************************************************
 @Function			length
 @Returns			the magnitude of the vector
 @Description		calculates the magnitude of the vector
****************************************************************************/
	VERTTYPE length() const
	{
		return (VERTTYPE) f2vt(sqrt(vt2f(x)*vt2f(x) + vt2f(y)*vt2f(y) + vt2f(z)*vt2f(z) + vt2f(w)*vt2f(w)));
	}

/*!***************************************************************************
 @Function			normalize
 @Returns			normalized vector
 @Description		calculates the normalized value of a Vec4
****************************************************************************/
	Vec4 normalize()
	{
		VERTTYPE len = length();
		x =VERTTYPEDIV(x,len);
		y =VERTTYPEDIV(y,len);
		z =VERTTYPEDIV(z,len);
		w =VERTTYPEDIV(w,len);
		return *this;
	}
/*!***************************************************************************
 @Function			normalized
 @Returns			normalized vector
 @Description		returns a normalized vector of the same direction as this
					vector
****************************************************************************/
	Vec4 normalized() const
	{
		Vec4 out;
		VERTTYPE len = length();
		out.x =VERTTYPEDIV(x,len);
		out.y =VERTTYPEDIV(y,len);
		out.z =VERTTYPEDIV(z,len);
		out.w =VERTTYPEDIV(w,len);
		return out;
	}

/*!***************************************************************************
 @Function			dot
 @Returns			scalar product
 @Description		returns a normalized vector of the same direction as this
					vector
****************************************************************************/
	VERTTYPE dot(const Vec4& rhs)
	{
		return VERTTYPEMUL(x,rhs.x)+VERTTYPEMUL(y,rhs.y)+VERTTYPEMUL(z,rhs.z)+VERTTYPEMUL(w,rhs.w);
	}

/*!***************************************************************************
 @Function			ptr
 @Returns			pointer to vector values
 @Description		returns a pointer to memory containing the values of the
					Vec3
****************************************************************************/
	VERTTYPE *ptr() { return (VERTTYPE*)this; }
};



#endif // VECTOR_H_