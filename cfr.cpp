#include "cfr.h"

#include <algorithm>
#include <iostream>

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
    return player == Player::FIRST ? _firstPlayerUtility
      : _secondPlayerUtility;
  }
  if (isChance(_player)) {
    double res = 0;
    for (auto child : _children) {
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
  for (auto node : _children) {
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
    for (auto node : _children) {
      res += node->computeValue(player);
    }
    return res / _children.size();
  }
  double res = 0;
  double totalStrategy = 0.0;
  for (int i = 0; i < _children.size(); i++) {
    res += _children[i]->computeValue(player) * _informationSet->_cumulativeStrategy[i];
    totalStrategy += _informationSet->_cumulativeStrategy[i];
  }
  if (totalStrategy == 0.0) {
    return 0.0;
  }
  return res / totalStrategy;
}

void Node::clearCumulativeStrategy() {
  haveCleared = true;
  if (_children.size() == 0) {
    return;
  }
  for (auto &child : _children) {
    child->clearCumulativeStrategy();
  }
  if (_informationSet == nullptr) {
    return;
  }
  for (auto &cumStrat : _informationSet->_cumulativeStrategy) {
    cumStrat = 0;
  }
}

std::ostream& operator<<(std::ostream& os, const InformationSet& infoSet) {
  os << infoSet._currentStrategy.size() << " ";
  for (const auto d : infoSet._cumulativeRegret)
    os << d << " ";
  for (const auto d : infoSet._cumulativeStrategy)
    os << d << " ";
  for (const auto d : infoSet._currentStrategy)
    os << d << " ";
}

friend std::istream& operator>>(std::istream& is, InformationSet& infoSet) {
  int size;
  is >> size;
  infoSet._cumulativeRegret.resize(size);
  for (auto& d : infoSet._cumulativeRegret)
    is >> d;
  infoSet._cumulativeStrategy.resize(size);
  for (auto& d: infoSet._cumulativeStrategy)
    is >> d;
  infoSet._currentStrategy.resize(size);
  for (auto& d : infoSet._currentStrategy)
    is >> d;
  return is;
}

void writeToFile(const Node* root) {
}

void readFromFile(const Node* root) {
}



























































