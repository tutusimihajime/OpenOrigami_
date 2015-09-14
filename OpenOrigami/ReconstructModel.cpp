#include "ReconstructingModel.h"
#include <list>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Sparse>

using namespace std;
using namespace Eigen;

void relocationFaces(Model *mod)
{
	for (int i = 0; i < mod->faces.size(); ++i)
	{
		for (int j = 0; j < i; ++j)
		{

		}
	}
}
void bridgeEdges(Model *mod)
{

}
void moveOverlappedVertices(Model *mod)
{

}
void reconstructModel(Model *mod)
{
	// relocation facs
	reconstructModel(mod);
	// bridge edges
	bridgeEdges(mod);
	// move overlapped vertices
	moveOverlappedVertices(mod);
}