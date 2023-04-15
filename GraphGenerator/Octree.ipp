#ifndef _OCTREE_IPP_
#define _OCTREE_IPP_
#include "Octree.hpp"
#include "Vector3.hpp"
#include <deque>
#include <stack>

namespace GraphGenerator
{
    template<typename Allocator>
    Octree<Allocator>::OctreeNode::OctreeNode(unsigned int tree, int layer, OctreeNode* parent, int relativeX, int relativeY, int relativeZ)
    {
        this->tree = tree;
        this->layer = layer;
        this->parent = Octree::forest[tree]->translate(parent);
        if (parent != nullptr)
        {
            worldIndex0 = (parent->worldIndex0 << 1) + relativeX;
            worldIndex1 = (parent->worldIndex1 << 1) + relativeY;
            worldIndex2 = (parent->worldIndex2 << 1) + relativeZ;
            centerPosition = parent->centerPosition + size() * cornerDirections[relativeX][relativeY][relativeZ];
        }
        else
        {
            worldIndex0 = relativeX;
            worldIndex1 = relativeY;
            worldIndex2 = relativeZ;
            centerPosition = Vector3{ .x = 0, .y = 0, .z = 0 };
        }
    }

    template<typename Allocator>
    Octree<Allocator>::OctreeNode::~OctreeNode()
    {
        OctreeNode* memory = Octree::forest[tree]->resolve(children);
        if (memory != nullptr)
        {
            Octree::forest[tree]->destroyNode(memory + 0);
            Octree::forest[tree]->destroyNode(memory + 1);
            Octree::forest[tree]->destroyNode(memory + 2);
            Octree::forest[tree]->destroyNode(memory + 3);
            Octree::forest[tree]->destroyNode(memory + 4);
            Octree::forest[tree]->destroyNode(memory + 5);
            Octree::forest[tree]->destroyNode(memory + 6);
            Octree::forest[tree]->destroyNode(memory + 7);
            Octree::forest[tree]->deallocateNodes(memory, 8);
        }
    }

    template<typename Allocator>
    bool Octree<Allocator>::OctreeNode::instantiateChildren()
    {
        if (children != NodeRef{})
        {
            return false;
        }
        OctreeNode* memory = Octree::forest[tree]->allocateNodes(8);
        children = Octree::forest[tree]->translate(memory);
        Octree::forest[tree]->constructNode(memory + 0, layer + 1, this, 0, 0, 0);
        Octree::forest[tree]->constructNode(memory + 1, layer + 1, this, 0, 0, 1);
        Octree::forest[tree]->constructNode(memory + 2, layer + 1, this, 0, 1, 0);
        Octree::forest[tree]->constructNode(memory + 3, layer + 1, this, 0, 1, 1);
        Octree::forest[tree]->constructNode(memory + 4, layer + 1, this, 1, 0, 0);
        Octree::forest[tree]->constructNode(memory + 5, layer + 1, this, 1, 0, 1);
        Octree::forest[tree]->constructNode(memory + 6, layer + 1, this, 1, 1, 0);
        Octree::forest[tree]->constructNode(memory + 7, layer + 1, this, 1, 1, 1);
        if (layer + 1 < Octree::forest[tree]->minLayer)
        {
            memory[0].instantiateChildren();
            memory[1].instantiateChildren();
            memory[2].instantiateChildren();
            memory[3].instantiateChildren();
            memory[4].instantiateChildren();
            memory[5].instantiateChildren();
            memory[6].instantiateChildren();
            memory[7].instantiateChildren();
        }
        return true;
    }

    template<typename Allocator>
    float Octree<Allocator>::OctreeNode::size() const
    {
        return Octree::forest[tree]->size / (1 << layer);
    }

    template<typename Allocator>
    void Octree<Allocator>::OctreeNode::leaves(std::vector<OctreeNode*>& result)
    {
        OctreeNode* childrenBase = Octree::forest[tree]->resolve(children);
        if (childrenBase != nullptr)
        {
            childrenBase[0].leaves(result);
            childrenBase[1].leaves(result);
            childrenBase[2].leaves(result);
            childrenBase[3].leaves(result);
            childrenBase[4].leaves(result);
            childrenBase[5].leaves(result);
            childrenBase[6].leaves(result);
            childrenBase[7].leaves(result);
        }
        else
        {
            result.push_back(this);
        }
    }

    template<typename Allocator>
    bool Octree<Allocator>::OctreeNode::contains(Vector3 const& point)
    {
        Vector3 diff = point - centerPosition;
        return std::abs(diff.x) <= size() && std::abs(diff.y) <= size() && std::abs(diff.z) <= size();
    }

    template<typename Allocator>
    bool Octree<Allocator>::OctreeNode::intersectWithTriangle(Vector3 point1, Vector3 point2, Vector3 point3, float expansion)
    {
        float r = size() + expansion;
        point1 = point1 - centerPosition;
        point2 = point2 - centerPosition;
        point3 = point3 - centerPosition;
        if (std::min(point1.x, std::min(point2.x, point3.x)) >= r ||
            std::max(point1.x, std::max(point2.x, point3.x)) <= -r ||
            std::min(point1.y, std::min(point2.y, point3.y)) >= r ||
            std::max(point1.y, std::max(point2.y, point3.y)) <= -r ||
            std::min(point1.z, std::min(point2.z, point3.z)) >= r ||
            std::max(point1.z, std::max(point2.z, point3.z)) <= -r)
        {
            return false;
        }

        Vector3 n = cross(point2 - point1, point3 - point1);
        if (std::abs(dot(point1, n)) > r * (std::abs(n.x) + std::abs(n.y) + std::abs(n.z)))
        {
            return false;
        }

        for (int i = 0; i < 3; i++)
        {
            // j = 0
            Vector3 ai = Vector3{ .x = 0, .y = 0, .z = 0 };
            ai[i] = 1;
            Vector3 a = cross(ai, point3 - point2);
            float d1 = dot(point1, a);
            float d2 = dot(point2, a);
            float rr = r * (std::abs(a[(i + 1) % 3]) + std::abs(a[(i + 2) % 3]));
            if (std::min(d1, d2) > rr || std::max(d1, d2) < -rr)
            {
                return false;
            }
            // j = 1
            a = cross(ai, point1 - point3);
            d1 = dot(point2, a);
            d2 = dot(point3, a);
            rr = r * (std::abs(a[(i + 1) % 3]) + std::abs(a[(i + 2) % 3]));
            if (std::min(d1, d2) > rr || std::max(d1, d2) < -rr)
            {
                return false;
            }
            // j = 2
            a = cross(ai, point2 - point1);
            d1 = dot(point3, a);
            d2 = dot(point1, a);
            rr = r * (std::abs(a[(i + 1) % 3]) + std::abs(a[(i + 2) % 3]));
            if (std::min(d1, d2) > rr || std::max(d1, d2) < -rr)
            {
                return false;
            }
        }
        return true;
    }

    template<typename Allocator>
    void Octree<Allocator>::OctreeNode::addTerrainTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3, int maxLayer,
        float expansion, bool wasMoveable)
    {
        if (isMoveable || wasMoveable || (layer >= Octree::forest[tree]->minLayer && expansion - size() > 0 &&
            ((point1 + point2 + point3) / 3 - centerPosition).sqrLength() < (expansion - size()) * (expansion - size())))
        {
            isContainsMoveableChildren = true;
            isMoveable = true;
            return;
        }
        if (intersectWithTriangle(point1, point2, point3, expansion))
        {
            isContainsMoveableChildren = true;
            if (layer < maxLayer)
            {
                instantiateChildren();
                OctreeNode* childrenBase = Octree::forest[tree]->resolve(children);
                childrenBase[0].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
                childrenBase[1].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
                childrenBase[2].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
                childrenBase[3].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
                childrenBase[4].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
                childrenBase[5].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
                childrenBase[6].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
                childrenBase[7].addTerrainTriangleMesh(point1, point2, point3, maxLayer, expansion, isMoveable);
            }
            else
            {
                isMoveable = true;
            }
        }
    }

    template<typename Allocator>
    void Octree<Allocator>::OctreeNode::addRuntimeTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
        int maxLayer, float expansion, int runtimeMeshIndex, std::unordered_set<OctreeNode*>& influencedOctreeNodes, bool wasMoveable)
    {
        if (isMoveable || wasMoveable)
        {
            isContainsMoveableChildren = true;
            isMoveable = true;
            return;
        }
        if (intersectWithTriangle(point1, point2, point3, expansion))
        {
            isContainsRuntimeMoveableChildren = true;
            if (layer < maxLayer)
            {
                // This node was a leaf node before add runtime triangle, recalculate path edge is required.
                instantiateChildren();
                OctreeNode* childrenBase = Octree::forest[tree]->resolve(children);
                auto edgesView = pathGraphEdges.view();
                if (not edgesView.empty())
                {
                    Octree::forest[tree]->toRecalculatePathGraph.insert(this);
                    for (NodeRef i : edgesView)
                    {
                        Octree::forest[tree]->toRecalculatePathGraph.insert(Octree::forest[tree]->resolve(i));
                    }
                }
                childrenBase[0].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
                childrenBase[1].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
                childrenBase[2].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
                childrenBase[3].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
                childrenBase[4].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
                childrenBase[5].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
                childrenBase[6].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
                childrenBase[7].addRuntimeTriangleMesh(point1, point2, point3, maxLayer,
                    expansion, runtimeMeshIndex, influencedOctreeNodes, isMoveable);
            }
            else if (bool newElementInserted = influencedOctreeNodes.insert(this).second;
                newElementInserted == true)
            {
                runtimeMoveableCounter++;
                Octree::forest[tree]->toRecalculatePathGraph.insert(this);
                for (NodeRef toRef : pathGraphEdges.view())
                {
                    auto to = Octree::forest[tree]->resolve(toRef);
                    Octree::forest[tree]->toRecalculatePathGraph.insert(to);
                }
            }
        }
        // This is a new node which was created just now
        else if ((children == NodeRef{}) and (pathGraphEdges.view().empty()))
        {
            Octree::forest[tree]->toRecalculatePathGraph.insert(this);
        }
    }

    template<typename Allocator>
    void Octree<Allocator>::OctreeNode::checkContainsRuntimeMoveableChildrenWhenRemove()
    {
        OctreeNode* childrenBase = Octree::forest[tree]->resolve(children);
        if (childrenBase != nullptr)
        {
            if (childrenBase[0].isContainsRuntimeMoveableChildren)
            {
                return;
            }
            if (childrenBase[1].isContainsRuntimeMoveableChildren)
            {
                return;
            }
            if (childrenBase[2].isContainsRuntimeMoveableChildren)
            {
                return;
            }
            if (childrenBase[3].isContainsRuntimeMoveableChildren)
            {
                return;
            }
            if (childrenBase[4].isContainsRuntimeMoveableChildren)
            {
                return;
            }
            if (childrenBase[5].isContainsRuntimeMoveableChildren)
            {
                return;
            }
            if (childrenBase[6].isContainsRuntimeMoveableChildren)
            {
                return;
            }
            if (childrenBase[7].isContainsRuntimeMoveableChildren)
            {
                return;
            }
        }
        isContainsRuntimeMoveableChildren = false;
        OctreeNode* parentPointer = Octree::forest[tree]->resolve(parent);
        if (parentPointer != nullptr)
        {
            parentPointer->checkContainsRuntimeMoveableChildrenWhenRemove();
        }
    }

    template<typename Allocator>
    void Octree<Allocator>::OctreeNode::removeRuntimeMesh(int runtimeMeshIndex)
    {
        if (runtimeMoveableCounter != 0)
        {
            runtimeMoveableCounter--;
            if (runtimeMoveableCounter == 0)
            {
                checkContainsRuntimeMoveableChildrenWhenRemove();
            }
        }
    }

    template<typename Allocator>
    Octree<Allocator>::Octree(PathGraph<Octree>* graph, float size, float radius, int minLayer, NodeAllocator&& nodeAllocator) :
        nodeAllocator{ nodeAllocator }
    {
        if (Octree<Allocator>::availableIndex.empty())
        {
            this->index = Octree<Allocator>::forest.size();
            forest.push_back(this);
        }
        else
        {
            this->index = Octree::availableIndex.top();
            Octree::availableIndex.pop();
            forest[index] = this;
        }
        this->graph = graph;
        this->size = size;
        this->radius = radius;
        this->minLayer = minLayer;
        root = allocateNodes(1);
        constructNode(root, 0, nullptr, 0, 0, 0);
        root->instantiateChildren();
    }

    template<typename Allocator>
    Octree<Allocator>::~Octree()
    {
        destroyNode(root);
        deallocateNodes(root, 1);
        Octree::availableIndex.push(index);
        Octree::forest[index] = nullptr;
    }

    template<typename Allocator>
    void Octree<Allocator>::addTerrainTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
        int maxLayer, bool considerRadius)
    {
        root->addTerrainTriangleMesh(point1, point2, point3, maxLayer < 15 ? maxLayer : 15, considerRadius ? radius : 0);
    }

    template<typename Allocator>
    void Octree<Allocator>::addRuntimeTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
        int maxLayer, bool considerRadius, int runtimeMeshIndex)
    {
        root->addRuntimeTriangleMesh(point1, point2, point3, maxLayer < 15 ? maxLayer : 15, considerRadius ? radius : 0,
            runtimeMeshIndex, runtimeMeshIndexToNodes[runtimeMeshIndex]);
    }

    template<typename Allocator>
    void Octree<Allocator>::removeRuntimeMesh(int runtimeMeshIndex)
    {
        if (runtimeMeshIndexToNodes.find(runtimeMeshIndex) == runtimeMeshIndexToNodes.end())
        {
            return;
        }
        else
        {
            for (auto i : runtimeMeshIndexToNodes[runtimeMeshIndex])
            {
                i->removeRuntimeMesh(runtimeMeshIndex);
                if (i->runtimeMoveableCounter == 0)
                {
                    toRecalculatePathGraph.insert(i);
                }
            }
            runtimeMeshIndexToNodes.erase(runtimeMeshIndex);
        }
    }

    template<typename Allocator>
    typename Octree<Allocator>::OctreeNode* Octree<Allocator>::positionToNode(Vector3 const& position)
    {
        OctreeNode* node = root;
        while (node->children and node->runtimeMoveableCounter == 0)
        {
            Vector3 diff = position - node->centerPosition;
            unsigned offset = 4U * (diff.x <= 0) + 2U * (diff.y <= 0) + (diff.z <= 0);
            node = resolve(node->children + offset);
        }
        return node;
    }

    template<typename Allocator>
    bool Octree<Allocator>::lineOfSight(Vector3 const& from, Vector3 const& to)
    {
        float length = (from - to).length();
        Vector3 direction = (from - to) / length;
        float constexpr fLowest = std::numeric_limits<float>::lowest();
        float constexpr fMax = std::numeric_limits<float>::max();
        // prevent nan in intersectWithRayBox
        float invDirX = std::clamp(1.f / direction.x, fLowest, fMax);
        float invDirY = std::clamp(1.f / direction.y, fLowest, fMax);
        float invDirZ = std::clamp(1.f / direction.z, fLowest, fMax);

        struct Stack : std::stack<NodeRef, std::vector<NodeRef>>
        {
            Stack()
            {
                this->c.reserve(256);
            }
        };
        Stack workList{};
        workList.push(translate(root));
        while (workList.size() != 0)
        {
            OctreeNode* node = resolve(workList.top());
            workList.pop();
            if (!node->isContainsMoveableChildren && !node->isContainsRuntimeMoveableChildren)
            {
                continue;
            }
            // Due to some strange reason, " * 1.01" can eliminate "false positive"
            Vector3 constexpr oneWithEpsilon = Vector3{ .x = 1.01f, .y = 1.01f, .z = 1.01f };
            Vector3 enlargedSize = oneWithEpsilon * node->size();
            if (intersectRayBox(node->centerPosition - enlargedSize, node->centerPosition + enlargedSize, to, invDirX, invDirY, invDirZ, length))
            {
                if (node->children != NodeRef{})
                {
                    workList.push(node->children + 0);
                    workList.push(node->children + 1);
                    workList.push(node->children + 2);
                    workList.push(node->children + 3);
                    workList.push(node->children + 4);
                    workList.push(node->children + 5);
                    workList.push(node->children + 6);
                    workList.push(node->children + 7);
                }
                else
                {
                    if (node->isMoveable || node->runtimeMoveableCounter != 0)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // 0 = +x, 1 = -x, 2 = +y, 3 = -y, 4 = +z, 5 = -z
    template<typename Allocator>
    typename Octree<Allocator>::OctreeNode* Octree<Allocator>::findAdjacentNode(OctreeNode* node, int directionIndex)
    {
        return findAdjacentNode(node->worldIndex0 + adjacentDirections[directionIndex][0],
            node->worldIndex1 + adjacentDirections[directionIndex][1],
            node->worldIndex2 + adjacentDirections[directionIndex][2],
            node->layer);
    }

    template<typename Allocator>
    void Octree<Allocator>::updateSCC()
    {
        componentMap.clear();
        std::vector<OctreeNode*> leaves;
        leaves.reserve(numberOfNodes);
        root->leaves(leaves);
        int currentConnectComponentIndex = 1;
        int nodesNumber = 0;
        for (OctreeNode* q : leaves)
        {
            if (q->pathGraphEdges.valid())
            {
                nodesNumber++;
                if (q->pathGraphConnectComponentIndex == OctreeNode::invalidComponentIndex)
                {
                    int count = 1;
                    q->pathGraphConnectComponentIndex = currentConnectComponentIndex;
                    std::list<PathGraphData*> workList{ &(q->pathGraphEdges) };
                    while (not workList.empty())
                    {
                        PathGraphData* nextEdges = *workList.begin();
                        workList.pop_front();
                        for (NodeRef toRef : nextEdges->view())
                        {
                            OctreeNode* to = resolve(toRef);
                            if (to->pathGraphConnectComponentIndex != currentConnectComponentIndex)
                            {
                                count++;
                                to->pathGraphConnectComponentIndex = currentConnectComponentIndex;
                                workList.push_back(&(to->pathGraphEdges));
                            }
                        }
                    }
                    componentMap.insert({ currentConnectComponentIndex, { q, count } });
                    // Prevent overflow
                    if (currentConnectComponentIndex != OctreeNode::maxComponentIndex)
                    {
                        currentConnectComponentIndex++;
                    }
                }
            }
        }
        // Prevent overflow
        for (OctreeNode* q : leaves)
        {
            if (q->pathGraphConnectComponentIndex == OctreeNode::maxComponentIndex)
            {
                q->pathGraphConnectComponentIndex = OctreeNode::invalidComponentIndex;
            }
        }
        graph->nodesNumber = nodesNumber;
    }

    template<typename Allocator>
    void Octree<Allocator>::calculateTerrainPathGraph()
    {
        std::vector<OctreeNode*> leaves;
        root->leaves(leaves);
        for (OctreeNode* q : leaves)
        {
            q->pathGraphEdges = {};
        }
        for (OctreeNode* q : leaves)
        {
            if (q->isMoveable)
            {
                if (q->layer == 0)
                {
                    continue;
                }
                NodeRef nRef = translate(q);
                for (int i = 0; i < 6; i++)
                {
                    auto found = findAdjacentNode(q->worldIndex0 + adjacentDirections[i][0],
                        q->worldIndex1 + adjacentDirections[i][1],
                        q->worldIndex2 + adjacentDirections[i][2],
                        q->layer);
                    if (found == nullptr || !found->isMoveable)
                    {
                        continue;
                    }
                    if (found->layer < q->layer || found->children == NodeRef{})
                    {
                        auto qEdges = q->pathGraphEdges.view();
                        auto foundEdges = found->pathGraphEdges.view();
                        NodeRef nFoundRef = translate(found);
                        if (qEdges.end() == std::find(qEdges.begin(), qEdges.end(), nFoundRef))
                        {
                            q->pathGraphEdges.add(nFoundRef);
                        }
                        if (foundEdges.end() == std::find(foundEdges.begin(), foundEdges.end(), nRef))
                        {
                            found->pathGraphEdges.add(nRef);
                        }
                    }
                }
            }
        }
        updateSCC();
    }

    template<typename Allocator>
    void Octree<Allocator>::calculateRuntimePathGraph()
    {
        for (auto q : toRecalculatePathGraph)
        {
            NodeRef qRef = translate(q);
            for (auto i : q->pathGraphEdges.view())
            {
                resolve(i)->pathGraphEdges.remove(qRef);
            }
            q->pathGraphEdges = {};
            if (!q->isMoveable && q->runtimeMoveableCounter == 0 && q->children == NodeRef{})
            {
                q->pathGraphEdges = {};
            }
        }
        for (auto q : toRecalculatePathGraph)
        {
            if (!q->isMoveable && q->runtimeMoveableCounter == 0 && q->children == NodeRef{})
            {
                if (q->layer == 0)
                {
                    continue;
                }
                NodeRef nRef = translate(q);
                for (int i = 0; i < 6; i++)
                {
                    auto found = findAdjacentNode(q->worldIndex0 + adjacentDirections[i][0],
                        q->worldIndex1 + adjacentDirections[i][1],
                        q->worldIndex2 + adjacentDirections[i][2],
                        q->layer);
                    if (found == nullptr || found->isMoveable || found->runtimeMoveableCounter != 0) continue;
                    if (found->layer < q->layer || found->children == NodeRef{})
                    {
                        auto qEdges = q->pathGraphEdges.view();
                        auto foundEdges = found->pathGraphEdges.view();
                        NodeRef nFoundRef = translate(found);

                        if (qEdges.end() == std::find(qEdges.begin(), qEdges.end(), nFoundRef))
                        {
                            q->pathGraphEdges.add(nFoundRef);
                        }
                        if (foundEdges.end() == std::find(foundEdges.begin(), foundEdges.end(), nRef))
                        {
                            found->pathGraphEdges.add(nRef);
                        }
                    }
                }
            }
        }
        toRecalculatePathGraph.clear();
        updateSCC();
    }

    template<typename Allocator>
    int Octree<Allocator>::samplePosition(Vector3 position, float radius, int scc, Vector3& result)
    {
        if (std::abs(position.x) > size || std::abs(position.y) > size || std::abs(position.z) > size)
        {
            result = position;
            return 0;
        }
        std::deque<OctreeNode*> worklist;
        std::unordered_set<OctreeNode*> testedNodes;
        worklist.push_front(positionToNode(position));
        while (not worklist.empty())
        {
            auto node = worklist.front();
            worklist.pop_front();
            bool alreadyContains = not testedNodes.insert(node).second;
            if (node == nullptr || alreadyContains)
            {
                continue;
            }
            if (not node->isMoveable and node->runtimeMoveableCounter == 0
                and node->pathGraphEdges.valid() and (node->pathGraphConnectComponentIndex == scc or scc <= 0))
            {
                Vector3 diff = position - node->centerPosition;
                float max = std::max(std::max(std::abs(diff.x), std::abs(diff.y)), std::abs(diff.z));
                if (max > node->size())
                {
                    float scale = node->size() / max;
                    result = node->centerPosition + diff * scale;
                    return node->pathGraphConnectComponentIndex;
                }
                result = position;
                return node->pathGraphConnectComponentIndex;
            }

            for (int i = 0; i < 6; i++)
            {
                auto next = findAdjacentNode(node, i);
                if (next != nullptr)
                {
                    if (next != nullptr and testedNodes.find(next) == testedNodes.end()
                        and (next->centerPosition - position).sqrLength() < radius * radius)
                    {
                        worklist.push_back(next);
                    }
                }
            }
        }

        result = position;
        return -1;
    }

    template<typename Allocator>
    typename Octree<Allocator>::OctreeNode* Octree<Allocator>::allocateNodes(std::size_t count)
    {
        return NodeAllocatorTraits::allocate(nodeAllocator, count);
    }

    template<typename Allocator>
    void Octree<Allocator>::deallocateNodes(OctreeNode* memory, std::size_t count)
    {
        return NodeAllocatorTraits::deallocate(nodeAllocator, memory, count);
    }

    template<typename Allocator>
    void Octree<Allocator>::constructNode(OctreeNode* memory, int layer, OctreeNode* parent, int relativeX, int relativeY, int relativeZ)
    {
        ++numberOfNodes;
        return NodeAllocatorTraits::construct(nodeAllocator, memory, index, layer, parent, relativeX, relativeY, relativeZ);
    }

    template<typename Allocator>
    void Octree<Allocator>::destroyNode(OctreeNode* object)
    {
        --numberOfNodes;
        return NodeAllocatorTraits::destroy(nodeAllocator, object);
    }

    template<typename Allocator>
    typename Octree<Allocator>::NodeRef Octree<Allocator>::translate(OctreeNode* object)
    {
        return NodeAllocatorTraits::translate(nodeAllocator, object);
    }

    template<typename Allocator>
    typename Octree<Allocator>::OctreeNode* Octree<Allocator>::resolve(NodeRef object)
    {
        return NodeAllocatorTraits::resolve(nodeAllocator, object);
    }

    template<typename Allocator>
    bool Octree<Allocator>::intersectRayBox
    (
        Vector3 const& min, Vector3 const& max, Vector3 const& origin,
        float invDirX, float invDirY, float invDirZ, float length
    )
    {
        Vector3 minDiff = min - origin;
        Vector3 maxDiff = max - origin;
        float near = std::numeric_limits<float>::lowest();
        float far = std::numeric_limits<float>::max();

        static_assert(std::numeric_limits<double>::is_iec559, "Please use IEEE754, you weirdo");
        float t1x = minDiff.x * invDirX;
        float t1y = minDiff.y * invDirY;
        float t1z = minDiff.z * invDirZ;
        float t2x = maxDiff.x * invDirX;
        float t2y = maxDiff.y * invDirY;
        float t2z = maxDiff.z * invDirZ;
        float tMinX = std::min(t1x, t2x);
        float tMinY = std::min(t1y, t2y);
        float tMinZ = std::min(t1z, t2z);
        float tMaxX = std::max(t1x, t2x);
        float tMaxY = std::max(t1y, t2y);
        float tMaxZ = std::max(t1z, t2z);
        near = std::max(std::max(tMinX, tMinY), tMinZ);
        far = std::min(std::min(tMaxX, tMaxY), tMaxZ);
        return not (far < 0 or near >= length or near > far);
    }

    template<typename Allocator>
    typename Octree<Allocator>::OctreeNode* Octree<Allocator>::findAdjacentNode(int x, int y, int z, int layer)
    {
        int t = 1 << layer;
        if (x >= t || x < 0 || y >= t || y < 0 || z >= t || z < 0)
        {
            return nullptr;
        }
        OctreeNode* current = root;
        for (int l = 0; l < layer; l++)
        {
            t >>= 1;
            OctreeNode* childrenbase = resolve(current->children);
            if (childrenbase == nullptr)
            {
                return current;
            }
            current = childrenbase + (4 * (x / t) + 2 * (y / t) + (z / t));
            x %= t;
            y %= t;
            z %= t;
        }
        return current;
    }
}

#endif // !_OCTREE_IPP_