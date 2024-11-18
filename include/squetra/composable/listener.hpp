#pragma once
#ifndef H_INCLUDED_SQUETRA_COMPOSABLE_LISTENER
#define H_INCLUDED_SQUETRA_COMPOSABLE_LISTENER

#include "squetra/util/typedef.hpp"

#include <span>

namespace squetra {

class Listener {

  public:
    virtual ~Listener() = default;

    virtual void onItemsAdded(std::span<const IndexT> itemIndices) = 0;
    virtual void onItemsRemoved(std::span<const IndexT> itemIndices) = 0;
    virtual void onItemsComponentModified(std::span<const IndexT> itemIndices,
                                          IndexT componentIndex) = 0;
    virtual void
    onContainerComponentsModified(std::span<const IndexT> componentIndices) = 0;
};
} // namespace squetra

#endif // H_INCLUDED_SQUETRA_COMPOSABLE_LISTENER