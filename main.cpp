#include <iostream>
#include "bpt.hpp"

using namespace std;

int main() {
  //freopen("test(1).txt", "r", stdin);
  //freopen("out2.txt", "w", stdout);
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  string operation;
  string name;
  int year;
  BPlusTree<string, int> pool("TreeMemory", "DataMemory");
  for (int i = 0; i < n; ++i) {
    cin >> operation >> name;
    if (operation == "insert") {
      cin >> year;
      pool.insert(make_pair(name, year));
    }
    if (operation == "find") {
      auto ret = pool.find(name);
      if (ret.second) cout << ret.first << '\n';
      else cout << "null\n";
    }
    if (operation == "delete") {
      cin >> year;
      pool.erase(name, year);
    }
  }
  return 0;
}