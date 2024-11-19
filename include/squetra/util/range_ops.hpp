#pragma once
#ifndef H_INCLUDED_SQUETRA_UTIL_RANGE_OPS
#define H_INCLUDED_SQUETRA_UTIL_RANGE_OPS

namespace squetra {

/**
 * Removes elements from first range depending on some criteria, and moves them
 * to the second range
 * @tparam ForwardIt The iterator type
 * @tparam OutputIt The output iterator type
 * @tparam Crit The criteria type
 * @param begin1 The input range
 * @param end1 The end of the input range
 * @param begin2 The output range
 * @param crit The criteria, whether to move to the second range
 * @returns Forward iterator to the end of the new input range, always <= end
 */
template <class ForwardIt, class OutputIt, class Crit>
ForwardIt extract_to(ForwardIt begin1, ForwardIt end1, OutputIt begin2,
                     Crit crit) {
    auto out1 = begin1;
    auto out2 = begin2;
    while (begin1 != end1) {
        if (crit(*begin1)) {
            *out2 = *begin1;
            ++out2;
        } else {
            *out1 = *begin1;
            ++out1;
        }

        ++begin1;
    }

    return out1;
}

} // namespace squetra

#endif // H_INCLUDED_SQUETRA_UTIL_RANGE_OPS