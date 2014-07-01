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
			Terrain( const std::string& heightmapPath );
			virtual ~Terrain();

		private:
			std::vector<unsigned int> elevation;
			unsigned int width;
			unsigned int height;
			int resolution;
		};
	}
}

#endif