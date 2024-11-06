#pragma once
#ifndef H_INCLUDED_SQUETRA_COMPOSABLE_FLUSHING_CONTAINER
#define H_INCLUDED_SQUETRA_COMPOSABLE_FLUSHING_CONTAINER

#include "squetra/composable/listener.hpp"

#include <span>
#include <vector>

namespace squetra {

class FlushingContainer {

  public:
    inline void flush();

    inline void addListener4ItemsAdded(Listener *listener);
    inline void addListener4ItemsRemoved(Listener *listener);
    inline void addListener4ItemsComponentModified(Listener *listener);
    inline void addListener4ComponentsModified(Listener *listener);

    inline void removeListener4ItemsAdded(Listener *listener);
    inline void removeListener4ItemsRemoved(Listener *listener);
    inline void removeListener4ItemsComponentModified(Listener *listener);
    inline void removeListener4ComponentsModified(Listener *listener);

  protected:
    inline void notifyItemAdded(IndexT itemIndex);
    inline void notifyItemRemoved(IndexT itemIndex);
    inline void notifyItemComponentModified(IndexT itemIndex,
                                            IndexT componentIndex);

    std::span<FlushingContainer *> m_upstreamNotifiers;
    std::vector<Listener *> m_itemsAddedListeners;
    std::vector<Listener *> m_itemsRemovedListeners;
    std::vector<Listener *> m_itemsComponentModifiedListeners;
    std::vector<Listener *> m_componentsModifiedListeners;
};

} // namespace squetra

#endif // H_INCLUDED_SQUETRA_COMPOSABLE_FLUSHING_CONTAINER