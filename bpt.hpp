#ifndef BPT__BPT_HPP_
#define BPT__BPT_HPP_
#include <string>
#include <cstring>
#include <fstream>

const int max_size = 100, min_size = 50;
const int max_son = 100, min_son = 50;

template<class T>
int UpperBound(T val, T *array, int l, int r) {
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

template <class T>
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
  using value_type = std::pair<const Key, T>;
  enum NodeState { leaf, middle };
 private:
  std::fstream tree, data;
  std::string tree_name, data_name;
  struct node {
    NodeState state = middle;
    int son_num, son_pos[max_son];
    Key index[max_son];
  } current_node;
  struct leaves {
    int next_pos, data_pos[max_size], data_num;
    Key key[max_size];
    T value[max_size];
  } current_leaf;
  struct begin_tree {
    int now_size = 0;
    int start_place = sizeof(begin_tree);
    int end_place = sizeof(begin_tree);
    node root;
  } tree_begin;
  struct begin_data {
    int now_size = 0;
    int start_place = sizeof(begin_data);
    int end_place = sizeof(begin_data);
    leaves head;
  } data_begin;
 public:
  BPlusTree(const std::string &_tree_name, const std::string &_data_name)
      : tree_name(_tree_name), data_name(_data_name) {
    init();
  }
  ~BPlusTree() = default;

  std::pair<T, bool> find(const Key &key) {
    tree.open(tree_name), tree.seekg(0);
    tree.read(reinterpret_cast<char *>(&tree_begin), sizeof(tree_begin));
    current_node = tree_begin.root;
    while (current_node.state != leaf) {
      int place = UpperBound(key, current_node.index, 1, current_node.son_num);
      tree.seekg(current_node.son_pos[place]);
      tree.read(reinterpret_cast<char *>(&current_node), sizeof(current_node));
    }
    int search = UpperBound(key, current_node.index, 1, current_node.son_num);
    tree.seekg(current_node.son_pos[search]);
    data.open(data_name), data.seekg(0);
    data.read(reinterpret_cast<char *>(&current_leaf), sizeof(current_leaf));
    int pos = BinarySearch(key, current_leaf.key, 1, current_leaf.data_num);
    if (pos == -1) return std::make_pair(T(), false);
    else return std::make_pair(current_leaf.value[pos], true);
  }

  bool insert(const value_type &val) {
    //todo
  }

  void erase(const Key &key) const {
    //todo
  }
 private:
  void init() {
    tree.open(tree_name), data.open(data_name);
    tree.seekg(0, std::ios::beg), data.seekg(0, std::ios::beg);
    if (!tree || !data) {
      tree.close(), data.close();
      tree.open(tree_name, std::ios::out), data.open(data_name, std::ios::out);
      tree.write(reinterpret_cast<char *>(&tree_begin), sizeof(tree_begin));
      data.write(reinterpret_cast<char *>(&data_begin), sizeof(data_begin));
    } else {
      tree.close(), data.close();
    }
  }
};
#endif //BPT__BPT_HPP_
