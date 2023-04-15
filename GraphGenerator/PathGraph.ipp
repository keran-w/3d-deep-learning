#ifndef _PATHGRAPH_IPP_
#define _PATHGRAPH_IPP_
#include "PathGraph.hpp"
#include "SimpleHashSet.hpp"
#include "SimpleHashMap.hpp"
#include "Matrix3.hpp"

#include <iostream>

namespace GraphGenerator
{
    template<typename OctreeType>
    void PathGraphDataClass<OctreeType>::Deleter::operator()(Data* p) const noexcept
    {
        std::free(p);
    }

    template<typename OctreeType>
    bool PathGraphDataClass<OctreeType>::valid() const noexcept
    {
        return static_cast<bool>(data);
    }

    template<typename OctreeType>
    std::span<typename PathGraphDataClass<OctreeType>::OctreeNodeRef> PathGraphDataClass<OctreeType>::view() const noexcept
    {
        if (data)
        {
            return { data->data, data->data + data->size };
        }
        return {};
    }

    template<typename OctreeType>
    void PathGraphDataClass<OctreeType>::add(OctreeNodeRef node)
    {
        SizeType size = 0;
        // this will cause the initial vector to have capacity of 6
        SizeType capacity = 4;
        if (data)
        {
            size = data->size;
            capacity = data->capacity;
            if (size < capacity)
            {
                data->data[size] = node;
                // update size
                data->size = size + 1;
                return;
            }
        }
        // Vector not large enough, grow by 1.5x
        // Why 1.5x:
        // https://github.com/facebook/folly/blob/main/folly/docs/FBVector.md#memory-handling
        capacity += capacity / 2;
        // std::realloc works even with null pointer
        // so no need to check the value of previous pointer
        void* newPointer = std::realloc(data.get(), metadataSize + capacity * elementSize);
        if (newPointer == nullptr)
        {
            throw std::bad_alloc{};
        }
        data.release();
        data.reset(static_cast<Data*>(newPointer));
        data->data[size] = node;
        data->capacity = capacity;
        data->size = size + 1;
    }

    template<typename OctreeType>
    void PathGraphDataClass<OctreeType>::remove(OctreeNodeRef node) noexcept
    {
        if (data == nullptr)
        {
            return;
        }
        SizeType oldSize = data->size;
        OctreeNodeRef* begin = data->data;
        data->size = static_cast<SizeType>(std::remove(begin, begin + oldSize, node) - begin);
        if (data->size == 0)
        {
            data = nullptr;
            return;
        }
        if (data->size >= oldSize / 2)
        {
            return;
        }
        // shrink if necessary
        void* newPointer = std::realloc(data.get(), metadataSize + data->size * elementSize);
        if (newPointer == nullptr)
        {
            // do not throw if std::realloc failed
            return;
        }
        data.release();
        data.reset(static_cast<Data*>(newPointer));
        data->capacity = data->size;
    }

    template<typename OctreeType>
    PathGraph<OctreeType>::PathGraph(float size, float radius, int minLayer, typename OctreeType::NodeAllocator&& nodeAllocator)
    {
        this->octree = new Octree{ this, size, radius, minLayer, std::move(nodeAllocator) };
        nodesNumber = 0;
    }

    template<typename OctreeType>
    PathGraph<OctreeType>::~PathGraph()
    {
        delete this->octree;
    }

    template<typename OctreeType>
    void PathGraph<OctreeType>::addTerrainTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
        int maxLayer, bool considerRadius)
    {
        octree->addTerrainTriangleMesh(point1, point2, point3, maxLayer, considerRadius);
    }

    template<typename OctreeType>
    void PathGraph<OctreeType>::addRuntimeTriangleMesh(Vector3 const& point1, Vector3 const& point2, Vector3 const& point3,
        int maxLayer, bool considerRadius, int runtimeMeshIndex)
    {
        octree->addRuntimeTriangleMesh(point1, point2, point3, maxLayer, runtimeMeshIndex, considerRadius);
    }

    template<typename OctreeType>
    void PathGraph<OctreeType>::removeRuntimeMesh(int runtimeMeshIndex)
    {
        octree->removeRuntimeMesh(runtimeMeshIndex);
    }

    template<typename OctreeType>
    void PathGraph<OctreeType>::calculateTerrainPathGraph()
    {
        octree->calculateTerrainPathGraph();
    }

    template<typename OctreeType>
    void PathGraph<OctreeType>::calculateRuntimePathGraph()
    {
        octree->calculateRuntimePathGraph();
    }

    template<typename OctreeType>
    int PathGraph<OctreeType>::samplePosition(Vector3 position, float radius, int scc, Vector3& result)
    {
        return octree->samplePosition(position, radius, scc, result);
    }

    template<typename OctreeType>
    int PathGraph<OctreeType>::getComponentTotalCount()
    {
        return static_cast<int>(octree->componentMap.size());
    }

    template<typename OctreeType>
    int PathGraph<OctreeType>::getComponentSize(int index)
    {
        return octree->componentMap[index].second;
    }

    std::vector<Vector3> static inline centerAndScale(const std::vector<Vector3>& data) {
        Vector3 meanPoint = mean(data);

        std::vector<Vector3> centeredData;
        centeredData.reserve(data.size());
        std::transform(data.begin(), data.end(), std::back_inserter(centeredData),
            [&meanPoint](const Vector3& point) { return point - meanPoint; });

        auto maxX = std::numeric_limits<float>::min();
        auto maxY = std::numeric_limits<float>::min();
        auto maxZ = std::numeric_limits<float>::min();
        for (const auto& point : centeredData)
        {
            maxX = point.x > maxX ? point.x : maxX;
            maxY = point.y > maxY ? point.y : maxY;
            maxZ = point.z > maxZ ? point.z : maxZ;
        }

        std::vector<Vector3> scaledData;
        scaledData.reserve(centeredData.size());
        std::transform(centeredData.begin(), centeredData.end(), std::back_inserter(scaledData), [&](const Vector3& point)
        {
            Vector3 result{ 0.5f, 0.5f, 0.5f };
            result.x += point.x * 0.5f / maxX;
            result.y += point.y * 0.5f / maxY;
            result.z += point.z * 0.5f / maxZ;
            return result;
        });

        return scaledData;
    }

    std::vector<Vector3> rotateAndScale(const std::vector<Vector3>& data)
    {
        Vector3 meanPoint = mean(data);

        std::vector<Vector3> centeredData;
        centeredData.reserve(data.size());
        std::transform(data.begin(), data.end(), std::back_inserter(centeredData),
            [&meanPoint](const Vector3& point) { return point - meanPoint; });

        Matrix3 covarianceMatrix;
        covarianceMatrix.computeCovariance(centeredData);

        // Here we should find the eigenvectors of the covariance matrix.
        // However, computing eigenvectors for a 3x3 matrix without using a library is complex.
        // Instead, we will use the covariance matrix directly as a rotation matrix.
        // Note that this is an approximation and may not produce accurate PCA results.

        std::vector<Vector3> rotatedData;
        rotatedData.reserve(data.size());
        std::transform(centeredData.begin(), centeredData.end(), std::back_inserter(rotatedData), [&covarianceMatrix](const Vector3& point) { return covarianceMatrix * point; });

        return centerAndScale(rotatedData);
    }

    template<typename OctreeType>
    std::pair<std::vector<Vector3>, std::vector<std::pair<int, int>>> PathGraph<OctreeType>::getComponentGraph(int index, bool rotate)
    {
        std::vector<OctreeNode*> leaves;
        octree->root->leaves(leaves);
        std::vector<Vector3> resultPositions;
        std::unordered_map<OctreeNode*, int> indexMap;
        for (OctreeNode* q : leaves)
        {
            q->runtimeMoveableCounter = 0;
            if (q->pathGraphConnectComponentIndex == index)
            {
                indexMap.insert({ q, static_cast<int>(indexMap.size()) });
                resultPositions.push_back(q->centerPosition);
            }
        }

        std::vector<std::pair<int, int>> resultLinks;
        for (OctreeNode* q : leaves)
        {
            if (q->pathGraphConnectComponentIndex == index)
            {
                for (auto& toRef : q->pathGraphEdges.view())
                {
                    resultLinks.push_back({ indexMap[q], indexMap[octree->resolve(toRef)] });
                }
            }
        }

        if (rotate)
        {
            return { rotateAndScale(resultPositions), resultLinks };
        }
        else
        {
            return { resultPositions, resultLinks };
        }
    }

    template<typename OctreeType>
    std::vector<std::vector<Vector3>> PathGraph<OctreeType>::getComponentColorGraph(int index, int layer)
    {
        std::vector<OctreeNode*> leaves;
        octree->root->leaves(leaves);
        std::unordered_map<OctreeNode*, int> indexMap;
        Vector3 center{ .x = 0, .y = 0, .z = 0 };
        for (OctreeNode* q : leaves)
        {
            q->runtimeMoveableCounter = 0;
            if (q->pathGraphConnectComponentIndex == index)
            {
                indexMap.insert({ q, static_cast<int>(indexMap.size()) });
                center = center + q->centerPosition;
            }
        }
        center = center / indexMap.size();

        std::vector<std::vector<Vector3>> result(indexMap.size(), std::vector<Vector3>(indexMap.size(), Vector3{ .x = 0, .y = 0, .z = 0 }));
        for (OctreeNode* q : leaves)
        {
            if (q->pathGraphConnectComponentIndex == index)
            {
                for (auto& toRef : q->pathGraphEdges.view())
                {
                    auto from = q;
                    auto to = octree->resolve(toRef);
                    auto& pos = result[indexMap[from]][indexMap[to]];
                    //pos.x = 0;
                    //pos.y = 1;
                    //pos.z = 1;
                    pos.x = (from->centerPosition - to->centerPosition).length() * std::pow(2, layer);
                    pos.y = dot((from->centerPosition - to->centerPosition).normalized(), (center - to->centerPosition).normalized()) / 2 + 0.5f;
                    pos.z = dot((to->centerPosition - from->centerPosition).normalized(), (center - from->centerPosition).normalized()) / 2 + 0.5f;
                }
            }
        }
        return result;
    }

    //template<typename OctreeType>
    //std::vector<std::vector<int>> PathGraph<OctreeType>::getComponentGridGraph(int index, int size)
    //{
    //    return std::vector<std::vector<Vector3>>();
    //}

    //template<typename OctreeType>
    //std::vector<std::vector<Vector3>> PathGraph<OctreeType>::getComponentGridRotatedGraph(int index, int size)
    //{
    //    return std::vector<std::vector<Vector3>>();
    //}
}
#endif // !_PATHGRAPH_IPP_