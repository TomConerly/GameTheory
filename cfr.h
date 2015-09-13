#pragma once

#include <iostream>
#include <string>
#include <vector>

enum class Player {
  FIRST = 0,
  SECOND = 1,
  CHANCE_FIRST = 2,
  CHANCE_SECOND = 3,
  CHANCE_PUBLIC = 4
};

bool isChance(Player player);

struct InformationSet {
  std::vector<double> _cumulativeRegret;
  std::vector<double> _cumulativeStrategy;
  std::vector<double> _currentStrategy;

  InformationSet(int numActions)
      : _cumulativeRegret(numActions),
        _cumulativeStrategy(numActions),
        _currentStrategy(numActions, 1. / numActions) {}

  friend std::ostream& operator<<(std::ostream& os,
                                  const InformationSet& infoSet);
  friend std::istream& operator>>(std::istream& is, InformationSet& infoSet)
};

struct Node {
  Player _player;
  InformationSet *_informationSet;
  int _firstPlayerUtility;
  int _secondPlayerUtility;

  std::vector<Node*> _children;
  std::vector<std::string> _labels;

  // returns counterfactual value
  double cfr(Player player, double probP1, double probP2, double probChance);

  void updateStrategy();
  double computeValue(Player player);
  void clearCumulativeStrategy();

  Node(int fpu, int spu)
      : _firstPlayerUtility(fpu), _secondPlayerUtility(spu) {}
  Node(Player p, InformationSet* is, std::vector<Node*> children)
      : _player(p), _informationSet(is), _children(children), _labels(children.size()) {}
  Node(Player p, InformationSet* is, std::vector<Node*> children, std::vector<std::string> labels)
      : _player(p), _informationSet(is), _children(children), _labels(labels) {}

};

void runRound(Node* root);

void writeToFile(const Node* root);
void readFromFile(const Node* root);
