/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef LATINIME_TYPING_TRAVERSAL_H
#define LATINIME_TYPING_TRAVERSAL_H

#include <stdint.h>

#include "char_utils.h"
#include "defines.h"
#include "proximity_info_state.h"
#include "suggest/core/dicnode/dic_node.h"
#include "suggest/core/dicnode/dic_node_vector.h"
#include "suggest/core/policy/traversal.h"
#include "suggest/core/session/dic_traverse_session.h"
#include "suggest/policyimpl/typing/scoring_params.h"

namespace latinime {
class TypingTraversal : public Traversal {
 public:
    static const TypingTraversal *getInstance() { return &sInstance; }

    AK_FORCE_INLINE int getMaxPointerCount() const {
        return MAX_POINTER_COUNT;
    }

    AK_FORCE_INLINE bool allowsErrorCorrections(const DicNode *const dicNode) const {
        return dicNode->getNormalizedSpatialDistance()
                < ScoringParams::NORMALIZED_SPATIAL_DISTANCE_THRESHOLD_FOR_EDIT;
    }

    AK_FORCE_INLINE bool isOmission(const DicTraverseSession *const traverseSession,
            const DicNode *const dicNode, const DicNode *const childDicNode,
            const bool allowsErrorCorrections) const {
        if (!CORRECT_OMISSION) {
            return false;
        }
        // Note: Always consider intentional omissions (like apostrophes) since they are common.
        const bool canConsiderOmission =
                allowsErrorCorrections || childDicNode->canBeIntentionalOmission();
        if (!canConsiderOmission) {
            return false;
        }
        const int inputSize = traverseSession->getInputSize();
        // TODO: Don't refer to isCompletion?
        if (dicNode->isCompletion(inputSize)) {
            return false;
        }
        if (dicNode->canBeIntentionalOmission()) {
            return true;
        }
        const int point0Index = dicNode->getInputIndex(0);
        const int currentBaseLowerCodePoint =
                toBaseLowerCase(childDicNode->getNodeCodePoint());
        const int typedBaseLowerCodePoint =
                toBaseLowerCase(traverseSession->getProximityInfoState(0)
                        ->getPrimaryCodePointAt(point0Index));
        return (currentBaseLowerCodePoint != typedBaseLowerCodePoint);
    }

    AK_FORCE_INLINE bool isSpaceSubstitutionTerminal(
            const DicTraverseSession *const traverseSession, const DicNode *const dicNode) const {
        if (!CORRECT_NEW_WORD_SPACE_SUBSTITUTION) {
            return false;
        }
        if (!canDoLookAheadCorrection(traverseSession, dicNode)) {
            return false;
        }
        const int point0Index = dicNode->getInputIndex(0);
        return dicNode->isTerminalWordNode()
                && traverseSession->getProximityInfoState(0)->
                        hasSpaceProximity(point0Index);
    }

    AK_FORCE_INLINE bool isSpaceOmissionTerminal(
            const DicTraverseSession *const traverseSession, const DicNode *const dicNode) const {
        if (!CORRECT_NEW_WORD_SPACE_OMISSION) {
            return false;
        }
        const int inputSize = traverseSession->getInputSize();
        // TODO: Don't refer to isCompletion?
        if (dicNode->isCompletion(inputSize)) {
            return false;
        }
        if (!dicNode->isTerminalWordNode()) {
            return false;
        }
        const int16_t pointIndex = dicNode->getInputIndex(0);
        return pointIndex <= inputSize && !dicNode->isTotalInputSizeExceedingLimit()
                && !dicNode->shouldBeFilterdBySafetyNetForBigram();
    }

    AK_FORCE_INLINE bool shouldDepthLevelCache(
            const DicTraverseSession *const traverseSession) const {
        const int inputSize = traverseSession->getInputSize();
        return traverseSession->isCacheBorderForTyping(inputSize);
    }

    AK_FORCE_INLINE bool shouldNodeLevelCache(
            const DicTraverseSession *const traverseSession, const DicNode *const dicNode) const {
        return false;
    }

    AK_FORCE_INLINE bool canDoLookAheadCorrection(
            const DicTraverseSession *const traverseSession, const DicNode *const dicNode) const {
        const int inputSize = traverseSession->getInputSize();
        return dicNode->canDoLookAheadCorrection(inputSize);
    }

    AK_FORCE_INLINE ProximityType getProximityType(
            const DicTraverseSession *const traverseSession, const DicNode *const dicNode,
            const DicNode *const childDicNode) const {
        return traverseSession->getProximityInfoState(0)->getProximityType(
                dicNode->getInputIndex(0), childDicNode->getNodeCodePoint(),
                true /* checkProximityChars */);
    }

    AK_FORCE_INLINE bool needsToTraverseAllUserInput() const {
        return true;
    }

    AK_FORCE_INLINE float getMaxSpatialDistance() const {
        return ScoringParams::MAX_SPATIAL_DISTANCE;
    }

    AK_FORCE_INLINE bool allowPartialCommit() const {
        return true;
    }

    AK_FORCE_INLINE int getDefaultExpandDicNodeSize() const {
        return DicNodeVector::DEFAULT_NODES_SIZE_FOR_OPTIMIZATION;
    }

    AK_FORCE_INLINE bool sameAsTyped(
            const DicTraverseSession *const traverseSession, const DicNode *const dicNode) const {
        return traverseSession->getProximityInfoState(0)->sameAsTyped(
                dicNode->getOutputWordBuf(), dicNode->getDepth());
    }

    AK_FORCE_INLINE int getMaxCacheSize() const {
        return ScoringParams::MAX_CACHE_DIC_NODE_SIZE;
    }

    AK_FORCE_INLINE bool isPossibleOmissionChildNode(
            const DicTraverseSession *const traverseSession, const DicNode *const parentDicNode,
            const DicNode *const dicNode) const {
        const ProximityType proximityType =
                getProximityType(traverseSession, parentDicNode, dicNode);
        if (!DicNodeUtils::isProximityChar(proximityType)) {
            return false;
        }
        return true;
    }

    AK_FORCE_INLINE bool isGoodToTraverseNextWord(const DicNode *const dicNode) const {
        const int probability = dicNode->getProbability();
        if (probability < ScoringParams::THRESHOLD_NEXT_WORD_PROBABILITY) {
            return false;
        }
        const int c = dicNode->getOutputWordBuf()[0];
        const bool shortCappedWord = dicNode->getDepth()
                < ScoringParams::THRESHOLD_SHORT_WORD_LENGTH && isAsciiUpper(c);
        return !shortCappedWord
                || probability >= ScoringParams::THRESHOLD_NEXT_WORD_PROBABILITY_FOR_CAPPED;
    }

 private:
    DISALLOW_COPY_AND_ASSIGN(TypingTraversal);
    static const bool CORRECT_OMISSION;
    static const bool CORRECT_NEW_WORD_SPACE_SUBSTITUTION;
    static const bool CORRECT_NEW_WORD_SPACE_OMISSION;
    static const TypingTraversal sInstance;

    TypingTraversal() {}
    ~TypingTraversal() {}
};
} // namespace latinime
#endif // LATINIME_TYPING_TRAVERSAL_H
