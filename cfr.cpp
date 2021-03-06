#include "cfr.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;
static bool haveCleared = false;

bool isChance(Player player) {
  return player == Player::CHANCE_FIRST || player == Player::CHANCE_SECOND ||
         player == Player::CHANCE_PUBLIC;
}

double Node::cfr(Player player,
                 double probP1,
                 double probP2,
                 double probChance) {
  if (_children.size() == 0) {
    return player == Player::FIRST ? _firstPlayerUtility : _secondPlayerUtility;
  }
  if (isChance(_player)) {
    double res = 0;
    for (auto& child : _children) {
      res += child->cfr(player, probP1, probP2, probChance / _children.size());
    }
    return res;
  }
  vector<double> counterfactualValues(_children.size());
  double counterfactualValue = 0.0;
  for (int i = 0; i < _children.size(); i++) {
    if (_player == Player::FIRST) {
      counterfactualValues[i] =
          _children[i]->cfr(player,
                            probP1 * _informationSet->_currentStrategy[i],
                            probP2,
                            probChance);
    } else {
      counterfactualValues[i] =
          _children[i]->cfr(player,
                            probP1,
                            probP2 * _informationSet->_currentStrategy[i],
                            probChance);
    }
    counterfactualValue +=
        _informationSet->_currentStrategy[i] * counterfactualValues[i];
  }
  if (_player == player) {
    for (int i = 0; i < _children.size(); i++) {
      double probMe = player == Player::FIRST ? probP1 : probP2;
      double probOther =
          probChance * (player == Player::FIRST ? probP2 : probP1);
      _informationSet->_cumulativeRegret[i] +=
          probOther * (counterfactualValues[i] - counterfactualValue);
      _informationSet->_cumulativeStrategy[i] +=
          probMe * _informationSet->_currentStrategy[i];
    }
  }
  return counterfactualValue;
}

void Node::updateStrategy() {
  for (auto& node : _children) {
    node->updateStrategy();
  }
  if (isChance(_player))
    return;

  double totalRegret = 0;
  for (int i = 0; i < _children.size(); i++) {
    totalRegret += max(0., _informationSet->_cumulativeRegret[i]);
  }
  for (int i = 0; i < _children.size(); i++) {
    if (totalRegret == 0) {
      _informationSet->_currentStrategy[i] = 1. / _children.size();
    } else {
      _informationSet->_currentStrategy[i] =
          max(0., _informationSet->_cumulativeRegret[i]) / totalRegret;
    }
  }
}

void runRound(Node* root) {
  root->cfr(Player::FIRST, 1.0, 1.0, 1.0);
  root->updateStrategy();
  root->cfr(Player::SECOND, 1.0, 1.0, 1.0);
  root->updateStrategy();
}

double Node::computeValue(Player player) {
  if (_children.size() == 0) {
    return player == Player::FIRST ? _firstPlayerUtility : _secondPlayerUtility;
  }
  if (isChance(_player)) {
    double res = 0;
    for (auto& node : _children) {
      res += node->computeValue(player);
    }
    return res / _children.size();
  }
  double res = 0;
  for (int i = 0; i < _children.size(); i++) {
    res += _children[i]->computeValue(player) *
           _informationSet->_currentStrategy[i];
  }
  return res;
}

void Node::clearCumulativeStrategy() {
  haveCleared = true;
  if (_children.size() == 0) {
    return;
  }
  for (auto& child : _children) {
    child->clearCumulativeStrategy();
  }
  if (_informationSet == nullptr) {
    return;
  }
  for (auto& cumStrat : _informationSet->_cumulativeStrategy) {
    cumStrat = 0;
  }
}

void setStrategyFromCumulativeStrategy(Node* at) {
  if (at->_children.size() == 0) {
    return;
  }
  for (auto& node : at->_children) {
    setStrategyFromCumulativeStrategy(node.get());
  }
  if (isChance(at->_player)) {
    return;
  }
  double totalStrategy = 0.0;
  for (int i = 0; i < at->_children.size(); i++) {
    totalStrategy += at->_informationSet->_cumulativeStrategy[i];
  }
  for (int i = 0; i < at->_children.size(); i++) {
    if (totalStrategy == 0.0) {
      at->_informationSet->_currentStrategy[i] = 1. / at->_children.size();
    } else {
      at->_informationSet->_currentStrategy[i] =
          at->_informationSet->_cumulativeStrategy[i] / totalStrategy;
    }
  }
}

void setStrategyFromCumulativeRegret(Node* at) {
  if (at->_children.size() == 0) {
    return;
  }
  for (auto& node : at->_children) {
    setStrategyFromCumulativeRegret(node.get());
  }
  if (isChance(at->_player)) {
    return;
  }
  double totalRegret = 0.0;
  for (int i = 0; i < at->_children.size(); i++) {
    totalRegret += max(0., at->_informationSet->_cumulativeRegret[i]);
  }
  for (int i = 0; i < at->_children.size(); i++) {
    if (totalRegret == 0.0) {
      at->_informationSet->_currentStrategy[i] = 1. / at->_children.size();
    } else {
      at->_informationSet->_currentStrategy[i] =
          max(0., at->_informationSet->_cumulativeRegret[i]) / totalRegret;
    }
  }
}

ostream& operator<<(ostream& os, const Player& player) {
  os << static_cast<int>(player);
  return os;
}
istream& operator>>(istream& is, Player& player) {
  int id;
  is >> id;
  player = static_cast<Player>(id);
  return is;
}

ostream& operator<<(ostream& os, const InformationSet& infoSet) {
  os << infoSet._currentStrategy.size() << " ";
  for (const auto d : infoSet._cumulativeRegret)
    os << d << " ";
  for (const auto d : infoSet._cumulativeStrategy)
    os << d << " ";
  for (const auto d : infoSet._currentStrategy)
    os << d << " ";
  return os;
}

istream& operator>>(istream& is, InformationSet& infoSet) {
  int size;
  is >> size;
  infoSet._cumulativeRegret.resize(size);
  for (auto& d : infoSet._cumulativeRegret)
    is >> d;
  infoSet._cumulativeStrategy.resize(size);
  for (auto& d : infoSet._cumulativeStrategy)
    is >> d;
  infoSet._currentStrategy.resize(size);
  for (auto& d : infoSet._currentStrategy)
    is >> d;
  return is;
}

void collectInfoSets(const Node* at,
                     unordered_map<InformationSet*, int>& isMap) {
  if (at->_informationSet != nullptr &&
      isMap.find(at->_informationSet.get()) == isMap.end()) {
    isMap.insert({at->_informationSet.get(), isMap.size()});
  }
  for (const auto& child : at->_children) {
    collectInfoSets(child.get(), isMap);
  }
}

void writeNode(const Node* at,
               const unordered_map<InformationSet*, int>& isMap,
               ostream& os) {
  os << at->_player << " ";
  if (at->_informationSet == nullptr)
    os << -1 << " ";
  else
    os << isMap.find(at->_informationSet.get())->second << " ";
  os << at->_firstPlayerUtility << " ";
  os << at->_secondPlayerUtility << " ";

  os << at->_children.size() << " ";
  for (int i = 0; i < at->_children.size(); i++) {
    for (auto ch : at->_labels[i])
      if (ch == ' ')
        os << "_";
      else
        os << ch;
    os << " ";
    writeNode(at->_children[i].get(), isMap, os);
  }
}

void writeToFile(const Node* root, string fileName) {
  ofstream outfile;
  outfile.open(fileName);

  unordered_map<InformationSet*, int> isMap;
  collectInfoSets(root, isMap);
  vector<InformationSet*> isReverseMap(isMap.size());
  for (auto it : isMap)
    isReverseMap[it.second] = it.first;
  outfile << isMap.size() << " " << endl;
  for (auto is : isReverseMap)
    outfile << *is << " " << endl;

  writeNode(root, isMap, outfile);
  outfile.close();
}

unique_ptr<Node> readNode(const vector<shared_ptr<InformationSet>>& isMap,
                          istream& is) {
  Player player;
  is >> player;
  int infoSetId;
  is >> infoSetId;
  int firstPlayerUtility;
  is >> firstPlayerUtility;
  int secondPlayerUtility;
  is >> secondPlayerUtility;
  int numChildren;
  is >> numChildren;
  vector<unique_ptr<Node>> children(numChildren);
  vector<string> labels(numChildren);
  for (int i = 0; i < numChildren; i++) {
    is >> labels[i];
    for (auto& ch : labels[i])
      if (ch == '_')
        ch = ' ';
    children[i] = readNode(isMap, is);
  }
  if (numChildren == 0) {
    return make_unique<Node>(firstPlayerUtility, secondPlayerUtility);
  } else {
    assert(firstPlayerUtility == 0);
    assert(secondPlayerUtility == 0);
    return make_unique<Node>(player,
                             infoSetId == -1 ? nullptr : isMap[infoSetId],
                             move(children),
                             move(labels));
  }
}

unique_ptr<Node> readFromFile(string fileName) {
  ifstream infile;
  infile.open(fileName);

  int isMapSize;
  infile >> isMapSize;
  vector<shared_ptr<InformationSet>> isMap(isMapSize);
  for (int i = 0; i < isMapSize; i++) {
    isMap[i] = make_shared<InformationSet>(0);
    infile >> *isMap[i];
  }
  auto res = readNode(isMap, infile);
  infile.close();
  return res;
}

unique_ptr<Node> copy(
    const Node* at,
    const unordered_map<InformationSet*, shared_ptr<InformationSet>>& isMap) {
  if (at->_children.size() == 0) {
    return make_unique<Node>(at->_firstPlayerUtility, at->_secondPlayerUtility);
  }
  vector<unique_ptr<Node>> children;
  for (const auto& child : at->_children) {
    children.push_back(copy(child.get(), isMap));
  }
  shared_ptr<InformationSet> newIs = nullptr;
  if (at->_informationSet.get() != nullptr) {
    newIs = isMap.find(at->_informationSet.get())->second;
  }
  return make_unique<Node>(at->_player, newIs, move(children), at->_labels);
}

unique_ptr<Node> copy(const Node* root) {
  unordered_map<InformationSet*, int> isMap;
  collectInfoSets(root, isMap);

  unordered_map<InformationSet*, shared_ptr<InformationSet>> isMap2;
  for (const auto it : isMap) {
    isMap2[it.first] = make_shared<InformationSet>(*it.first);
  }
  return copy(root, isMap2);
}

struct Entry {
  Node* node;
  double probOther;
  Entry(Node* n, double p) : node(n), probOther(p) {}
};

void setStrategyToCounterStrategy(Node* root, Player player) {
  assert(player == Player::FIRST || player == Player::SECOND);
  vector<vector<Entry>> bfsOrder = {{Entry(root, 1.0)}};
  for (int i = 0; i < bfsOrder.size(); i++) {
    for (auto entry : bfsOrder[i]) {
      auto node = entry.node;
      if (node->_informationSet != nullptr) {
        fill(node->_informationSet->_currentEV.begin(),
             node->_informationSet->_currentEV.end(),
             0);
      }
      if (node->_children.size() == 0) {
        continue;
      } else {
        if (i + 1 == bfsOrder.size()) {
          bfsOrder.push_back({});
        }
        if (isChance(node->_player)) {
          for (auto& child : node->_children) {
            bfsOrder[i + 1].push_back(
                Entry(child.get(), entry.probOther / node->_children.size()));
          }
        } else if (node->_player != player) {
          for (int j = 0; j < node->_children.size(); j++) {
            bfsOrder[i + 1].push_back(
                Entry(node->_children[j].get(),
                      node->_informationSet->_currentStrategy[j]));
          }
        } else {
          for (auto& child : node->_children) {
            bfsOrder[i + 1].push_back(Entry(child.get(), entry.probOther));
          }
        }
      }
    }
  }

  for (auto it = bfsOrder.rbegin(); it != bfsOrder.rend(); ++it) {
    for (auto entry : *it) {
      auto node = entry.node;
      if (node->_children.size() == 0) {
        node->_ev = player == Player::FIRST ? node->_firstPlayerUtility
                                            : node->_secondPlayerUtility;
      } else if (isChance(node->_player)) {
        double res = 0.0;
        for (auto& child : node->_children) {
          res += child->_ev;
        }
        node->_ev = res / node->_children.size();
      } else if (node->_player != player) {
        double res = 0.0;
        for (int i = 0; i < node->_children.size(); i++) {
          res += node->_informationSet->_currentStrategy[i] *
                 node->_children[i]->_ev;
        }
        node->_ev = res;
      } else {
        for (int i = 0; i < node->_children.size(); i++) {
          node->_informationSet->_currentEV[i] +=
              node->_children[i]->_ev * entry.probOther;
        }
      }
    }
    for (auto entry : *it) {
      auto node = entry.node;
      if (node->_player != player || node->_children.size() == 0) {
        continue;
      }
      int bestIdx = 0;
      for (int i = 0; i < node->_children.size(); i++) {
        if (node->_informationSet->_currentEV[i] >
            node->_informationSet->_currentEV[bestIdx]) {
          bestIdx = i;
        }
      }
      for (int i = 0; i < node->_children.size(); i++) {
        node->_informationSet->_currentStrategy[i] = 0.0;
      }
      node->_informationSet->_currentStrategy[bestIdx] = 1.0;
      node->_ev = node->_children[bestIdx]->_ev;
    }
  }
}
