#ifndef PATHGRAPH_INTERFACE_HPP
#define PATHGRAPH_INTERFACE_HPP
#include "Vector3.hpp"
#include <list>
#include <vector>

namespace GraphGenerator
{
    class IPathGraph
    {
    public:
        virtual ~IPathGraph() = default;

        virtual void addTerrainTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
            int maxLayer, bool considerRadius) = 0;
        virtual void addRuntimeTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
            int maxLayer, bool considerRadius, int runtimeMeshIndex) = 0;
        virtual void removeRuntimeMesh(int runtimeMeshIndex) = 0;
        virtual void calculateTerrainPathGraph() = 0;
        virtual void calculateRuntimePathGraph() = 0;
        virtual int samplePosition(Vector3 position, float radius, int scc, Vector3& result) = 0;
        virtual int getComponentTotalCount() = 0;
        virtual int getComponentSize(int index) = 0;
        virtual std::pair<std::vector<Vector3>, std::vector<std::pair<int, int>>> getComponentGraph(int index, bool rotate) = 0;
        virtual std::vector<std::vector<Vector3>> getComponentColorGraph(int index, int layer) = 0;
        //virtual std::vector<std::vector<Vector3>> getComponentGridGraph(int index, int size) = 0;
        //virtual std::vector<std::vector<Vector3>> getComponentGridRotatedGraph(int index, int size) = 0;
    };

    IPathGraph* makePathGraph(float size, float radius, int minLayer);
    IPathGraph* makePathGraphWithMemoryPool(float size, float radius, int minLayer);
    void destroyPathGraph(IPathGraph* p);
}

#endif // !PATHGRAPH_INTERFACE_HPP