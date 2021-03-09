#include <list>
#include <functional>
#include <vector>
#include <utility>
#include <initializer_list>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
public:
    typedef typename std::list<std::pair<const KeyType, ValueType> >::iterator iterator;
    typedef typename std::list<std::pair<const KeyType, ValueType> >::const_iterator const_iterator;

private:
    Hash hasher;
    std::list<std::pair<const KeyType, ValueType> > content;
    size_t capacity;
    std::vector<std::list<iterator> > table;
    size_t sz;

    void expand() {
        table.clear();
        capacity *= 2;
        table.resize(capacity);
        for (auto it = begin(); it != end(); ++it) {
            int hash = hasher(it->first) % capacity;
            table[hash].push_back(it);
        }
    }

    void add(const std::pair<const KeyType, ValueType>& insertValue) {
        sz++;
        int hash = hasher(insertValue.first) % capacity;
        if (size() == capacity) {
            expand();
            hash = hasher(insertValue.first) % capacity;
        }
        content.push_front(insertValue);
        table[hash].push_back(content.begin());
    }

public:
    HashMap(const Hash& _hasher = Hash()):
        hasher(_hasher),
        capacity(1),
        table(1),
        sz(0) {}

    template<class inputIterator> HashMap(const inputIterator inputBegin,
                                          const inputIterator inputEnd,
                                          const Hash& _hasher = Hash()):
        hasher(_hasher),
        capacity(std::distance(inputBegin, inputEnd)),
        table(std::distance(inputBegin, inputEnd)),
        sz(0) {
            if (capacity == 0) {
                capacity = 1;
                table.resize(capacity);
                return;
            }
            for (auto it = inputBegin; it != inputEnd; ++it) {
                int hash = hasher(it->first) % capacity;
                bool flag = false;
                for (iterator contentIt : table[hash])
                    if (contentIt->first == it->first)
                        flag = true;
                if (flag)
                    continue;
                content.push_front(*it);
                table[hash].push_back(content.begin());
                sz++;
            }
        }

    HashMap(const std::initializer_list<std::pair<const KeyType, ValueType> >& input,
            const Hash& _hasher = Hash()):
            hasher(_hasher),
            capacity(input.size()),
            table(input.size()),
            sz(0) {
            if (input.begin() == input.end()) {
                capacity = 1;
                table.resize(1);
                return;
            }
        for (auto it = input.begin(); it != input.end(); ++it) {
            int hash = hasher(it->first) % capacity;
            bool flag = false;
            for (iterator contentIt : table[hash])
                if (contentIt->first == it->first)
                    flag = true;
            if (flag)
                continue;
            sz++;
            content.push_front(*it);
            table[hash].push_back(content.begin());
        }
    }

    HashMap(const HashMap<KeyType, ValueType, Hash>& other):
        hasher(other.hash_function()),
        content(other.content),
        capacity(other.size()),
        table(other.size()),
        sz(other.size()) {
            if (capacity == 0) {
                capacity = 1;
                table.resize(capacity);
            }
            for (auto it = content.begin(); it != content.end(); ++it) {
                int hash = hasher(it->first) % capacity;
                table[hash].push_back(it);
            }
    }

    HashMap<KeyType, ValueType, Hash>& operator = (const HashMap<KeyType, ValueType, Hash>& other) {
        size_t other_size = other.size();
        size_t my_size = size();
        auto _it = other.begin();
        for (size_t i = 0; i < other_size; i++) {
            std::pair<const KeyType, ValueType> currentPair(*_it);
            content.push_back(currentPair);
            _it++;
        }
        for (size_t i = 0; i < my_size; i++)
            content.pop_front();
        capacity = size();
        if (capacity == 0)
            capacity = 1;
        table.clear();
        table.resize(capacity);
        for (auto it = content.begin(); it != content.end(); ++it) {
            int hash = hasher(it->first) % capacity;
            table[hash].push_back(it);
        }
        sz = other_size;
        return (*this);
    }


    size_t size() const {
        return sz;
    }

    bool empty() const {
        return content.empty();
    }

    Hash hash_function() const {
        return hasher;
    }

    void insert(const std::pair<const KeyType, ValueType>& insertValue) {
        int hash = hasher(insertValue.first) % capacity;
        bool flag = false;
        for (auto it : table[hash]) {
            if (it->first == insertValue.first) {
                flag = true;
                break;
            }
        }
        if (flag)
            return;
        add(insertValue);
    }

    void erase(const KeyType& key) {
        int hash = hasher(key) % capacity;
        for (auto it = table[hash].begin(); it != table[hash].end(); ++it) {
            if ((*it)->first == key) {
                content.erase(*it);
                table[hash].erase(it);
                sz--;
                break;
            }
        }
    }

    iterator begin() {
        return content.begin();
    }

    iterator end() {
        return content.end();
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return content.cbegin();
    }

    const_iterator cend() const {
        return content.cend();
    }

    iterator find(const KeyType& key) {
        int hash = hasher(key) % capacity;
        for (auto it : table[hash]) {
            if (it->first == key)
                return it;
        }
        return end();
    }

    const_iterator find(const KeyType& key) const {
        int hash = hasher(key) % capacity;
        for (auto it : table[hash]) {
            if (it->first == key) {
                const_iterator returnItem = it;
                return returnItem;
            }
        }
        return cend();
    }

    ValueType& operator [] (const KeyType& key) {
        auto it = find(key);
        if (it == end()) {
            add(std::make_pair(key, ValueType()));
            return begin()->second;
        }
        return it->second;
    }

    const ValueType& at(const KeyType& key) const {
        auto it = find(key);
        if (it == cend()) {
            throw std::out_of_range("No such key exists in map");
        }
        return it->second;
    }

    void clear() {
        for (auto it = begin(); it != end(); ++it) {
            int hash = hasher(it->first) % capacity;
            table[hash].clear();
        }
        content.clear();
        sz = 0;
    }
};
