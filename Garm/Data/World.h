#ifndef DATA_WORLD_H
#define DATA_WORLD_H

#include <map>
#include <memory>
#include "../TypeDefs.h"
#include "PhysicsEntity.h"
#include "Terrain.h"
#include "Camera.h"

using std::map;
using std::shared_ptr;
using Garm::Data::PhysicsEntity;

namespace Garm
{
	namespace Data
	{
		class World
		{
		public:
			World();
			virtual ~World();
			map<entityId, shared_ptr<PhysicsEntity>> physics;
			shared_ptr<Terrain> terrain;
			shared_ptr<Garm::Data::Camera> camera;
		};
	}
}

#endif