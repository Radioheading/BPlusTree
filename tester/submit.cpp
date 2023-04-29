#ifndef BPT__EXCEPTIONS_HPP_
#define BPT__EXCEPTIONS_HPP_

#include <cstddef>
#include <cstring>
#include <string>

#ifndef BPT__RECYCLE_HPP_
#define BPT__RECYCLE_HPP_
#include <string>
#include <fstream>

const int max_bin = 1e3 + 5;

struct before {
  int size = 0;
  int address[max_bin];
};
class bin {
 private:
  before store;
  std::fstream garbage;
 public:
  explicit bin(const std::string &bin_name) {
    garbage.open(bin_name);
    garbage.seekg(0, std::ios::beg);
    if (!garbage) {
      garbage.open(bin_name, std::ios::out);
      garbage.write(reinterpret_cast<char *>(&store), sizeof(store));
      garbage.close();
      garbage.open(bin_name);
    } else {
      garbage.read(reinterpret_cast<char *>(&store), sizeof(store));
    }
  }

  ~bin() {
    garbage.seekp(0);
    garbage.write(reinterpret_cast<char *>(&store), sizeof(store));
    garbage.close();
  }
  bool empty() const {
    return !store.size;
  }

  int pop_back() {
    return store.address[--store.size];
  }

  void push_back(const int &todo) {
    if (store.size == max_bin - 1) return;
    store.address[store.size++] = todo;
  }
};
#endif //BPT__RECYCLE_HPP_

namespace sjtu {

class exception {
 protected:
  const std::string variant = "";
  std::string detail = "";
 public:
  exception() {}
  exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
  virtual std::string what() {
    return variant + " " + detail;
  }
};

class index_out_of_bound : public exception {
  /* __________________________ */
};

class runtime_error : public exception {
  /* __________________________ */
};

class invalid_iterator : public exception {
  /* __________________________ */
};

class container_is_empty : public exception {
  /* __________________________ */
};
}

#endif //BPT__EXCEPTIONS_HPP_

#ifndef BPT__CACHELIST_HPP_
#define BPT__CACHELIST_HPP_

#include <iostream>
#include <fstream>

using std::pair;

const int max_cache = 3000;
const int map_size = 23475;
const int mod = 23473;
int rehash[20] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};

template<class T>
class CachePool {
 private:
  struct cache_node {
    int address = 0;
    T data;
    cache_node *prev = nullptr, *next = nullptr;
    cache_node(int address_ = 0,
               cache_node *prev_ = nullptr,
               cache_node *next_ = nullptr)
        : address(address_), prev(prev_), next(next_) {}
  };
  class cache {
   public:
   private:
    int size = 0;
    std::fstream &out;
   public:
    cache_node *head, *tail;
    explicit cache(std::fstream &out_) : out(out_), size(0) {
      head = new cache_node(), tail = new cache_node();
      head->next = tail, tail->prev = head;
    }
    ~cache() {
      size = 0;
      cache_node *now = head->next;
      while (now != tail) {
        cache_node *temp = now;
        now = now->next;
        if (temp->data.changed && temp->address != 0) {
          temp->data.changed = false;
          out.seekp(temp->address);
          out.write(reinterpret_cast<char *>(&temp->data), sizeof(temp->data));
        }
        delete temp;
      }
      out.close();
      delete now, delete head;
    }

    void erase(cache_node *todo) {
      todo->prev->next = todo->next;
      todo->next->prev = todo->prev;
      delete todo;
      --size;
    }

    void pop_back() {
      cache_node *todo = tail->prev;
      if (todo->data.changed) {
        todo->data.changed = false;
        out.seekp(todo->address);
        out.write(reinterpret_cast<char *>(&todo->data), sizeof(todo->data));
      }
      todo->prev->next = tail;
      tail->prev = todo->prev;
      delete todo;
      --size;
    }

    std::pair<cache_node *, bool> push_front(const T &val) {
      // std::cout << "pushing front " << val.address << '\n';
      auto *todo = new cache_node(val.address);
      // std::cout << "first address: " << val.address << '\n';
      todo->data = val;
      todo->prev = head;
      head->next->prev = todo;
      todo->next = head->next;
      head->next = todo;
      ++size;
      // std::cout << "size = " << size << '\n';
      if (size == max_cache) {
        return std::make_pair(todo, false);
      }
      return std::make_pair(todo, true);
    }
  };
  cache block_pool;
  cache_node *storage[map_size] = {nullptr};
  pair<cache_node *, int> find(int id) {
    int pos = id % mod;
    int cnt = 0;
    while (pos < map_size) {
      if (storage[pos] && storage[pos]->address == id) {
        return pair<cache_node *, int>(storage[pos], pos);
      } else {
        pos += rehash[cnt];
        ++cnt;
      }
    }
    return pair<cache_node *, int>(nullptr, -1);
  }

  int find_valid(int id) {
    int pos = id % mod, cnt = 0;
    while (pos < map_size) {
      if (storage[pos]) {
        pos += rehash[cnt];
        ++cnt;
      } else {
        return pos;
      }
    }
    return -1;
  }

  int find_place(int id) {
    int pos = id % mod, cnt = 0;
    while (pos < map_size) {
      if (storage[pos] && storage[pos]->address == id) {
        return pos;
      } else {
        pos += rehash[cnt];
        ++cnt;
      }
    }
    return -1;
  }

  void erase_last() {
    cache_node *todo = block_pool.tail->prev;
    if (!todo) return;
    storage[find_place(todo->address)] = nullptr;
    block_pool.pop_back();
  }

  void Delete(cache_node *todo) {
    storage[find_place(todo->address)] = nullptr;
    block_pool.erase(todo);
  }
 public:
  CachePool(std::fstream &out_) : block_pool(out_) {}
  bool GetNode(T &todo, int id) {
    // std::cout << id << '\n';
    cache_node *search = find(id).first;
    if (search) {
      todo = search->data;
      Delete(search);
      return true;
    } else {
      return false;
    }
  }
  void InsertFront(const T &value) {
    auto res = block_pool.push_front(value);
    if (!res.second) {
      erase_last();
    }
    storage[find_valid(value.address)] = res.first;
  }

};
#endif //BPT__CACHELIST_HPP_

#ifndef BPT__VECTOR_HPP_
#define BPT__VECTOR_HPP_
#include <climits>
#include <cstddef>

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector {
 private:
  T *data;
  size_t capacity, current;
  /* -----------------------function : ChangeSize---------------------------
   * vector is, in essence, a dynamic array, which means we can't know
   * the real size of the array, thus dynamic size expansion ought to be
   * done.
   * According to the article found by @Polaris_Dane, the optimal size
   * expansion may be 1.68 * ans when it comes to memory saving. However,
   * the detailed proof was omitted
   * */
  void ChangeSize() {
    T *new_data = (T *) malloc((size_t) (2 * capacity) * sizeof(T));
    for (int i = 0; i < current; ++i) {
      // new_data[i] = data[i];
      /* warning: we haven't used constructor upon the allocated memory
       * (and the object may lack default constructor)
       * thus the method **placement new** can be employed to use copy constructor */
      new(new_data + i) T(data[i]);
      data[i].~T();
    }
    std::swap(data, new_data);
    free(new_data);
    capacity = (size_t) (2 * capacity);
  }
 public:
  /**
   * TODO
   * a type for actions of the elements of a vector, and you should write
   *   a class named const_iterator with same interfaces.
   */
  /**
   * you can see RandomAccessIterator at CppReference for help.
   */
  class const_iterator;
  class iterator {
    // The following code is written for the C++ type_traits library.
    // Type traits is a C++ feature for describing certain properties of a type.
    // For instance, for an iterator, iterator::value_type is the type that the
    // iterator points to.
    // STL algorithms and containers may use these type_traits (e.g. the following
    // typedef) to work properly. In particular, without the following code,
    // @code{std::sort(iter, iter1);} would not compile.
    // See these websites for more information:
    // https://en.cppreference.com/w/cpp/header/type_traits
    // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
    // About iterator_category: https://en.cppreference.com/w/cpp/iterator
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;
    friend class vector;
   private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    size_t position;
    vector *from;
   public:
    /*
     * constructors: default ones with given parameters
     */
    iterator(int _position = 0, vector *_from = nullptr) : position(_position), from(_from) {}
    iterator(const iterator &other) : position(other.position), from(other.from) {};
    /**
     * return a new iterator which pointer n-next elements
     * as well as operator-
     */

    iterator operator+(const int &n) const {
      return iterator(position + n, from);
    }
    iterator operator-(const int &n) const {
      return iterator(position - n, from);
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw invalid_iterator.
    int operator-(const iterator &rhs) const {
      if (from != rhs.from) throw invalid_iterator();
      return (difference_type) (position - rhs.position);
    }
    iterator &operator+=(const int &n) {
      position += n;
      return *this;
    }
    iterator &operator-=(const int &n) {
      position -= n;
      return *this;
    }
    /**
     * TODO iter++
     */
    iterator operator++(int) {
      iterator stable_iter(position, from);
      ++position;
      return stable_iter;
    }
    /**
     * TODO ++iter
     */
    iterator &operator++() {
      ++position;
      return *this;
    }
    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator stable_iter(position, from);
      --position;
      return stable_iter;
    }
    /**
     * TODO --iter
     */
    iterator &operator--() {
      --position;
      return *this;
    }
    /**
     * TODO *it
     */
    T &operator*() const {
      return from->data[position];
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory address).
     */
    bool operator==(const iterator &rhs) const {
      return (rhs.from == from && position == rhs.position);
    }
    bool operator==(const const_iterator &rhs) const {
      return (rhs.from == from && position == rhs.position);
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return (rhs.from != from || position != rhs.position);
    }
    bool operator!=(const const_iterator &rhs) const {
      return (rhs.from != from || position != rhs.position);
    }
  };
  /**
   * TODO
   * has same function as iterator, just for a const object.
   */
  class const_iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;
    friend class vector;
   private:
    size_t position;
    const vector *from;
   public:
    // the member functions and the constructors
    const_iterator(int _position = 0, const vector *_from = nullptr) : position(_position), from(_from) {};
    const_iterator(const const_iterator &other) : position(other.position), from(other.from) {};
    const_iterator operator+(const int &n) const {
      return const_iterator(position + n, from);
    }
    const_iterator operator-(const int &n) const {
      return const_iterator(position - n, from);
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw invalid_iterator.
    int operator-(const const_iterator &rhs) const {
      if (from != rhs.from) throw invalid_iterator();
      return (difference_type) (position - rhs.position);
    }
    const_iterator &operator+=(const int &n) {
      position += n;
      return *this;
    }
    const_iterator &operator-=(const int &n) {
      position -= n;
      return *this;
    }
    /**
     * TODO iter++
     */
    const_iterator operator++(int) {
      const_iterator stable_iter(position, from);
      ++position;
      return stable_iter;
    }
    /**
     * TODO ++iter
     */
    const_iterator operator++() {
      ++position;
      return *this;
    }
    /**
     * TODO iter--
     */
    const_iterator operator--(int) {
      const_iterator stable_iter(position, from);
      --position;
      return stable_iter;
    }
    /**
     * TODO --iter
     */
    const_iterator &operator--() {
      --position;
      return *this;
    }
    /**
     * TODO *it
     */
    const T &operator*() const {
      return from->data[position];
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory address).
     */
    bool operator==(const iterator &rhs) const {
      return (rhs.from == from && rhs.position == position);
    }
    bool operator==(const const_iterator &rhs) const {
      return (rhs.from == from && rhs.position == position);
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return (rhs.from != from || rhs.position != position);
    }
    bool operator!=(const const_iterator &rhs) const {
      return (rhs.from != from || rhs.position != position);
    }
  };
  /**
   * TODO Constructs
   * At least two: default constructor, copy constructor
   */
  vector() : data(nullptr), capacity(2), current(0) {
    data = (T *) malloc(capacity * sizeof(T));
  }
  vector(const vector &other) : capacity(other.capacity), current(other.current) {
    data = (T *) malloc(capacity * sizeof(T));
    for (int i = 0; i < current; ++i) {
      //data[i] = other[i];
      new(data + i) T(other[i]);
    }
  }
  /**
   * TODO Destructor
   */
  ~vector() {
    for (int i = 0; i < current; ++i) {
      data[i].~T();
    }
    free(data);
  }
  /**
   * TODO Assignment operator
   */
  vector &operator=(const vector &other) {
    if (this == &other) return *this;
    for (int i = 0; i < current; ++i) {
      data[i].~T();
    }
    free(data);
    capacity = other.capacity, current = other.current;
    data = (T *) malloc(other.capacity * sizeof(T));
    for (int i = 0; i < current; ++i) {
      new(data + i) T(other[i]);
    }
    return *this;
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   */
  T &at(const size_t &pos) {
    if (pos < 0 || pos >= current) throw index_out_of_bound();
    return data[pos];
  }
  const T &at(const size_t &pos) const {
    if (pos < 0 || pos >= current) throw index_out_of_bound();
    return data[pos];
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   * !!! Pay attentions
   *   In STL this operator does not check the boundary but I want you to do.
   */
  T &operator[](const size_t &pos) {
    if (pos < 0 || pos >= current) throw index_out_of_bound();
    return data[pos];
  }
  const T &operator[](const size_t &pos) const {
    if (pos < 0 || pos >= current) throw index_out_of_bound();
    return data[pos];
  }
  /**
   * access the first element.
   * throw container_is_empty if size == 0
   */
  const T &front() const {
    if (current == 0) throw container_is_empty();
    return data[0];
  }
  /**
   * access the last element.
   * throw container_is_empty if size == 0
   */
  const T &back() const {
    if (current == 0) throw container_is_empty();
    return data[current - 1];
  }
  /**
   * returns an iterator to the beginning.
   */
  iterator begin() {
    return iterator(0, this);
  }
  const_iterator cbegin() const {
    return const_iterator(0, this);
  }
  /**
   * returns an iterator to the end.
   */
  iterator end() {
    return iterator(current, this);
  }
  const_iterator cend() const {
    return const_iterator(current, this);
  }
  /**
   * checks whether the container is empty
   */
  bool empty() const {
    return (current == 0);
  }
  /**
   * returns the number of elements
   */
  size_t size() const {
    return current;
  }
  /**
   * clears the contents
   */
  void clear() {
    for (int i = 0; i < current; ++i) {
      data[i].~T();
    }
    current = 0;
  }
  /**
   * inserts value before pos
   * returns an iterator pointing to the inserted value.
   */
  iterator insert(iterator pos, const T &value) {
    if (current + 1 == capacity) ChangeSize();
    new(data + current) T(data[current - 1]);
    for (int i = current - 1; i >= (int) pos.position; --i) {
      data[i + 1] = data[i];
    }
    data[pos.position] = value;
    ++current;
    return iterator(pos.position, this);
  }
  /**
   * inserts value at index ind.
   * after inserting, this->at(ind) == value
   * returns an iterator pointing to the inserted value.
   * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
   */
  iterator insert(const size_t &ind, const T &value) {
    if (ind > size()) throw index_out_of_bound();
    iterator insert_iter(ind, this);
    if (current + 1 == capacity) ChangeSize();
    new(data + current) T(data[current - 1]);
    for (int i = current - 1; i >= (int) ind; --i) {
      data[i + 1] = data[i];
    }
    data[ind] = value;
    ++current;
    return insert_iter;
  }
  /**
   * removes the element at pos.
   * return an iterator pointing to the following element.
   * If the iterator pos refers the last element, the end() iterator is returned.
   */
  iterator erase(iterator pos) {
    data[pos.position].~T();
    for (int i = pos.position; i < current - 1; ++i) {
      data[i] = data[i + 1];
    }
    --current;
    return iterator(pos.position, this);
  }
  /**
   * removes the element with index ind.
   * return an iterator pointing to the following element.
   * throw index_out_of_bound if ind >= size
   */
  iterator erase(const size_t &ind) {
    if (ind >= size()) throw index_out_of_bound();
    data[ind].~T();
    for (int i = ind; i < current - 1; ++i) {
      data[i] = data[i + 1];
    }
    --current;
    return iterator(ind, this);
  }
  /**
   * adds an element to the end.
   */
  void push_back(const T &value) {
    if (current + 1 == capacity) ChangeSize();
    // data[current] = value;
    new(data + current) T(value);
    ++current;
  }
  /**
   * remove the last element from the end.
   * throw container_is_empty if size() == 0
   */
  void pop_back() {
    if (current == 0) throw container_is_empty();
    --current;
    data[current].~T();
  }
};
}
#endif
#ifndef BPT__BPT_HPP_
#define BPT__BPT_HPP_
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
// #include "CacheList.hpp"
// #include "recycle.hpp"
// #include "vector.hpp"

const int max_size = 202, min_size = 101;
const int max_son = 202, min_son = 101;

/*
 * @supplementary functions
 * these are several means to locate specific element
 * including upper_bound, lower_bound, lower_search and binary_search
 */
template<class T>
int LowerBound(T val, T *array, int l, int r) {
  int ans = r + 1;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (val < array[mid]) {
      r = mid - 1, ans = mid;
    } else {
      l = mid + 1;
    }
  }
  return ans;
}
template<class T>
int LowerSearch(T val, T *array, int l, int r) {
  int ans = r + 1;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (val < array[mid] || val == array[mid]) {
      r = mid - 1, ans = mid;
    } else {
      l = mid + 1;
    }
  }
  return ans;
}
template<class T>
int UpperBound(T val, T *array, int l, int r) {
  int ans = 0;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (!(val < array[mid])) {
      l = mid + 1, ans = mid;
    } else {
      r = mid - 1;
    }
  }
  return ans;
}
template<class T>
int BinarySearch(T val, T *array, int l, int r) {
  int ans = r + 1;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (val.key < array[mid].key) {
      r = mid - 1;
    } else if (array[mid].key < val.key) {
      l = mid + 1;
    } else {
      r = mid - 1, ans = mid;
    }
  }
  return ans;
}

template<class Key, class T>
class BPlusTree {
  enum NodeState { leaf, middle };
 private:
  bin tree_bin, data_bin;
  std::fstream tree, data;
  std::string tree_name, data_name;
  struct element {
    Key key;
    T value;

    inline friend bool operator<(const element &cmp_1, const element &cmp_2) {
      return cmp_1.key < cmp_2.key
          || cmp_1.key == cmp_2.key && cmp_1.value < cmp_2.value;
    }

    inline friend bool operator==(const element &cmp_1, const element &cmp_2) {
      return cmp_1.key == cmp_2.key && cmp_1.value == cmp_2.value;
    }
    element &operator=(const element &obj) {
      key = obj.key;
      value = obj.value;
      return *this;
    }
    element() : key(""), value(T()) {}
    element(const Key &index, const T &number) : key(index), value(number) {}
  };
  struct node {
    int address = 0;
    bool changed = false;
    NodeState state = middle;
    int son_num = 0, son_pos[max_son + 1];
    element index[max_son + 1];
    node(bool did = false) : changed(did) {}
  } current_node;
  struct leaves {
    int address = 0;
    bool changed = false;
    int next_pos = 0, data_num = 0;
    element storage[max_size + 1];
    leaves(bool did = false) : changed(did) {}
  } current_leaf;
  struct begin_tree {
    int start_place = sizeof(begin_tree);
    int end_place = sizeof(begin_tree) + sizeof(node);
  } tree_begin;
  struct begin_data {
    int start_place = sizeof(begin_data);
    int end_place = sizeof(begin_data);
  } data_begin;
  const int node_size = sizeof(node);
  const int leaf_size = sizeof(leaves);
  CachePool<node> node_cache;
  CachePool<leaves> leaf_cache;
 public:
  friend class CachePool<node>;
  friend class CachePool<leaves>;
  node root;
  BPlusTree(const std::string &_tree_name, const std::string &_data_name)
      : tree_name(_tree_name),
        data_name(_data_name),
        tree_bin(_tree_name + "'s garbage"),
        data_bin(_data_name + "'s garbage"),
        node_cache(tree),
        leaf_cache(data) {
    init();
  }
  ~BPlusTree() {
    UpdateTree(), UpdateData();
    tree.seekp(root.address);
    tree.write(reinterpret_cast<char *>(&root), node_size);
  };

  void Traverse() {
    std::cout << "traversing\n";
    ReadLeaf(current_leaf, data_begin.start_place);
    while (true) {
      std::cout << "//";
      for (int i = 1; i <= current_leaf.data_num; ++i) {
        std::cout << current_leaf.storage[i].key << ' ' << current_leaf.storage[i].value << '/';
      }
      WriteLeaves(current_leaf);
      if (current_leaf.next_pos) {
        std::cout << '\n';
        ReadLeaf(current_leaf, current_leaf.next_pos);
      } else {
        std::cout << '\n';
        return;
      }
    }
  }

  sjtu::vector<T> find(const Key &key) {
    element another(key, -1);
    sjtu::vector<T> ret;
    current_node = root;
    while (current_node.state != leaf) {
      if (current_node.son_num == 0) {
        return ret;
      }
      int place = LowerBound(another, current_node.index, 1, current_node.son_num - 1);
      ReadNode(current_node, current_node.son_pos[place]);
      WriteNode(current_node);
    }
    if (current_node.son_num == 0) {
      return ret;
    }
    int search = LowerSearch(another, current_node.index, 1, current_node.son_num - 1);
    ReadLeaf(current_leaf, current_node.son_pos[search]);
    int pos = BinarySearch(another, current_leaf.storage, 1, current_leaf.data_num);
    while (true) {
      for (int i = pos; i <= current_leaf.data_num; ++i) {
        if (current_leaf.storage[i].key == key) {
          ret.push_back(current_leaf.storage[i].value);
        } else {
          WriteLeaves(current_leaf);
          return ret;
        }
      }
      WriteLeaves(current_leaf);
      if (current_leaf.next_pos) { // getting next leaf
        ReadLeaf(current_leaf, current_leaf.next_pos);
        pos = 1;
      } else break;
    }
    return ret;
  }

  void insert(const Key &key, const T &val) {
    element another(key, val);
    if (root.son_num == 0) { // nothing exist, first insert
      leaves first_leaf(false);
      first_leaf.address = data_begin.start_place;
      if (data_begin.start_place == data_begin.end_place) {
        data_begin.end_place += leaf_size;
      }
      first_leaf.data_num = 1, first_leaf.storage[1] = another;
      root.son_num = 1, root.son_pos[1] = first_leaf.address;
      data.seekp(first_leaf.address);
      data.write(reinterpret_cast<char *>(&first_leaf), sizeof(first_leaf));
      WriteLeaves(first_leaf);
      return;
    }
    if (!InternalInsert(root, another)) {// root splitting
      node new_root(false), vice_root(false);
      vice_root.state = root.state, new_root.state = middle;
      root.son_num = vice_root.son_num = min_son;
      for (int i = 1; i <= min_son; ++i) {
        vice_root.son_pos[i] = root.son_pos[i + min_son];
      }
      for (int i = 1; i < min_son; ++i) {
        vice_root.index[i] = root.index[i + min_son];
      }
      if (tree_bin.empty()) {
        vice_root.address = tree_begin.end_place, tree_begin.end_place += node_size;
      } else {
        vice_root.address = tree_bin.pop_back();
      }
      new_root.address = root.address;
      if (tree_bin.empty()) {
        root.address = tree_begin.end_place, tree_begin.end_place += node_size;
      } else {
        root.address = tree_bin.pop_back();
      }
      new_root.son_num = 2;
      new_root.index[1] = root.index[min_son];
      new_root.son_pos[1] = root.address, new_root.son_pos[2] = vice_root.address;
      tree.seekp(vice_root.address);
      tree.write(reinterpret_cast<char *>(&vice_root), node_size);
      tree.seekp(root.address);
      tree.write(reinterpret_cast<char *>(&root), node_size);
      WriteNode(root), WriteNode(vice_root);
      root = new_root;
    }
  }

  void erase(const Key &key, const T &val) {
    element another(key, val);
    bool checker = InternalErase(another, root);
    if (!checker && root.state == middle && root.son_num == 1) {
      // lowering the tree
      node new_root;
      ReadNode(new_root, root.son_pos[1]);
      tree_bin.push_back(new_root.address);
      new_root.address = root.address;
      root = new_root;
    }
  }

 private:
  void init() {
    tree.open(tree_name), data.open(data_name);
    tree.seekg(0, std::ios::beg), data.seekg(0, std::ios::beg);
    if (!tree || !data) {
      tree.open(tree_name, std::ios::out), data.open(data_name, std::ios::out);
      tree.write(reinterpret_cast<char *>(&tree_begin), sizeof(tree_begin));
      data.write(reinterpret_cast<char *>(&data_begin), sizeof(data_begin));
      root.address = tree_begin.start_place, root.son_num = 0, root.state = leaf;
      tree.seekp(root.address);
      tree.write(reinterpret_cast<char *>(&root), node_size);
      tree.close(), data.close();
      tree.open(tree_name), data.open(data_name);
    } else {
      tree.read(reinterpret_cast<char *>(&tree_begin), sizeof(tree_begin));
      data.read(reinterpret_cast<char *>(&data_begin), sizeof(data_begin));
      tree.seekg(tree_begin.start_place);
      tree.read(reinterpret_cast<char *>(&root), sizeof(root));
    }
  }

  bool InternalInsert(node &todo, const element &another) {
    // false means its father ought to be modified
    int pos = LowerBound(another, todo.index, 1, todo.son_num - 1);
    if (todo.state == leaf) {
      leaves todo_leaf;
      ReadLeaf(todo_leaf, todo.son_pos[pos]);
      int search = LowerBound(another, todo_leaf.storage, 1, todo_leaf.data_num);
      for (int i = todo_leaf.data_num + 1; i > search; --i) {
        todo_leaf.storage[i] = todo_leaf.storage[i - 1];
      }
      todo_leaf.storage[search] = another;
      ++todo_leaf.data_num, todo_leaf.changed = true;
      if (todo_leaf.data_num == max_size) {// block splitting
        leaves new_block(false);
        new_block.data_num = min_size, todo_leaf.data_num = min_size;
        for (int i = 1; i <= min_size; ++i) {
          new_block.storage[i] = todo_leaf.storage[i + min_size];
        }
        if (data_bin.empty()) {
          new_block.address = data_begin.end_place, data_begin.end_place += leaf_size;
        } else {
          new_block.address = data_bin.pop_back();
        }
        new_block.next_pos = todo_leaf.next_pos, todo_leaf.next_pos = new_block.address;
        data.seekp(new_block.address);
        data.write(reinterpret_cast<char *>(&new_block), leaf_size);
        WriteLeaves(todo_leaf), WriteLeaves(new_block);
        // updating the node
        ++todo.son_num;
        element new_index = new_block.storage[1];
        int new_pos = new_block.address;
        for (int i = todo.son_num; i > pos + 1; --i) {
          todo.son_pos[i] = todo.son_pos[i - 1];
        }
        for (int i = todo.son_num - 1; i > pos; --i) {
          todo.index[i] = todo.index[i - 1];
        }
        todo.son_pos[pos + 1] = new_pos, todo.index[pos] = new_index;
        todo.changed = true;
        if (todo.son_num == max_son) { // going up
          return false;
        } else {
          WriteNode(todo);
          return true;
        }
      } else {
        WriteNode(todo);
        WriteLeaves(todo_leaf);
        return true;
      }
    } else { // this is the node
      node todo_node;
      ReadNode(todo_node, todo.son_pos[pos]);
      if (InternalInsert(todo_node, another)) {
        WriteNode(todo);
        return true;
      } else { // needing to split
        todo_node.changed = true;
        node new_node(false);
        new_node.son_num = todo_node.son_num = min_son;
        for (int i = 1; i <= min_son; ++i) {
          new_node.son_pos[i] = todo_node.son_pos[i + min_son];
        }
        for (int i = 1; i < min_son; ++i) {
          new_node.index[i] = todo_node.index[i + min_son];
        }
        new_node.state = todo_node.state;
        if (tree_bin.empty()) {
          new_node.address = tree_begin.end_place, tree_begin.end_place += node_size;
        } else {
          new_node.address = tree_bin.pop_back();
        }
        tree.seekp(new_node.address);
        tree.write(reinterpret_cast<char *>(&new_node), node_size);
        WriteNode(todo_node), WriteNode(new_node);
        // updating todo
        element new_index = todo_node.index[min_son];
        int new_pos = new_node.address;
        for (int i = todo.son_num + 1; i > pos + 1; --i) {
          todo.son_pos[i] = todo.son_pos[i - 1];
        }
        for (int i = todo.son_num; i > pos; --i) {
          todo.index[i] = todo.index[i - 1];
        }
        todo.son_pos[pos + 1] = new_pos, todo.index[pos] = new_index;
        ++todo.son_num, todo.changed = true;
        if (todo.son_num == max_son) { // going up
          return false;
        } else {
          WriteNode(todo);
          return true;
        }
      }
    }
  }

  bool InternalErase(const element &another, node &todo) {
    int pos = LowerBound(another, todo.index, 1, todo.son_num - 1);
    if (todo.state == leaf) {
      leaves todo_leaf;
      ReadLeaf(todo_leaf, todo.son_pos[pos]);
      int search = UpperBound(another, todo_leaf.storage, 1, todo_leaf.data_num);
      if (!(another == todo_leaf.storage[search])) {
        // not even deleting
        WriteNode(todo);
        WriteLeaves(todo_leaf);
        return true;
      }
      for (int i = search; i < todo_leaf.data_num; ++i) {
        todo_leaf.storage[i] = todo_leaf.storage[i + 1];
      }
      --todo_leaf.data_num, todo_leaf.changed = true;
      if (todo_leaf.data_num < min_size) {
        // std::cout << "adjusting" << '\n';
        // leaf adjusting
        todo.changed = true;
        leaves before, after;
        if (pos < todo.son_num) { // borrowing behind
          ReadLeaf(after, todo.son_pos[pos + 1]);
          if (after.data_num > min_size) { // can borrow
            todo_leaf.storage[todo_leaf.data_num + 1] = after.storage[1];
            ++todo_leaf.data_num;
            for (int i = 1; i < after.data_num; ++i) {
              after.storage[i] = after.storage[i + 1];
            }
            --after.data_num, after.changed = true;
            todo.index[pos] = after.storage[1];
            WriteNode(todo), WriteLeaves(todo_leaf), WriteLeaves(after);
            return true;
          }
        }
        if (pos > 1) { // borrowing front
          ReadLeaf(before, todo.son_pos[pos - 1]);
          if (before.data_num > min_size) {// can borrow
            if (after.address) {
              WriteLeaves(after);
            }
            for (int i = todo_leaf.data_num + 1; i > 1; --i) {
              todo_leaf.storage[i] = todo_leaf.storage[i - 1];
            }
            ++todo_leaf.data_num, todo_leaf.storage[1] = before.storage[before.data_num];
            --before.data_num, before.changed = true;
            todo.index[pos - 1] = todo_leaf.storage[1];
            WriteNode(todo), WriteLeaves(todo_leaf), WriteLeaves(before);
            return true;
          }
        }
        if (pos < todo.son_num) {
          if (before.next_pos) {
            WriteLeaves(before);
          }
          // merging the one behind
          for (int i = 1; i <= after.data_num; ++i) {
            todo_leaf.storage[todo_leaf.data_num + i] = after.storage[i];
          }
          todo_leaf.data_num += after.data_num, todo_leaf.next_pos = after.next_pos;
          WriteLeaves(todo_leaf), data_bin.push_back(after.address);
          for (int i = pos + 1; i < todo.son_num; ++i) {
            todo.son_pos[i] = todo.son_pos[i + 1];
          }
          for (int i = pos; i < todo.son_num - 1; ++i) {
            todo.index[i] = todo.index[i + 1];
          }
          --todo.son_num;
          if (todo.son_num < min_size) {
            return false;
          } else {
            WriteNode(todo);
            return true;
          }
        }
        if (pos > 1) {
          if (after.next_pos) {
            WriteLeaves(after);
          }
          // merging the one at front
          for (int i = 1; i <= todo_leaf.data_num; ++i) {
            before.storage[before.data_num + i] = todo_leaf.storage[i];
          }
          before.data_num += todo_leaf.data_num, before.next_pos = todo_leaf.next_pos;
          before.changed = true;
          WriteLeaves(before), data_bin.push_back(todo_leaf.address);
          for (int i = pos; i < todo.son_num; ++i) {
            todo.son_pos[i] = todo.son_pos[i + 1];
          }
          for (int i = pos - 1; i < todo.son_num - 1; ++i) {
            todo.index[i] = todo.index[i + 1];
          }
          --todo.son_num;
          if (todo.son_num < min_size) {
            return false;
          } else {
            WriteNode(todo);
            return true;
          }
        }
        // only son, can't do anything
        WriteNode(todo), WriteLeaves(todo_leaf);
        return true;
      } else {
        // need no adjustment
        WriteNode(todo), WriteLeaves(todo_leaf);
        return true;
      }
    } else {
      node todo_node;
      ReadNode(todo_node, todo.son_pos[pos]);
      if (InternalErase(another, todo_node)) {
        WriteNode(todo);
        return true;
      } else {
        todo_node.changed = true, todo.changed = true;
        node before, after;
        // node adjusting
        if (pos < todo.son_num) { // borrowing behind
          ReadNode(after, todo.son_pos[pos + 1]);
          if (after.son_num > min_size) { // can borrow
            todo_node.son_pos[todo_node.son_num + 1] = after.son_pos[1];
            todo_node.index[todo_node.son_num] = todo.index[pos], todo.index[pos] = after.index[1];
            ++todo_node.son_num;
            for (int i = 1; i < after.son_num; ++i) {
              after.son_pos[i] = after.son_pos[i + 1];
            }
            for (int i = 1; i < after.son_num - 1; ++i) {
              after.index[i] = after.index[i + 1];
            }
            --after.son_num, after.changed = true;
            WriteNode(todo), WriteNode(todo_node), WriteNode(after);
            return true;
          }
        }
        if (pos > 1) { // borrowing front
          ReadNode(before, todo.son_pos[pos - 1]);
          if (before.son_num > min_size) { // can borrow
            if (after.address) {
              WriteNode(after);
            }
            for (int i = todo_node.son_num + 1; i > 1; --i) {
              todo_node.son_pos[i] = todo_node.son_pos[i - 1];
            }
            for (int i = todo_node.son_num; i > 1; --i) {
              todo_node.index[i] = todo_node.index[i - 1];
            }
            todo_node.son_pos[1] = before.son_pos[before.son_num];
            todo_node.index[1] = todo.index[pos - 1];
            todo.index[pos - 1] = before.index[before.son_num - 1];
            ++todo_node.son_num;
            --before.son_num, before.changed = true;
            WriteNode(todo), WriteNode(todo_node), WriteNode(before);
            return true;
          }
        }
        if (pos < todo.son_num) {
          if (before.address) {
            WriteNode(before);
          }
          // merging the one behind
          for (int i = 1; i <= after.son_num; ++i) {
            todo_node.son_pos[todo_node.son_num + i] = after.son_pos[i];
          }
          for (int i = 1; i < after.son_num; ++i) {
            todo_node.index[todo_node.son_num + i] = after.index[i];
          }
          todo_node.index[todo_node.son_num] = todo.index[pos];
          todo_node.son_num += after.son_num;
          WriteNode(todo_node), tree_bin.push_back(after.address);
          for (int i = pos + 1; i < todo.son_num; ++i) {
            todo.son_pos[i] = todo.son_pos[i + 1];
          }
          for (int i = pos; i < todo.son_num - 1; ++i) {
            todo.index[i] = todo.index[i + 1];
          }
          --todo.son_num;
          if (todo.son_num < min_size) {
            return false;
          } else {
            WriteNode(todo);
            return true;
          }
        }
        if (pos > 1) {
          if (after.address) {
            WriteNode(after);
          }
          // merging the one at front
          for (int i = 1; i <= todo_node.son_num; ++i) {
            before.son_pos[before.son_num + i] = todo_node.son_pos[i];
          }
          for (int i = 1; i < todo_node.son_num; ++i) {
            before.index[before.son_num + i] = todo_node.index[i];
          }
          before.index[before.son_num] = todo.index[pos - 1];
          before.son_num += todo_node.son_num;
          WriteNode(before), tree_bin.push_back(todo_node.address);
          for (int i = pos; i < todo.son_num; ++i) {
            todo.son_pos[i] = todo.son_pos[i + 1];
          }
          for (int i = pos - 1; i < todo.son_num - 1; ++i) {
            todo.index[i] = todo.index[i + 1];
          }
          --todo.son_num;
          if (todo.son_num < min_size) {
            return false;
          } else {
            WriteNode(todo);
            return true;
          }
        }
      }
    }
    WriteNode(todo);
    return true;
  }
  void ReadNode(node &obj, int place) {
    // std::cout << "node place: " << place << '\n';
    if (!node_cache.GetNode(obj, place)) {
      tree.seekg(place);
      tree.read(reinterpret_cast<char *>(&obj), sizeof(obj));
      // std::cout << "node_cache miss!\n";
    } else {
      // std::cout << "node_cache shot!\n";
    }
  }
  void ReadLeaf(leaves &obj, int place) {
    // std::cout << "leaf place: " << place << '\n';
    if (!leaf_cache.GetNode(obj, place)) {
      data.seekg(place);
      data.read(reinterpret_cast<char *>(&obj), sizeof(obj));
      // std::cout << "leaf_cache miss!\n";
    } else {
      // std::cout << "leaf_cache shot!\n";
    }
  }
  void WriteNode(node &obj) {
    if (obj.address == 8) {// do not write root!
      return;
    }
    node_cache.InsertFront(obj);
  }
  void WriteLeaves(leaves &obj) {
    leaf_cache.InsertFront(obj);
  }
  void UpdateData() {
    data.seekp(0);
    data.write(reinterpret_cast<char *>(&data_begin), sizeof(data_begin));
  }
  void UpdateTree() {
    data.seekp(0);
    data.write(reinterpret_cast<char *>(&tree_begin), sizeof(tree_begin));
  }
};
#endif //BPT__BPT_HPP_
#include <iostream>
// #include "bpt.hpp"
#include <string>

using namespace std;

class my_string {
 private:
  char info[65];
 public:
  my_string &operator=(const my_string &other) {
    strcpy(info, other.info);
    return *this;
  }
  friend ostream &operator<<(ostream &out, const my_string &obj) {
    out << obj.info;
    return out;
  }
  my_string(const string &obj = "") {
    strcpy(info, obj.c_str());
  }
  friend bool operator<(const my_string &obj_1, const my_string &obj_2) {
    return strcmp(obj_1.info, obj_2.info) < 0;
  }
  friend bool operator==(const my_string &obj_1, const my_string &obj_2) {
    return strcmp(obj_1.info, obj_2.info) == 0;
  }
};

int main() {
//  freopen("input.txt", "r", stdin);
//  freopen("bptout.txt", "w", stdout);
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  string operation;
  string name;
  int year;
  BPlusTree<my_string, int> pool("TreeMemory", "DataMemory");
  for (int i = 0; i < n; ++i) {
    cin >> operation >> name;
    // std::cout << operation << ' ' << name << ' ';
    if (operation == "insert") {
      cin >> year;
      // std::cout << year << '\n';
      pool.insert(name, year);
    }
    if (operation == "find") {
      // std::cout << '\n';
      auto ret = pool.find(name);
      if (!ret.empty()) {
        for (int j = 0; j < ret.size(); ++j) {
          cout << ret[j] << ' ';
        }
        cout << '\n';
      }
      else {
        cout << "null\n";
      }
    }
    if (operation == "delete") {
      cin >> year;
      // std::cout << year << '\n';
      pool.erase(name, year);
//      if (year < 100) {
//        pool.Traverse();
//      }
    }
  }
  return 0;
}