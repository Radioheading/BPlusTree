#ifndef BPT__BPT_HPP_
#define BPT__BPT_HPP_
#include <string>
#include <cstring>
#include <fstream>
#include "vector.hpp"
#include <iostream>

const int max_size = 304, min_size = 152;
const int max_son = 304, min_son = 152;

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
int BinarySearch(T val, T *array, int l, int r) {
  // std::cout << "binary searching: " << l << ' ' << r << '\n';
  int ans = r + 1;
  while (l <= r) {
    int mid = (l + r) >> 1;
    // std::cout << mid << ' ' << val.key << ' ' << array[mid].key << '\n';
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

  sjtu::vector<T> find(const Key &key) {
    element another(key, T());
    sjtu::vector<T> ret;
    current_node = root;
    while (current_node.state != leaf) {
      int place = LowerBound(another, current_node.index, 1, current_node.son_num - 1);
      tree.seekg(current_node.son_pos[place]);
      tree.read(reinterpret_cast<char *>(&current_node), sizeof(current_node));
    }
    int search = UpperBound(another, current_node.index, 1, current_node.son_num - 1) + 1;
    if (search == 0) {
      return ret;
    }
    data.seekg(current_node.son_pos[search]);
    data.read(reinterpret_cast<char *>(&current_leaf), sizeof(current_leaf));
    int pos = BinarySearch(another, current_leaf.storage, 1, current_leaf.data_num);
    while (true) {
      for (int i = pos; i <= current_leaf.data_num; ++i) {
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
    return ret;
  }

  void insert(const Key &key, const T &val) {
    element another(key, val);
    if (root.son_num == 0) { // nothing exist, first insert
      leaves first_leaf;
      first_leaf.address = data_begin.start_place, data_begin.end_place += leave_size;
      first_leaf.data_num = 1, first_leaf.storage[1] = another;
      root.son_num = 1, root.son_pos[1] = first_leaf.address;\
      WriteNode(root), WriteLeaves(first_leaf), UpdateData();
      return;
    }
    bool checker = InternalInsert(root, another);
    if (!checker) {// root splitting
      node new_root, vice_root;
      vice_root.state = root.state, new_root.state = middle;
      root.son_num = vice_root.son_num = min_son;
      for (int i = 1; i <= min_son; ++i) {
        vice_root.son_pos[i] = root.son_pos[i + min_son];
      }
      for (int i = 1; i < min_son; ++i) {
        vice_root.index[i] = root.index[i + min_son];
      }
      vice_root.address = tree_begin.end_place, tree_begin.end_place += node_size;
      new_root.address = root.address;
      root.address = tree_begin.end_place, tree_begin.end_place += node_size;
      new_root.son_num = 2;
      new_root.index[1] = root.index[min_son];
      // std::cout << root.index[min_son].key << ", the middle index\n";
      new_root.son_pos[1] = root.address, new_root.son_pos[2] = vice_root.address;
      WriteNode(root), WriteNode(new_root), WriteNode(vice_root), UpdateTree();
      root = new_root;
    }
  }

  void erase(const Key &key, const T &val) {
    element another(key, val);
    bool checker = InternalErase(another, root);
    if (!checker && root.state == middle && root.son_num == 1) {
      node new_root;
      tree.seekg(root.son_pos[1]);
      tree.read(reinterpret_cast<char*>(&new_root), node_size);
      new_root.address = root.address;
      root = new_root;
      WriteNode(root);
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
      if (todo_leave.data_num == max_size) {// block splitting
        leaves new_block;
        new_block.data_num = min_size, todo_leave.data_num = min_size;
        for (int i = 1; i <= min_size; ++i) {
          new_block.storage[i] = todo_leave.storage[i + min_size];
        }
        new_block.address = data_begin.end_place, data_begin.end_place += leave_size;
        new_block.next_pos = todo_leave.next_pos, todo_leave.next_pos = new_block.address;
        WriteLeaves(todo_leave), WriteLeaves(new_block), UpdateData();
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
        new_node.son_num = todo_node.son_num = min_son;
        for (int i = 1; i <= min_son; ++i) {
          new_node.son_pos[i] = todo_node.son_pos[i + min_son];
        }
        for (int i = 1; i < min_son; ++i) {
          new_node.index[i] = todo_node.index[i + min_son];
        }
        new_node.state = todo_node.state;
        new_node.address = tree_begin.end_place, tree_begin.end_place += node_size;
        WriteNode(todo_node), WriteNode(new_node), UpdateTree();
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
        ++todo.son_num;
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
      leaves todo_leave;
      data.seekg(todo.son_pos[pos]);
      data.read(reinterpret_cast<char*>(&todo_leave), leave_size);
      int search = UpperBound(another, todo_leave.storage, 1, todo_leave.data_num);
      if (!(another == todo_leave.storage[search])) {
        // not even deleting
        return true;
      }
      for (int i = search; i < todo_leave.data_num; ++i) {
        todo_leave.storage[i] = todo_leave.storage[i + 1];
      }
      --todo_leave.data_num;
      if (todo_leave.data_num < min_size) {
        // leaf adjusting
        leaves before, after;
        if (pos < todo.son_num) { // borrowing behind
          data.seekg(todo.son_pos[pos + 1]);
          data.read(reinterpret_cast<char*>(&after), leave_size);
          if (after.data_num > min_size) { // can borrow
            todo_leave.storage[todo_leave.data_num + 1] = after.storage[1];
            ++todo_leave.data_num;
            for (int i = 1; i < after.data_num; ++i) {
              after.storage[i] = after.storage[i + 1];
            }
            --after.data_num;
            todo.index[pos] = after.storage[1];
            WriteNode(todo), WriteLeaves(todo_leave), WriteLeaves(after);
            return true;
          }
        }
        if (pos > 1) { // borrowing front
          data.seekg(todo.son_pos[pos - 1]);
          data.read(reinterpret_cast<char*>(&before), leave_size);
          if (before.data_num > min_size) { // can borrow
            for (int i = todo_leave.data_num + 1; i > 1; --i) {
              todo_leave.storage[i] = todo_leave.storage[i - 1];
            }
            ++todo_leave.data_num, todo_leave.storage[1] = before.storage[before.data_num];
            --before.data_num;
            todo.index[pos - 1] = todo_leave.storage[1];
            WriteNode(todo), WriteLeaves(todo_leave), WriteLeaves(before);
            return true;
          }
        }
        if (pos < todo.son_num) {
          // merging the one behind
          for (int i = 1; i <= after.data_num; ++i) {
            todo_leave.storage[todo_leave.data_num + i] = after.storage[i];
          }
          todo_leave.data_num += after.data_num, todo_leave.next_pos = after.next_pos;
          WriteLeaves(todo_leave);
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
          // merging the one at front
          for (int i = 1; i <= todo_leave.data_num; ++i) {
            before.storage[before.data_num + i] = todo_leave.storage[i];
          }
          before.data_num += todo_leave.data_num, before.next_pos = todo_leave.next_pos;
          WriteLeaves(before);
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
        WriteLeaves(todo_leave);
        return true;
      } else {
        // need no adjustment
        WriteLeaves(todo_leave);
        return true;
      }
    } else {
      node todo_node;
      tree.seekg(todo.son_pos[pos]);
      tree.read(reinterpret_cast<char*>(&todo_node), node_size);
      if (InternalErase(another, todo_node)) {
        return true;
      } else {
        node before, after;
        // node adjusting
        if (pos < todo.son_num) { // borrowing behind
          tree.seekg(todo.son_pos[pos + 1]);
          tree.read(reinterpret_cast<char*>(&after), node_size);
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
            --after.son_num;
            WriteNode(todo), WriteNode(todo_node), WriteNode(after);
            return true;
          }
        }
        if (pos > 1) { // borrowing front
          tree.seekg(todo.son_pos[pos - 1]);
          tree.read(reinterpret_cast<char*>(&before), node_size);
          if (before.son_num > min_size) { // can borrow
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
            --before.son_num;
            WriteNode(todo), WriteNode(todo_node), WriteNode(before);
            return true;
          }
        }
        if (pos < todo.son_num) {
          // merging the one behind
          for (int i = 1; i <= after.son_num; ++i) {
            todo_node.son_pos[todo_node.son_num + i] = after.son_pos[i];
          }
          for (int i = 1; i < after.son_num; ++i) {
            todo_node.index[todo_node.son_num + i] = after.index[i];
          }
          todo_node.index[todo_node.son_num] = todo.index[pos];
          todo_node.son_num += after.son_num;
          WriteNode(todo_node);
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
          // merging the one at front
          for (int i = 1; i <= todo_node.son_num; ++i) {
            before.son_pos[before.son_num + i] = todo_node.son_pos[i];
          }
          for (int i = 1; i < todo_node.son_num; ++i) {
            before.index[before.son_num + i] = todo_node.index[i];
          }
          before.index[before.son_num] = todo.index[pos - 1];
          before.son_num += todo_node.son_num;
          WriteNode(before);
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
    return true;
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