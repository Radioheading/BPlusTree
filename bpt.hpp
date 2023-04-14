#ifndef BPT__BPT_HPP_
#define BPT__BPT_HPP_
#include <string>
#include <cstring>
#include <fstream>
#include "vector.hpp"

const int max_size = 100, min_size = 50;
const int max_son = 100, min_son = 50;

template<class Key, class T>
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

template<class T>
int LowerBound(T val, T *array, int l, int r) {
  int ans = r + 1;
  int mid = (l + r) >> 1;
  while (l <= r) {
    if (val < array[mid]) {
      r = mid - 1, ans = mid;
    } else {
      l = mid + 1;
    }
  }
  return ans;
}

template<class T>
int BinarySearch(T val, T *array, int l, int r) {
  int ans = -1;
  int mid = (l + r) >> 1;
  while (l <= r) {
    if (val == array[mid]) {
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
  struct node {
    int address = 0;
    NodeState state = middle;
    int son_num = 0, son_pos[max_son + 1];
    element<Key, T> index[max_son + 1];
  } current_node, root;
  struct leaves {
    int address;
    int next_pos, data_num;
    element<Key, T> storage[max_size + 1];
  } current_leaf;
  struct begin_tree {
    int start_place = sizeof(begin_tree);
    int end_place = sizeof(begin_tree);
  } tree_begin;
  struct begin_data {
    int start_place = sizeof(begin_data);
    int end_place = sizeof(begin_data);
  } data_begin;
  const int node_size = sizeof(node);
  const int leave_size = sizeof(leaves);
 public:
  BPlusTree(const std::string &_tree_name, const std::string &_data_name)
      : tree_name(_tree_name), data_name(_data_name) {
    init();
  }
  ~BPlusTree() {
    tree.close(), data.close();
  }

  sjtu::vector<T> find(const Key &key) {
    element<Key, T> another(key, T());
    sjtu::vector<T> ret;
    tree.open(tree_name), tree.seekg(0);
    data.open(data_name), data.seekg(0);
    tree.read(reinterpret_cast<char *>(&tree_begin), sizeof(tree_begin));
    current_node = root;
    while (current_node.state != leaf) {
      int place = LowerBound(another, current_node.index, 1, current_node.son_num - 1);
      tree.seekg(current_node.son_pos[place]);
      tree.read(reinterpret_cast<char *>(&current_node), sizeof(current_node));
    }
    int search = LowerBound(another, current_node.index, 1, current_node.son_num - 1);
    data.seekg(current_node.son_pos[search]);
    data.read(reinterpret_cast<char *>(&current_leaf), sizeof(current_leaf));
    int pos = BinarySearch(another, current_leaf.storage, 1, current_leaf.data_num);
    while (current_leaf.next_pos) {
      for (int i = pos; i <= current_leaf.data_num; ++i) {
        if (current_leaf.storage[pos].key == key) {
          ret.push_back(current_leaf.storage[pos].value);
        } else {
          return ret;
        }
      }
      data.seekg(current_leaf.next_pos);
      data.read(reinterpret_cast<char *>(&current_leaf), sizeof(current_leaf));
      pos = 0;
    }
    return ret;
  }

  void insert(const Key &key, const T &val) {
    if (!InternalInsert(root, element<Key, T>(key, val))) { // root splitting
      node new_root, vice_root;
      vice_root.state = root.state;
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
      tree.close(), data.close();
      tree.open(tree_name), data.open(data_name);
    } else {
      tree.read(reinterpret_cast<char*>(&tree_begin), sizeof(tree_begin));
      data.read(reinterpret_cast<char*>(&data_begin), sizeof(data_begin));
      tree.seekg(tree_begin.start_place);
      tree.read(reinterpret_cast<char*>(&root), sizeof(root));
    }
  }
  /*
   * -------------------------function: InternalInsert------------------------------
   * Inserting recursively to save information, dealing with the current node's son
   * and maintain the node. the root is special, so it is handled in public function
   * find.
   */
  bool InternalInsert(node &todo, const element<Key, T> &another) {
    // false means its father ought to be modified
    int pos = LowerBound(another, todo.index, 1, todo.son_num - 1);
    if (todo.state == leaf) {
      leaves todo_leave;
      data.seekg(todo.son_pos[pos]);
      data.read(reinterpret_cast<char *>(&todo_leave), sizeof(todo_leave));
      int search = LowerBound(another, todo_leave.storage, 1, todo_leave.data_num);
      for (int i = todo_leave.data_num + 1; i > search; --i) {
        todo_leave.storage[i] = todo_leave.storage[i - 1];
      }
      todo_leave.storage[search] = another;
      ++todo_leave.data_num;
      if (todo_leave.data_num == max_size) { // block splitting
        // updating the leaves
        leaves new_block;
        new_block.data_num = max_size >> 1, todo_leave.data_num -= new_block.data_num;
        for (int i = 1; i <= max_size >> 1; ++i) {
          new_block.storage[i] = todo_leave.storage[i + (max_size >> 1)];
        }
        new_block.address = data_begin.end_place + leave_size;
        data_begin.end_place = new_block.address;
        todo_leave.next_pos = new_block.address;
        WriteLeaves(todo_leave), WriteLeaves(new_block), UpdateData();
        // updating the node
        ++todo.son_num;
        element<Key, T> new_index = new_block.storage[1];
        int new_pos = new_block.address;
        for (int i = todo.son_num + 1; i > pos; --i) {
          todo.son_pos[i] = todo.son_pos[i - 1];
          todo.index[i] = todo.index[i - 1];
        }
        todo.son_pos[pos] = new_pos, todo.index[pos] = new_index;
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
        element<Key, T> new_index = new_node.index[1];
        int new_pos = new_node.address;
        for (int i = todo.son_num + 1; i > pos; --i) {
          todo.son_pos[i] = todo.son_pos[i - 1];
          todo.index[i] = todo.index[i - 1];
        }
        todo.son_pos[pos] = new_pos, todo.index[pos] = new_index;
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
