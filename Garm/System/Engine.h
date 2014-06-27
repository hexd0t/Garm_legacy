#ifndef ENGINE_H
#define ENGINE_H

#include <memory>
#include "../Data/World.h"

using std::shared_ptr;

namespace Garm
{
	namespace System
	{
		class Engine
		{
		public:
			Engine();
			virtual ~Engine();

			shared_ptr<Garm::Data::World> World;

			void Run();
		};
	}
}

#endif