#include "Terrain.h"
#include "../Dependencies/lodePNG/lodepng.h"
#include <fstream>
#include <stdexcept>
#include <locale>

using Garm::Data::Terrain;

Terrain::Terrain(const std::string& heightmapPath) : elevation(), width(0), height(0), resolution(3)
{
	std::vector<unsigned char> png;
	lodepng::State state;
	lodepng::decode( png, width, height, heightmapPath, LodePNGColorType::LCT_RGB, 8U );

	elevation.resize( width * height );
	for (size_t i = 0; i < width * height; i++)
	{
		elevation[i] = (static_cast<int>(png[i * 3]) << 16) +
			(static_cast<int>(png[i * 3 + 1]) << 8) +
			static_cast<int>(png[i * 3 + 2]);
	}

}

Terrain::~Terrain()
{

}