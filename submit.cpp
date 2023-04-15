#ifndef BPT__EXCEPTIONS_HPP_
#define BPT__EXCEPTIONS_HPP_

#include <cstddef>
#include <cstring>
#include <string>

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

#ifndef BPT__VECTOR_HPP_
#define BPT__VECTOR_HPP_
#include "exceptions.hpp"

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

#endif //BPT__VECTOR_HPP_
#ifndef BPT__BPT_HPP_
#define BPT__BPT_HPP_
#include <string>
#include <cstring>
#include <fstream>
#include "vector.hpp"

const int max_size = 200, min_size = 100;
const int max_son = 200, min_son = 100;

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
int UpperBound(T val, T *array, int l, int r) {
  int ans = 0;
  while (l <= r) {
    int mid = (l + r) >> 1;
    // std::cout << mid << ' ' << array[mid].key << '\n';
    if (!(val < array[mid])) {
      // puts("correct");
      l = mid + 1, ans = mid;
    } else {
      // puts("fail");
      r = mid - 1;
    }
  }
  return ans;
}
template<class T>
int BinarySearch(T val, T *array, int l, int r, bool (*op)(const T &, const T &)) {
  int ans = r + 1;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (op(val, array[mid])) {
      r = mid - 1, ans = mid;
    } else {
      l = mid + 1;
    }
  }
  return ans;
}

template<class Key, class T>
class BPlusTree {
  enum NodeState { leaf, middle };
 private:
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
    element() : key(""), value(T()) {}
    element(const Key &index, const T &number) : key(index), value(number) {}
    inline void initial(const Key &index, const T &number) {
      key = index;
      value = number;
    }
  };
  static bool same(const element &cmp_1, const element &cmp_2) {
    return cmp_1.key == cmp_2.key;
  }
  struct node {
    int address = 0;
    NodeState state = middle;
    int son_num = 0, son_pos[max_son + 1];
    element index[max_son + 1];
  } current_node;
  struct leaves {
    int address = 0;
    int next_pos = 0, data_num = 0;
    element storage[max_size + 1];
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
  const int leave_size = sizeof(leaves);
 public:
  node root;
  BPlusTree(const std::string &_tree_name, const std::string &_data_name)
      : tree_name(_tree_name), data_name(_data_name) {
    init();
  }
  ~BPlusTree() {
    tree.close(), data.close();
  }

  void Traverse(int pos) {
    node cur;
    tree.read(reinterpret_cast<char*>(&cur), sizeof(node));
    // puts("this is a node");
    for (int i = 1; i < cur.son_num; ++i) {
      // std::cout << cur.index[i].key << ' ' << cur.index[i].value << '/';
    }
    // puts("");
    if (cur.state == leaf) return;
    for (int i = 1; i <= cur.son_num; ++i) {
      Traverse(cur.son_pos[i]);
    }
  }

  sjtu::vector<T> find(const Key &key) {
    element another(key, T());
    sjtu::vector<T> ret;
    for (int i = 1; i < root.son_num; ++i) {
      // std::cout << root.index[i].key << ' ';
    }
    // puts("");
    current_node = root;
    while (current_node.state != leaf) {
      int place = LowerBound(another, current_node.index, 1, current_node.son_num - 1);
      // std::cout << place << '\n';
      tree.seekg(current_node.son_pos[place]);
      tree.read(reinterpret_cast<char *>(&current_node), sizeof(current_node));
    }
    int search = UpperBound(another, current_node.index, 1, current_node.son_num - 1) + 1;
    // std::cout << search << '\n';
    if (search == 0) {
      return ret;
    }
    data.seekg(current_node.son_pos[search]);
    data.read(reinterpret_cast<char *>(&current_leaf), sizeof(current_leaf));
    int pos = BinarySearch(another, current_leaf.storage, 1, current_leaf.data_num, same);
    // std::cout << pos << '\n';
    while (true) {
      // std::cout << "data number: " << current_leaf.data_num << '\n';
      for (int i = pos; i <= current_leaf.data_num; ++i) {
        // std::cout << "checking\n";
        // std::cout << current_leaf.storage[i].key << '\n';
        if (current_leaf.storage[i].key == key) {
          ret.push_back(current_leaf.storage[i].value);
        } else {
          return ret;
        }
      }
      if (current_leaf.next_pos) {
        data.seekg(current_leaf.next_pos);
        data.read(reinterpret_cast<char *>(&current_leaf), sizeof(current_leaf));
        pos = 1;
      } else break;
    }
  }

  void insert(const Key &key, const T &val) {
    element another(key, val);
    if (root.son_num == 0) { // nothing exist, first insert
      leaves first_leaf;
      first_leaf.address = data_begin.start_place, data_begin.end_place += leave_size;
      first_leaf.data_num = 1, first_leaf.storage[1] = another;
      root.son_num = 1, root.son_pos[1] = first_leaf.address;
      // std::cout << root.index[1].key << '\n';
      WriteNode(root), WriteLeaves(first_leaf);
      return;
    }
    if (!InternalInsert(root, another)) {// root splitting
      // puts("split root!");
      node new_root, vice_root;
      vice_root.state = root.state, new_root.state = middle;
      root.son_num = vice_root.son_num = max_son >> 1;
      for (int i = 1; i <= max_son >> 1; ++i) {
        vice_root.index[i] = root.index[i + (max_son >> 1)];
        vice_root.son_pos[i] = vice_root.son_pos[i + (max_son >> 1)];
      }
      vice_root.address = tree_begin.end_place, tree_begin.end_place += node_size;
      new_root.address = root.address;
      root.address = tree_begin.end_place, tree_begin.end_place += node_size;
      new_root.son_num = 2;
      new_root.index[1] = vice_root.index[1];
      new_root.son_pos[1] = root.address, new_root.son_pos[2] = vice_root.address;
      WriteNode(root), WriteNode(new_root), WriteNode(vice_root);
      root = new_root;
    }
  }

  void erase(const Key &key, const T &val) {
    //todo
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
      WriteNode(root);
      tree.close(), data.close();
      tree.open(tree_name), data.open(data_name);
    } else {
      tree.read(reinterpret_cast<char *>(&tree_begin), sizeof(tree_begin));
      data.read(reinterpret_cast<char *>(&data_begin), sizeof(data_begin));
      tree.seekg(tree_begin.start_place);
      tree.read(reinterpret_cast<char *>(&root), sizeof(root));
    }
  }
  /*
   * -------------------------function: InternalInsert------------------------------
   * Inserting recursively to save information, dealing with the current node's son
   * and maintain the node. the root is special, so it is handled in public function
   * find.
   */
  bool InternalInsert(node &todo, const element &another) {
    // puts("inserting");
    // false means its father ought to be modified
    int pos = LowerBound(another, todo.index, 1, todo.son_num - 1);
    // std::cout << "inserting in: " << pos << '\n';
    if (todo.state == leaf) {
      // std::cout << "finding data\n";
      leaves todo_leave;
      data.seekg(todo.son_pos[pos]);
      data.read(reinterpret_cast<char *>(&todo_leave), sizeof(todo_leave));
      int search = LowerBound(another, todo_leave.storage, 1, todo_leave.data_num);
      // std::cout << "inserting data in: " << pos << '\n';
      for (int i = todo_leave.data_num + 1; i > search; --i) {
        todo_leave.storage[i] = todo_leave.storage[i - 1];
      }
      todo_leave.storage[search] = another;
      ++todo_leave.data_num;
      for (int i = 1; i <= todo_leave.data_num; ++i) {
        // std::cout << todo_leave.storage[i].key << ' ' << todo_leave.storage[i].value << '\n';
      }
      if (todo_leave.data_num == max_size) {// block splitting
        // std::cout << "splitting leaves\n";
        // updating the leaves
        leaves new_block;
        new_block.data_num = max_size >> 1, todo_leave.data_num = max_size >> 1;
        for (int i = 1; i <= max_size >> 1; ++i) {
          new_block.storage[i] = todo_leave.storage[i + (max_size >> 1)];
        }
        new_block.address = data_begin.end_place, data_begin.end_place += leave_size;
        new_block.next_pos = todo_leave.next_pos, todo_leave.next_pos = new_block.address;
        // std::cout << new_block.data_num << ' ' << todo_leave.data_num << '\n';
        WriteLeaves(todo_leave), WriteLeaves(new_block), UpdateData();
        // updating the node
        ++todo.son_num;
        element new_index = new_block.storage[1];
        int new_pos = new_block.address;
        for (int i = todo.son_num + 1; i > pos + 1; --i) {
          todo.son_pos[i] = todo.son_pos[i - 1];
        }
        for (int i = todo.son_num; i > pos; --i) {
          todo.index[i] = todo.index[i - 1];
        }
        todo.son_pos[pos + 1] = new_pos, todo.index[pos] = new_index;
        if (todo.son_num == max_son) { // going up
          return false;
        } else {
          WriteNode(todo);
          return true;
        }
      } else {
        WriteLeaves(todo_leave);
        return true;
      }
    } else { // this is the node
      node todo_node;
      tree.seekg(todo.son_pos[pos]);
      tree.read(reinterpret_cast<char *>(&todo_node), sizeof(todo_node));
      if (InternalInsert(todo_node, another)) {
        return true;
      } else { // needing to split
        // puts("splitting node!\n");
        node new_node;
        new_node.son_num = todo_node.son_num = max_son >> 1;
        for (int i = 1; i <= max_son >> 1; ++i) {
          new_node.index[i] = todo_node.index[i + (max_son >> 1)];
          new_node.son_pos[i] = todo_node.son_pos[i + (max_son >> 1)];
        }
        new_node.state = todo_node.state;
        new_node.address = tree_begin.end_place, tree_begin.end_place += node_size;
        WriteNode(todo_node), WriteNode(new_node), UpdateTree();
        // updating todo
        ++todo.son_num;
        element new_index = todo_node.index[1];
        int new_pos = todo_node.address;
        for (int i = todo.son_num + 1; i > pos + 1; --i) {
          todo.son_pos[i] = todo.son_pos[i - 1];
        }
        for (int i = todo.son_num; i > pos; --i) {
          todo.index[i] = todo.index[i - 1];
        }
        todo.son_pos[pos + 1] = new_pos, todo.index[pos] = new_index;
        if (todo.son_num == max_son) { // going up
          return false;
        } else {
          WriteNode(todo);
          return true;
        }
      }
    }
  }

  bool MergeBlock(node &todo) {

  }
  void UpdateIndex(node &todo, int position) {

  }
  void WriteNode(node &obj) {
    tree.seekp(obj.address);
    tree.write(reinterpret_cast<char *>(&obj), sizeof(obj));
  }
  void WriteLeaves(leaves &obj) {
    data.seekp(obj.address);
    data.write(reinterpret_cast<char *>(&obj), sizeof(obj));
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
#include "bpt.hpp"
#include <string>

using namespace std;

class my_string {
 private:
  char info[65];
 public:
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
  // freopen("test.txt", "r", stdin);
  //freopen("out2.txt", "w", stdout);
  // ios::sync_with_stdio(false);
  // cin.tie(nullptr);
  // cout.tie(nullptr);
  int n;
  cin >> n;
  string operation;
  string name;
  int year;
  BPlusTree<my_string, int> pool("TreeMemory", "DataMemory");
  for (int i = 0; i < n; ++i) {
    cin >> operation >> name;
    if (operation == "insert") {
      cin >> year;
      pool.insert(name, year);
    }
    if (operation == "find") {
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
      pool.erase(name, year);
    }
    if (operation == "traverse") {
      pool.Traverse(8);
    }
  }
  return 0;
}