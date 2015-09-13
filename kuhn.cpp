#include <iostream>
#include <sstream>
#include <string>

#include "cfr.h"
#include "play.h"

using namespace std;

// JQ, JK, QJ, QK, KJ, KQ 
vector<bool> fpWin = {false, false, true, false, true, true};
vector<int> fpIS = {0, 0, 1, 1, 2, 2};
vector<int> spIS = {1, 2, 0, 2, 0, 1};

string toString(InformationSet *is) {
  double res = 0;
  double totalStrategy = 0.0;
  for (int i = 0; i < is->_cumulativeStrategy.size(); i++) {
    totalStrategy += is->_cumulativeStrategy[i];
  }

  ostringstream os;
  for (int i = 0; i < is->_cumulativeStrategy.size(); i++) {
    os << is->_cumulativeStrategy[i] / totalStrategy;
    os << " ";
  }
  return os.str();
}

int main() {
  vector<Node> checkCheck;
  vector<Node> checkBetCall;
  vector<Node> checkBetFold;
  vector<Node> betFold;
  vector<Node> betCall;
  for (int i = 0; i < 6; i++) {
    checkCheck.push_back(Node(fpWin[i] ? 1 : -1, fpWin[i] ? -1 : 1));
    checkBetCall.push_back(Node(fpWin[i] ? 2 : -2, fpWin[i] ? -2 : 2));
    checkBetFold.push_back(Node(-1, 1));
    betFold.push_back(Node(1, -1));
    betCall.push_back(Node(fpWin[i] ? 2 : -2, fpWin[i] ? -2 : 2));
  }
  vector<Node> checkBet;
  vector<InformationSet> checkBetIS(3, 2);
  for (int i = 0; i < 6; i++) {
    checkBet.push_back(Node(Player::FIRST,
                            &checkBetIS[fpIS[i]],
                            {&checkBetFold[i], &checkBetCall[i]},
                            {"Fold", "Call"}));
  }
  vector<Node> check;
  vector<InformationSet> checkIS(3, 2);
  for (int i = 0; i < 6; i++) {
    check.push_back(Node(Player::SECOND,
                         &checkIS[spIS[i]],
                         {&checkCheck[i], &checkBet[i]},
                         {"Check", "Bet"}));
  }

  vector<Node> bet;
  vector<InformationSet> betIS(3, 2);
  for (int i = 0; i < 6; i++) {
    bet.push_back(Node(Player::SECOND,
                       &betIS[spIS[i]],
                       {&betFold[i], &betCall[i]},
                       {"Fold", "Call"}));
  }

  vector<Node> start;
  vector<InformationSet> startIS(3, 2);
  for (int i = 0; i < 6; i++) {
    start.push_back(Node(Player::FIRST,
                         &startIS[fpIS[i]],
                         {&check[i], &bet[i]},
                         {"Check", "Bet"}));
  }

  // JQ, JK, QJ, QK, KJ, KQ 
  vector<Node> dealSP;
  dealSP.push_back(Node(Player::CHANCE_SECOND, nullptr, {&start[0], &start[1]}, {"SP dealt Q", "SP dealt K"}));
  dealSP.push_back(Node(Player::CHANCE_SECOND, nullptr, {&start[2], &start[3]}, {"SP dealt J", "SP dealt K"}));
  dealSP.push_back(Node(Player::CHANCE_SECOND, nullptr, {&start[4], &start[5]}, {"SP dealt J", "SP dealt Q"}));

  Node root(Player::CHANCE_FIRST,
            nullptr,
            {&dealSP[0], &dealSP[1], &dealSP[2]},
            {"FP dealt J", "FP dealt Q", "FP dealt K"});

  for (int i = 0; i < 20000; i++) {
    if (i == 10000) {
      root.clearCumulativeStrategy();
    }
    runRound(&root);
  }

  cout << "value: " << root.computeValue(Player::FIRST) << " "
       << root.computeValue(Player::SECOND) << endl;

  cout << "Open J: " << toString(&startIS[0]) << endl;
  cout << "Open Q: " << toString(&startIS[1]) << endl;
  cout << "Open K: " << toString(&startIS[2]) << endl;

  cout << "Facing Bet J: " << toString(&betIS[0]) << endl;
  cout << "Facing Bet Q: " << toString(&betIS[1]) << endl;
  cout << "Facing Bet K: " << toString(&betIS[2]) << endl;

  cout << "Facing Check J: " << toString(&checkIS[0]) << endl;
  cout << "Facing Check Q: " << toString(&checkIS[1]) << endl;
  cout << "Facing Check K: " << toString(&checkIS[2]) << endl;

  cout << "Facing Check Bet J: " << toString(&checkBetIS[0]) << endl;
  cout << "Facing Check Bet Q: " << toString(&checkBetIS[1]) << endl;
  cout << "Facing Check Bet K: " << toString(&checkBetIS[2]) << endl;

  cout << endl << endl << endl;
  while (true) {
    playInstance(&root);
    cout << endl;
  }

}
