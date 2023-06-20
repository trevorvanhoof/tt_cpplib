#pragma once

#include <algorithm>

// From https://github.com/TheNumbat/hashtables/blob/main/code/chaining.h but using std::hash.
// This is WAY faster than std::unordered_map even with max optimization and no iterator debugging.
template<typename T>
struct Dict {
public:
    typedef size_t Key;
    typedef size_t Value;
    struct Pair {
        Key key;
        Value value;
    };

private:
    static const std::hash<size_t> hasher;
    static constexpr size_t EMPTY = UINT64_MAX;
    static constexpr double LF = 0.9; // % of memory usage required before resize is allowed, 0.5 is a lot faster
    static constexpr int CACHE_LINE = 64;

    static size_t stub;

    void remove(size_t index) {
        for (;;) {
            data[index].key = EMPTY;
            size_t next = (index + 1) & (capacity - 1);
            if (data[next].key == EMPTY) return;
            size_t desired = index_for(data[next].key);
            if (next == desired) return;
            data[index].key = data[next].key;
            data[index].value = data[next].value;
            index = next;
        }
    }

    void grow() {
        size_t old_capacity = capacity;
        Pair* old_data = data;
        size_ = 0;
        capacity *= 2;
        data = reinterpret_cast<Pair*>(_aligned_malloc(sizeof(Pair) * capacity, CACHE_LINE));
        memset(data, 0xff, sizeof(Pair) * capacity);
        for (size_t i = 0; i < old_capacity; i++) {
            if (old_data[i].key < EMPTY)
                set(old_data[i].key, old_data[i].value);
        }
        _aligned_free(old_data);
    }

    size_t index_for(size_t key) const {
        size_t hash = hasher(key);
        size_t index = hash & (capacity - 1);
        return index;
    }

    const size_t& index_from_key(size_t key) const {
        size_t hash = hasher(key);
        size_t index = hash & (capacity - 1);
        size_t dist = 0;
        for (;;) {
            if (data[index].key == EMPTY)
                return EMPTY;
            if (data[index].key == key)
                return index;
            size_t desired = index_for(data[index].key);
            size_t cur_dist = (index + capacity - desired) & (capacity - 1);
            if (cur_dist < dist)
                return EMPTY;
            dist++;
            index = (index + 1) & (capacity - 1);
        }
    }

    Pair* data;
    size_t capacity;
    size_t size_;

public:
    Dict() {
        size_ = 0;
        capacity = 8;
        data = reinterpret_cast<Pair*>(_aligned_malloc(sizeof(Pair) * capacity, CACHE_LINE));
        memset(data, 0xff, sizeof(Pair) * capacity);
    }

    ~Dict() { _aligned_free(data); }

    Dict(const Dict&) = delete;
    Dict& operator=(const Dict&) = delete;

    Dict(Dict&& rhs) {
        data = rhs.data;
        rhs.data = nullptr;

        capacity = rhs.capacity;
        rhs.capacity = 0;

        size_ = rhs.size_;
        rhs.size_ = 0;
    }
    Dict&& operator=(Dict&&) = delete;

    bool contains(size_t key) const {
        return index_from_key(key) != EMPTY;
    }

    // returns stub if key was not found
    size_t pop(size_t key, bool& found) {
        size_t hash = hasher(key);
        size_t index = hash & (capacity - 1);
        size_t dist = 0;
        for (;;) {
            if (data[index].key == key) {
                size_--;
                Value result = data[index].value;
                remove(index);
                found = true;
                return result;
            }
            size_t desired = index_for(data[index].key);
            size_t cur_dist = (index + capacity - desired) & (capacity - 1);
            if (cur_dist < dist) {
                found = false;
                return stub;
            }
            dist++;
            index = (index + 1) & (capacity - 1);
        }
    }

    // returns stub if key was not found (rather than throwing an exception)
    size_t pop(size_t key) {
        bool found;
        return pop(key, found);
    }

    // returns true if key was found (and thus removed)
    bool del(size_t key) {
        bool found;
        pop(key, found);
        return found;
    }

    void clear() {
        size_ = 0;
        std::memset(data, 0xff, sizeof(Pair) * capacity);
    }

    size_t size() const { return size_; }

    // returns false when overwriting, may grow unnecessarily but would only be a perf & memory hit if dict capacity * LF == dict size and we are never going to insert a new key again
    bool set(size_t key, size_t value) {
        if (size_ >= capacity * LF)
            grow();
        size_t hash = hasher(key);
        size_t index = hash & (capacity - 1);
        size_t dist = 0;
        for (;;) {
            // overwrite
            if (data[index].key == key) {
                data[index].value = value;
                return false;
            }
            if (data[index].key == EMPTY) {
                data[index].key = key;
                data[index].value = value;
                size_++;
                return true;
            }
            size_t desired = index_for(data[index].key);
            size_t cur_dist = (index + capacity - desired) & (capacity - 1);
            if (cur_dist < dist) {
                std::swap(key, data[index].key);
                std::swap(value, data[index].value);
                dist = cur_dist;
            }
            dist++;
            index = (index + 1) & (capacity - 1);
        }
        size_++;
    }

    // getters return a stub value when key not found, the goal is not to crash
    // const getters
    const size_t& get(size_t key, bool& found) const {
        size_t index = index_from_key(key);
        if (index == EMPTY) {
            found = false;
            return stub;
        }
        found = true;
        return data[index].value;
    }

    const size_t& get(size_t key) const {
        bool found;
        return get(key, found);
    }

    // non-const getters
    size_t& get(size_t key, bool& found) {
        size_t index = index_from_key(key);
        if (index == EMPTY) {
            found = false;
            return stub;
        }
        found = true;
        return data[index].value;
    }

    size_t& get(size_t key) {
        bool found;
        return get(key, found);
    }

    // pythonic methods
    size_t len() const { return size_; }

    size_t& setdefault(size_t key, size_t value) {
        size_t index = index_from_key(key);
        if (index == EMPTY) {
            data[index].key = key;
            data[index].value = value;
            size_++;
        }
        return data[index].value;
    }

    void update(const Dict& other, bool overwrite = true) {
        for (const Pair& pair : other) {
            if (!overwrite && contains(pair.key))
                continue;
            set(pair.key, pair.value);
        }
    }

    template<typename T, size_t inc>
    struct Iterator {
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(char* ptr) : m_ptr(ptr) {}

        reference operator*() const { return *(pointer)m_ptr; }
        pointer operator->() { return (pointer)m_ptr; }

        // Prefix increment
        Iterator& operator++() { m_ptr += inc; return *this; }

        // Postfix increment
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

        char* m_ptr;
    };

    template<typename T, size_t inc>
    struct ConstIterator {
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;

        ConstIterator(const char* ptr) : m_ptr(ptr) {}

        reference operator*() const { return *(pointer)m_ptr; }
        pointer operator->() { return (pointer)m_ptr; }

        // Prefix increment
        ConstIterator& operator++() { m_ptr += inc; return *this; }

        // Postfix increment
        ConstIterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const ConstIterator& a, const ConstIterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const ConstIterator& a, const ConstIterator& b) { return a.m_ptr != b.m_ptr; };

        const char* m_ptr;
    };

    typedef Iterator<Pair, sizeof(Pair)> PairIterator;
    typedef Iterator<size_t, sizeof(Pair)> KeyIterator;
    typedef Iterator<size_t, sizeof(Pair)> ValueIterator;
    typedef ConstIterator<Pair, sizeof(Pair)> PairConstIterator;
    typedef ConstIterator<size_t, sizeof(Pair)> KeyConstIterator;
    typedef ConstIterator<size_t, sizeof(Pair)> ValueConstIterator;

    PairIterator begin() { return PairIterator((char*)data); }
    PairIterator end() { return PairIterator((char*)(data + size_)); }

    KeyIterator keys_begin() { return KeyIterator((char*)data + offsetof(Pair, key)); }
    KeyIterator keys_end() { return KeyIterator((char*)data + offsetof(Pair, key) + sizeof(Pair) * size_); }

    ValueIterator values_begin() { return ValueIterator((char*)data + offsetof(Pair, value)); }
    ValueIterator values_end() { return ValueIterator((char*)data + offsetof(Pair, value) + sizeof(Pair) * size_); }

    PairConstIterator begin() const { return PairConstIterator((const char*)data); }
    PairConstIterator end() const { return PairConstIterator((const char*)(data + size_)); }

    KeyConstIterator keys_begin() const { return KeyConstIterator((const char*)data + offsetof(Pair, key)); }
    KeyConstIterator keys_end() const { return KeyConstIterator((const char*)data + offsetof(Pair, key) + sizeof(Pair) * size_); }

    ValueConstIterator values_begin() const { return ValueConstIterator((const char*)data + offsetof(Pair, value)); }
    ValueConstIterator values_end() const { return ValueConstIterator((const char*)data + offsetof(Pair, value) + sizeof(Pair) * size_); }
};
