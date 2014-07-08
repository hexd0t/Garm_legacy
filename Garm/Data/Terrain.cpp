#include "Terrain.h"
#include "../Dependencies/lodePNG/lodepng.h"
#include "../TypeDefs.h"
#include <fstream>
#include <stdexcept>
#include <locale>

using Garm::Data::Terrain;

Terrain::Terrain( const std::string& heightmapPath, const int& heightRes, const int& tpu )
	: elevation(), width( 0 ), depth( 0 ), texelPerUnit( tpu ), heightResolution( heightRes )
{
	std::vector<unsigned char> png;
	lodepng::State state;
	lodepng::decode( png, width, depth, heightmapPath, LodePNGColorType::LCT_RGB, 8U );

	elevation.resize( width * depth );
	for (size_t i = 0; i < width * depth; i++)
	{
		elevation[i] = (static_cast<int>(png[i * 3]) << 16) +
			(static_cast<int>(png[i * 3 + 1]) << 8) +
			static_cast<int>(png[i * 3 + 2]);
	}
}


unsigned int Terrain::GetWidth() const
{
	return width;
}
unsigned int Terrain::GetDepth() const
{
	return depth;
}
int Terrain::GetTexelPerUnit() const
{
	return texelPerUnit;
}
int Terrain::GetHeightResolution() const
{
	return heightResolution;
}
unsigned int Terrain::GetElevation( unsigned int x, unsigned int z ) const
{
#if defined(_DEBUG)
	if (x >= width || z >= depth)
		throw std::runtime_error( "Invalid coordinates @" __FILE__ ":" S__LINE__ );
#endif
	return elevation[x + width * z];
}
float Terrain::GetElevation( float x, float z ) const
{
	return GetElevation( static_cast<unsigned int>(x * texelPerUnit), static_cast<unsigned int>(z * texelPerUnit) );
}

Terrain::~Terrain()
{

}