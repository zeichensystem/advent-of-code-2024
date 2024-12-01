#pragma once

#include <array>
#include <unordered_map>

namespace aocutil 
{
/*
    General idea: https://stackoverflow.com/questions/2504178/lru-cache-design/54272232#54272232 (last retrieved 2024-06-16)
    Instead of using std::list, a custom intrusive doubly linked list is used, which means
    all nodes of the linked list will be stored in the same array instead of potentially all over the heap. 
*/ 

template<typename Key, typename Val, std::size_t N>
class LRUCache 
{
private:
    using ValNodeIdx = uint32_t; 
    static const ValNodeIdx IDX_NULL = std::numeric_limits<ValNodeIdx>::max(); 
    static_assert(N > 0 && N < IDX_NULL);
    
    struct ValNode {
        Key key;
        Val data;
        ValNodeIdx prev_idx, next_idx; 
    };

    std::unordered_map<Key, ValNodeIdx> map;

    // The array is used as an object pool holding the nodes of a doubly-linked intrusive linked list.  
    // cf. http://gameprogrammingpatterns.com/object-pool.html (last retrieved 2024-06-16)
    std::array<ValNode, N> nodes; 
    ValNodeIdx size_;
    ValNodeIdx first_free_idx;
    ValNodeIdx head_idx, tail_idx;

    ValNodeIdx get_free_idx()
    {
        if (first_free_idx == IDX_NULL) {
            return IDX_NULL;
        }
        assert(first_free_idx < N); 
        ValNodeIdx free_idx = first_free_idx;

        ValNode& vn = nodes.at(first_free_idx);
        first_free_idx = vn.next_idx; 

        return free_idx;
    }

    void reset_free_list() {
        size_ = first_free_idx = 0; 
        head_idx = tail_idx = IDX_NULL; 
        for (ValNodeIdx i = 0; i < nodes.size(); ++i) { // Set up the free list. 
            nodes.at(i).prev_idx = (i == 0) ? IDX_NULL : i - 1; 
            nodes.at(i).next_idx = (i == nodes.size() - 1) ? IDX_NULL : i + 1; 
        }
    }

public:
    LRUCache() : size_{0}, first_free_idx{0}, head_idx{IDX_NULL}, tail_idx{IDX_NULL}
    {
        reset_free_list();
    }

    void clear() 
    {
        reset_free_list();
        map.clear();
    }

    ValNodeIdx size() const 
    {
        return size_;
    }

    void insert(const Key& key, const Val& val)
    {
        // 1.) The key is already inside the cache: 
        if (map.contains(key)) { 
            ValNodeIdx vn_idx = map.at(key); 
            assert(vn_idx != IDX_NULL);
            ValNode& vn = nodes.at(vn_idx);
            if (vn.data != val) {
                vn.data = val; 
            }

            if (vn_idx == head_idx) { // Node was already head.
                assert(vn.prev_idx == IDX_NULL);
                return; 
            }

            if (vn_idx == tail_idx) { // Node was tail.
                assert(vn.next_idx == IDX_NULL);
                if (head_idx == tail_idx) {
                    assert(size_ == 1); 
                } else {
                    tail_idx = vn.prev_idx;
                }
            }

            // Pull out of pool. 
            assert(vn.prev_idx != IDX_NULL);
            if (vn.prev_idx != IDX_NULL) { 
                nodes.at(vn.prev_idx).next_idx = vn.next_idx; 
            } 
            if (vn.next_idx != IDX_NULL) {
                nodes.at(vn.next_idx).prev_idx = vn.prev_idx; 
            } 

            // Re-insert at the head of the list.
            vn.prev_idx = IDX_NULL; 
            vn.next_idx = head_idx; 
            if (head_idx != IDX_NULL) {
                nodes.at(head_idx).prev_idx = vn_idx;
            }
            head_idx = vn_idx; 
            return; 
        }

        // 2.) The key is not yet inside the cache:
        ValNodeIdx idx = get_free_idx(); 
        if (idx == IDX_NULL) { // a) Cache is full, remove the least recently used element to make space.
            assert(size_ == N);
            assert(tail_idx != IDX_NULL);
            ValNodeIdx to_delete_idx = tail_idx; 
            assert(to_delete_idx != IDX_NULL && to_delete_idx < N);
            ValNode& to_del = nodes.at(to_delete_idx); 
            map.erase(to_del.key);

            if (to_del.prev_idx != IDX_NULL) { // Update Tail. 
                assert(to_del.prev_idx < N);
                ValNode& new_tail_node = nodes.at(to_del.prev_idx); 
                new_tail_node.next_idx = IDX_NULL; 
                tail_idx = to_del.prev_idx;
                assert(to_del.prev_idx == to_del.prev_idx);
            }
            // Update free list. 
            to_del.prev_idx = IDX_NULL; 
            to_del.next_idx = first_free_idx; 
            first_free_idx = to_delete_idx;  

            idx = get_free_idx(); 
            assert(idx != IDX_NULL);
        } else { // b) Cache was not full.
            ++size_; 
            assert(size_ <= N);
        }

        ValNode& vn = nodes.at(idx); 
        vn.key = key;
        vn.data = val;

        ValNodeIdx old_head_idx = head_idx; 
        head_idx = idx; 
        if (tail_idx == IDX_NULL) {
            assert(size_ == 1); 
            tail_idx = head_idx; 
        }

        vn.prev_idx = IDX_NULL; 
        vn.next_idx = old_head_idx; 
        if (old_head_idx != IDX_NULL) {
            nodes.at(old_head_idx).prev_idx = idx;
        }
        map.insert({key, idx});
    }

    bool contains(const Key& key) const 
    {
        return map.contains(key);
    }

    std::optional<Val> get_copy(const Key& key) 
    {
        if (!map.contains(key)) {
            return {};
        }
        ValNodeIdx vn_idx = map.at(key); 
        assert(vn_idx != IDX_NULL);
        ValNode& vn = nodes.at(vn_idx);
        insert(key, vn.data); // Re-insert into head. 
        return vn.data;
    }

    /* 
        Dangerous:   
            Only dereference the pointer obtained by get_ptr as long as you haven't
            inserted any new keys into the lru-cache yet after having obtained the pointer.
            (As soon you have inserted new keys into the lru-cache after having obtained a pointer with get_ptr, 
            that pointer might point to an incorrect value, i.e. the ponter points only to the right value if you have 
            not yet called .insert after having obtained the pointer.)
            -> In most cases, just use get_copy. 
    */
    Val *get_ptr(const Key& key) 
    {
        if (!map.contains(key)) {
            return NULL; 
        }
        ValNodeIdx vn_idx = map.at(key); 
        assert(vn_idx != IDX_NULL);
        ValNode& vn = nodes.at(vn_idx);
        insert(key, vn.data); // Re-insert into head. 
        return &vn.data;
    }

    friend std::ostream& operator<<(std::ostream& os, const LRUCache<Key, Val, N>& cache)
    {
        os << "size: " << cache.size <<"\n"; 
        size_t idx = cache.head_idx; 
        [[maybe_unused]] size_t prev_idx = LRUCache::IDX_NULL; 
        while (idx != LRUCache<Key, Val, N>::IDX_NULL ) {
            const auto& v = cache.nodes.at(idx); 

            os << "key: " << v.key << ", val: " << v.data;
            if (idx == cache.head_idx) {
                os << " (HEAD)"; 
            }
            if (idx == cache.tail_idx) {
                os << " (TAIL)";
            }
            os << "\n";
            assert(v.prev_idx == prev_idx);

            prev_idx = idx;
            idx = v.next_idx;
        }
        return os;
    }
};

}