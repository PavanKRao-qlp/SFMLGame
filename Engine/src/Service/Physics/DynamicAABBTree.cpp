#include "Service/Physics/DynamicAABBTree.h"

#include "Math/MathUtils.h"

namespace Umbra {

    DynamicAABBTree::DynamicAABBTree() {
        mNodeCapacity = 64;
        mNodes.resize(mNodeCapacity);

        // Build free list
        for (uint32 i = 0; i < mNodeCapacity - 1; ++i) {
            mNodes[i].NextFree = static_cast<int32>(i + 1);
            mNodes[i].Height   = -1;
        }
        mNodes[mNodeCapacity - 1].NextFree = NullNode;
        mNodes[mNodeCapacity - 1].Height   = -1;
        mFreeList = 0;
    }

    // ============== Read-only Accessors ==============

    int32 DynamicAABBTree::GetRootNodeId() const {
        return mRoot;
    }

    const AABBTreeNode& DynamicAABBTree::GetNode(int32 _nodeId) const {
        return mNodes[_nodeId];
    }

    uint32 DynamicAABBTree::GetProxyCount() const {
        return mProxyCount;
    }

    uint32 DynamicAABBTree::GetHeight() const {
        if (mRoot == NullNode) {
            return 0;
        }
        return static_cast<uint32>(mNodes[mRoot].Height);
    }

    // ============== Configuration ==============

    void DynamicAABBTree::SetFattenMargin(float _margin) {
        mFattenMargin = _margin;
    }

    void DynamicAABBTree::SetDisplacementMultiplier(float _multiplier) {
        mDisplacementMultiplier = _multiplier;
    }

    // ============== Proxy API ==============

    int32 DynamicAABBTree::InsertProxy(const Math::Bounds2D& _aabb, uint32 _bodyIndex) {
        int32 proxyId = AllocateNode();

        // Fatten the AABB with zero displacement
        mNodes[proxyId].Aabb      = FattenAABB(_aabb, Math::Vector2f(0, 0));
        mNodes[proxyId].BodyIndex = _bodyIndex;
        mNodes[proxyId].Height    = 0;

        InsertLeaf(proxyId);
        ++mProxyCount;

        return proxyId;
    }

    void DynamicAABBTree::RemoveProxy(int32 _proxyId) {
        if (_proxyId < 0 || static_cast<uint32>(_proxyId) >= mNodeCapacity) {
            return;
        }

        RemoveLeaf(_proxyId);
        FreeNode(_proxyId);
        --mProxyCount;
    }

    bool DynamicAABBTree::MoveProxy(int32 _proxyId, const Math::Bounds2D& _newAabb, const Math::Vector2f& _displacement) {
        if (_proxyId < 0 || static_cast<uint32>(_proxyId) >= mNodeCapacity) {
            return false;
        }

        // If the new tight AABB is still inside the existing fat AABB, skip
        if (mNodes[_proxyId].Aabb.Contains(_newAabb)) {
            return false;
        }

        RemoveLeaf(_proxyId);

        // Re-fatten with displacement prediction
        mNodes[_proxyId].Aabb = FattenAABB(_newAabb, _displacement);

        InsertLeaf(_proxyId);

        return true;
    }

    const Math::Bounds2D& DynamicAABBTree::GetFatAABB(int32 _proxyId) const {
        return mNodes[_proxyId].Aabb;
    }

    uint32 DynamicAABBTree::GetBodyIndex(int32 _proxyId) const {
        return mNodes[_proxyId].BodyIndex;
    }

    // ============== Node Management ==============

    int32 DynamicAABBTree::AllocateNode() {
        // Expand capacity if free list is empty
        if (mFreeList == NullNode) {
            uint32 oldCapacity = mNodeCapacity;
            mNodeCapacity *= 2;
            mNodes.resize(mNodeCapacity);

            for (uint32 i = oldCapacity; i < mNodeCapacity - 1; ++i) {
                mNodes[i].NextFree = static_cast<int32>(i + 1);
                mNodes[i].Height   = -1;
            }
            mNodes[mNodeCapacity - 1].NextFree = NullNode;
            mNodes[mNodeCapacity - 1].Height   = -1;
            mFreeList = static_cast<int32>(oldCapacity);
        }

        int32 nodeId = mFreeList;
        mFreeList    = mNodes[nodeId].NextFree;

        mNodes[nodeId].Parent   = NullNode;
        mNodes[nodeId].Left     = NullNode;
        mNodes[nodeId].Right    = NullNode;
        mNodes[nodeId].Height   = 0;
        mNodes[nodeId].NextFree = NullNode;

        ++mNodeCount;
        return nodeId;
    }

    void DynamicAABBTree::FreeNode(int32 _nodeId) {
        mNodes[_nodeId].NextFree = mFreeList;
        mNodes[_nodeId].Height   = -1;
        mNodes[_nodeId].Parent   = NullNode;
        mNodes[_nodeId].Left     = NullNode;
        mNodes[_nodeId].Right    = NullNode;
        mFreeList = _nodeId;
        --mNodeCount;
    }

    // ============== Tree Operations ==============

    void DynamicAABBTree::InsertLeaf(int32 _leafId) {
        // If tree is empty, make this the root
        if (mRoot == NullNode) {
            mRoot = _leafId;
            mNodes[_leafId].Parent = NullNode;
            return;
        }

        // Find the best sibling using SAH (Surface Area Heuristic)
        Math::Bounds2D leafAABB = mNodes[_leafId].Aabb;
        int32 bestSibling       = mRoot;
        float bestCost          = SurfaceArea(Union(mNodes[mRoot].Aabb, leafAABB));

        // Cost inherited from ancestors when inserting deeper
        // Uses a priority-free traversal (Box2D-style single pass down the tree)
        struct NodeCost {
            int32 NodeId;
            float InheritedCost;
        };

        Stack<NodeCost> searchStack;
        searchStack.push({mRoot, 0.0f});

        while (!searchStack.empty()) {
            NodeCost current = searchStack.top();
            searchStack.pop();

            int32 currentId       = current.NodeId;
            float inheritedCost   = current.InheritedCost;
            const auto& node      = mNodes[currentId];

            // Cost of creating a new parent for this node and the new leaf
            Math::Bounds2D combined = Union(node.Aabb, leafAABB);
            float directCost        = SurfaceArea(combined);
            float totalCost         = directCost + inheritedCost;

            if (totalCost < bestCost) {
                bestCost    = totalCost;
                bestSibling = currentId;
            }

            // Inherited cost for children = increase in SA at this node
            float newInherited = inheritedCost + (SurfaceArea(combined) - SurfaceArea(node.Aabb));

            // Lower bound for any descendant: leaf SA + inherited cost so far
            float lowerBound = SurfaceArea(leafAABB) + newInherited;
            if (lowerBound < bestCost && !node.IsLeaf()) {
                searchStack.push({node.Left, newInherited});
                searchStack.push({node.Right, newInherited});
            }
        }

        // Create a new internal node as parent of bestSibling and the new leaf
        int32 oldParent = mNodes[bestSibling].Parent;
        int32 newParent = AllocateNode();

        mNodes[newParent].Parent = oldParent;
        mNodes[newParent].Aabb   = Union(leafAABB, mNodes[bestSibling].Aabb);
        mNodes[newParent].Height = mNodes[bestSibling].Height + 1;

        if (oldParent != NullNode) {
            // bestSibling is not the root
            if (mNodes[oldParent].Left == bestSibling) {
                mNodes[oldParent].Left = newParent;
            } else {
                mNodes[oldParent].Right = newParent;
            }
        } else {
            // bestSibling was the root
            mRoot = newParent;
        }

        mNodes[newParent].Left        = bestSibling;
        mNodes[newParent].Right       = _leafId;
        mNodes[bestSibling].Parent    = newParent;
        mNodes[_leafId].Parent        = newParent;

        // Walk back up fixing heights and AABBs, and rebalance
        FixUpwards(mNodes[_leafId].Parent);
    }

    void DynamicAABBTree::RemoveLeaf(int32 _leafId) {
        if (_leafId == mRoot) {
            mRoot = NullNode;
            return;
        }

        int32 parent      = mNodes[_leafId].Parent;
        int32 grandParent = mNodes[parent].Parent;
        int32 sibling     = (mNodes[parent].Left == _leafId) ? mNodes[parent].Right : mNodes[parent].Left;

        if (grandParent != NullNode) {
            // Replace parent with sibling in grandparent
            if (mNodes[grandParent].Left == parent) {
                mNodes[grandParent].Left = sibling;
            } else {
                mNodes[grandParent].Right = sibling;
            }
            mNodes[sibling].Parent = grandParent;

            // Free the old parent node
            FreeNode(parent);

            // Fix tree upwards from grandparent
            FixUpwards(grandParent);
        } else {
            // Parent was the root
            mRoot = sibling;
            mNodes[sibling].Parent = NullNode;
            FreeNode(parent);
        }

        mNodes[_leafId].Parent = NullNode;
    }

    void DynamicAABBTree::FixUpwards(int32 _nodeId) {
        int32 index = _nodeId;
        while (index != NullNode) {
            index = Balance(index);

            int32 left  = mNodes[index].Left;
            int32 right = mNodes[index].Right;

            if (left == NullNode || right == NullNode) {
                break;
            }

            mNodes[index].Height = 1 + std::max(mNodes[left].Height, mNodes[right].Height);
            mNodes[index].Aabb   = Union(mNodes[left].Aabb, mNodes[right].Aabb);

            index = mNodes[index].Parent;
        }
    }

    int32 DynamicAABBTree::Balance(int32 _nodeId) {
        if (mNodes[_nodeId].IsLeaf() || mNodes[_nodeId].Height < 2) {
            return _nodeId;
        }

        int32 leftId  = mNodes[_nodeId].Left;
        int32 rightId = mNodes[_nodeId].Right;

        int32 balance = mNodes[rightId].Height - mNodes[leftId].Height;

        // Rotate right child up
        if (balance > 1) {
            int32 rightLeft  = mNodes[rightId].Left;
            int32 rightRight = mNodes[rightId].Right;

            // Swap _nodeId and rightId
            mNodes[rightId].Left   = _nodeId;
            mNodes[rightId].Parent = mNodes[_nodeId].Parent;
            mNodes[_nodeId].Parent = rightId;

            // Fix old parent to point to rightId
            if (mNodes[rightId].Parent != NullNode) {
                if (mNodes[mNodes[rightId].Parent].Left == _nodeId) {
                    mNodes[mNodes[rightId].Parent].Left = rightId;
                } else {
                    mNodes[mNodes[rightId].Parent].Right = rightId;
                }
            } else {
                mRoot = rightId;
            }

            // Rotate: pick shorter grandchild to become child of demoted node
            if (mNodes[rightLeft].Height > mNodes[rightRight].Height) {
                mNodes[rightId].Right  = rightLeft;
                mNodes[_nodeId].Right  = rightRight;
                mNodes[rightRight].Parent = _nodeId;

                mNodes[_nodeId].Aabb   = Union(mNodes[leftId].Aabb, mNodes[rightRight].Aabb);
                mNodes[rightId].Aabb   = Union(mNodes[_nodeId].Aabb, mNodes[rightLeft].Aabb);

                mNodes[_nodeId].Height = 1 + std::max(mNodes[leftId].Height, mNodes[rightRight].Height);
                mNodes[rightId].Height = 1 + std::max(mNodes[_nodeId].Height, mNodes[rightLeft].Height);
            } else {
                mNodes[rightId].Right  = rightRight;
                mNodes[_nodeId].Right  = rightLeft;
                mNodes[rightLeft].Parent = _nodeId;

                mNodes[_nodeId].Aabb   = Union(mNodes[leftId].Aabb, mNodes[rightLeft].Aabb);
                mNodes[rightId].Aabb   = Union(mNodes[_nodeId].Aabb, mNodes[rightRight].Aabb);

                mNodes[_nodeId].Height = 1 + std::max(mNodes[leftId].Height, mNodes[rightLeft].Height);
                mNodes[rightId].Height = 1 + std::max(mNodes[_nodeId].Height, mNodes[rightRight].Height);
            }

            return rightId;
        }

        // Rotate left child up
        if (balance < -1) {
            int32 leftLeft  = mNodes[leftId].Left;
            int32 leftRight = mNodes[leftId].Right;

            // Swap _nodeId and leftId
            mNodes[leftId].Right  = _nodeId;
            mNodes[leftId].Parent = mNodes[_nodeId].Parent;
            mNodes[_nodeId].Parent = leftId;

            // Fix old parent
            if (mNodes[leftId].Parent != NullNode) {
                if (mNodes[mNodes[leftId].Parent].Left == _nodeId) {
                    mNodes[mNodes[leftId].Parent].Left = leftId;
                } else {
                    mNodes[mNodes[leftId].Parent].Right = leftId;
                }
            } else {
                mRoot = leftId;
            }

            // Rotate
            if (mNodes[leftLeft].Height > mNodes[leftRight].Height) {
                mNodes[leftId].Left    = leftLeft;
                mNodes[_nodeId].Left   = leftRight;
                mNodes[leftRight].Parent = _nodeId;

                mNodes[_nodeId].Aabb   = Union(mNodes[rightId].Aabb, mNodes[leftRight].Aabb);
                mNodes[leftId].Aabb    = Union(mNodes[_nodeId].Aabb, mNodes[leftLeft].Aabb);

                mNodes[_nodeId].Height = 1 + std::max(mNodes[rightId].Height, mNodes[leftRight].Height);
                mNodes[leftId].Height  = 1 + std::max(mNodes[_nodeId].Height, mNodes[leftLeft].Height);
            } else {
                mNodes[leftId].Left    = leftRight;
                mNodes[_nodeId].Left   = leftLeft;
                mNodes[leftLeft].Parent = _nodeId;

                mNodes[_nodeId].Aabb   = Union(mNodes[rightId].Aabb, mNodes[leftLeft].Aabb);
                mNodes[leftId].Aabb    = Union(mNodes[_nodeId].Aabb, mNodes[leftRight].Aabb);

                mNodes[_nodeId].Height = 1 + std::max(mNodes[rightId].Height, mNodes[leftLeft].Height);
                mNodes[leftId].Height  = 1 + std::max(mNodes[_nodeId].Height, mNodes[leftRight].Height);
            }

            return leftId;
        }

        return _nodeId;
    }

    // ============== AABB Helpers ==============

    Math::Bounds2D DynamicAABBTree::FattenAABB(const Math::Bounds2D& _aabb, const Math::Vector2f& _displacement) const {
        Math::Vector2f margin(mFattenMargin, mFattenMargin);
        Math::Vector2f newMin = _aabb.Min() - margin;
        Math::Vector2f newMax = _aabb.Max() + margin;

        // Extend in the direction of displacement
        Math::Vector2f d = _displacement * mDisplacementMultiplier;

        if (d.x < 0.0f) {
            newMin.x += d.x;
        } else {
            newMax.x += d.x;
        }

        if (d.y < 0.0f) {
            newMin.y += d.y;
        } else {
            newMax.y += d.y;
        }

        Math::Vector2f center = (newMin + newMax) * 0.5f;
        Math::Vector2f size   = newMax - newMin;
        return Math::Bounds2D(center, size);
    }

    float DynamicAABBTree::SurfaceArea(const Math::Bounds2D& _aabb) {
        // For 2D, "surface area" is the perimeter: 2 * (width + height)
        return 2.0f * (_aabb.Size.x + _aabb.Size.y);
    }

    Math::Bounds2D DynamicAABBTree::Union(const Math::Bounds2D& _a, const Math::Bounds2D& _b) {
        Math::Vector2f minPoint(
            Math::Min(_a.Min().x, _b.Min().x),
            Math::Min(_a.Min().y, _b.Min().y)
        );
        Math::Vector2f maxPoint(
            Math::Max(_a.Max().x, _b.Max().x),
            Math::Max(_a.Max().y, _b.Max().y)
        );
        Math::Vector2f center = (minPoint + maxPoint) * 0.5f;
        Math::Vector2f size   = maxPoint - minPoint;
        return Math::Bounds2D(center, size);
    }

} // namespace Umbra
