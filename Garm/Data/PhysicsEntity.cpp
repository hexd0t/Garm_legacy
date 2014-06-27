#include "PhysicsEntity.h"
#include <stdexcept>

using Garm::Data::PhysicsEntity;

PhysicsEntity::PhysicsEntity()
	: Location( DirectX::XMVectorReplicate( 0.0f ) ),
	Speed( DirectX::XMVectorReplicate( 0.0f ) ),
	Mass( DirectX::XMVectorReplicate( 1.0f ) ),
	OwnForce( DirectX::XMVectorReplicate( 0.0f ) )
{

}

PhysicsEntity::PhysicsEntity( const XMFLOAT3& location, const float& mass )
	: Location( DirectX::XMLoadFloat3( &location ) ),
	Speed( DirectX::XMVectorReplicate( 0.0f ) ),
	Mass( DirectX::XMVectorReplicate( mass ) ),
	OwnForce( DirectX::XMVectorReplicate( 0.0f ) )
{

}
PhysicsEntity::PhysicsEntity( const XMFLOAT3& location, const XMFLOAT3& speed, const float& mass )
	: Location( DirectX::XMLoadFloat3( &location ) ),
	Speed( DirectX::XMLoadFloat3( &speed ) ),
	Mass( DirectX::XMVectorReplicate( mass ) ),
	OwnForce( DirectX::XMVectorReplicate( 0.0f ) )
{

}

PhysicsEntity::PhysicsEntity( const XMFLOAT3& location, const XMFLOAT3& speed, const float& mass, const XMFLOAT3& ownForce )
	: Location( DirectX::XMLoadFloat3( &location ) ),
	Speed( DirectX::XMLoadFloat3( &speed ) ),
	Mass( DirectX::XMVectorReplicate( mass ) ),
	OwnForce( DirectX::XMLoadFloat3( &ownForce ) )
{

}

PhysicsEntity::PhysicsEntity( const PhysicsEntity& other )
	: Location( other.Location ),
	Speed( other.Speed ),
	Mass( other.Mass ),
	OwnForce( other.OwnForce )
{

}

PhysicsEntity::~PhysicsEntity()
{

}