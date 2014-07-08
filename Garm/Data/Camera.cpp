#include "Camera.h"
#include <stdexcept>
#include "../TypeDefs.h"

using Garm::Data::Camera;

Camera::Camera() : location(DirectX::XMVectorSet( 0.0f, 1.0f, -1.0f, 1.0f )),
lookAt(DirectX::XMVectorSet(0.0f,0.0f,0.0f,1.0f))
{
}

Camera::~Camera()
{
}

void Camera::Update( XMVECTOR loc )
{
	using namespace DirectX;
#if defined(_DEBUG)
	if (loc.m128_f32[3] != 1.0f)
		throw std::runtime_error( "Got location vector with 4 component != 1 @" __FILE__ ":" S__LINE__ );
#endif
	XMVECTOR movement = loc - location;
	location = loc;
	lookAt = lookAt + movement;
}

void Camera::Update( XMVECTOR loc, XMVECTOR lAt )
{
#if defined(_DEBUG)
	if (loc.m128_f32[3] != 1.0f || lAt.m128_f32[3] != 1.0f)
		throw std::runtime_error( "Got location vector with 4 component != 1 @" __FILE__ ":" S__LINE__ );
#endif
	location = loc;
	lookAt = lAt;
}

void Camera::Update( XMVECTOR loc, float azimuth, float altitude )
{
#if defined(_DEBUG)
	if (loc.m128_f32[3] != 1.0f)
		throw std::runtime_error( "Got location vector with 4 component != 1 @" __FILE__ ":" S__LINE__ );
#endif
	using namespace DirectX;
	location = loc;
	lookAt = loc +
		(DirectX::XMVectorSet( DirectX::XMScalarSin( azimuth ), DirectX::XMScalarCos( azimuth ), 1.0f, 0.0f )
		* DirectX::XMVectorSet( DirectX::XMScalarCos( altitude ), DirectX::XMScalarCos( altitude ), DirectX::XMScalarSin( azimuth ), 0.0f ));
}

XMVECTOR Camera::GetLocation()
{
	return location;
}

XMVECTOR Camera::GetLookAt()
{
	return lookAt;
}