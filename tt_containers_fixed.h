#pragma once

#include <algorithm>

constexpr size_t CACHE_LINE = 64;

template<typename KEY, typename VALUE>
struct Dict {
    std::hash<KEY> hasher = std::hash<KEY>();
    KEY EMPTY;
    static constexpr double LF = 0.5;

    Dict(const KEY& empty) : EMPTY(empty) {
        size_ = 0;
        capacity = 8;
        data = new Pair[capacity];
        for (size_t i = 0; i < capacity; ++i)
            data[i].key = EMPTY;
    }

    ~Dict() {
        delete[] data;
    }

    void insert(KEY key, VALUE value) {
        // if key in map, overwrite and return
        // VALUE* old = find(key);
        // if (old) {
        //     *old = value;
        //     return;
        // }

        if (size_ >= capacity * LF) {
            grow();
        }

        uint64_t hash = hasher(key);
        uint64_t index = hash & (capacity - 1);
        uint64_t dist = 0;
        size_++;
        for (;;) {
            // overwrite
            if (data[index].key == key) {
                data[index].value = value;
                return false;
            }
            // insert
            if (data[index].key == EMPTY) {
                data[index].key = key;
                data[index].value = value;
                return;
            }
            uint64_t desired = index_for(data[index].key);
            uint64_t cur_dist = (index + capacity - desired) & (capacity - 1);
            if (cur_dist < dist) {
                std::swap(key, data[index].key);
                std::swap(value, data[index].value);
                dist = cur_dist;
            }
            dist++;
            index = (index + 1) & (capacity - 1);
        }
    }

    /*
    VALUE* find(const KEY& key) const {
        uint64_t hash = hasher(key);
        uint64_t index = hash & (capacity - 1);
        for (;;) {
            if (data[index].key == key)
                return &data[index].value;
            index = (index + 1) & (capacity - 1);
        }
        return nullptr;
    }
    */

    VALUE* find(const KEY& key) const {
        uint64_t hash = hasher(key);
        uint64_t index = hash & (capacity - 1);
        uint64_t dist = 0;
        for (;;) {
            if (data[index].key == EMPTY)
                return nullptr;
            if (data[index].key == key)
                return &data[index].value;
            uint64_t desired = index_for(data[index].key);
            uint64_t cur_dist = (index + capacity - desired) & (capacity - 1);
            if (cur_dist < dist)
                return nullptr;
            dist++;
            index = (index + 1) & (capacity - 1);
        }
    }

    bool contains(const KEY& key) {
        uint64_t hash = hasher(key);
        uint64_t index = hash & (capacity - 1);
        uint64_t dist = 0;
        for (;;) {
            if (data[index].key == EMPTY)
                return false;
            if (data[index].key == key)
                return true;
            uint64_t desired = index_for(data[index].key);
            uint64_t cur_dist = (index + capacity - desired) & (capacity - 1);
            if (cur_dist < dist)
                return false;
            dist++;
            index = (index + 1) & (capacity - 1);
        }
    }

    void erase(const KEY& key) {
        uint64_t hash = hasher(key);
        uint64_t index = hash & (capacity - 1);
        for (;;) {
            if (data[index].key == key) {
                size_--;
                remove(index);
                return;
            }
            index = (index + 1) & (capacity - 1);
        }
    }

    void clear() {
        size_ = 0;
        std::memset(data, 0xff, sizeof(Pair) * capacity);
    }

    uint64_t size() { return size_; }

    struct Pair {
        KEY key;
        VALUE value;
    };

    // This is not how we can iterate, data is sparse
    /*const Pair* begin() const { return data; }
    const Pair* end() const { return data + size_; }
    Pair* begin() { return data; }
    Pair* end() { return data + size_; }*/

    // private:
    uint64_t index_for(KEY key) const {
        uint64_t hash = hasher(key);
        uint64_t index = hash & (capacity - 1);
        return index;
    }
    /*
    uint64_t prefetch(KEY key) {
        uint64_t index = index_for(key);
        ::prefetch(&data[index]);
        return index;
    }
    uint64_t find_indexed(KEY key, uint64_t index) {
        for(;;) {
            if(data[index].key == key)
                return data[index].value;
            index = (index + 1) & (capacity - 1);
        }
    }
    */

    void remove(uint64_t index) {
        for (;;) {
            data[index].key = EMPTY;
            uint64_t next = (index + 1) & (capacity - 1);
            if (data[next].key == EMPTY)
                return;
            uint64_t desired = index_for(data[next].key);
            if (next == desired)
                return;
            data[index].key = data[next].key;
            data[index].value = data[next].value;
            index = next;
        }
    }

    void grow() {
        uint64_t old_capacity = capacity;
        Pair* old_data = data;
        size_ = 0;
        capacity *= 2;
        // data = reinterpret_cast<Pair*>(_aligned_malloc(CACHE_LINE, sizeof(Pair) * capacity));
        data = new Pair[capacity];
        for (size_t i = 0; i < capacity; ++i)
            data[i].key = EMPTY;
        for (uint64_t i = 0; i < old_capacity; i++) {
            if (old_data[i].key != EMPTY) // TODO: This check was <, not !=, but empty was MAX_UINT64 when key was size_t so that should amount to the same
                insert(old_data[i].key, old_data[i].value);
        }
        // _aligned_free(old_data);
        delete[] old_data;
    }


    Pair* data;
    uint64_t capacity;
    uint64_t size_;
    // uint64_t max_probe;
};
