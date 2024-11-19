#pragma once
#ifndef H_INCLUDED_SQUETRA_UTIL_SORTED_OPS
#define H_INCLUDED_SQUETRA_UTIL_SORTED_OPS

#include <span>
#include <vector>

namespace squetra {

/**
 * Remove elements from a vector
 * @tparam T The type of the elements
 * @tparam Compare The comparison functor
 * @param sortedVec The sorted ascending vector to remove from
 * @param sortedItemsToRemove The sorted ascending items to remove
 * @param comp The comparison function
 */
template <class T, class Compare>
void remove_from_sorted(std::vector<T> &sortedVec,
                        std::span<const T> sortedItemsToRemove, Compare comp) {
    auto beginTestRange = sortedVec.begin();
    decltype(beginTestRange) beginPlaceRange;
    auto endRange = sortedVec.end();

    auto toRemoveIt = sortedItemsToRemove.begin();

    // simpler loop until we have to remove something
    for (; toRemoveIt != sortedItemsToRemove.end(); toRemoveIt++) {
        const auto &toRemove = *toRemoveIt;

        // get the first element >= toRemove
        auto lb = std::lower_bound(beginTestRange, endRange, toRemove, comp);

        // early exit
        if (lb == endRange)
            return;

        // if LB doesn't point to the item to remove, start the next test range
        // with it
        // If LB points to the item to remove, skip it for the next test
        // range and setup the placing position before continuing to the item
        // moving loop
        if (comp(toRemove, *lb)) {
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
        while (comp(beginTestRange, lb)) {
            (*beginPlaceRange) = (*beginTestRange);
            beginPlaceRange++;
            beginTestRange++;
        }

        // if LB points to the item to remove, skip it for the next test
        // range
        if (!comp(toRemove, *lb)) {
            beginTestRange++;
        }
    }

    // move final items
    while (comp(beginTestRange, endRange)) {
        (*beginPlaceRange) = (*beginTestRange);
        beginPlaceRange++;
        beginTestRange++;
    }

    // truncate the copied items
    sortedVec.resize(beginPlaceRange - sortedVec.begin());
}

template <class T>
void remove_from_sorted(std::vector<T> &sortedVec,
                        std::span<const T> sortedItemsToRemove) {
    remove_from_sorted(sortedVec, sortedItemsToRemove, std::less<T>());
}

/**
 * Adds items from one sorted container into another one, preserving the order
 * of both containers
 * @tparam T The type of the items
 * @tparam Compare The comparison functor type
 * @param into The sorted container to add to
 * @param from The sorted container to add
 * @param comp The comparison function
 */
template <class T, class Compare>
void merge_into(std::vector<T> &into, std::span<const T> from, Compare comp) {
    // we allocate elements on the end, and iterate from the last existing
    // element to the begin
    std::size_t origSize = into.size();
    into.resize(origSize + from.size());
    auto outPos = into.rbegin();
    auto pos1 = into.rend() - origSize;
    auto pos2 = from.rbegin();
    auto end1 = into.rend();
    auto end2 = from.rend();

    for (;;) {
        while (comp(*pos2, *pos1)) {
            *outPos = *pos1;
            ++outPos;
            ++pos1;
            if (pos1 == end1) {
                // we only have the second container to use
                std::copy(pos2, end2, outPos);
                return;
            }
        }

        while (!comp(*pos2, *pos1)) {
            *outPos = *pos2;
            ++outPos;
            ++pos2;
            if (pos2 == end2) {
                // the first container's elements are already in place
                return;
            }
        }
    }
}

template <class T>
void merge_into(std::vector<T> &into, std::span<const T> from) {
    merge_into(into, from, std::less<T>());
}

} // namespace squetra

#endif // H_INCLUDED_SQUETRA_UTIL_SORTED_OPS