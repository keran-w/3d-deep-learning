#ifndef OCTREE_HPP
#define OCTREE_HPP

#include "AllocatorTraits.hpp"
#include "PathGraph.hpp"
#include "Vector3.hpp"
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace GraphGenerator
{
    template<typename OctreeType>
    class PathGraph;
    template<typename OctreeType>
    class PathGraphDataClass;
    template<typename OctreeType>
    class PathGraphNode;

    template<typename Allocator>
    class Octree
    {
    public:
        using PathGraphData = PathGraphDataClass<Octree>;
        
        class OctreeNode
        {
        public:
            using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<OctreeNode>;
            using NodeRef = typename AllocatorTraits<NodeAllocator>::handle;
            static unsigned int constexpr invalidComponentIndex = 0;
            static unsigned int constexpr maxComponentIndex = 0xfffff; // 2^20 - 1

            NodeRef parent = {};
            NodeRef children = {};

            Vector3 centerPosition;

            // At least on MSVC, 
            // bit fields of different types might introduce extra padding,
            // which increases the Octree node size.
            // So we have to use int instead of bool to avoid padding.
            // Do not change the order!!! It is carefully designed for padding!
            // Max layer = 15!
            unsigned int tree : 16;
            unsigned int worldIndex0 : 16;
            unsigned int worldIndex1 : 16;
            unsigned int worldIndex2 : 16;

            unsigned int layer : 4;
            unsigned int isContainsMoveableChildren : 1 = false;  // if its children contain meshes
            unsigned int isMoveable : 1 = false;  // if itself contains a mesh
            unsigned int isContainsRuntimeMoveableChildren : 1 = false;  // if its children contain runtime meshes
            unsigned int runtimeMoveableCounter : 5 = 0;
            unsigned int pathGraphConnectComponentIndex : 20 = invalidComponentIndex;

            PathGraphData pathGraphEdges = {};

            OctreeNode(unsigned int tree, int layer, OctreeNode* parent, int relativeX, int relativeY, int relativeZ);
            OctreeNode(OctreeNode&&) = delete;
            ~OctreeNode();
            bool instantiateChildren();

            inline static constexpr Vector3 cornerDirections[2][2][2] =
            {
                {
                    { Vector3{.x = 1, .y = 1, .z = 1 }, Vector3{.x = 1, .y = 1, .z = -1 } },
                    { Vector3{.x = 1, .y = -1, .z = 1 }, Vector3{.x = 1, .y = -1, .z = -1 } },
                },
                {
                    { Vector3{.x = -1, .y = 1, .z = 1 }, Vector3{.x = -1, .y = 1, .z = -1 } },
                    { Vector3{.x = -1, .y = -1, .z = 1 }, Vector3{.x = -1, .y = -1, .z = -1 } },
                }
            };

            float size() const;
            void leaves(std::vector<OctreeNode*>& result);
            bool contains(Vector3 const& point);
            bool intersectWithTriangle(Vector3 point1, Vector3 point2, Vector3 point3, float expansion);

            void addTerrainTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3, int maxLayer,
                float expansion, bool wasMoveable = false);
            void addRuntimeTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3, int maxLayer,
                float expansion, int runtimeMeshIndex, std::unordered_set<OctreeNode*>& influencedOctreeNodes, bool wasMoveable = false);
            void checkContainsRuntimeMoveableChildrenWhenRemove();
            void removeRuntimeMesh(int runtimeMeshIndex);
        };

        using NodeRef = typename OctreeNode::NodeRef;
        using NodeAllocator = typename OctreeNode::NodeAllocator;
        using NodeAllocatorTraits = AllocatorTraits<NodeAllocator>;
        NodeAllocator nodeAllocator;

        unsigned int index;
        float size;
        float radius;
        int minLayer;

        OctreeNode* root;
        PathGraph<Octree>* graph;
        std::size_t numberOfNodes = 0;
        std::unordered_map<int, std::pair<OctreeNode*, int>> componentMap;  // <index, <node*, size>>

        std::map<int, std::unordered_set<OctreeNode*>> runtimeMeshIndexToNodes;
        std::unordered_set<OctreeNode*> toRecalculatePathGraph;

        inline static constexpr int adjacentDirections[6][3] =
        {
            { 1, 0, 0 },
            { -1, 0, 0 },
            { 0, 1, 0 },
            { 0, -1, 0 },
            { 0, 0, 1 },
            { 0, 0, -1 }
        };

        inline static std::priority_queue<unsigned int> availableIndex;
        inline static std::vector<Octree*> forest;

        Octree(PathGraph<Octree>* graph, float size, float radius, int minLayer, NodeAllocator&& nodeAllocator);
        Octree& operator=(Octree&&) = delete;
        ~Octree();
        void addTerrainTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3, int maxLayer, 
            bool considerRadius);
        void addRuntimeTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3, int maxLayer,
            bool considerRadius, int runtimeMeshIndex);
        void removeRuntimeMesh(int runtimeMeshIndex);

        OctreeNode* positionToNode(Vector3 const& position);
        bool lineOfSight(Vector3 const& from, Vector3 const& to);
        // 0 = +x, 1 = -x, 2 = +y, 3 = -y, 4 = +z, 5 = -z
        OctreeNode* findAdjacentNode(OctreeNode* node, int directionIndex);
        void updateSCC();
        void calculateTerrainPathGraph();
        void calculateRuntimePathGraph();
        int samplePosition(Vector3 position, float radius, int scc, Vector3& result);

        OctreeNode* allocateNodes(std::size_t count);
        void deallocateNodes(OctreeNode* memory, std::size_t count);
        void constructNode(OctreeNode* memory, int layer, OctreeNode* parent, int relativeX, int relativeY, int relativeZ);
        void destroyNode(OctreeNode* object);
        NodeRef translate(OctreeNode* object);
        OctreeNode* resolve(NodeRef object);

    private:
        static bool intersectRayBox
        (
            Vector3 const& min, Vector3 const& max, Vector3 const& origin,
            float invDirX, float invDirY, float invDirZ, float length
        );
        OctreeNode* findAdjacentNode(int x, int y, int z, int layer);
    };
}

#endif // !OCTREE_HPP