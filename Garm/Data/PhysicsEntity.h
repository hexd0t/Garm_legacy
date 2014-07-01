#ifndef DATA_PHYSICSENTITY_H
#define DATA_PHYSICSENTITY_H

#include <DirectXMath.h>

using DirectX::XMVECTOR;
using DirectX::XMFLOAT3;

namespace Garm
{
	namespace Data
	{
		class PhysicsEntity
		{
		public:
			XMVECTOR Location;
			XMVECTOR Speed;
			XMVECTOR Mass;
			XMVECTOR OwnForce;

			PhysicsEntity();
			PhysicsEntity( const XMFLOAT3& location, const float& mass );
			PhysicsEntity( const XMFLOAT3& location, const XMFLOAT3& speed, const float& mass );
			PhysicsEntity( const XMFLOAT3& location, const XMFLOAT3& speed, const float& mass, const XMFLOAT3& ownForce );
			PhysicsEntity( const PhysicsEntity& other );
			virtual ~PhysicsEntity();
		};
	}
}
#endif