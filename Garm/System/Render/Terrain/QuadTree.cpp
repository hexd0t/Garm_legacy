#include "QuadTree.h"
#include <cmath>
#include "../../../TypeDefs.h"
#include <stdexcept>

using Garm::System::Render::Terrain::QuadTree;

QuadTree::QuadTree( const Garm::Data::Terrain& terrain, int levels )
{
	quads = generateQuads( terrain, 0U, 0U, terrain.GetWidth() - 1U, terrain.GetDepth() - 1U, levels );
}

std::unique_ptr<QuadTree::Quad> QuadTree::generateQuads( const Garm::Data::Terrain& terrain, unsigned int startX, unsigned int startZ, unsigned int endX, unsigned int endZ, int levels )
{
	if (levels == 1)
	{
		unsigned int min = terrain.GetElevation( startX, startZ );
		unsigned int max = terrain.GetElevation( startX, startZ );
		unsigned int cur = 0;
		for (unsigned int x = 0; x <= endX; ++x)
		{
			for (unsigned int z = 0; z <= endZ; ++z)
			{
				cur = terrain.GetElevation( x, z );
				if (cur < min)
					min = cur;
				if (cur > max)
					max = cur;
			}
		}

		return std::unique_ptr<QuadTree::Quad>( new QuadTree::Quad(
			static_cast<float>(startX)	/ terrain.GetTexelPerUnit(),
			static_cast<float>(min)		/ terrain.GetHeightResolution(),
			static_cast<float>(startZ)	/ terrain.GetTexelPerUnit(),
			static_cast<float>(endX)	/ terrain.GetTexelPerUnit(), 
			static_cast<float>(max)		/ terrain.GetHeightResolution(),
			static_cast<float>(endZ)	/ terrain.GetTexelPerUnit() ) );
	}

	{
		unsigned int subWidth = (endX - startX)/2;
		unsigned int subDepth = (endZ - startZ)/2;

		return std::unique_ptr<QuadTree::Quad>( new QuadTree::Quad(
			generateQuads( terrain, startX, startZ, startX + subWidth, startZ + subDepth, levels - 1 ),
			generateQuads( terrain, startX, startZ + subDepth, startX + subWidth, endZ , levels - 1 ),
			generateQuads( terrain, startX + subWidth, startZ, endX, startZ + subDepth, levels - 1 ),
			generateQuads( terrain, startX + subWidth, startZ + subDepth, endX, endZ, levels - 1 )
			) );
	}
}

QuadTree::~QuadTree()
{

}

QuadTree::Quad::Quad( std::unique_ptr<Quad>&& subA, std::unique_ptr<Quad>&& subB, std::unique_ptr<Quad>&& subC, std::unique_ptr<Quad>&& subD )
{
	DirectX::BoundingBox::CreateMerged( Box, subA->Box, subB->Box );
	DirectX::BoundingBox tmp;
	DirectX::BoundingBox::CreateMerged( tmp, Box, subC->Box );
	DirectX::BoundingBox::CreateMerged( Box, tmp, subD->Box );
	Subquads[0] = std::move(subA);
	Subquads[1] = std::move(subB);
	Subquads[2] = std::move(subC);
	Subquads[3] = std::move(subD);
}

QuadTree::Quad::Quad(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) : Box()
{
	Subquads[0] = std::unique_ptr<Quad>();
	Subquads[1] = std::unique_ptr<Quad>();
	Subquads[2] = std::unique_ptr<Quad>();
	Subquads[3] = std::unique_ptr<Quad>();
	DirectX::BoundingBox::CreateFromPoints( Box,
		DirectX::XMVectorSet( minX, minY, minZ, 1.0f ),
		DirectX::XMVectorSet( maxX, maxY, maxZ, 1.0f ) );
}

QuadTree::Quad::~Quad()
{

}