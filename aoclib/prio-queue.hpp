#pragma once

#include <map>
#include <unordered_map>
#include <optional>

namespace aocutil 
{

template<typename T, typename PrioType = int>
class PrioQueue 
{
private:
    std::multimap<PrioType, T> prio_to_elem; 
    std::unordered_map<T, typename decltype(prio_to_elem)::iterator> elem_to_prio; // Necessary so we don't have to do linear search when updating an element's priority.

public:
    void insert(const T& elem, const PrioType& priority)
    {
        if (elem_to_prio.contains(elem)) {
            throw std::invalid_argument("PrioQueue insert: Element already in queue"); 
        }
        auto it = prio_to_elem.insert({priority, elem});
        assert(it != prio_to_elem.end() && !elem_to_prio.contains(it->second));
        elem_to_prio.insert({elem, it}); 
    } 

    void update_prio(const T& elem, const PrioType& new_priority)
    {
        if (!elem_to_prio.contains(elem)) {
            throw std::out_of_range("PrioQueue update_prio: Element not in queue.");
        }
        auto it = elem_to_prio.at(elem); 
        assert(it != prio_to_elem.end());
        elem_to_prio.erase(elem);
        prio_to_elem.erase(it);
        insert(elem, new_priority); // Re-insert elem at the new priority.
    }

    void insert_or_update(const T& elem, const PrioType& prio)
    {
        if (contains(elem)) {
            update_prio(elem, prio); 
        } else {
            insert(elem, prio);
        }
    }

    T extract_min()
    {
        auto min_elem = prio_to_elem.begin(); 
        if (min_elem == prio_to_elem.end()) { // Queue is empty.
            assert(empty());
            throw std::out_of_range("PrioQueue extract_min: Queue already empty"); 
        }
        T elem = min_elem->second; 
        prio_to_elem.erase(min_elem);
        elem_to_prio.erase(elem); 
        return elem;
    }

    T extract_min(PrioType& prio)
    {
        auto min_elem = prio_to_elem.begin(); 
        if (min_elem == prio_to_elem.end()) { // Queue is empty.
            assert(empty());
            throw std::out_of_range("PrioQueue extract_min: Queue already empty"); 
        }
        T elem = min_elem->second; 
        prio_to_elem.erase(min_elem);
        elem_to_prio.erase(elem); 
        prio = min_elem->first;
        return elem;
    }

    bool contains(const T& elem) const {
        return elem_to_prio.contains(elem); 
    }

    std::size_t size() const {
        return elem_to_prio.size();
    }

    bool empty() const 
    {
        const bool empt = prio_to_elem.size() == 0; 
        if (empt) {
            assert(elem_to_prio.size() == 0);
        }
        return empt;
    }
};

// Is slower...
// template<typename T, typename HashFun = void>
// class BinaryHeap 
// {
//     enum class Child {Left, Right};
//     std::vector<T> nodes; 
//     using MapType = typename std::conditional_t<std::is_same_v<HashFun, void>, phmap::flat_hash_map<T, std::size_t>, phmap::flat_hash_map<T, std::size_t, HashFun>>;
//     MapType elem_to_idx; 

//     std::size_t get_parent_idx(std::size_t idx) const {
//         return (idx - 1) / 2;
//     }

//     std::optional<std::size_t> get_children_idx(std::size_t idx, Child ct) const 
//     {
//         std::size_t child_idx = 0;
//         if (ct == Child::Left) {
//             child_idx = 2 * idx + 1;
//         } else if (ct == Child::Right) {
//             child_idx = 2 * idx + 2; 
//         } else {
//             assert(false);
//         }
        
//         if (child_idx < nodes.size()) {
//             return child_idx; 
//         } else {
//             return {};
//         }
//     }

//     bool is_ordered(const T& parent, const T& child) const 
//     {
//         if (type == BinaryHeapType::Min) {
//             return parent <= child;
//         } else if (type == BinaryHeapType::Max) {
//             return parent >= child;
//         } else {
//             throw std::invalid_argument("Binary heap is_ordered: Invalid binary heap type");
//         }
//     }


//     void swap_nodes(std::size_t idx_a, std::size_t idx_b) {
//         assert(elem_to_idx.contains(nodes.at(idx_a)));
//         assert(elem_to_idx.contains(nodes.at(idx_b)));

//         elem_to_idx.at(nodes.at(idx_a)) = idx_b; 
//         elem_to_idx.at(nodes.at(idx_b)) = idx_a; 
//         std::swap(nodes.at(idx_a), nodes.at(idx_b));
//     }

//     void heapify_up(std::size_t idx) 
//     {
//         if (nodes.size() <= 1) {
//             return;
//         }

//         while (idx > 0 && !is_ordered(nodes.at(get_parent_idx(idx)), nodes.at(idx))) {
//             swap_nodes(get_parent_idx(idx), idx);
//             idx = get_parent_idx(idx);
//         }
//     }

//     void heapify_down(std::size_t idx) 
//     {
//         if (nodes.size() <= 1) {
//             return;
//         }
//         do {
//             auto idx_left_child = get_children_idx(idx, Child::Left);
//             auto idx_right_child = get_children_idx(idx, Child::Right);
//             if (!idx_left_child) {
//                 assert(!idx_right_child);
//                 return;
//             }
//             if (!idx_right_child) {
//                 idx_right_child = idx_left_child;
//             }

//             assert(idx_left_child && idx_right_child);
//             const T& left = nodes.at(idx_left_child.value()); 
//             const T& right = nodes.at(idx_right_child.value()); 
//             const T& min_max_child = (type == BinaryHeapType::Min) ? std::min(left, right) : std::max(left, right); 
//             std::size_t min_max_child_idx = min_max_child == left ? idx_left_child.value() : idx_right_child.value();

//             if (!is_ordered(nodes.at(idx), nodes.at(min_max_child_idx))) {
//                 swap_nodes(idx, min_max_child_idx);
//                 idx = min_max_child_idx;
//             } else {
//                 break;
//             }
//         } while (idx < nodes.size() - 1);
//     }

//     bool is_heap() const {
//         return (type == BinaryHeapType::Min) ? std::is_heap(nodes.cbegin(), nodes.cend(), std::greater<T>{}) : std::is_heap(nodes.begin(), nodes.end());
//     }

// public: 
//     enum class BinaryHeapType {Max, Min};
//     const BinaryHeapType type =  BinaryHeapType::Min;
    
//     BinaryHeap(BinaryHeapType type = BinaryHeapType::Min) :type(type) {};

//     void insert(const T& elem) 
//     {
//         if (elem_to_idx.contains(elem)) {
//             throw "BinaryHeap insert: elem already inside";
//         }

//         nodes.push_back(elem);
//         std::size_t idx = nodes.size() - 1; 
//         elem_to_idx.insert({elem, idx});

//         heapify_up(idx);
//         assert(is_heap());
//     }

//     T extract_min() 
//     {
//         if (size() == 0) {
//             throw std::out_of_range("BinaryHeap extract_min: Heap is empty");
//         }
//         T extracted_elem = nodes.at(0);
//         std::size_t last_idx = nodes.size() - 1;
//         if (nodes.size() > 1) {
//             swap_nodes(0, last_idx);
//         }
//         nodes.pop_back();

   
//         heapify_down(0);

//         elem_to_idx.erase(extracted_elem);

//         assert(is_heap());
//         return extracted_elem;
//     }

//     void delete_elem(const T& elem) 
//     {

//         // cf. https://en.wikipedia.org/wiki/Binary_heap#Delete (last retrieved 2024-06-20)
//         if (!elem_to_idx.contains(elem)) {
//             throw std::out_of_range("BinaryHeap delete_elem: elem not in heap");
//         }
//         std::size_t deleted_idx = elem_to_idx.at(elem);
//         std::size_t last_idx = nodes.size() - 1;
        
//         if (nodes.size() == 1) {
//             elem_to_idx.erase(elem);
//             nodes.pop_back();
//             assert(is_heap());
//             return;
//         } 

//         swap_nodes(deleted_idx, last_idx);
//         bool require_heapify_up = (type == BinaryHeapType::Min) ? nodes.at(deleted_idx) < nodes.at(last_idx) : nodes.at(deleted_idx) > nodes.at(last_idx) ;

//         elem_to_idx.erase(nodes.at(last_idx));
//         nodes.pop_back();
//         if (nodes.size() == 0) {
//             assert(is_heap());
//             return;
//         }

//         if (require_heapify_up) {
//             heapify_up(deleted_idx);
//         } else {
//             heapify_down(deleted_idx);
//         }
//         assert(is_heap());
//     }

//     typename std::vector<T>::const_iterator cbegin() const {
//         return nodes.cbegin();
//     }

//     typename std::vector<T>::const_iterator cend() const {
//         return nodes.cend();
//     }

//     std::size_t size() const {
//         return nodes.size();
//     }

//     bool contains(const T& elem) const {
//         return elem_to_idx.contains(elem);
//     }
// };

// template<typename T, typename PrioType = int>
// class PrioQueueHeap 
// {
// private:
//     struct KeyVal {
//         PrioType key; 
//         T val; 
//         auto operator<=>(const KeyVal& other) const {return key - other.key;};
//         auto operator==(const KeyVal& other) const {return val == other.val;};
//         // KeyVal() = default;
//         KeyVal(PrioType key, const T& val) :key(key), val(val) {};        
//         KeyVal(const T& val) : val(val) {};


//         struct KeyValHash {
//             std::size_t operator()(const KeyVal& kv) const noexcept {
//                     return std::hash<T>{}(kv.val); 
//             }
//         };
//     };

//     BinaryHeap<KeyVal, typename KeyVal::KeyValHash> heap;

// public:
//     void insert(const T& elem, const PrioType& priority)
//     {
//         KeyVal kv(priority, elem);
//         if (heap.contains(kv)) {
//             throw std::invalid_argument("PrioQueueHeap insert: Element already in queue"); 
//         }
//         heap.insert(kv);
//     } 

//     void update_prio(const T& elem, const PrioType& new_priority)
//     {
//         KeyVal kv(new_priority, elem);

//         if (!heap.contains(kv)) {
//             throw std::out_of_range("PrioQueueHeap update_prio: Element not in queue.");
//         }
//         heap.delete_elem(kv);
//         heap.insert(kv);
//     }

//     void insert_or_update(const T& elem, const PrioType& prio)
//     {
//         if (heap.contains(KeyVal(prio, elem))) {
//             update_prio(elem, prio); 
//         } else {
//             insert(elem, prio);
//         }
//     }

//     T extract_min()
//     {
//         return heap.extract_min().val;
//     }

//     bool contains(const T& elem) const {
//         return heap.contains(KeyVal(elem)); // The key stays undefined but that's okay.
//     }

//     std::size_t size() const {
//         return heap.size();
//     }

//     bool empty() const 
//     {
//         return heap.size() == 0;
//     }
// };

}