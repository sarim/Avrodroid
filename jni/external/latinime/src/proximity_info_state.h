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

#ifndef LATINIME_PROXIMITY_INFO_STATE_H
#define LATINIME_PROXIMITY_INFO_STATE_H

#include <cstring> // for memset()
#include <vector>

#include "char_utils.h"
#include "defines.h"
#include "hash_map_compat.h"
#include "proximity_info_params.h"
#include "proximity_info_state_utils.h"

namespace latinime {

class ProximityInfo;

class ProximityInfoState {
 public:
    /////////////////////////////////////////
    // Defined in proximity_info_state.cpp //
    /////////////////////////////////////////
    void initInputParams(const int pointerId, const float maxPointToKeyLength,
            const ProximityInfo *proximityInfo, const int *const inputCodes,
            const int inputSize, const int *xCoordinates, const int *yCoordinates,
            const int *const times, const int *const pointerIds, const bool isGeometric);

    /////////////////////////////////////////
    // Defined here                        //
    /////////////////////////////////////////
    AK_FORCE_INLINE ProximityInfoState()
            : mProximityInfo(0), mMaxPointToKeyLength(0.0f), mAverageSpeed(0.0f),
              mHasTouchPositionCorrectionData(false), mMostCommonKeyWidthSquare(0),
              mKeyCount(0), mCellHeight(0), mCellWidth(0), mGridHeight(0), mGridWidth(0),
              mIsContinuousSuggestionPossible(false), mSampledInputXs(), mSampledInputYs(),
              mSampledTimes(), mSampledInputIndice(), mSampledLengthCache(),
              mBeelineSpeedPercentiles(), mSampledNormalizedSquaredLengthCache(), mSpeedRates(),
              mDirections(), mCharProbabilities(), mSampledNearKeySets(), mSampledSearchKeySets(),
              mSampledSearchKeyVectors(), mTouchPositionCorrectionEnabled(false),
              mSampledInputSize(0), mMostProbableStringProbability(0.0f) {
        memset(mInputProximities, 0, sizeof(mInputProximities));
        memset(mNormalizedSquaredDistances, 0, sizeof(mNormalizedSquaredDistances));
        memset(mPrimaryInputWord, 0, sizeof(mPrimaryInputWord));
        memset(mMostProbableString, 0, sizeof(mMostProbableString));
    }

    // Non virtual inline destructor -- never inherit this class
    AK_FORCE_INLINE ~ProximityInfoState() {}

    inline int getPrimaryCodePointAt(const int index) const {
        return getProximityCodePointsAt(index)[0];
    }

    inline bool sameAsTyped(const int *word, int length) const {
        if (length != mSampledInputSize) {
            return false;
        }
        const int *inputProximities = mInputProximities;
        while (length--) {
            if (*inputProximities != *word) {
                return false;
            }
            inputProximities += MAX_PROXIMITY_CHARS_SIZE;
            word++;
        }
        return true;
    }

    AK_FORCE_INLINE bool existsCodePointInProximityAt(const int index, const int c) const {
        const int *codePoints = getProximityCodePointsAt(index);
        int i = 0;
        while (codePoints[i] > 0 && i < MAX_PROXIMITY_CHARS_SIZE) {
            if (codePoints[i++] == c) {
                return true;
            }
        }
        return false;
    }

    inline bool existsAdjacentProximityChars(const int index) const {
        if (index < 0 || index >= mSampledInputSize) return false;
        const int currentCodePoint = getPrimaryCodePointAt(index);
        const int leftIndex = index - 1;
        if (leftIndex >= 0 && existsCodePointInProximityAt(leftIndex, currentCodePoint)) {
            return true;
        }
        const int rightIndex = index + 1;
        if (rightIndex < mSampledInputSize
                && existsCodePointInProximityAt(rightIndex, currentCodePoint)) {
            return true;
        }
        return false;
    }

    inline int getNormalizedSquaredDistance(
            const int inputIndex, const int proximityIndex) const {
        return mNormalizedSquaredDistances[
                inputIndex * MAX_PROXIMITY_CHARS_SIZE + proximityIndex];
    }

    inline const int *getPrimaryInputWord() const {
        return mPrimaryInputWord;
    }

    inline bool touchPositionCorrectionEnabled() const {
        return mTouchPositionCorrectionEnabled;
    }

    bool isUsed() const {
        return mSampledInputSize > 0;
    }

    int size() const {
        return mSampledInputSize;
    }

    int getInputX(const int index) const {
        return mSampledInputXs[index];
    }

    int getInputY(const int index) const {
        return mSampledInputYs[index];
    }

    bool hasSpaceProximity(const int index) const;

    int getLengthCache(const int index) const {
        return mSampledLengthCache[index];
    }

    bool isContinuousSuggestionPossible() const {
        return mIsContinuousSuggestionPossible;
    }

    // TODO: Rename s/Length/NormalizedSquaredLength/
    float getPointToKeyByIdLength(const int inputIndex, const int keyId) const;
    // TODO: Rename s/Length/NormalizedSquaredLength/
    float getPointToKeyLength(const int inputIndex, const int codePoint) const;

    ProximityType getProximityType(const int index, const int codePoint,
            const bool checkProximityChars, int *proximityIndex = 0) const;

    ProximityType getProximityTypeG(const int index, const int codePoint) const;

    const std::vector<int> *getSearchKeyVector(const int index) const {
        return &mSampledSearchKeyVectors[index];
    }

    float getSpeedRate(const int index) const {
        return mSpeedRates[index];
    }

    AK_FORCE_INLINE int getBeelineSpeedPercentile(const int id) const {
        return mBeelineSpeedPercentiles[id];
    }

    AK_FORCE_INLINE DoubleLetterLevel getDoubleLetterLevel(const int id) const {
        const int beelineSpeedRate = getBeelineSpeedPercentile(id);
        if (beelineSpeedRate == 0) {
            return A_STRONG_DOUBLE_LETTER;
        } else if (beelineSpeedRate
                < ProximityInfoParams::MIN_DOUBLE_LETTER_BEELINE_SPEED_PERCENTILE) {
            return A_DOUBLE_LETTER;
        } else {
            return NOT_A_DOUBLE_LETTER;
        }
    }

    float getDirection(const int index) const {
        return mDirections[index];
    }
    // get xy direction
    float getDirection(const int x, const int y) const;

    float getMostProbableString(int *const codePointBuf) const;

    float getProbability(const int index, const int charCode) const;

    float getLineToKeyDistance(
            const int from, const int to, const int keyId, const bool extend) const;

    bool isKeyInSerchKeysAfterIndex(const int index, const int keyId) const;

 private:
    DISALLOW_COPY_AND_ASSIGN(ProximityInfoState);
    /////////////////////////////////////////
    // Defined in proximity_info_state.cpp //
    /////////////////////////////////////////
    float calculateNormalizedSquaredDistance(const int keyIndex, const int inputIndex) const;

    float calculateSquaredDistanceFromSweetSpotCenter(
            const int keyIndex, const int inputIndex) const;

    /////////////////////////////////////////
    // Defined here                        //
    /////////////////////////////////////////

    inline const int *getProximityCodePointsAt(const int index) const {
        return ProximityInfoStateUtils::getProximityCodePointsAt(mInputProximities, index);
    }

    // const
    const ProximityInfo *mProximityInfo;
    float mMaxPointToKeyLength;
    float mAverageSpeed;
    bool mHasTouchPositionCorrectionData;
    int mMostCommonKeyWidthSquare;
    int mKeyCount;
    int mCellHeight;
    int mCellWidth;
    int mGridHeight;
    int mGridWidth;
    bool mIsContinuousSuggestionPossible;

    std::vector<int> mSampledInputXs;
    std::vector<int> mSampledInputYs;
    std::vector<int> mSampledTimes;
    std::vector<int> mSampledInputIndice;
    std::vector<int> mSampledLengthCache;
    std::vector<int> mBeelineSpeedPercentiles;
    std::vector<float> mSampledNormalizedSquaredLengthCache;
    std::vector<float> mSpeedRates;
    std::vector<float> mDirections;
    // probabilities of skipping or mapping to a key for each point.
    std::vector<hash_map_compat<int, float> > mCharProbabilities;
    // The vector for the key code set which holds nearby keys for each sampled input point
    // 1. Used to calculate the probability of the key
    // 2. Used to calculate mSampledSearchKeySets
    std::vector<ProximityInfoStateUtils::NearKeycodesSet> mSampledNearKeySets;
    // The vector for the key code set which holds nearby keys of some trailing sampled input points
    // for each sampled input point. These nearby keys contain the next characters which can be in
    // the dictionary. Specifically, currently we are looking for keys nearby trailing sampled
    // inputs including the current input point.
    std::vector<ProximityInfoStateUtils::NearKeycodesSet> mSampledSearchKeySets;
    std::vector<std::vector<int> > mSampledSearchKeyVectors;
    bool mTouchPositionCorrectionEnabled;
    int mInputProximities[MAX_PROXIMITY_CHARS_SIZE * MAX_WORD_LENGTH];
    int mNormalizedSquaredDistances[MAX_PROXIMITY_CHARS_SIZE * MAX_WORD_LENGTH];
    int mSampledInputSize;
    int mPrimaryInputWord[MAX_WORD_LENGTH];
    float mMostProbableStringProbability;
    int mMostProbableString[MAX_WORD_LENGTH];
};
} // namespace latinime
#endif // LATINIME_PROXIMITY_INFO_STATE_H
