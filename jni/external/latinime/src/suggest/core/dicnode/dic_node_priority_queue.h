/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LATINIME_DIC_NODE_PRIORITY_QUEUE_H
#define LATINIME_DIC_NODE_PRIORITY_QUEUE_H

#include <queue>
#include <vector>

#include "defines.h"
#include "dic_node.h"
#include "dic_node_release_listener.h"

#define MAX_DIC_NODE_PRIORITY_QUEUE_CAPACITY 200

namespace latinime {

class DicNodePriorityQueue : public DicNodeReleaseListener {
 public:
    AK_FORCE_INLINE DicNodePriorityQueue()
            : MAX_CAPACITY(MAX_DIC_NODE_PRIORITY_QUEUE_CAPACITY),
              mMaxSize(MAX_DIC_NODE_PRIORITY_QUEUE_CAPACITY), mDicNodesBuf(), mUnusedNodeIndices(),
              mNextUnusedNodeId(0), mDicNodesQueue() {
        mDicNodesBuf.resize(MAX_CAPACITY + 1);
        mUnusedNodeIndices.resize(MAX_CAPACITY + 1);
        reset();
    }

    // Non virtual inline destructor -- never inherit this class
    AK_FORCE_INLINE ~DicNodePriorityQueue() {}

    int getSize() const {
        return static_cast<int>(mDicNodesQueue.size());
    }

    int getMaxSize() const {
        return mMaxSize;
    }

    AK_FORCE_INLINE void setMaxSize(const int maxSize) {
        mMaxSize = min(maxSize, MAX_CAPACITY);
    }

    AK_FORCE_INLINE void reset() {
        clearAndResize(MAX_CAPACITY);
    }

    AK_FORCE_INLINE void clear() {
        clearAndResize(mMaxSize);
    }

    AK_FORCE_INLINE void clearAndResize(const int maxSize) {
        while (!mDicNodesQueue.empty()) {
            mDicNodesQueue.pop();
        }
        setMaxSize(maxSize);
        for (int i = 0; i < MAX_CAPACITY + 1; ++i) {
            mDicNodesBuf[i].remove();
            mDicNodesBuf[i].setReleaseListener(this);
            mUnusedNodeIndices[i] = i == MAX_CAPACITY ? NOT_A_NODE_ID : static_cast<int>(i) + 1;
        }
        mNextUnusedNodeId = 0;
    }

    AK_FORCE_INLINE DicNode *newDicNode(DicNode *dicNode) {
        DicNode *newNode = searchEmptyDicNode();
        if (newNode) {
            DicNodeUtils::initByCopy(dicNode, newNode);
            return newNode;
        }
        return 0;
    }

    // Copy
    AK_FORCE_INLINE DicNode *copyPush(DicNode *dicNode) {
        return copyPush(dicNode, mMaxSize);
    }

    AK_FORCE_INLINE void copyPop(DicNode *dest) {
        if (mDicNodesQueue.empty()) {
            ASSERT(false);
            return;
        }
        DicNode *node = mDicNodesQueue.top();
        if (dest) {
            DicNodeUtils::initByCopy(node, dest);
        }
        node->remove();
        mDicNodesQueue.pop();
    }

    void onReleased(DicNode *dicNode) {
        const int index = static_cast<int>(dicNode - &mDicNodesBuf[0]);
        if (mUnusedNodeIndices[index] != NOT_A_NODE_ID) {
            // it's already released
            return;
        }
        mUnusedNodeIndices[index] = mNextUnusedNodeId;
        mNextUnusedNodeId = index;
        ASSERT(index >= 0 && index < (MAX_CAPACITY + 1));
    }

    AK_FORCE_INLINE void dump() const {
        AKLOGI("\n\n\n\n\n===========================");
        for (int i = 0; i < MAX_CAPACITY + 1; ++i) {
            if (mDicNodesBuf[i].isUsed()) {
                mDicNodesBuf[i].dump("QUEUE: ");
            }
        }
        AKLOGI("===========================\n\n\n\n\n");
    }

 private:
    DISALLOW_COPY_AND_ASSIGN(DicNodePriorityQueue);
    static const int NOT_A_NODE_ID = -1;

    AK_FORCE_INLINE static bool compareDicNode(DicNode *left, DicNode *right) {
        return left->compare(right);
    }

    struct DicNodeComparator {
        bool operator ()(DicNode *left, DicNode *right) {
            return compareDicNode(left, right);
        }
    };

    typedef std::priority_queue<DicNode *, std::vector<DicNode *>, DicNodeComparator> DicNodesQueue;
    const int MAX_CAPACITY;
    int mMaxSize;
    std::vector<DicNode> mDicNodesBuf; // of each element of mDicNodesBuf respectively
    std::vector<int> mUnusedNodeIndices;
    int mNextUnusedNodeId;
    DicNodesQueue mDicNodesQueue;

    inline bool isFull(const int maxSize) const {
        return getSize() >= maxSize;
    }

    AK_FORCE_INLINE void pop() {
        copyPop(0);
    }

    AK_FORCE_INLINE bool betterThanWorstDicNode(DicNode *dicNode) const {
        DicNode *worstNode = mDicNodesQueue.top();
        if (!worstNode) {
            return true;
        }
        return compareDicNode(dicNode, worstNode);
    }

    AK_FORCE_INLINE DicNode *searchEmptyDicNode() {
        // TODO: Currently O(n) but should be improved to O(1)
        if (MAX_CAPACITY == 0) {
            return 0;
        }
        if (mNextUnusedNodeId == NOT_A_NODE_ID) {
            AKLOGI("No unused node found.");
            for (int i = 0; i < MAX_CAPACITY + 1; ++i) {
                AKLOGI("Dump node availability, %d, %d, %d",
                        i, mDicNodesBuf[i].isUsed(), mUnusedNodeIndices[i]);
            }
            ASSERT(false);
            return 0;
        }
        DicNode *dicNode = &mDicNodesBuf[mNextUnusedNodeId];
        markNodeAsUsed(dicNode);
        return dicNode;
    }

    AK_FORCE_INLINE void markNodeAsUsed(DicNode *dicNode) {
        const int index = static_cast<int>(dicNode - &mDicNodesBuf[0]);
        mNextUnusedNodeId = mUnusedNodeIndices[index];
        mUnusedNodeIndices[index] = NOT_A_NODE_ID;
        ASSERT(index >= 0 && index < (MAX_CAPACITY + 1));
    }

    AK_FORCE_INLINE DicNode *pushPoolNodeWithMaxSize(DicNode *dicNode, const int maxSize) {
        if (!dicNode) {
            return 0;
        }
        if (!isFull(maxSize)) {
            mDicNodesQueue.push(dicNode);
            return dicNode;
        }
        if (betterThanWorstDicNode(dicNode)) {
            pop();
            mDicNodesQueue.push(dicNode);
            return dicNode;
        }
        dicNode->remove();
        return 0;
    }

    // Copy
    AK_FORCE_INLINE DicNode *copyPush(DicNode *dicNode, const int maxSize) {
        return pushPoolNodeWithMaxSize(newDicNode(dicNode), maxSize);
    }
};
} // namespace latinime
#endif // LATINIME_DIC_NODE_PRIORITY_QUEUE_H
