#ifndef DATA_TERRAIN_H
#define DATA_TERRAIN_H

#include <string>
#include <vector>

namespace Garm
{
	namespace Data
	{
		class Terrain
		{
		public:
			Terrain( const std::string& heightmapPath, const int& heightResolution = 100, const int& texelPerUnit = 5 );
			virtual ~Terrain();

			unsigned int GetWidth() const;
			unsigned int GetDepth() const;
			int GetTexelPerUnit() const;
			int GetHeightResolution() const;
			unsigned int GetElevation( unsigned int x, unsigned int z ) const;
			float GetElevation( float x, float z ) const;

		private:
			std::vector<unsigned int> elevation;
			unsigned int width;
			unsigned int depth;
			int texelPerUnit;
			int heightResolution;
		};
	}
}

#endif