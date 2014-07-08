#include "World.h"

using Garm::Data::World;

World::World() : physics(), terrain(new Garm::Data::Terrain("map.png")), camera()
{

}

World::~World()
{

}