#include "World.h"

using Garm::Data::World;

World::World() : physics(), terrain(Garm::Data::Terrain("map.png")), camera()
{

}

World::~World()
{

}