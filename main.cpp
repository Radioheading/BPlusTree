#include <iostream>
#include "bpt.hpp"
#include <string>

using namespace std;

class my_string {
 private:
  char info[65];
 public:
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
  //freopen("test(1).txt", "r", stdin);
  //freopen("out2.txt", "w", stdout);
  ios::sync_with_stdio(false);
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
  }
  return 0;
}