#ifndef PATHGRAPH_HPP
#define PATHGRAPH_HPP

#include "PathGraphInterface.hpp"
#include "Vector3.hpp"
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <span>
#include <vector>

namespace GraphGenerator
{
    template<typename OctreeType>
    class PathGraphDataClass
    {
    private:
        using OctreeNodeRef = typename OctreeType::OctreeNode::NodeRef;
        using SizeType = std::conditional_t<sizeof(OctreeNodeRef) == 4, std::uint32_t, std::size_t>;
        static SizeType constexpr elementSize = static_cast<SizeType>(sizeof(OctreeNodeRef));
        static SizeType constexpr metadataSize = static_cast<SizeType>(sizeof(SizeType) * 2);
        static_assert(sizeof(SizeType) == elementSize, "sizeof(size) needs to be same as sizeof(OctreeNodeRef) because they are going to be stored in the same array");
        struct Data
        {
            SizeType size;
            SizeType capacity;
            OctreeNodeRef data[1];
        };
        static_assert(offsetof(Data, data) == metadataSize);
        struct Deleter
        {
            void operator()(Data* p) const noexcept;
        };
        std::unique_ptr<Data, Deleter> data = nullptr;

    public:
        PathGraphDataClass() noexcept = default;
        bool valid() const noexcept;
        std::span<OctreeNodeRef> view() const noexcept;
        void add(OctreeNodeRef node);
        void remove(OctreeNodeRef node) noexcept;
    };

    template<typename OctreeType>
    class PathGraphNodeAStarInfoClass
    {
    public:
        using OctreeNodeRef = typename OctreeType::OctreeNode::NodeRef;

        OctreeNodeRef node = OctreeNodeRef{};
        OctreeNodeRef parent = OctreeNodeRef{};
        float gScore = std::numeric_limits<float>::max();
        float hScore = std::numeric_limits<float>::max();

        PathGraphNodeAStarInfoClass() = default;
        PathGraphNodeAStarInfoClass(OctreeNodeRef node, float gScore, float hScore);
        PathGraphNodeAStarInfoClass(OctreeNodeRef node, float hScore);
        float getEstimateCost() const;
    };

    template<typename OctreeType>
    class PathGraph : public IPathGraph
    {
    public:
        using Octree = OctreeType;
        using OctreeNode = typename Octree::OctreeNode;
        using OctreeNodeRef = typename Octree::NodeRef;
        using PathGraphNodeAStarInfo = PathGraphNodeAStarInfoClass<Octree>;
        Octree* octree;
        int nodesNumber;

        PathGraph(float size, float radius, int minLayer = 0, typename OctreeType::NodeAllocator&& nodeAllocator = {});
        PathGraph& operator=(PathGraph&&) = delete;
        ~PathGraph() override;

        void addTerrainTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3, int maxLayer, 
            bool considerRadius) override;
        void addRuntimeTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3, int maxLayer,
            bool considerRadius, int runtimeMeshIndex) override;
        void removeRuntimeMesh(int runtimeMeshIndex) override;
        void calculateTerrainPathGraph() override;
        void calculateRuntimePathGraph() override;
        int samplePosition(Vector3 position, float radius, int scc, Vector3& result) override;
        int getComponentTotalCount() override;
        int getComponentSize(int index) override;
        std::pair<std::vector<Vector3>, std::vector<std::pair<int, int>>> getComponentGraph(int index, bool rotate) override;
        std::vector<std::vector<Vector3>> getComponentColorGraph(int index, int layer) override;
        //std::vector<std::vector<Vector3>> getComponentGridGraph(int index, int size) override;
        //std::vector<std::vector<Vector3>> getComponentGridRotatedGraph(int index, int size) override;
    };
}

#endif // !PATHGRAPH_HPP