#ifndef SYSTEM_ENGINE_H
#define SYSTEM_ENGINE_H

#include <memory>
#include "../Data/World.h"
#include "Render/Graphics.h"

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
			shared_ptr<Garm::System::Render::Graphics> Graphics;

			void Run();
		};
	}
}

#endif