#include <vector>

using namespace std;

enum class Player { FIRST, SECOND, CHANCE };

struct Choice {
  Node _node;
  vector<double> _cumulativeRegret;
  vector<double> _cumulativeStrategy;
  vector<double> _currentStrategy;

  vector<double> _counterfactualValue;

  Choice(Node n, int numDeals, double cs)
      : _node(n),
        _cumulativeRegret(numDeals),
        _cumulativeStrategy(numDeals),
        _currentStrategy(numDeals, cs),
        _counterfactualValue(numDeals) {}
}

struct Node {
  Player _player;
  vector<Choice> _choices;
  vector<int> _firstPlayerUtility;
  vector<int> _secondPlayerUtility;
  int _numDeals;

  Node(Player p, int numDeals, vector<Node> c)
      : _player(p), _numDeals(numDeals)
  {
    _choices.reserve(c.size());
    for (auto& node : c) {
      _choices.push_back(Choice(node, numDeals, 1. / c.size()));
    }
  }

  Node(vector<int> fpu, vector<int> spu)
      : _firstPlayerUtility(fpu),
        _secondPlayerUtility(spu),
        _numDeals(_firstPlayerUtility.size()) {
    assert(_firstPlayerUtility.size() == _secondPlayerUtility.size());
  }

  vector<double> cfg(Player player,
                     const vector<double>& probP1,
                     const vector<double>& probP2) {
    if (_choices.size() == 0) {
      return player == Player::FIRST ? _firstPlayerUtility
                                     : _secondPlayerUtility;
    }
    if (_player == Player::CHANCE) {
      // TODO is average ok or do we actually need to sample?
      vector<double> res(_numDeals);
      for (auto& choice : _choices) {
        auto utility = choice._node.cfg(player, probP1, probP2);
        for (int i = 0; i < _numDeals; i++)
          res[i] += utility[i] / _choices.size();
      }
      return res;
    }
    vector<double> counterfactualValue(_numDeals);
    for (auto& choice : _choices) {
      vector<double> newProbP1 = probP1;
      vector<double> newProbP2 = probP2;
      if (player == Player::FIRST) {
        for (int i = 0; i < _numDeals; i++) {
          newProbP1[i] *= choice._currentStrategy[i];
        }
        choice._counterfactualValue =
            choice._node.cfg(player, newProbP1, newProbP2);
      } else {
        assert(player == Player::SECOND);
        for (int i = 0; i < _numDeals; i++) {
          newProbP2[i] *= choice._currentStrategy[i];
        }
        choice._counterfactualValue =
            choice._node.cfg(player, newProbP1, newProbP2);
      }
      for (int i = 0; i < _numDeals; i++) {
        counterfactualValue[i] +=
            choice._counterfactualValue[i] * choice._currentStrategy[i];
      }
    }

    if (player == _player) {
      auto& probMe = player == Player::FIRST ? probP1 : probP2;
      auto& probOther = player == Player::FIRST ? probP2 : probP1;
      for (auto& choice : _choices) {
        for (int i = 0; i < _numDeals; i++) {
          regret += probOther * ( - counterfactualValue[i]);
          cumulativeStrategy += probMe * choice._currentStrategy[i];
        }
      }
    }
    return counterfactualValue;
  }
};


void runCounterFactualRegretMinimizationRound(Node* root) {


}
