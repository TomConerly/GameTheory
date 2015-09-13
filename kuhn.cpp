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

string toString(InformationSet* is) {
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
  vector<unique_ptr<Node>> checkCheck;
  vector<unique_ptr<Node>> checkBetCall;
  vector<unique_ptr<Node>> checkBetFold;
  vector<unique_ptr<Node>> betFold;
  vector<unique_ptr<Node>> betCall;
  for (int i = 0; i < 6; i++) {
    checkCheck.push_back(
        make_unique<Node>(fpWin[i] ? 1 : -1, fpWin[i] ? -1 : 1));
    checkBetCall.push_back(
        make_unique<Node>(fpWin[i] ? 2 : -2, fpWin[i] ? -2 : 2));
    checkBetFold.push_back(make_unique<Node>(-1, 1));
    betFold.push_back(make_unique<Node>(1, -1));
    betCall.push_back(make_unique<Node>(fpWin[i] ? 2 : -2, fpWin[i] ? -2 : 2));
  }
  vector<unique_ptr<Node>> checkBet;
  vector<shared_ptr<InformationSet>> checkBetIS(3);
  for (auto& is : checkBetIS)
    is = make_shared<InformationSet>(2);
  for (int i = 0; i < 6; i++) {
    vector<unique_ptr<Node>> children;
    children.push_back(move(checkBetFold[i]));
    children.push_back(move(checkBetCall[i]));
    vector<string> labels = {"Fold", "Call"};
    checkBet.push_back(make_unique<Node>(
        Player::FIRST, checkBetIS[fpIS[i]], move(children), labels));
  }
  vector<unique_ptr<Node>> check;
  vector<shared_ptr<InformationSet>> checkIS(3);
  for (auto& is : checkIS)
    is = make_shared<InformationSet>(2);
  for (int i = 0; i < 6; i++) {
    vector<unique_ptr<Node>> children;
    children.push_back(move(checkCheck[i]));
    children.push_back(move(checkBet[i]));
    vector<string> labels = {"Check", "Bet"};
    check.push_back(make_unique<Node>(
        Player::SECOND, checkIS[spIS[i]], move(children), labels));
  }

  vector<unique_ptr<Node>> bet;
  vector<shared_ptr<InformationSet>> betIS(3);
  for (auto& is : betIS)
    is = make_shared<InformationSet>(2);
  for (int i = 0; i < 6; i++) {
    vector<unique_ptr<Node>> children;
    children.push_back(move(betFold[i]));
    children.push_back(move(betCall[i]));
    vector<string> labels = {"Fold", "Call"};
    bet.push_back(make_unique<Node>(
        Player::SECOND, betIS[spIS[i]], move(children), labels));
  }

  vector<unique_ptr<Node>> start;
  vector<shared_ptr<InformationSet>> startIS(3);
  for (auto& is : startIS)
    is = make_shared<InformationSet>(2);
  for (int i = 0; i < 6; i++) {
    vector<unique_ptr<Node>> children;
    children.push_back(move(check[i]));
    children.push_back(move(bet[i]));
    vector<string> labels = {"Check", "Bet"};
    start.push_back(make_unique<Node>(
        Player::FIRST, startIS[fpIS[i]], move(children), labels));
  }

  vector<unique_ptr<Node>> children;
  children.push_back(move(start[0]));
  children.push_back(move(start[1]));
  vector<string> labels = {"SP dealt Q", "SP dealt K"};
  vector<unique_ptr<Node>> dealSP;
  dealSP.push_back(make_unique<Node>(
      Player::CHANCE_SECOND, nullptr, move(children), labels));

  children.clear();
  children.push_back(move(start[2]));
  children.push_back(move(start[3]));
  labels = {"SP dealt J", "SP dealt K"};
  dealSP.push_back(make_unique<Node>(
      Player::CHANCE_SECOND, nullptr, move(children), labels));

  children.clear();
  children.push_back(move(start[4]));
  children.push_back(move(start[5]));
  labels = {"SP dealt J", "SP dealt Q"};
  dealSP.push_back(make_unique<Node>(
      Player::CHANCE_SECOND, nullptr, move(children), labels));

  children.clear();
  children.push_back(move(dealSP[0]));
  children.push_back(move(dealSP[1]));
  children.push_back(move(dealSP[2]));
  labels = {"FP dealt J", "FP dealt Q", "FP dealt K"};
  Node root(Player::CHANCE_FIRST, nullptr, move(children), labels);

  for (int i = 0; i < 20000; i++) {
    if (i == 10000) {
      root.clearCumulativeStrategy();
    }
    runRound(&root);
  }

  cout << "value: " << root.computeValue(Player::FIRST) << " "
       << root.computeValue(Player::SECOND) << endl;

  cout << "Open J: " << toString(startIS[0].get()) << endl;
  cout << "Open Q: " << toString(startIS[1].get()) << endl;
  cout << "Open K: " << toString(startIS[2].get()) << endl;

  cout << "Facing Bet J: " << toString(betIS[0].get()) << endl;
  cout << "Facing Bet Q: " << toString(betIS[1].get()) << endl;
  cout << "Facing Bet K: " << toString(betIS[2].get()) << endl;

  cout << "Facing Check J: " << toString(checkIS[0].get()) << endl;
  cout << "Facing Check Q: " << toString(checkIS[1].get()) << endl;
  cout << "Facing Check K: " << toString(checkIS[2].get()) << endl;

  cout << "Facing Check Bet J: " << toString(checkBetIS[0].get()) << endl;
  cout << "Facing Check Bet Q: " << toString(checkBetIS[1].get()) << endl;
  cout << "Facing Check Bet K: " << toString(checkBetIS[2].get()) << endl;

  writeToFile(&root, "kuhn.game");
}
