#pragma once
#ifndef H_INCLUDED_SQUETRA_COMPOSABLE_FLUSHING_CONTAINER
#define H_INCLUDED_SQUETRA_COMPOSABLE_FLUSHING_CONTAINER

#include "squetra/composable/listener.hpp"
#include "squetra/util/sorted_ops.hpp"

#include <algorithm>
#include <span>
#include <variant>
#include <vector>

namespace squetra {

/**
 * A container that has a notification mechanism for changes to its items
 * @note Adding an item implies its modification. Modification notifications
 * follow the adding notifications
 * @warning Adding components might not notify listeners for  *item* component
 * changes, only for *container* component changes
 * @warning Listeners added after changes *might* not get notified of them. It's
 * up to the listener to handle existing items and their changes beforehand.
 */
class FlushingContainer {

  public:
    /**
     * Create a new flushing container
     * @param upstreamContainers A span of dependency containers. They get
     * flushed before this one.
     */
    inline FlushingContainer(std::span<FlushingContainer *> upstreamContainers);

    /// Destructor
    inline ~FlushingContainer();

    FlushingContainer() = delete;
    FlushingContainer(const FlushingContainer &) = delete;
    FlushingContainer(FlushingContainer &&) = delete;

    FlushingContainer &operator=(const FlushingContainer &) = delete;
    FlushingContainer &operator=(FlushingContainer &&) = delete;

    inline void addListener4ItemsAdded(Listener *listener);
    inline void addListener4ItemsRemoved(Listener *listener);
    inline void addListener4ItemsComponentModified(Listener *listener);
    inline void addListener4ContainerComponentsModified(Listener *listener);

    inline void removeListener4ItemsAdded(Listener *listener);
    inline void removeListener4ItemsRemoved(Listener *listener);
    inline void removeListener4ItemsComponentModified(Listener *listener);
    inline void removeListener4ContainerComponentsModified(Listener *listener);
    inline void removeListener4PointersInvalidated(Listener *listener);

    // Info

    /**
     * @returns the number of used indices
     */
    inline CountT size() const;

    /**
     * Whether a free index can be popped
     */
    inline bool hasFreeIndices() const;

    /**
     * Returns a maximum index for which tracking data is allocated
     */
    inline IndexT getMaxUsedIndex() const;

    /**
     * @returns whether the index is free
     */
    inline bool isIndexFree(IndexT index) const;

    /**
     * Returns the number of components
     */
    inline std::size_t getComponentCount() const;

    // Updating

    /**
     * Notifies all listeners of unflushed changes.
     * Flushes the upstream containers.
     */
    inline void flush();

  protected:
    inline void notifyItemAdded(IndexT itemIndex);
    inline void notifyItemRemoved(IndexT itemIndex);
    inline void notifyItemComponentModified(IndexT itemIndex,
                                            IndexT componentIndex);
    inline void notifyComponentsAdded(std::size_t additionalComponentCount);

    /**
     * Returns an unused index that was previously freed
     */
    inline IndexT popFreeIndex();

  private:
    /**
     * Mark the whole container as dirty
     */
    inline void notifyBecameDirty();

    /**
     * Mark the whole component as dirty
     */
    inline void notifyContainerComponentModified(IndexT componentIndex);

    // Notification hierarchy

    std::span<FlushingContainer *> m_upstreamContainers;
    std::vector<FlushingContainer *> m_downstreamContainers;
    std::vector<Listener *> m_itemsAddedListeners;
    std::vector<Listener *> m_itemsRemovedListeners;
    std::vector<Listener *> m_itemsComponentModifiedListeners;
    std::vector<Listener *> m_containerComponentsModifiedListeners;

    // State tracking

    CountT m_maxIndex;

    bool m_isDirty;
    std::vector<bool> m_isContainerComponentDirty;
    std::vector<std::vector<bool>> m_areComponentItemsDirty;

    std::vector<IndexT> m_itemAddedIndices;
    std::vector<IndexT> m_itemRemovedIndices;
    std::vector<std::vector<IndexT>> m_itemComponentModifiedIndices;
    std::vector<IndexT> m_containerComponentModifiedIndices;

    /**
     * List of indices that are free to use after removal and flushing of the
     * container
     */
    std::vector<IndexT> m_freeItemIndices;
};

// === Constructor/Destructor ===

inline FlushingContainer::FlushingContainer(
    std::span<FlushingContainer *> upstreamContainers)
    : m_upstreamContainers(upstreamContainers), m_isDirty(true) {
    for (FlushingContainer *p_upstreamContainer : m_upstreamContainers) {
        p_upstreamContainer->m_downstreamContainers.push_back(this);
    }
}

inline FlushingContainer::~FlushingContainer() {
    for (FlushingContainer *p_upstreamContainer : m_upstreamContainers) {
        auto &upsDown = p_upstreamContainer->m_downstreamContainers;
        upsDown.erase(std::find(upsDown.begin(), upsDown.end(), this));
    }
}

// === Listener adding/removing ===

inline void FlushingContainer::addListener4ItemsAdded(Listener *listener) {
    m_itemsAddedListeners.push_back(listener);
}

inline void FlushingContainer::addListener4ItemsRemoved(Listener *listener) {
    m_itemsRemovedListeners.push_back(listener);
}

inline void
FlushingContainer::addListener4ItemsComponentModified(Listener *listener) {
    m_itemsComponentModifiedListeners.push_back(listener);
}

inline void
FlushingContainer::addListener4ContainerComponentsModified(Listener *listener) {
    m_containerComponentsModifiedListeners.push_back(listener);
}

inline void FlushingContainer::removeListener4ItemsAdded(Listener *listener) {
    m_itemsAddedListeners.erase(std::find(
        m_itemsAddedListeners.begin(), m_itemsAddedListeners.end(), listener));
}

inline void FlushingContainer::removeListener4ItemsRemoved(Listener *listener) {
    m_itemsRemovedListeners.erase(std::find(m_itemsRemovedListeners.begin(),
                                            m_itemsRemovedListeners.end(),
                                            listener));
}

inline void
FlushingContainer::removeListener4ItemsComponentModified(Listener *listener) {
    m_itemsComponentModifiedListeners.erase(
        std::find(m_itemsComponentModifiedListeners.begin(),
                  m_itemsComponentModifiedListeners.end(), listener));
}

inline void FlushingContainer::removeListener4ContainerComponentsModified(
    Listener *listener) {
    m_containerComponentsModifiedListeners.erase(
        std::find(m_containerComponentsModifiedListeners.begin(),
                  m_containerComponentsModifiedListeners.end(), listener));
}

// === Flush ===

inline void FlushingContainer::flush() {
    // early return
    if (!m_isDirty)
        return;

    m_isDirty = false;

    // === Flush ancestors ===

    for (FlushingContainer *p_upstreamContainer : m_upstreamContainers) {
        p_upstreamContainer->flush();
    }

    // === Free indices ===

    m_freeItemIndices.insert(m_freeItemIndices.end(),
                             m_itemRemovedIndices.begin(),
                             m_itemRemovedIndices.end());

    // === Notify listeners ===
    // for each listener we first sort the indices
    // (for both thze ability to quickly remove unneeded indices and for cache)
    // and then notify the listeners

    // removed items
    std::sort(m_itemRemovedIndices.begin(), m_itemRemovedIndices.end());
    for (Listener *p_listener : m_itemsRemovedListeners) {
        p_listener->onItemsRemoved(m_itemRemovedIndices);
    }

    // added items
    std::sort(m_itemAddedIndices.begin(), m_itemAddedIndices.end());
    remove_from_sorted<IndexT>(m_itemAddedIndices, m_itemRemovedIndices);

    for (Listener *p_listener : m_itemsAddedListeners) {
        p_listener->onItemsAdded(m_itemAddedIndices);
    }

    // modified items
    for (IndexT componentIndex = 0;
         componentIndex < m_containerComponentModifiedIndices.size();
         componentIndex++) {

        auto &itemModifiedIndices =
            m_itemComponentModifiedIndices[componentIndex];

        std::sort(itemModifiedIndices.begin(), itemModifiedIndices.end());
        remove_from_sorted<IndexT>(itemModifiedIndices, m_itemRemovedIndices);

        for (Listener *p_listener : m_itemsComponentModifiedListeners) {
            p_listener->onItemsComponentModified(itemModifiedIndices,
                                                 componentIndex);
        }
    }

    // modified container components
    for (Listener *p_listener : m_containerComponentsModifiedListeners) {
        p_listener->onContainerComponentsModified(
            m_containerComponentModifiedIndices);
    }

    // === Reset ===

    m_itemAddedIndices.clear();
    m_itemRemovedIndices.clear();
    m_itemComponentModifiedIndices.clear();
    m_containerComponentModifiedIndices.clear();
}

// === Notification ===

inline void FlushingContainer::notifyItemAdded(IndexT itemIndex) {

    // adding new entry
    if (m_maxIndex >= itemIndex) {
        m_freeItemIndices.reserve(m_freeItemIndices.size() +
                                  (itemIndex - m_maxIndex));
        for (IndexT i = m_maxIndex; i < itemIndex; i++) {
            m_freeItemIndices.push_back(i);
        }

        for (auto &isComponentItemDirty : m_areComponentItemsDirty) {
            isComponentItemDirty.resize(itemIndex + 1, false);
        }
    }

    // change state
    m_itemAddedIndices.push_back(itemIndex);

    // notify all components
    for (IndexT componentIndex = 0; componentIndex < getComponentCount();
         componentIndex++) {
        notifyItemComponentModified(itemIndex, componentIndex);
    }

    notifyBecameDirty();
}

inline void FlushingContainer::notifyItemRemoved(IndexT itemIndex) {
    --m_usedIndexCount;

    // change state
    m_itemRemovedIndices.push_back(itemIndex);

    // notify all components
    for (IndexT componentIndex = 0; componentIndex < getComponentCount();
         componentIndex++) {
        notifyContainerComponentModified(componentIndex);
    }

    notifyBecameDirty();
}

inline void
FlushingContainer::notifyItemComponentModified(IndexT itemIndex,
                                               IndexT componentIndex) {
    // if needed
    if (!m_areComponentItemsDirty[componentIndex][itemIndex]) {

        m_areComponentItemsDirty[componentIndex][itemIndex] = true;

        // change state
        m_itemComponentModifiedIndices[componentIndex].push_back(itemIndex);

        // notify component
        notifyContainerComponentModified(componentIndex);
        notifyBecameDirty();
    }
}

inline void
FlushingContainer::notifyComponentsAdded(std::size_t additionalComponentCount) {
    std::size_t oldCount = getComponentCount();
    std::size_t newComponentCount = oldCount + additionalComponentCount;

    m_isContainerComponentDirty.resize(newComponentCount, true);
    m_areComponentItemsDirty.reserve(newComponentCount);
    for (IndexT componentIndex = oldCount; componentIndex < newComponentCount;
         componentIndex++) {
        m_containerComponentModifiedIndices.push_back(componentIndex);
        m_areComponentItemsDirty.emplace_back(m_maxIndex, false);
    }
}

inline void FlushingContainer::notifyBecameDirty() {
    if (!m_isDirty) {
        m_isDirty = true;
        for (FlushingContainer *p_downstreamContainer :
             m_downstreamContainers) {
            p_downstreamContainer->notifyBecameDirty();
        }
    }
}

inline void
FlushingContainer::notifyContainerComponentModified(IndexT componentIndex) {
    if (!m_isContainerComponentDirty[componentIndex]) {
        m_isContainerComponentDirty[componentIndex] = true;
        m_containerComponentModifiedIndices.push_back(componentIndex);
    }
}

// === Info ===

inline CountT FlushingContainer::size() const { return m_usedIndexCount; }

inline bool FlushingContainer::hasFreeIndices() const {
    return !m_freeItemIndices.empty();
}

inline IndexT FlushingContainer::getMaxUsedIndex() const { return m_maxIndex; }

inline IndexT FlushingContainer::popFreeIndex() {
    IndexT ret = m_freeItemIndices.back();
    m_freeItemIndices.pop_back();
    return ret;
}

inline IndexT FlushingContainer::getMaxUsedIndex() const { return m_maxIndex; }

inline std::size_t FlushingContainer::getComponentCount() const {
    return m_isContainerComponentDirty.size();
}

} // namespace squetra

#endif // H_INCLUDED_SQUETRA_COMPOSABLE_FLUSHING_CONTAINER