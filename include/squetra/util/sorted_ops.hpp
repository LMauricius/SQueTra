#pragma once
#ifndef H_INCLUDED_SQUETRA_UTIL_SORTED_OPS
#define H_INCLUDED_SQUETRA_UTIL_SORTED_OPS

#include <span>
#include <vector>

namespace squetra {

/**
 * Remove elements from a vector
 * @param sortedVec The sorted ascending vector to remove from
 * @param sortedItemsToRemove The sorted ascending items to remove
 */
template <class T>
void remove_from_sorted(std::vector<T> &sortedVec,
                        std::span<const T> sortedItemsToRemove) {
    auto beginTestRange = sortedVec.begin();
    decltype(beginTestRange) beginPlaceRange;
    auto endRange = sortedVec.end();

    auto toRemoveIt = sortedItemsToRemove.begin();

    // simpler loop until we have to remove something
    for (; toRemoveIt != sortedItemsToRemove.end(); toRemoveIt++) {
        const auto &toRemove = *toRemoveIt;

        // get the first element >= toRemove
        auto lb = std::lower_bound(beginTestRange, endRange, toRemove);

        // early exit
        if (lb == endRange)
            return;

        // if LB doesn't point to the item to remove, start the next test range
        // with it
        // If LB points to the item to remove, skip it for the next test
        // range and setup the placing position before continuing to the item
        // moving loop
        if (toRemove < *lb) {
            beginTestRange = lb;
        } else {
            beginPlaceRange = lb;
            beginTestRange = lb + 1;
            break;
        }
    }

    // early exit if no items can be removed
    // Important! because the placing position hasn't been setup yet
    if (toRemoveIt == sortedItemsToRemove.end())
        return;

    // item moving loop
    for (; toRemoveIt != sortedItemsToRemove.end(); toRemoveIt++) {
        const auto &toRemove = *toRemoveIt;

        // get the first element >= toRemove
        auto lb = std::lower_bound(beginTestRange, endRange, toRemove);

        // early exit
        if (lb == endRange)
            break;

        // move items while increasing the beginTestRange to the lower bound
        while (beginTestRange < lb) {
            (*beginPlaceRange) = (*beginTestRange);
            beginPlaceRange++;
            beginTestRange++;
        }

        // if LB points to the item to remove, skip it for the next test
        // range
        if (!(toRemove < *lb)) {
            beginTestRange++;
        }
    }

    // move final items
    while (beginTestRange < endRange) {
        (*beginPlaceRange) = (*beginTestRange);
        beginPlaceRange++;
        beginTestRange++;
    }

    // truncate the copied items
    sortedVec.resize(beginPlaceRange - sortedVec.begin());
}

} // namespace squetra

#endif // H_INCLUDED_SQUETRA_UTIL_SORTED_OPS