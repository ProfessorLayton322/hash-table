#include <list>
#include <functional>
#include <vector>
#include <utility>
#include <initializer_list>
#include <stdexcept>


/*
Hash table.
Using chain method.
It doubles the table size if needed to expand.
Check https://pastebin.com/esx8ckYw for more.
*/


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

    //Doubles the capacity and rebuilds the table.
    void expand() {
        table.clear();
        capacity *= 2;
        table.resize(capacity);
        for (auto it = begin(); it != end(); ++it) {
            int hash = hasher(it->first) % capacity;
            table[hash].push_back(it);
        }
    }

    //Adds <Key, Value> pair into the table (use if doesn't have this key), calls expand when needed.
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
    //Default constructor.
    HashMap(const Hash& _hasher = Hash()):
        hasher(_hasher),
        capacity(1),
        table(1),
        sz(0) {}

    //Iterator constructor.
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

    //Initializer list constructor.
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

    //Copy constructor.
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

    //Assign operator.
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

    //Returns size.
    size_t size() const {
        return sz;
    }

    //Returns true if size is 0.
    bool empty() const {
        return content.empty();
    }

    //Returns hasher.
    Hash hash_function() const {
        return hasher;
    }

    //Inserts new <Key, Value> pair. Checks if table already already has this key.
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

    //Erases pair with given key from the table.
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

    //These are iterators, same as std::vector.
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

    //Give an iterator to <Key, Value> pair with give key. If fails returns end().
    iterator find(const KeyType& key) {
        int hash = hasher(key) % capacity;
        for (auto it : table[hash]) {
            if (it->first == key)
                return it;
        }
        return end();
    }

    //Same, but const iterator.
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

    //Operator [], allows to edit hashMap[key] value.
    ValueType& operator [] (const KeyType& key) {
        auto it = find(key);
        if (it == end()) {
            add(std::make_pair(key, ValueType()));
            return begin()->second;
        }
        return it->second;
    }

    //Get value for given key, readonly.
    const ValueType& at(const KeyType& key) const {
        auto it = find(key);
        if (it == cend()) {
            throw std::out_of_range("No such key exists in map");
        }
        return it->second;
    }

    //Clears the table.
    void clear() {
        for (auto it = begin(); it != end(); ++it) {
            int hash = hasher(it->first) % capacity;
            table[hash].clear();
        }
        content.clear();
        sz = 0;
    }
};
