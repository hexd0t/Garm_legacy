#include "QuadTree.h"

using Garm::System::Render::Terrain::QuadTree;

QuadTree::QuadTree(const std::shared_ptr<Garm::Data::Terrain>& terrain, const char& levelCount) : QuadsMaxHeight(levelCount), QuadsMinHeight(levelCount)
{

}