#include "PathGraphInterface.hpp"
#include "PathGraph.hpp"
#include "Windows/MonotonicAllocator.hpp"
#include <memory>

#include "Octree.ipp"
#include "PathGraph.ipp"

namespace GraphGenerator
{
    template<typename T>
    using Allocate = T * (int numberOfElements);

    IPathGraph* makePathGraph(float size, float radius, int minLayer) 
    {
        using OctreeType = Octree<std::allocator<void>>;
        return new PathGraph<OctreeType>{ size, radius, minLayer };
    }
    IPathGraph* makePathGraphWithMemoryPool(float size, float radius, int minLayer)
    {
        using OctreeType = Octree<Windows::MonotonicAllocator<void>>;
        OctreeType::NodeAllocator allocator{ 48 * 1024 * 1024 };
        PathGraph<OctreeType>* pointer = new PathGraph<OctreeType>{ size, radius, minLayer, std::move(allocator) };
        return pointer;
    }
    void destroyPathGraph(IPathGraph* p) { return delete p; }
    void addTerrainTriangleMesh(IPathGraph* p, Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
        int maxLayer, bool considerRadius)
    {
        return p->addTerrainTriangleMesh(point1, point2, point3, maxLayer, considerRadius);
    }
    void addTerrainTriangleArrayMesh(IPathGraph* p, int length, Vector3* triangles, int maxLayer, bool considerRadius)
    {
        for (int i = 0; i < length; i++)
        {
            int i3 = i * 3;
            p->addTerrainTriangleMesh(triangles[i3 + 0], triangles[i3 + 1], triangles[i3 + 2], maxLayer, considerRadius);
        }
        return;
    }
    void addRuntimeTriangleMesh(IPathGraph* p, Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
        int maxLayer, bool considerRadius, int runtimeMeshIndex)
    {
        return p->addRuntimeTriangleMesh(point1, point2, point3, maxLayer, considerRadius, runtimeMeshIndex);
    }
    void addRuntimeTriangleArrayMesh(IPathGraph* p, int length, Vector3* triangles, int maxLayer, bool considerRadius, int runtimeMeshIndex)
    {
        for (int i = 0; i < length; i++)
        {
            int i3 = i * 3;
            p->addRuntimeTriangleMesh(triangles[i3 + 0], triangles[i3 + 1], triangles[i3 + 2], maxLayer, considerRadius, runtimeMeshIndex);
        }
        return;
    }
    void removeRuntimeMesh(IPathGraph* p, int runtimeMeshIndex) { return p->removeRuntimeMesh(runtimeMeshIndex); }
    void calculateTerrainPathGraph(IPathGraph* p) { return p->calculateTerrainPathGraph(); }
    void calculateRuntimePathGraph(IPathGraph* p) { return p->calculateRuntimePathGraph(); }
    int samplePosition(IPathGraph* p, Vector3 position, float radius, int scc, Vector3& result) { return p->samplePosition(position, radius, scc, result); }
    int getComponentTotalCount(IPathGraph* p) { return p->getComponentTotalCount(); }
    int getComponentSize(IPathGraph* p, int index) { return p->getComponentSize(index); }
    std::pair<std::vector<Vector3>, std::vector<std::pair<int, int>>> getComponentGraph(IPathGraph* p, int index, bool rotate) { return p->getComponentGraph(index, rotate); }
    std::vector<std::vector<Vector3>> getComponentColorGraph(IPathGraph* p, int index, int layer) { return p->getComponentColorGraph(index, layer); }
    //std::vector<std::vector<Vector3>> getComponentGridGraph(IPathGraph* p, int index, int size) { return p->getComponentGridGraph(index, size); }
    //std::vector<std::vector<Vector3>> getComponentGridRotatedGraph(IPathGraph* p, int index, int size) { return p->getComponentGridRotatedGraph(index, size); }
}