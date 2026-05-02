#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>

struct Hobbit {
  std::string name;
  int hp, off, def;

  friend bool operator == (const Hobbit&, const Hobbit&) = default;
};

std::ostream& operator << (std::ostream& out, const Hobbit& h) {
  return out
    << "Hobbit{\"" << h.name << "\", "
    << ".hp=" << h.hp << ", "
    << ".off=" << h.off << ", "
    << ".def=" << h.def << "}";
}

template < typename T >
std::ostream& operator << (std::ostream& out, const std::optional<T>& x) {
  if (!x) return out << "EMPTY_OPTIONAL";
  return out << "Optional{" << *x << "}";
}

#endif
struct HobbitArmyNaive {
  static constexpr bool CHECK_NEGATIVE_HP = true;

  std::vector<Hobbit> hobbits;


  bool add(const Hobbit& hobbit) {
    if (hobbit.hp <= 0)
    {
      return false;
    }

    for (size_t i = 0; i < hobbits.size(); ++i)
    {
      if (hobbits[i].name == hobbit.name)
      {
        return false;
      }
    }

    hobbits.push_back(hobbit);
    return true;
  }

  std::optional<Hobbit> erase(const std::string& hobbit_name) {
    size_t n = hobbits.size();
    size_t i = 0;
    for (i = 0; i < n; ++i)
    {
      if (hobbits[i].name == hobbit_name)
      {
        Hobbit h = hobbits.at(i);
        hobbits.erase(hobbits.begin() + i);
        return {h};
      }
    }

    return {};
  }

  std::optional<Hobbit> stats(const std::string& hobbit_name) const {

    for (auto& h : hobbits)
    {
      if (h.name == hobbit_name)
      {
        return {h};
      }
    }
    return {};
  }

  void abortEnchant(std::vector<size_t>& toAbort,  int hp_diff, int off_diff, int def_diff)
  {
    for (auto idx : toAbort)
    {
      hobbits[idx].hp -= hp_diff;
      hobbits[idx].off -= off_diff;
      hobbits[idx].def -= def_diff;
    }
  }

  bool enchant(const std::string& first, const std::string& last, int hp_diff, int off_diff, int def_diff) {
    std::vector<size_t> abortVector;

    if (first > last)
    {
      return true;
    }

    size_t i = 0;
    for (auto& h : hobbits)
    {
      if (h.name >= first && h.name <= last)
      {
        if (h.hp + hp_diff <= 0)
        {
          abortEnchant(abortVector, hp_diff, off_diff, def_diff);
          return false;
        }
        h.hp += hp_diff;
        h.off += off_diff;
        h.def += def_diff;
        abortVector.push_back(i);
      }

      ++i;
    }

    return true;
  }

  void for_each(auto&& fun) const {
    for_each_impl(*this, fun);
  }

  private:
  static void for_each_impl(const HobbitArmyNaive& node, auto& fun) {
    if (node.hobbits.empty()) return;


    std::vector<Hobbit> tmp = node.hobbits;
    std::sort(tmp.begin(), tmp.end(),
      [](const Hobbit& a, const Hobbit& b)
      {
        return a.name < b.name;
      });

    for (const Hobbit &h : tmp)
    {
      fun(h);
    }

  }
};


class HobbitArmy {
  struct HobbitStats
  {
    HobbitStats(int hp, int off, int def): hp(hp), off(off), def(def) {}
    HobbitStats() = default;
    int hp = 0;
    int off = 0;
    int def = 0;

    void updateStats(const HobbitStats& upd)
    {
      hp += upd.hp;
      off += upd.off;
      def += upd.def;
    }
  };

  struct Node
  {

    Node(Hobbit h, Node* parent) : hobbit(std::move(h)), minHp(hobbit.hp), parent(parent)
    {name = hobbit.name;}

    Hobbit hobbit;
    std::string name;

    int subTreeDepth = 0;
    HobbitStats offset = {};
    //minHp is a value not including own offset
    int minHp;

    Node* parent;
    Node* left = nullptr;
    Node* right = nullptr;

    Hobbit& getHobbit()
    {
      return hobbit;
    }
    const Hobbit& getHobbit() const
    {
      return hobbit;
    }

    Hobbit moveHobbit()
    {
      return Hobbit(std::move(hobbit.name), hobbit.hp, hobbit.off, hobbit.def);
    }

    const std::string& key() const
    {
      return hobbit.name;
    }

    void updateHobbit(const HobbitStats& stats)
    {
      hobbit.hp += stats.hp;
      hobbit.off += stats.off;
      hobbit.def += stats.def;
    }

    void pushDownSubTreeOffsets()
    {
      if (left != nullptr)
      {
        left->updateOffset(offset);
      }
      if (right != nullptr)
      {
        right->updateOffset(offset);
      }

      updateHobbit(offset);
      minHp += offset.hp;

      offset = {0, 0, 0};
    }

    void updateOffset(const HobbitStats& change)
    {
      offset.updateStats(change);
    }

    void updateMinHp()
    {
      //minHp doesn't include this tree's offset
      minHp = hobbit.hp;

      if (left != nullptr)
      {
        minHp = std::min(minHp, left->minHp + left->offset.hp);
      }
      if (right != nullptr)
      {
        minHp = std::min(minHp, right->minHp + right->offset.hp);
      }
    }
  };

public:
  HobbitArmy() = default;

  HobbitArmy(const HobbitArmy& o) = default;

  HobbitArmy& operator=(const HobbitArmy& o) = default;

  ~HobbitArmy(){
      if (root == nullptr)
            return;

      std::queue<Node*> que{};

       que.push(root);

      while (!que.empty())
      {
            Node* node = que.front();
            que.pop();

        if (node->left != nullptr)
         que.push(node->left);

       if (node->right != nullptr)
       que.push(node->right);

        delete node;
      }
    root = nullptr;
  }

  static constexpr bool CHECK_NEGATIVE_HP = true;

private:
  Node* root = nullptr;
  ///takes a parent node and newSon node</summary>
  ///assigns parent node as newSon's parent</summary>
  ///assigns newSon as parent's son (left if leftSon = true)</summary>
  static void changeParentsPtr(Node* parent, Node* newSon, const bool leftSon = true)
  {
    if (newSon != nullptr)
      newSon->parent = parent;

    if (parent == nullptr)
      return;

    if (leftSon)
    {
      parent->left = newSon;
      return;
    }
    parent->right = newSon;
  }
  static void changeParentsPtr(Node* parent, Node* oldSon, Node* newSon)
  {
    if (newSon != nullptr)
      newSon->parent = parent;

    if (parent == nullptr) {
      return;
    }

    if (parent->left == oldSon)
    {
      parent->left = newSon;
      return;
    }
    parent->right = newSon;
  }

  ///updates values as it goes by
  ///thanks to that the returning node will have exactly the value it's supposed to
  ///<returns>pair <searchedNode, parentNode>
  std::pair<Node*, Node*> findWithPrev(const std::string& key) const
  {
    Node* prev = nullptr;
    Node* node = root;

    while (node != nullptr)
    {
      node->pushDownSubTreeOffsets();

      if (node->key() == key)
      {
        return {node, prev};
      }

      if (key < node->key())
      {
        prev = node;
        node = node->left;
        continue;
      }
      prev = node;
      node = node->right;
    }
    return {node, prev};
  }

  Node* lowerBound(const std::string& key)const
  {
    Node* node = root;
    Node* candidate = nullptr;

    while (node != nullptr)
    {
      node->pushDownSubTreeOffsets();
      if (node->key() <= key) {
        candidate = node;
        node = node->right;
      } else {
        node = node->left;
      }
    }
    return candidate;
  }
  Node* upperBound(const std::string& key)const
  {
    Node* node = root;
    Node* candidate = nullptr;

    while (node != nullptr)
    {
      node->pushDownSubTreeOffsets();
      if (node->key() >= key) {
        candidate = node;
        node = node->left;
      } else {
        node = node->right;
      }
    }
    return candidate;
  }

  Node* lowestCommonAncestor(Node* lhs, Node* rhs)const
  {
    if (root == nullptr)
    {
      return nullptr;
    }
    Node* node = root;

    while (node != nullptr)
    {
      if (lhs->key() < node->key() && rhs->key() < node->key())
      {
        node = node->left;
        continue;
      }
      if (lhs->key() > node->key() && rhs->key() > node->key())
      {
        node = node->right;
        continue;
      }
      //found the LCA
      return node;
    }
    //should NOT ever happen since that means that either lhs or rhs doesn't exist
    return nullptr;
  }

  void avlRotate(Node* x, Node* y, Node* z)
  {
    if(z != nullptr){

      //double rotation
      changeParentsPtr(x->parent, x, z);

      //rewire sons of z and make x,y sons of z
      if(x->subTreeDepth == -2){
        changeParentsPtr(x, z->right, true);
        changeParentsPtr(y, z->left, false);

        changeParentsPtr(z, x, false);
        changeParentsPtr(z, y, true);
      }
      else if(x->subTreeDepth == 2){
        changeParentsPtr(x, z->left, false);
        changeParentsPtr(y, z->right, true);

        changeParentsPtr(z, x, true);
        changeParentsPtr(z, y, false);
      }

      if (x == root)
      {
        root = z;
      }

      return;
    }

    //single rotations

    //first rewire x's parent
    changeParentsPtr(x->parent, x, y);

    //swap x and y parents
    x->parent = y;

    //assign x as y's son
    //assign y subtree to x
    if(x->subTreeDepth == -2){
      changeParentsPtr(x, y->right, true);
      y->right = x;
    }
    else{
      changeParentsPtr(x, y->left, false);
      y->left = x;
    }

    if (root == x)
    {
      root = y;
    }
  }

  static bool fixDepthInRotatedNodes(Node* x, Node* y, Node* z, int rotationSide)
  {
    //update min Hp values (in this order, because x might be below y)
    x->updateMinHp();
    y->updateMinHp();
    //only one rotation
    if (z == nullptr)
    {
      //happens only in erase
      if (y->subTreeDepth == 0)
      {
        //the result is the opposite for x and y depends on the direction of the rotation
        if (x->subTreeDepth == 2)
        {
          x->subTreeDepth = +1;
          y->subTreeDepth = -1;
          return false;
        }
        x->subTreeDepth = -1;
        y->subTreeDepth = +1;
        return false;
      }

      //last rotation case:
      //(x == +2 and y == +1) or (x == -2 and y == -1)
      x->subTreeDepth = 0;
      y->subTreeDepth = 0;

      //need to propagate the change up tho if I erased
      //since I rotated the firstDepthChangeNode down, y now took its place
      return true;
    }


    //two rotations
    //z is updated last because it's above x and y
    z->updateMinHp();

    x->subTreeDepth = 0;
    y->subTreeDepth = 0;

    //there are a two cases for each one and they're mirrored
    //could be nicer, but tbh at least it simply shows all the cases
    if (rotationSide == -2) {
      if (z->subTreeDepth == 1)
      {
        y->subTreeDepth = -1;
      }
      //else-if should work fine here (one of the z subtrees should be more deep)
      //but just in case...
      if (z->subTreeDepth == -1)
      {
        x->subTreeDepth = +1;
      }
    }
    else {
      if (z->subTreeDepth == 1)
      {
        x->subTreeDepth = -1;
      }
      //else-if should work fine here (one of the z subtrees should be more deep)
      //but just in case...
      if (z->subTreeDepth == -1)
      {
        y->subTreeDepth = +1;
      }
    }

    z->subTreeDepth = 0;
    return true;
  }

  void propagateDepthChange(Node* firstDepthChangeNode, bool leftSonChanged, const bool erase, bool subTreePropagationEnded = false) noexcept
  {
    if (firstDepthChangeNode == nullptr)
    {
      return;
    }

    //all the nodes coming up should have a neutral offset, since I took care of that on the way down
    while (firstDepthChangeNode != nullptr)
    {
      //update possible MinHp changes
      firstDepthChangeNode->updateMinHp();

      //don't need to care about balancing anymore (already taken care of)
      //now just take care of the minHp changes
      if (subTreePropagationEnded)
      {
        firstDepthChangeNode = firstDepthChangeNode->parent;
        continue;
      }

      if(erase){
        //reversed +-1 than in insertPropagate
        firstDepthChangeNode->subTreeDepth += leftSonChanged ? +1 : -1;

        if ((firstDepthChangeNode->subTreeDepth == +1 || firstDepthChangeNode->subTreeDepth == -1))
        {
          subTreePropagationEnded = true; //propagation up no longer needed
          continue;
        }
      }
      else{
        firstDepthChangeNode->subTreeDepth += leftSonChanged ? -1 : +1;

        if(firstDepthChangeNode->subTreeDepth == 0){
          subTreePropagationEnded = true;
          continue;
        }
      }

      if (firstDepthChangeNode->subTreeDepth == 2 || firstDepthChangeNode->subTreeDepth == -2)
      {
        int rotationSide = firstDepthChangeNode->subTreeDepth;

        Node* x = firstDepthChangeNode;
        Node* y = nullptr;
        Node* z = nullptr;

        //assign y and z depending on the case
        //also need to neutralise offsets since I might be doing an avl rotation
        //in some cases they might already be neutralised (insert for instance), but not in all of them
        if(x->subTreeDepth == -2){
          y = x->left;
          y->pushDownSubTreeOffsets();
          if(y->subTreeDepth == 1){
            z = y->right;
            z->pushDownSubTreeOffsets();
          }
        }
        else{
          y = x->right;
          y->pushDownSubTreeOffsets();
          if(y->subTreeDepth == -1){
            z = y->left;
            z->pushDownSubTreeOffsets();
          }
        }

        //avl rotation needed
        avlRotate(x, y, z);

        //need to propagate the change up
        //returns the node to propagate up from (if propagating is required)
        bool propagate = fixDepthInRotatedNodes(x, y, z, rotationSide);
        //continue the propagation from node at the top
        firstDepthChangeNode = z == nullptr ? y : z;

        if (!propagate || !erase)
        {
          subTreePropagationEnded = true;
          continue;
        }
      }

      //firstDepthChangeNode == root
      if (firstDepthChangeNode->parent == nullptr)
      {
        return;
      }

      leftSonChanged = firstDepthChangeNode->parent->left == firstDepthChangeNode;
      firstDepthChangeNode = firstDepthChangeNode->parent;
    }
  }

  static Node* next(Node* node)
  {
    node->pushDownSubTreeOffsets();

    if (node->right == nullptr)
    {
      if (node->parent == nullptr)
      {
        return nullptr;
      }
      node->pushDownSubTreeOffsets();

      Node* parent = node->parent;

      while (parent != nullptr)
      {
        parent->pushDownSubTreeOffsets();
        if (parent->left == node)
        {
          return parent;
        }
        node = parent;
        parent = node->parent;
      }
      return nullptr;
    }

    node = node->right;

    while (true)
    {
      node->pushDownSubTreeOffsets();
      if (node->left == nullptr)
        break;

      node = node->left;
    }
    return node;
  }

public:
  bool add(const Hobbit& hobbit) {
    //checkEntireTreeMinHp();

    if (hobbit.hp <= 0)
    {
      return false;
    }

      std::pair<Node*, Node*> findRes = findWithPrev(hobbit.name);

      //searched node is found => value exists
      if (findRes.first != nullptr)
      {
        return false;
      }

      Node* inserted = new Node(hobbit, findRes.second);

      if (root == nullptr)
      {
        root = inserted;
        return true;
      }

      inserted->parent = findRes.second;

      bool leftSon;
      if (hobbit.name < findRes.second->key())
      {
        findRes.second->left = inserted;
        leftSon = true;
      }
      else
      {
        findRes.second->right = inserted;
        leftSon = false;
      }

      propagateDepthChange(findRes.second, leftSon, false);

      return true;
    }

  std::optional<Hobbit> erase(const std::string& key) {
    std::pair<Node*, Node*> findRes = findWithPrev(key);
    Node* found = findRes.first;

    if (found == nullptr)
    {
      return {};
    }

    //first take care of when found has either no right or no left son (or neither)
    //don't need to change subTreeDepth in the node directly, just propagate it up
    if (found->left == nullptr) //here found->right can also be nullptr
    {
      //just swap the right son as the root
      //since the tree is avl balanced if he has no left son, the right son has to be a leaf (so only 1 element left)
      if (found == root)
      {
        changeParentsPtr(found->parent, found, found->right);
        //as it's root, no reason to propagate change up
        root = found->right;
        //if no left and no right son -> empty tree
        Hobbit val = std::move(found->hobbit);

        delete found;
        return {val};
      }

      bool isLeftSon = found->parent->left == found;
      //propagate change up
      changeParentsPtr(found->parent, found, found->right);
      propagateDepthChange(found->parent, isLeftSon, true);

      Hobbit val = found->moveHobbit();
      delete found;
      return {val};
    }

    //right is nullptr, but left is not
    if (found->right == nullptr)
    {
      if (found == root)
      {
        //the same as before, just swap with the non-null son and don't propUp
        found->left->parent = nullptr;
        root = found->left;
        Hobbit val = found->moveHobbit();
        delete found;
        return {val};
      }

      bool isLeftSon = found->parent->left == found;
      //rewire parent and left subtree
      changeParentsPtr(found->parent, found, found->left);
      //propagate change up
      propagateDepthChange(found->parent, isLeftSon, true);

      Hobbit val = found->moveHobbit();
      delete found;
      return {val};
    }

    //has both sons -> find the lowest higher node
    Node* node = found->right;
    //propagateSubTreeOffset, since this one needs to have neutral offset
    node->pushDownSubTreeOffsets();

    //right son of found has no left sons
    //so just rewire the right son as the found node
    if (node->left == nullptr)
    {
      changeParentsPtr(node, found->left, true);

      if (found == root)
        root = node;

      //rewire the parent's ptr and set node's parent
      changeParentsPtr(found->parent, found, node);
      node->subTreeDepth = found->subTreeDepth;
      //put node instead of found, so just made the right subtree less deep
      //so let's simulate: node->subTreeDepth += -1;
      //I need it with propagate func so it calls for avl rotation if needed
      propagateDepthChange(node, false, true);

      Hobbit val = found->moveHobbit();
      delete found;
      return {val};
    }

    //go as far left as possible (at least once is guaranteed)
    while (node->left != nullptr)
    {
      node = node->left;
      //again needed to have the replacing node neutral offset
      node->pushDownSubTreeOffsets();
    }

    //right tree of the successor (succ has no left child)
    Node* rightNodeOfSucc = node->right;

    //assign found sons as node's
    changeParentsPtr(node, found->left, true);
    changeParentsPtr(node, found->right, false);

    //replace found's succ with his right tree
    //must be left child of his parent (that's how we got here)
    changeParentsPtr(node->parent, rightNodeOfSucc, true);

    //change the subTreeDepth
    node->subTreeDepth = found->subTreeDepth;

    Node* nodeOriginalParent = node->parent;
    changeParentsPtr(found->parent, found, node);

    if (found == root)
    {
      root = node;
    }

    //the found's successor just got deleted, so his parent's left subtree isn't as deep now
    propagateDepthChange(nodeOriginalParent, true, true);

    Hobbit val = found->moveHobbit();
    delete found;
    return {val};
  }

  std::optional<Hobbit> stats(const std::string& hobbit_name) const {
    const auto pair = findWithPrev(hobbit_name);

    if (pair.first == nullptr)
    {
      return {};
    }

    return {pair.first->getHobbit()};
  }

private:

  void static acceptChanges(const std::vector<std::pair<Node*, bool>>& changes, const HobbitStats& offset)
  {
    for (int i = static_cast<int>(changes.size()) - 1; i >= 0; --i)
    {
      auto pair = changes[i];

      //value was changed
      if (pair.second)
      {
        pair.first->hobbit.hp += offset.hp;
        pair.first->hobbit.off += offset.off;
        pair.first->hobbit.def += offset.def;
        continue;
      }
      //offset changed
      pair.first->updateOffset(offset);
    }
  }
public:
  bool enchant(const std::string& first, const std::string& last, int hp_diff, int off_diff, int def_diff)
  {
    //checkEntireTreeMinHp();
    const HobbitStats offset(hp_diff, off_diff, def_diff);

    //the .second signals if the offset of the value was updated (true = value updated)
    std::vector<std::pair<Node*, bool>> toUpdateVector;

    if (first > last)
    {
      return true;
    }
    const auto updateValid = [&toUpdateVector, &offset](Node* node, bool updateValue)
    {
      toUpdateVector.emplace_back(node, updateValue);
      if (updateValue)
      {
        return node->hobbit.hp + node->offset.hp + offset.hp > 0;
      }
      return node->minHp + node->offset.hp + offset.hp > 0;
    };

    Node* lhs = upperBound(first);
    Node* rhs = lowerBound(last);

    if (lhs == nullptr || rhs == nullptr || lhs->key() > rhs->key())
    {
      //means lhs is higher than all keys or rhs is lower than all keys
      //but that doesn't mean failure, technically all nodes within range got updated
      return true;
    }

    //lca must exist (I have two existing nodes, therefore at least root is the lca)
    Node* lca = lowestCommonAncestor(lhs, rhs);

    //lca is always included in the range
    bool validUpdate = updateValid(lca, true);

    Node* node = lca;//to stop the while cycle in case lca == lhs
    //update lhs, rhs subtrees withing range
    if (lhs != lca)
    {
      //go down from lca to lhs
      //always going left since lca must be higher than lhs
      node = lca->left;
      validUpdate = updateValid(lhs, true) && validUpdate;
      if (lhs->right != nullptr)
      {
        validUpdate = updateValid(lhs->right, false) && validUpdate;
      }
    }

    if (rhs != lca)
    {
      validUpdate = updateValid(rhs, true) && validUpdate;
      if (rhs->left != nullptr)
      {
        validUpdate = updateValid(rhs->left, false) && validUpdate;
      }
    }

    //if lhs = lca => lhs is parent (or identical) to rhs the loop condition won't be satisfied
    //don't have to check for nullptr since lhs and rhs exist
    while (node != lhs)
    {
      if (node->key() < lhs->key())
      {
        //go right, left subtree isn't in the range
        node = node->right;
        continue;
      }

      //is in range
      validUpdate = updateValid(node, true) && validUpdate;
      //so is the entire right subTree
      if (node->right != nullptr)
      {
        validUpdate = updateValid(node->right, false) && validUpdate;
      }
      //continue left
      node = node->left;
    }


    node = lca != rhs ? lca->right : rhs;
    //go down from lca to rhs
    while (node != rhs)
    {
      if (node->key() < rhs->key())
      {
        //is in range
        validUpdate = updateValid(node, true) && validUpdate;
        //so is the entire left subTree
        if (node->left != nullptr)
        {
          validUpdate = updateValid(node->left, false) && validUpdate;
        }
        //continue left
        node = node->right;
        continue;
      }

      //go left, right subtree isn't in the range
      node = node->left;
    }

    //if it fails, I don't need to update any minHp back, because I just reverse it into state it was in before
    if (!validUpdate)
    {
      return false;
    }

    acceptChanges(toUpdateVector, offset);
    //doesn't fail --> let's update the minHp
    propagateDepthChange(lhs, false , false, true);
    propagateDepthChange(rhs, false , false, true);

    return true;
  }

  void for_each(auto&& fun) const {
    for_each_impl(root, fun);
  }

  private:
  static void for_each_impl(Node *node, auto& fun) {
    if (!node) return;

    //find min
    while (node->left != nullptr)
    {
      node->pushDownSubTreeOffsets();
      node = node->left;
    }

    //go through the successors
    while (node != nullptr)
    {
      node->pushDownSubTreeOffsets();
      fun(node->getHobbit());
      node = next(node);
    }
  }
};

#ifndef __PROGTEST__

////////////////// Dark magic, ignore ////////////////////////

template < typename T >
auto quote(const T& t) { return t; }

std::string quote(const std::string& s) {
  std::string ret = "\"";
  for (char c : s) if (c != '\n') ret += c; else ret += "\\n";
  return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)

 
////////////////// End of dark magic ////////////////////////


void check_army(const HobbitArmy& A, const std::vector<Hobbit>& ref, int& ok, int& fail) {
  size_t i = 0;

  A.for_each([&](const Hobbit& h) {
    std::cout << i << ": " << h.name << std::endl;
    CHECK(i < ref.size(), true);
    CHECK(h, ref[i]);
    i++;

  });

  CHECK(i, ref.size());
}

void test1(int& ok, int& fail) {
  HobbitArmy A;
  check_army(A, {}, ok, fail);

  CHECK(A.add({"Frodo", 100, 10, 3}), true);
  CHECK(A.add({"Frodo", 200, 10, 3}), false);
  CHECK(A.erase("Frodo"), std::optional(Hobbit("Frodo", 100, 10, 3)));
  CHECK(A.add({"Frodo", 200, 10, 3}), true);

  CHECK(A.add({"Sam", 80, 10, 4}), true);
  CHECK(A.add({"Pippin", 60, 12, 2}), true);
  CHECK(A.add({"Merry", 60, 15, -3}), true);
  CHECK(A.add({"Smeagol", 0, 100, 100}), false);


  if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.add({"Smeagol", -100, 100, 100}), false);

  CHECK(A.add({"Smeagol", 200, 100, 100}), true);


  if (false)
  {
    //not hobbits I know :(
    CHECK(A.add({"Aragorn", 2000, 1000, 100}), true);
    CHECK(A.add({"Boromir", 1, 1000, 100}), true);
    CHECK(A.add({"Legolas", 20, 10000, 100}), true);
    CHECK(A.add({"Gandalf", 100, 100, 100}), true);

    CHECK(A.enchant("Boromir", "Boromir", 1, -1, -1), true);
    CHECK(A.enchant("Boromir", "Boromir", -10, -1, -1), false);

    CHECK(A.enchant("Arnie", "Zelda", -10, -1, -1), false);
    CHECK(A.enchant("Arnie", "Zelda", 100, -1, -1), true);
    CHECK(A.enchant("Legolas", "Zelda", -120, 9999999, 999999), false);
    CHECK(A.enchant("Legolas", "Zelda", -119, -1, -1), true);
    return;
  }

  CHECK(A.enchant("Frodo", "Frodo", 10, 1, 1), true);
  CHECK(A.enchant("Sam", "Frodo", -1000, 1, 1), true); // empty range
  CHECK(A.enchant("Bilbo", "Bungo", 1000, 0, 0), true); // empty range
  
  if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), false);

  CHECK(A.enchant("Frodo", "Sam", 1, 0, 0), true);
  CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), true);

  CHECK(A.stats("Gandalf"), std::optional<Hobbit>{});
  CHECK(A.stats("Frodo"), std::optional(Hobbit("Frodo", 151, 12, 5)));
  CHECK(A.stats("Merry"), std::optional(Hobbit("Merry", 1, 16, -2)));


  check_army(A, {
    {"Frodo", 151, 12, 5},
    {"Merry", 1, 16, -2},
    {"Pippin", 1, 13, 3},
    {"Sam", 21, 11, 5},
    {"Smeagol", 200, 100, 100},
  }, ok, fail);
}

int main()
{
  int ok = 0, fail = 0;
  test1(ok, fail);

  if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
  else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
  std::cout << "Fr?" << std::endl;

  {
    HobbitArmy A;

    // Keep the structure simple so traversals are deterministic
    assert(A.add({"M", 500, 0, 0}));
    assert(A.add({"A", 1000, 0, 0}));
    assert(A.add({"Z", 1000, 0, 0}));

    // --- CRUCIAL PART ---
    // Add a node inside the range whose effective HP is barely safe,
    // and a deep descendant whose lazy offset will be mis-evaluated.
    assert(A.add({"T", 200, 0, 0}));
    assert(A.add({"U", 900, 0, 0}));

    // Apply a lazy enchant that makes subtree minHp depend on offsets.
    assert(A.enchant("M", "Z", -100, 0, 0));   // safe: T = 100 HP, U = 800 HP

    // If enchant() incorrectly applies offsets *during the check-phase*,
    // the next call will be incorrectly accepted.
    bool r = A.enchant("A", "Z", -150, 0, 0);

    // CORRECT BEHAVIOR: This MUST be rejected, because T would drop to -50 HP.
    // If your check-phase applies changes prematurely or pushes offsets incorrectly,
    // this assert will FAIL.
    assert(r == false);
  }
  {
    HobbitArmy A;

    // Insert in order that unbalances the tree
    assert(A.add({"M", 100, 0, 0}));
    assert(A.add({"E", 100, 0, 0}));
    assert(A.add({"T", 100, 0, 0}));
    assert(A.add({"C", 100, 0, 0}));
    assert(A.add({"G", 100, 0, 0}));
    assert(A.add({"R", 100, 0, 0}));
    assert(A.add({"Y", 100, 0, 0}));

    // Now insert a problematic middle key
    assert(A.add({"H", 20, 0, 0}));

    // We enchant a range that SHOULD include "H"
    // but many students' range traversal accidentally skips it.
    bool ok = A.enchant("F", "S", -30, 0, 0);

    // If "H" is included correctly: HP=20-30 = -10 → must FAIL → ok=false
    // If "H" gets SKIPPED by traversal: enchant incorrectly succeeds → ok=true → FAILS assert
    assert(ok == false);
  }

}
#endif


