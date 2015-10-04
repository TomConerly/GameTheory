#include "cfr.h"
#include "play.h"

#include <gflags/gflags.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

DEFINE_int32(diceSides, 6, "Number of sides a die.");
DEFINE_int32(numRounds, 10000, "Number of rounds to run");
DEFINE_int32(clearAfter,
             5000,
             "Clear the cumulative strategy after this many rounds.");
DEFINE_int32(printEvery, 100, "When to compute the current EV");
DEFINE_string(outputFile, "dice.game", "");

struct Bid {
  int count, value;

  Bid(int c, int v) : count(c), value(v) {}

  friend ostream& operator<<(ostream&, const Bid&);
  string toString() {
    ostringstream os;
    os << *this;
    return os.str();
  }
};

ostream& operator<<(ostream& os, const Bid& b) {
  if (b.count == -1) {
    os << "Call";
  } else {
    os << b.count << " " << (b.value + 1);
  }
  return os;
}

vector<Bid> allBids;

bool satisfied(Bid b, vector<int> dice) {
  int count = 0;
  for (int d : dice) {
    if (d == 0 || d == b.value)
      count++;
  }
  return count >= b.count;
}

vector<unique_ptr<Node>> makeNodes(Player toAct, vector<Bid>& bids) {
  if (bids.size() > 0 && bids[bids.size() - 1].count == -1) {
    Bid lastBid = bids[bids.size() - 2];
    vector<unique_ptr<Node>> res;
    for (int i = 0; i < FLAGS_diceSides * FLAGS_diceSides; i++) {
      int fpDie = i % FLAGS_diceSides;
      int spDie = i / FLAGS_diceSides;
      if ((toAct == Player::FIRST) == satisfied(lastBid, {fpDie, spDie})) {
        res.push_back(make_unique<Node>(1, 0));
      } else {
        res.push_back(make_unique<Node>(0, 1));
      }
    }
    return res;
  }
  vector<vector<unique_ptr<Node>>> children(FLAGS_diceSides * FLAGS_diceSides);
  vector<string> labels;
  for (auto bid : allBids) {
    if (bids.size() > 0 && bid.count != -1) {
      Bid lastBid = bids[bids.size() - 1];
      if (lastBid.count > bid.count) {
        continue;
      }
      if (lastBid.count == bid.count &&
          ((lastBid.value >= bid.value && bid.value != 0) ||
           lastBid.value == 0)) {
        continue;
      }
    }
    bids.push_back(bid);
    auto nodes = makeNodes(
        toAct == Player::FIRST ? Player::SECOND : Player::FIRST, bids);
    bids.pop_back();
    for (int i = 0; i < FLAGS_diceSides * FLAGS_diceSides; i++) {
      children[i].push_back(move(nodes[i]));
    }
    labels.push_back(bid.toString());
  }
  vector<unique_ptr<Node>> res;
  vector<shared_ptr<InformationSet>> is;
  for (int i = 0; i < FLAGS_diceSides; i++)
    is.push_back(make_shared<InformationSet>(children[0].size()));

  for (int i = 0; i < FLAGS_diceSides * FLAGS_diceSides; i++) {
    int isIdx =
        toAct == Player::FIRST ? i % FLAGS_diceSides : i / FLAGS_diceSides;
    res.push_back(
        make_unique<Node>(toAct, is[isIdx], move(children[i]), labels));
  }
  return res;
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  for (int count = 1; count <= 2; count++) {
    for (int value = 0; value < FLAGS_diceSides; value++) {
      allBids.push_back(Bid(count, value));
    }
  }
  allBids.push_back(Bid(-1, -1));

  vector<Bid> bids;
  auto start = makeNodes(Player::FIRST, bids);

  vector<unique_ptr<Node>> spStart;
  for (int i = 0; i < FLAGS_diceSides; i++) {
    vector<unique_ptr<Node>> children;
    vector<string> labels;
    for (int j = 0; j < FLAGS_diceSides; j++) {
      children.push_back(move(start[i + j * FLAGS_diceSides]));
      ostringstream os;
      os << "SP rolled: " << (j + 1);
      labels.push_back(os.str());
    }
    spStart.push_back(make_unique<Node>(
        Player::CHANCE_SECOND, nullptr, move(children), labels));
  }

  vector<string> labels;
  for (int i = 0; i < FLAGS_diceSides; i++) {
    ostringstream os;
    os << "FP rolled: " << (i + 1);
    labels.push_back(os.str());
  }

  Node root(Player::CHANCE_FIRST, nullptr, move(spStart), labels);

  for (int i = 0; i < FLAGS_numRounds; i++) {
    if (i % FLAGS_printEvery == 0) {
      setStrategyFromCumulativeStrategy(&root);
      cout << i << " value: " << root.computeValue(Player::FIRST) << " "
           << root.computeValue(Player::SECOND) << endl;
      setStrategyFromCumulativeRegret(&root);
    }
    if (i == FLAGS_clearAfter) {
      root.clearCumulativeStrategy();
    }
    runRound(&root);
  }
  setStrategyFromCumulativeStrategy(&root);

  cout << "value: " << root.computeValue(Player::FIRST) << " "
       << root.computeValue(Player::SECOND) << endl;

  writeToFile(&root, FLAGS_outputFile);

  setStrategyToCounterStrategy(&root, Player::FIRST);
  cout << "First player countering value: " << root.computeValue(Player::FIRST)
       << " " << root.computeValue(Player::SECOND) << endl;
  setStrategyFromCumulativeStrategy(&root);

  setStrategyToCounterStrategy(&root, Player::SECOND);
  cout << "Second player countering value: " << root.computeValue(Player::FIRST)
       << " " << root.computeValue(Player::SECOND) << endl;
  setStrategyFromCumulativeStrategy(&root);
}
