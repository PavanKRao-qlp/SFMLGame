#pragma once
#include "EnginePCH.h"
#include "Math/Bounds.h"

namespace Umbra {

    constexpr int32 NullNode = -1;

    struct AABBTreeNode {
        Math::Bounds2D Aabb;
        int32 Parent = NullNode;
        int32 Left   = NullNode;
        int32 Right  = NullNode;
        int32 Height = -1; // -1 = free node

        // Leaf data
        uint32 BodyIndex = 0;

        // Free list link (union with Left when node is free)
        int32 NextFree = NullNode;

        inline bool IsLeaf() const {
            return Left == NullNode;
        }
    };

    class DynamicAABBTree {
    public:
        DynamicAABBTree();
        ~DynamicAABBTree() = default;

        // ============== Proxy API ==============

        int32 InsertProxy(const Math::Bounds2D& _aabb, uint32 _bodyIndex);
        void RemoveProxy(int32 _proxyId);
        bool MoveProxy(int32 _proxyId, const Math::Bounds2D& _newAabb, const Math::Vector2f& _displacement);

        const Math::Bounds2D& GetFatAABB(int32 _proxyId) const;
        uint32 GetBodyIndex(int32 _proxyId) const;

        // ============== Query API ==============

        template <typename Func>
        void Query(const Math::Bounds2D& _aabb, Func&& _callback) const;

        /// @brief Traverses the tree testing ray-AABB intersection and invokes callback for each leaf hit
        /// @param _origin Ray origin
        /// @param _invDirection Inverse of ray direction (1/dir.x, 1/dir.y)
        /// @param _maxDistance Maximum ray distance
        /// @param _callback Called with proxy ID for each leaf whose AABB the ray intersects
        template <typename Func>
        void RayCast(const Math::Vector2f& _origin, const Math::Vector2f& _invDirection, float _maxDistance,
            Func&& _callback) const;

        // ============== Read-only Accessors ==============

        int32 GetRootNodeId() const;
        const AABBTreeNode& GetNode(int32 _nodeId) const;
        uint32 GetProxyCount() const;
        uint32 GetHeight() const;

        // ============== Configuration ==============

        void SetFattenMargin(float _margin);
        void SetDisplacementMultiplier(float _multiplier);

    private:
        int32 AllocateNode();
        void FreeNode(int32 _nodeId);
        void InsertLeaf(int32 _leafId);
        void RemoveLeaf(int32 _leafId);
        void FixUpwards(int32 _nodeId);
        int32 Balance(int32 _nodeId);
        Math::Bounds2D FattenAABB(const Math::Bounds2D& _aabb, const Math::Vector2f& _displacement) const;

        static float SurfaceArea(const Math::Bounds2D& _aabb);
        static Math::Bounds2D Union(const Math::Bounds2D& _a, const Math::Bounds2D& _b);

    private:
        Vector<AABBTreeNode> mNodes;
        int32 mRoot      = NullNode;
        int32 mFreeList  = NullNode;
        uint32 mNodeCount    = 0;
        uint32 mNodeCapacity = 0;
        uint32 mProxyCount   = 0;
        float mFattenMargin  = 0.1f;
        float mDisplacementMultiplier = 2.0f;
    };

    // ============== Template Implementation ==============

    template <typename Func>
    void DynamicAABBTree::Query(const Math::Bounds2D& _aabb, Func&& _callback) const {
        if (mRoot == NullNode) {
            return;
        }

        Stack<int32> stack;
        stack.push(mRoot);

        while (!stack.empty()) {
            int32 nodeId = stack.top();
            stack.pop();

            if (nodeId == NullNode) {
                continue;
            }

            const AABBTreeNode& node = mNodes[nodeId];

            if (node.Aabb.Intersects(_aabb)) {
                if (node.IsLeaf()) {
                    _callback(nodeId);
                } else {
                    stack.push(node.Left);
                    stack.push(node.Right);
                }
            }
        }
    }

    template <typename Func>
    void DynamicAABBTree::RayCast(const Math::Vector2f& _origin, const Math::Vector2f& _invDirection,
        float _maxDistance, Func&& _callback) const {
        if (mRoot == NullNode) {
            return;
        }

        Stack<int32> stack;
        stack.push(mRoot);

        while (!stack.empty()) {
            int32 nodeId = stack.top();
            stack.pop();

            if (nodeId == NullNode) {
                continue;
            }

            const AABBTreeNode& node = mNodes[nodeId];

            if (node.Aabb.RayIntersects(_origin, _invDirection, _maxDistance)) {
                if (node.IsLeaf()) {
                    _callback(nodeId);
                } else {
                    stack.push(node.Left);
                    stack.push(node.Right);
                }
            }
        }
    }

} // namespace Umbra
