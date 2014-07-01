#ifndef SYSTEM_RENDER_TERRAIN_QUADTREE_H
#define SYSTEM_RENDER_TERRAIN_QUADTREE_H

#include <vector>
#include <memory>
#include "../../../Data/Terrain.h"

namespace Garm
{
	namespace System
	{
		namespace Render
		{
			namespace Terrain
			{
				class QuadTree
				{
				public:
					QuadTree(const std::shared_ptr<Garm::Data::Terrain>& terrain);
					virtual ~QuadTree();
					std::vector<std::vector<float>> QuadsMaxHeight;
					std::vector<std::vector<float>> QuadsMinHeight;
					
				private:
				};
			}
		}
	}
}

#endif