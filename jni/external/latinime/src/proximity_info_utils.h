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

#ifndef LATINIME_PROXIMITY_INFO_UTILS_H
#define LATINIME_PROXIMITY_INFO_UTILS_H

#include <cmath>

#include "additional_proximity_chars.h"
#include "char_utils.h"
#include "defines.h"
#include "geometry_utils.h"
#include "hash_map_compat.h"

namespace latinime {
class ProximityInfoUtils {
 public:
    static AK_FORCE_INLINE int getKeyIndexOf(const int keyCount, const int c,
            const hash_map_compat<int, int> *const codeToKeyMap) {
        if (keyCount == 0) {
            // We do not have the coordinate data
            return NOT_AN_INDEX;
        }
        if (c == NOT_A_CODE_POINT) {
            return NOT_AN_INDEX;
        }
        const int lowerCode = toLowerCase(c);
        hash_map_compat<int, int>::const_iterator mapPos = codeToKeyMap->find(lowerCode);
        if (mapPos != codeToKeyMap->end()) {
            return mapPos->second;
        }
        return NOT_AN_INDEX;
    }

    static AK_FORCE_INLINE void initializeProximities(const int *const inputCodes,
            const int *const inputXCoordinates, const int *const inputYCoordinates,
            const int inputSize, const int *const keyXCoordinates,
            const int *const keyYCoordinates, const int *const keyWidths, const int *keyHeights,
            const int *const proximityCharsArray, const int cellHeight, const int cellWidth,
            const int gridWidth, const int mostCommonKeyWidth, const int keyCount,
            const char *const localeStr,
            const hash_map_compat<int, int> *const codeToKeyMap, int *inputProximities) {
        // Initialize
        // - mInputCodes
        // - mNormalizedSquaredDistances
        // TODO: Merge
        for (int i = 0; i < inputSize; ++i) {
            const int primaryKey = inputCodes[i];
            const int x = inputXCoordinates[i];
            const int y = inputYCoordinates[i];
            int *proximities = &inputProximities[i * MAX_PROXIMITY_CHARS_SIZE];
            calculateProximities(keyXCoordinates, keyYCoordinates, keyWidths, keyHeights,
                    proximityCharsArray, cellHeight, cellWidth, gridWidth, mostCommonKeyWidth,
                    keyCount, x, y, primaryKey, localeStr, codeToKeyMap, proximities);
        }

        if (DEBUG_PROXIMITY_CHARS) {
            for (int i = 0; i < inputSize; ++i) {
                AKLOGI("---");
                for (int j = 0; j < MAX_PROXIMITY_CHARS_SIZE; ++j) {
                    int proximityChar =
                            inputProximities[i * MAX_PROXIMITY_CHARS_SIZE + j];
                    proximityChar += 0;
                    AKLOGI("--- (%d)%c", i, proximityChar);
                }
            }
        }
    }

    static AK_FORCE_INLINE int getStartIndexFromCoordinates(const int x, const int y,
            const int cellHeight, const int cellWidth, const int gridWidth) {
        return ((y / cellHeight) * gridWidth + (x / cellWidth)) * MAX_PROXIMITY_CHARS_SIZE;
    }

    static inline float getSquaredDistanceFloat(const float x1, const float y1, const float x2,
            const float y2) {
        return SQUARE_FLOAT(x1 - x2) + SQUARE_FLOAT(y1 - y2);
    }

    static inline float pointToLineSegSquaredDistanceFloat(const float x, const float y,
        const float x1, const float y1, const float x2, const float y2, const bool extend) {
        const float ray1x = x - x1;
        const float ray1y = y - y1;
        const float ray2x = x2 - x1;
        const float ray2y = y2 - y1;

        const float dotProduct = ray1x * ray2x + ray1y * ray2y;
        const float lineLengthSqr = SQUARE_FLOAT(ray2x) + SQUARE_FLOAT(ray2y);
        const float projectionLengthSqr = dotProduct / lineLengthSqr;

        float projectionX;
        float projectionY;
        if (!extend && projectionLengthSqr < 0.0f) {
            projectionX = x1;
            projectionY = y1;
        } else if (!extend && projectionLengthSqr > 1.0f) {
            projectionX = x2;
            projectionY = y2;
        } else {
            projectionX = x1 + projectionLengthSqr * ray2x;
            projectionY = y1 + projectionLengthSqr * ray2y;
        }
        return getSquaredDistanceFloat(x, y, projectionX, projectionY);
    }

    // Normal distribution N(u, sigma^2).
    struct NormalDistribution {
     public:
        NormalDistribution(const float u, const float sigma)
                : mU(u), mSigma(sigma),
                  mPreComputedNonExpPart(1.0f / sqrtf(2.0f * M_PI_F * SQUARE_FLOAT(sigma))),
                  mPreComputedExponentPart(-1.0f / (2.0f * SQUARE_FLOAT(sigma))) {}

        float getProbabilityDensity(const float x) const {
            const float shiftedX = x - mU;
            return mPreComputedNonExpPart * expf(mPreComputedExponentPart * SQUARE_FLOAT(shiftedX));
        }

     private:
        DISALLOW_IMPLICIT_CONSTRUCTORS(NormalDistribution);
        const float mU; // mean value
        const float mSigma; // standard deviation
        const float mPreComputedNonExpPart; // = 1 / sqrt(2 * PI * sigma^2)
        const float mPreComputedExponentPart; // = -1 / (2 * sigma^2)
    }; // struct NormalDistribution

 private:
    DISALLOW_IMPLICIT_CONSTRUCTORS(ProximityInfoUtils);

    static bool isOnKey(const int *const keyXCoordinates, const int *const keyYCoordinates,
            const int *const keyWidths, const int *keyHeights, const int keyId, const int x,
            const int y) {
        if (keyId < 0) return true; // NOT_A_ID is -1, but return whenever < 0 just in case
        const int left = keyXCoordinates[keyId];
        const int top = keyYCoordinates[keyId];
        const int right = left + keyWidths[keyId] + 1;
        const int bottom = top + keyHeights[keyId];
        return left < right && top < bottom && x >= left && x < right && y >= top && y < bottom;
    }

    static AK_FORCE_INLINE void calculateProximities(const int *const keyXCoordinates,
            const int *const keyYCoordinates, const int *const keyWidths, const int *keyHeights,
            const int *const proximityCharsArray, const int cellHeight, const int cellWidth,
            const int gridWidth, const int mostCommonKeyWidth, const int keyCount,
            const int x, const int y, const int primaryKey, const char *const localeStr,
            const hash_map_compat<int, int> *const codeToKeyMap, int *proximities) {
        const int mostCommonKeyWidthSquare = mostCommonKeyWidth * mostCommonKeyWidth;
        int insertPos = 0;
        proximities[insertPos++] = primaryKey;
        const int startIndex = getStartIndexFromCoordinates(x, y, cellHeight, cellWidth, gridWidth);
        if (startIndex >= 0) {
            for (int i = 0; i < MAX_PROXIMITY_CHARS_SIZE; ++i) {
                const int c = proximityCharsArray[startIndex + i];
                if (c < KEYCODE_SPACE || c == primaryKey) {
                    continue;
                }
                const int keyIndex = getKeyIndexOf(keyCount, c, codeToKeyMap);
                const bool onKey = isOnKey(keyXCoordinates, keyYCoordinates, keyWidths, keyHeights,
                        keyIndex, x, y);
                const int distance = squaredLengthToEdge(keyXCoordinates, keyYCoordinates,
                        keyWidths, keyHeights, keyIndex, x, y);
                if (onKey || distance < mostCommonKeyWidthSquare) {
                    proximities[insertPos++] = c;
                    if (insertPos >= MAX_PROXIMITY_CHARS_SIZE) {
                        if (DEBUG_DICT) {
                            ASSERT(false);
                        }
                        return;
                    }
                }
            }
            const int additionalProximitySize =
                    AdditionalProximityChars::getAdditionalCharsSize(localeStr, primaryKey);
            if (additionalProximitySize > 0) {
                proximities[insertPos++] = ADDITIONAL_PROXIMITY_CHAR_DELIMITER_CODE;
                if (insertPos >= MAX_PROXIMITY_CHARS_SIZE) {
                    if (DEBUG_DICT) {
                        ASSERT(false);
                    }
                    return;
                }

                const int *additionalProximityChars =
                        AdditionalProximityChars::getAdditionalChars(localeStr, primaryKey);
                for (int j = 0; j < additionalProximitySize; ++j) {
                    const int ac = additionalProximityChars[j];
                    int k = 0;
                    for (; k < insertPos; ++k) {
                        if (ac == proximities[k]) {
                            break;
                        }
                    }
                    if (k < insertPos) {
                        continue;
                    }
                    proximities[insertPos++] = ac;
                    if (insertPos >= MAX_PROXIMITY_CHARS_SIZE) {
                        if (DEBUG_DICT) {
                            ASSERT(false);
                        }
                        return;
                    }
                }
            }
        }
        // Add a delimiter for the proximity characters
        for (int i = insertPos; i < MAX_PROXIMITY_CHARS_SIZE; ++i) {
            proximities[i] = NOT_A_CODE_POINT;
        }
    }

    static int squaredLengthToEdge(const int *const keyXCoordinates,
            const int *const keyYCoordinates, const int *const keyWidths, const int *keyHeights,
            const int keyId, const int x, const int y) {
        // NOT_A_ID is -1, but return whenever < 0 just in case
        if (keyId < 0) return MAX_VALUE_FOR_WEIGHTING;
        const int left = keyXCoordinates[keyId];
        const int top = keyYCoordinates[keyId];
        const int right = left + keyWidths[keyId];
        const int bottom = top + keyHeights[keyId];
        const int edgeX = x < left ? left : (x > right ? right : x);
        const int edgeY = y < top ? top : (y > bottom ? bottom : y);
        const int dx = x - edgeX;
        const int dy = y - edgeY;
        return dx * dx + dy * dy;
    }
};
} // namespace latinime
#endif // LATINIME_PROXIMITY_INFO_UTILS_H
