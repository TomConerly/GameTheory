#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class Player {
  FIRST = 0,
  SECOND = 1,
  CHANCE_FIRST = 2,
  CHANCE_SECOND = 3,
  CHANCE_PUBLIC = 4
};

std::ostream& operator<<(std::ostream& os, const Player& player);
std::istream& operator>>(std::istream& is, Player& player);

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
  friend std::istream& operator>>(std::istream& is, InformationSet& infoSet);
};

struct Node {
  Player _player;
  std::shared_ptr<InformationSet> _informationSet = nullptr;
  int _firstPlayerUtility = 0;
  int _secondPlayerUtility = 0;

  std::vector<std::unique_ptr<Node>> _children;
  std::vector<std::string> _labels;

  // returns counterfactual value
  double cfr(Player player, double probP1, double probP2, double probChance);

  void updateStrategy();
  double computeValue(Player player);
  void clearCumulativeStrategy();

  Node(int fpu, int spu)
      : _firstPlayerUtility(fpu), _secondPlayerUtility(spu) {}
  Node(Player p,
       std::shared_ptr<InformationSet> is,
       std::vector<std::unique_ptr<Node>> children)
      : _player(p),
        _informationSet(is),
        _children(std::move(children)),
        _labels(children.size()) {}
  Node(Player p,
       std::shared_ptr<InformationSet> is,
       std::vector<std::unique_ptr<Node>> children,
       std::vector<std::string> labels)
      : _player(p),
        _informationSet(is),
        _children(std::move(children)),
        _labels(labels) {}
};

void runRound(Node* root);

void writeToFile(const Node* root, std::string fileName);
std::unique_ptr<Node> readFromFile(std::string fileName);
