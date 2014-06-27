#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <memory>
#include "../TypeDefs.h"
#include "PhysicsEntity.h"

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
			map<entityId, shared_ptr<PhysicsEntity>> physics;

		};
	}
}

#endif