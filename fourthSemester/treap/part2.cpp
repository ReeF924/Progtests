#ifndef __PROGTEST__
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <type_traits>
#include <utility>

// We use std::vector as a reference to check our implementation.
// It is not available in progtest :)
#include <vector>

template <typename T> struct Ref {
  bool empty() const { return _data.empty(); }
  size_t size() const { return _data.size(); }

  Ref cut(size_t index) {
    if (index > _data.size())
      throw std::out_of_range("cut index too big");

    Ref ret;
    for (size_t i = index; i < _data.size(); i++)
      ret._data.push_back(std::move(_data[i]));

    _data.erase(_data.begin() + index, _data.end());

    return ret;
  }

  void join(Ref &&other) {
    for (auto &x : other)
      _data.push_back(std::move(x));
  }

  void flip() { std::ranges::reverse(_data); }

  const T &operator[](size_t index) const { return _data.at(index); }
  T &operator[](size_t index) { return _data.at(index); }

  void insert(size_t index, T value) {
    if (index > _data.size())
      throw std::out_of_range("oops");
    _data.insert(_data.begin() + index, std::move(value));
  }

  T erase(size_t index) {
    T ret = std::move(_data.at(index));
    _data.erase(_data.begin() + index);
    return ret;
  }

  auto begin() const { return _data.begin(); }
  auto end() const { return _data.end(); }

private:
  std::vector<T> _data;
};

#endif

namespace config {
inline constexpr bool PARENT_POINTERS = true;
}

template <typename T> struct Array {
  // using T = int;
  struct Node {
    T value;
    double weight;
    Node *parent;
    Node *left = nullptr;
    Node *right = nullptr;
    size_t size = 1;
    bool reverse = false;

    Node(T value, Node *parent, double priority) : value(std::move(value)), weight(priority), parent(parent) {}

    Node(T value, Node *parent) : value(std::move(value)), parent(parent) {
      static std::random_device rd;
      static std::mt19937 gen(rd());
      static std::uniform_real_distribution<double> dis(0, 1);

      weight = dis(gen);
    }

    void updateNodeSize() { size = 1 + getNodeSize(left) + getNodeSize(right); }
  };

  Node *root = nullptr;

  Array() = default;
  Array(Node *root) : root(root) {}
  Array(const Array &copy) = delete;
  Array(Array &&move) noexcept : root(move.root) { move.root = nullptr; }

  ~Array() {
    Node *current = root;
    Node *last = nullptr;

    while (current != nullptr) {
      if (last == current->parent) {
        if (current->left != nullptr) {
          last = current;
          current = current->left;
          continue;
        }
        if (current->right != nullptr) {
          last = current;
          current = current->right;
          continue;
        }

        // found leaf, delete it
        last = current;
        Node *parent = current->parent;
        delete current;
        current = parent;
      } else if (last == current->left) {
        if (current->right != nullptr) {
          // coming up from left subtree
          last = current;
          current = current->right;
          continue;
        }

        // No right child, delete current
        last = current;
        Node *parent = current->parent;
        delete current;
        current = parent;
      } else if (last == current->right) {
        // Coming up from right subtree
        last = current;
        Node *parent = current->parent;
        delete current;
        current = parent;
      }
    }
  }

  Array &operator=(Array array) noexcept {
    root = array.root;
    array.root = nullptr;
    return *this;
  }

  bool empty() const { return !root; }

  size_t size() const { return getNodeSize(root); }

  void flip() {
    if (!root)
      return;

    root->reverse = !root->reverse;
  }
  Array cut(size_t index) {
    auto splitInfo = treapSplit(root, index);
    root = splitInfo.t1;

    return Array(splitInfo.t2);
  }
  void join(Array &&arr) {
    auto res = treapJoin(root, arr.root);

    root = res;
    arr.root = nullptr;
  }

  void insert(size_t index, T value) { return insert(index, new Node(std::move(value), nullptr)); }

  void insert(size_t index, Node *node) {
    if (index && !checkInBounds(index - 1)) {
      throw std::out_of_range("Index out of range");
    }

    pushReverseDown(root);

    auto splitInfo = treapSplit(root, index);

    root = treapJoin(treapJoin(splitInfo.t1, node), splitInfo.t2);
  }

  T erase(size_t index) {
    if (!checkInBounds(index)) {
      throw std::out_of_range("Index out of range");
    }

    auto splitInfo = treapSplit(root, index);

    // isolate x
    auto eraseElInfo = treapSplit(splitInfo.t2, 1);

    assert(eraseElInfo.t1);
    assert(!eraseElInfo.t1->left);
    assert(!eraseElInfo.t1->right);

    // delete node
    T retValue = std::move(eraseElInfo.t1->value);
    delete eraseElInfo.t1;

    root = treapJoin(splitInfo.t1, eraseElInfo.t2);
    return retValue;
  }

  const T &operator[](size_t i) const { return find(i); }
  T &operator[](size_t i) { return find(i); }

  void for_each(auto &&fun) const { for_each_rec(root, fun); }

  void for_each_rec(Node *node, auto &&fun) const {
    pushReverseDown(node);

    if (!node)
      return;

    for_each_rec(node->left, fun);
    fun(node->value);
    for_each_rec(node->right, fun);
  }

  T &find(size_t idx) const {
    if (!checkInBounds(idx)) {
      throw std::runtime_error("Out of range");
    }

    Node *curr = root;

    while (curr) {
      pushReverseDown(curr);
      size_t leftSize = getNodeSize(curr->left);

      // go left
      if (idx < leftSize) {
        curr = curr->left;
        continue;
      }

      // go right
      if (leftSize < idx) {
        idx -= leftSize + 1;
        curr = curr->right;
        continue;
      }

      return curr->value;
    }

    // should never happen
    throw std::runtime_error("Unable to find idx");
  }

  static size_t getNodeSize(Node *n) { return n ? n->size : 0; }

  bool checkInBounds(size_t idx) const {
    if (!root) {
      return false;
    }

    return idx < root->size;
  }

  static void pushReverseDown(Node *n) {
    if (!n || !n->reverse)
      return;

    n->reverse = false;
    Node *tmp = n->left;
    n->left = n->right;
    n->right = tmp;

    if (n->left)
      n->left->reverse = !n->left->reverse;
    if (n->right)
      n->right->reverse = !n->right->reverse;
  }

  static Node *treapJoin(Node *r1, Node *r2) {
    pushReverseDown(r1);
    pushReverseDown(r2);

    if (!r1)
      return r2;
    if (!r2)
      return r1;

    if (r1->weight < r2->weight) {
      r1->right = treapJoin(r1->right, r2);
      r1->right->parent = r1;
      r1->updateNodeSize();
      return r1;
    }

    r2->left = treapJoin(r1, r2->left);
    r2->left->parent = r2;
    r2->updateNodeSize();
    return r2;
  }

  struct SplitInfo {
    Node *t1;
    Node *t2;
  };

  static SplitInfo treapSplit(Node *r, size_t leftTargetSize) {
    if (!r) {
      return SplitInfo{.t1 = nullptr, .t2 = nullptr};
    }
    pushReverseDown(r);
    size_t leftSize = getNodeSize(r->left);

    // element at searched idx is in left subtree
    if (leftTargetSize <= leftSize) {
      auto info = treapSplit(r->left, leftTargetSize);

      // left subTree is now the splitted part
      r->left = info.t2;
      if (r->left) {
        r->left->parent = r;
      }

      r->parent = nullptr;
      r->updateNodeSize();
      return SplitInfo{.t1 = info.t1, .t2 = r};
    }

    // idx is in right subtree, search for it in there
    //(offset the idx by the size of left subtree + current vertex)
    auto info = treapSplit(r->right, leftTargetSize - leftSize - 1);
    // r.right is now what is left of the right subtree that's been split
    r->right = info.t1;

    if (r->right) {
      r->right->parent = r;
    }
    r->parent = nullptr;
    r->updateNodeSize();

    return SplitInfo{.t1 = r, .t2 = info.t2};
  }

  /*
  Node *testJoin(std::vector<T> vals) const {
    Node *r = new Node(vals[0], nullptr);
    r->weight = 1;
    r->size = 4;

    Node *rl = new Node(vals[1], r);
    rl->weight = 5;
    rl->size = 1;

    Node *rr = new Node(vals[2], r);
    rr->weight = 20;
    rr->size = 2;

    r->left = rl;
    r->right = rr;

    Node *rrl = new Node(vals[3], rr);
    rrl->weight = 31;
    rrl->size = 1;

    rr->left = rrl;

    Node *r2 = new Node(vals[4], nullptr);
    r2->weight = 3;
    r2->size = 3;

    Node *r2l = new Node(vals[5], r2);
    r2l->weight = 8;
    r2l->size = 1;

    Node *r2r = new Node(vals[6], r2);
    r2r->weight = 4;
    r2r->size = 1;

    r2->left = r2l;
    r2->right = r2r;

    Node *res = treapJoin(r, r2);
    return res;
  }

  Node *testInsert(std::vector<T> vals) {
    Node *r = new Node(vals[0], nullptr);
    r->weight = 1;

    insert(0, r);

    Node *rl = new Node(vals[1], r);
    rl->weight = 5;

    insert(1, rl);

    Node *rr = new Node(vals[2], r);
    rr->weight = 20;

    insert(2, rr);

    Node *rrl = new Node(vals[3], rr);
    rrl->weight = 31;

    insert(3, rrl);

    Node *r2 = new Node(vals[4], nullptr);
    r2->weight = 3;

    insert(4, r2);

    Node *r2l = new Node(vals[5], r2);
    r2l->weight = 8;

    insert(5, r2l);

    Node *r2r = new Node(vals[6], r2);
    r2r->weight = 4;

    insert(6, r2r);

    return root;
  }

  void testErase() {
    for (int i = 0; i < 7; ++i) {
      erase(0);
    }
  }
  */

  // Needed to test the structure of the tree.
  // Replace Node with the real type of your nodes
  // and implementations with the ones matching
  // your attributes.
  struct TesterInterface {
    // using Node = ...
    static const Node *root(const Array *t) { return t->root; }
    // Parent of root must be nullptr, delete if config::PARENT_POINTERS ==
    // false
    static const Node *parent(const Node *n) { return n->parent; }
    static const Node *right(const Node *n) { return n->right; }
    static const Node *left(const Node *n) { return n->left; }
    static const T &value(const Node *n) { return n->value; }
    static uint64_t priority(const Node *n) { return n->weight; }
  };
};

#ifndef __PROGTEST__

struct TestFailed : std::runtime_error {
  using std::runtime_error::runtime_error;
};

std::string fmt(const char *f, ...) {
  va_list args1;
  va_list args2;
  va_start(args1, f);
  va_copy(args2, args1);

  std::string buf(vsnprintf(nullptr, 0, f, args1), '\0');
  va_end(args1);

  vsnprintf(buf.data(), buf.size() + 1, f, args2);
  va_end(args2);

  return buf;
}

template <typename T> struct Tester {
  size_t size() const {
    bool te = tested.empty();
    size_t r = ref.size();
    size_t t = tested.size();
    if (te != !t)
      throw TestFailed(fmt("Size: size %zu but empty is %s.", t, te ? "true" : "false"));
    if (r != t)
      throw TestFailed(fmt("Size: got %zu but expected %zu.", t, r));
    return r;
  }

  Tester cut(const size_t index) { return {ref.cut(index), tested.cut(index)}; }

  void join(Tester &&t) {
    ref.join(std::move(t.ref));
    tested.join(std::move(t.tested));
  }

  void flip() {
    ref.flip();
    tested.flip();
  }

  const T &operator[](size_t index) const {
    const T &r = ref[index];
    const T &t = tested[index];
    if (r != t)
      throw TestFailed("Op [] const mismatch.");
    return t;
  }

  void assign(size_t index, T x) {
    ref[index] = x;
    tested[index] = std::move(x);
    operator[](index);
  }

  void insert(size_t i, T x, bool check_tree_ = false) {
    ref.insert(i, x);
    tested.insert(i, std::move(x));
    size();
    if (check_tree_)
      check_tree();
  }

  T erase(size_t i, bool check_tree_ = false) {
    T r = ref.erase(i);
    T t = tested.erase(i);
    if (r != t)
      throw TestFailed(fmt("Erase mismatch at %zu.", i));
    size();
    if (check_tree_)
      check_tree();
    return t;
  }

  void check_tree() const {
    using TI = typename Array<T>::TesterInterface;
    size_t count = 0;
    auto check_value = [&](const T &) { count++; };

    size();

    check_node(TI::root(&tested), decltype(TI::root(&tested))(nullptr), check_value);

    if (count != ref.size())
      throw TestFailed("Check tree: node count mismatch");

    auto ref_it = ref.begin();
    bool check_value_failed = false;
    tested.for_each([&](const T &v) {
      if (check_value_failed)
        return;
      check_value_failed = (ref_it == ref.end() || *ref_it != v);
      if (!check_value_failed)
        ++ref_it;
    });

    if (ref_it != ref.end() || check_value_failed)
      throw TestFailed("Check tree: element mismatch");
  }

  template <typename Node, typename F> void check_node(const Node *n, const Node *p, F &check_value) const {
    if (!n)
      return;

    using TI = typename Array<T>::TesterInterface;
    if constexpr (config::PARENT_POINTERS) {
      if (TI::parent(n) != p)
        throw TestFailed("Parent mismatch.");
    }

    check_node(TI::left(n), n, check_value);
    check_value(TI::value(n));
    check_node(TI::right(n), n, check_value);
  }

  static void _throw(const char *msg, bool s) { throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed")); }

  Ref<T> ref;
  Array<T> tested;
};

void test_insert() {
  Tester<int> t;

  std::cout << "insert 0" << std::endl;
  for (int i = 0; i < 10; i++)
    t.insert(i, i, true);
  std::cout << "insert 1" << std::endl;
  for (int i = 0; i < 10; i++)
    t.insert(i, -i, true);
  std::cout << "insert 2" << std::endl;
  for (size_t i = 0; i < t.size(); i++)
    t[i];
  std::cout << "insert 3" << std::endl;

  for (int i = 0; i < 5; i++)
    t.insert(15, (1 + i * 7) % 17, true);
  std::cout << "insert 4" << std::endl;
  for (int i = 0; i < 10; i++)
    t.assign(2 * i, 3 * t[2 * i]);
  std::cout << "insert 5" << std::endl;
  for (size_t i = 0; i < t.size(); i++)
    t[i];
  std::cout << "insert 6" << std::endl;
}

void test_erase() {
  Tester<int> t;

  for (int i = 0; i < 10; i++)
    t.insert(i, i, true);
  for (int i = 0; i < 10; i++)
    t.insert(i, -i, true);

  std::cout << "erase 0" << std::endl;
  for (size_t i = 3; i < t.size(); i += 2)
    t.erase(i, true);
  std::cout << "erase 1" << std::endl;
  for (size_t i = 0; i < t.size(); i++)
    t[i];

  std::cout << "erase first end" << std::endl;
  for (int i = 0; i < 5; i++)
    t.insert(3, (1 + i * 7) % 17, true);
  std::cout << "erase 2" << std::endl;
  for (size_t i = 1; i < t.size(); i += 3)
    t.erase(i, true);
  std::cout << "erase second end" << std::endl;

  for (int i = 0; i < 20; i++)
    t.insert(3, 100 + i, true);

  std::cout << "erase 3" << std::endl;
  for (int i = 0; i < 5; i++)
    t.erase(t.size() - 1, true);
  std::cout << "erase 4" << std::endl;
  for (int i = 0; i < 5; i++)
    t.erase(0, true);
  std::cout << "erase third end" << std::endl;

  for (int i = 0; i < 4; i++)
    t.insert(i, i, true);
  for (size_t i = 0; i < t.size(); i++)
    t[i];
}

enum RandomTestFlags : unsigned { SEQ = 1, NO_ERASE = 2, CHECK_TREE = 4, CUT = 8, FLIP = 16 };

void test_random(size_t size, unsigned flags = 0) {
  std::vector<Tester<size_t>> ts(10);
  std::mt19937 my_rand(24707 + size);

  const bool seq = flags & SEQ, erase = !(flags & NO_ERASE), check_tree = flags & CHECK_TREE, cut = flags & CUT,
             flip = flags & FLIP;

  for (size_t i = 0; i < size; i++) {
    size_t ti = my_rand() % ts.size();
    size_t pos = seq ? 0 : my_rand() % (ts[ti].size() + 1);
    ts[ti].insert(pos, my_rand() % (3 * size), check_tree);
  }

  for (auto &t : ts) {
    t.check_tree();
    for (size_t i = 0; i < t.size(); i++)
      t[i];
  }

  for (size_t i = 0; i < 30 * size; i++) {
    if (cut && ts.size() > 2 && my_rand() % 12 == 1) {
      size_t i, j;
      do {
        i = my_rand() % ts.size();
        j = my_rand() % ts.size();
      } while (i == j);

      ts[i].join(std::move(ts[j]));
      ts[j] = std::move(ts.back());
      ts.pop_back();

      continue;
    }

    auto &t = ts[my_rand() % ts.size()];

    if (t.size() == 0)
      goto insert;

    if (cut && my_rand() % 11 == 1) {
      ts.push_back(t.cut(my_rand() % (t.size() + 1)));
      continue;
    }

    if (flip && my_rand() % 10 == 1) {
      t.flip();
      continue;
    }

    switch (my_rand() % 7) {
    case 1: {
      if (!erase && i % 3 == 0)
        break;
    insert:
      size_t pos = seq ? 0 : my_rand() % (t.size() + 1);
      t.insert(pos, my_rand() % 1'000'000, check_tree);
      break;
    }
    case 2:
      if (erase)
        t.erase(my_rand() % t.size(), check_tree);
      break;
    case 3:
      t.assign(my_rand() % t.size(), 155 + i);
      break;
    default:
      t[my_rand() % t.size()];
    }
  }

  for (auto &t : ts)
    t.check_tree();
}

int main() {
  /*
  {
    Array<int> t;

    Array<int>::Node *r = t.testJoin({5, 2, 8, 6, 16, 10, 21});
    Array<int>::Node *parent = nullptr;
    auto ogParent = r;

    assert(r->value == 5 && r->weight == 1 && r->parent == nullptr && r->size == 7);

    assert(r->left->value == 2 && r->left->weight == 5 && r->left->parent == r && r->left->size == 1);
    parent = r;
    r = r->right;
    assert(r->value == 16 && r->weight == 3 && r->parent == parent && r->size == 5);

    assert(r->right->value == 21 && r->right->weight == 4 && r->right->parent == r && r->right->size == 1);
    parent = r;
    r = r->left;
    assert(r->value == 10 && r->weight == 8 && r->parent == parent && r->size == 3);
    parent = r;
    r = r->left;
    assert(r->value == 8 && r->weight == 20 && r->parent == parent && r->size == 2);
    parent = r;
    r = r->left;
    assert(r->value == 6 && r->weight == 31 && r->parent == parent && r->size == 1);

    t.root = ogParent;

    for (size_t i = 0; i < 7; ++i) {
      std::cout << "found: " << i << std::endl;
      assert(t.find(i));
    }

    auto splitInfo = t.treapSplit(ogParent, 4);

    r = splitInfo.t1;
    assert(r->value == 5 && r->weight == 1 && r->parent == nullptr && r->size == 4);
    assert(r->left->value == 2 && r->left->weight == 5 && r->left->parent == r && r->left->size == 1);
    assert(r->right->value == 8 && r->right->weight == 20 && r->right->parent == r && r->right->size == 2);
    r = r->right;
    assert(r->left->value == 6 && r->left->weight == 31 && r->left->parent == r && r->left->size == 1);

    r = splitInfo.t2;
    assert(r->value == 16 && r->weight == 3 && r->parent == nullptr && r->size == 3);
    assert(r->left->value == 10 && r->left->weight == 8 && r->left->parent == r && r->left->size == 1);
    assert(r->right->value == 21 && r->right->weight == 4 && r->right->parent == r && r->right->size == 1);
    r = r->left;

    auto res = t.treapJoin(splitInfo.t1, splitInfo.t2);

    t.root = res;

    int i = 0;
    t.for_each([&i](int val) {
      std::cout << i << ":foreach in value: " << val << std::endl;
      ++i;
    });

  }


  {
    Array<int> t;
    t.testInsert({5, 2, 8, 6, 16, 10, 21});

    t.testErase();
    assert(t.size() == 0);
    assert(t.root == nullptr);
  }
  */

  try {
    std::cout << "Insert test..." << std::endl;
    test_insert();

    std::cout << "Erase test..." << std::endl;
    test_erase();

    std::cout << "Tiny random test without cut and flip..." << std::endl;
    test_random(20, CHECK_TREE);

    std::cout << "Tiny random test without flip..." << std::endl;
    test_random(20, CHECK_TREE | CUT);

    std::cout << "Tiny random test..." << std::endl;
    test_random(20, CHECK_TREE | CUT | FLIP);

    std::cout << "Small random test..." << std::endl;
    test_random(200, CHECK_TREE | CUT | FLIP);

    std::cout << "Bigger random test..." << std::endl;
    test_random(5'000, CUT | FLIP);

    std::cout << "Bigger sequential test..." << std::endl;
    test_random(5'000, SEQ | CUT | FLIP);

    std::cout << "All tests passed." << std::endl;
  } catch (const TestFailed &e) {
    std::cout << "Test failed: " << e.what() << std::endl;
  }

  return 0;
}

#endif
