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
Iteration through map is linear because it has a linked list where all <Key, Value> pairs are stored. It takes O(1) to remove something from a linked list.
See more: https://en.wikipedia.org/wiki/Hash_table#Separate_chaining_with_linked_lists
*/


template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
  public:
    typedef typename std::list<std::pair<const KeyType, ValueType> >::iterator iterator;
    typedef typename std::list<std::pair<const KeyType, ValueType> >::const_iterator const_iterator;

    //Default constructor.
    HashMap(const Hash& _hasher = Hash()):
        _hasher(_hasher),
        _capacity(1),
        _table(1),
        _sz(0) {}

    //Iterator constructor.
    template<class inputIterator> HashMap(const inputIterator inputBegin,
                                          const inputIterator inputEnd,
                                          const Hash& _hasher = Hash()):
        _hasher(_hasher),
        _capacity(std::distance(inputBegin, inputEnd)),
        _table(std::distance(inputBegin, inputEnd)),
        _sz(0) {
            if (_capacity == 0) {
                _capacity = 1;
                _table.resize(_capacity);
                return;
            }
            for (auto it = inputBegin; it != inputEnd; ++it) {
                size_t hash = _hasher(it->first) % _capacity;
                bool flag = false;
                for (iterator contentIt : _table[hash])
                    if (contentIt->first == it->first)
                        flag = true;
                if (flag)
                    continue;
                _content.push_front(*it);
                _table[hash].push_back(_content.begin());
                _sz++;
            }
        }

    //Initializer list constructor.
    HashMap(const std::initializer_list<std::pair<const KeyType, ValueType> >& input,
            const Hash& _hasher = Hash()):
            _hasher(_hasher),
            _capacity(input.size()),
            _table(input.size()),
            _sz(0) {
            if (input.begin() == input.end()) {
                _capacity = 1;
                _table.resize(1);
                return;
            }
        for (auto it = input.begin(); it != input.end(); ++it) {
            size_t hash = _hasher(it->first) % _capacity;
            bool flag = false;
            for (iterator contentIt : _table[hash])
                if (contentIt->first == it->first)
                    flag = true;
            if (flag)
                continue;
            _sz++;
            _content.push_front(*it);
            _table[hash].push_back(_content.begin());
        }
    }

    //Copy constructor.
    HashMap(const HashMap<KeyType, ValueType, Hash>& other):
        _hasher(other.hash_function()),
        _content(other._content),
        _capacity(other.size()),
        _table(other.size()),
        _sz(other.size()) {
            if (_capacity == 0) {
                _capacity = 1;
                _table.resize(_capacity);
            }
            for (auto it = _content.begin(); it != _content.end(); ++it) {
                size_t hash = _hasher(it->first) % _capacity;
                _table[hash].push_back(it);
            }
    }

    //Assign operator.
    HashMap<KeyType, ValueType, Hash>& operator = (const HashMap<KeyType, ValueType, Hash>& other) {
        size_t other_size = other.size();
        size_t my_size = size();
        auto _it = other.begin();
        for (size_t i = 0; i < other_size; i++) {
            std::pair<const KeyType, ValueType> currentPair(*_it);
            _content.push_back(currentPair);
            _it++;
        }
        for (size_t i = 0; i < my_size; i++)
            _content.pop_front();
        _capacity = size();
        if (_capacity == 0)
            _capacity = 1;
        _table.clear();
        _table.resize(_capacity);
        for (auto it = _content.begin(); it != _content.end(); ++it) {
            size_t hash = _hasher(it->first) % _capacity;
            _table[hash].push_back(it);
        }
        _sz = other_size;
        return (*this);
    }

    //Returns size.
    size_t size() const {
        return _sz;
    }

    //Returns true if size is 0.
    bool empty() const {
        return _content.empty();
    }

    //Returns hasher.
    Hash hash_function() const {
        return _hasher;
    }

    //Inserts new <Key, Value> pair. Checks if table already already has this key.
    void insert(const std::pair<const KeyType, ValueType>& insertValue) {
        size_t hash = _hasher(insertValue.first) % _capacity;
        bool flag = false;
        for (auto it : _table[hash]) {
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
        size_t hash = _hasher(key) % _capacity;
        for (auto it = _table[hash].begin(); it != _table[hash].end(); ++it) {
            if ((*it)->first == key) {
                _content.erase(*it);
                _table[hash].erase(it);
                _sz--;
                break;
            }
        }
    }

    //These are iterators, same as std::vector.
    iterator begin() {
        return _content.begin();
    }

    iterator end() {
        return _content.end();
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return _content.cbegin();
    }

    const_iterator cend() const {
        return _content.cend();
    }

    //Give an iterator to <Key, Value> pair with give key. If fails returns end().
    iterator find(const KeyType& key) {
        size_t hash = _hasher(key) % _capacity;
        for (auto it : _table[hash]) {
            if (it->first == key)
                return it;
        }
        return end();
    }

    //Same, but const iterator.
    const_iterator find(const KeyType& key) const {
        size_t hash = _hasher(key) % _capacity;
        for (auto it : _table[hash]) {
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
            size_t hash = _hasher(it->first) % _capacity;
            _table[hash].clear();
        }
        _content.clear();
        _sz = 0;
    }

  private:
    Hash _hasher;
    std::list<std::pair<const KeyType, ValueType> > _content;
    size_t _capacity;
    std::vector<std::list<iterator> > _table;
    size_t _sz;

    //Doubles the capacity and rebuilds the _table.
    void expand() {
        _table.clear();
        _capacity *= 2;
        _table.resize(_capacity);
        for (auto it = begin(); it != end(); ++it) {
            size_t hash = _hasher(it->first) % _capacity;
            _table[hash].push_back(it);
        }
    }

    //Adds <Key, Value> pair into the table (use if doesn't have this key), calls expand when needed.
    void add(const std::pair<const KeyType, ValueType>& insertValue) {
        _sz++;
        size_t hash = _hasher(insertValue.first) % _capacity;
        if (size() == _capacity) {
            expand();
            hash = _hasher(insertValue.first) % _capacity;
        }
        _content.push_front(insertValue);
        _table[hash].push_back(_content.begin());
    }
};
