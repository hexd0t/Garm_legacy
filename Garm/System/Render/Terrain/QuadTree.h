#ifndef SYSTEM_RENDER_TERRAIN_QUADTREE_H
#define SYSTEM_RENDER_TERRAIN_QUADTREE_H

#include <vector>
#include <memory>
#include "../../../Data/Terrain.h"
#include <DirectXMath.h>
#include <DirectXCollision.h>

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
					QuadTree(const Garm::Data::Terrain& terrain, int levels);
					virtual ~QuadTree();
					
				private:
					class Quad
					{
					public:
						Quad( float minX, float minY, float minZ, float maxX, float maxY, float maxZ );
						Quad( std::unique_ptr<Quad>&& subA, std::unique_ptr<Quad>&& subB, std::unique_ptr<Quad>&& subC, std::unique_ptr<Quad>&& subD );
						virtual ~Quad();
						DirectX::BoundingBox Box;
						std::unique_ptr<Quad> Subquads[4];
					};
					std::unique_ptr<Quad> quads;
					std::unique_ptr<Quad> generateQuads( const Garm::Data::Terrain& terrain, unsigned int startX, unsigned int startZ, unsigned int endX, unsigned int endZ, int levels );
				};
			}
		}
	}
}

#endif