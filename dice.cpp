#include "cfr.h"
#include "play.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

static constexpr int diceSides = 6;

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

vector<Node*> makeNodes(Player toAct, vector<Bid>& bids) {
  if (bids.size() > 0 && bids[bids.size() - 1].count == -1) {
    Bid lastBid = bids[bids.size() - 2];
    vector<Node*> res;
    for (int i = 0; i < diceSides * diceSides; i++) {
      int fpDie = i % diceSides;
      int spDie = i / diceSides;
      if ((toAct == Player::FIRST) == satisfied(lastBid, {fpDie, spDie})) {
        res.push_back(new Node(1, -1));
      } else {
        res.push_back(new Node(-1, 1));
      }
    }
    return res;
  }
  vector<vector<Node*>> children(diceSides * diceSides);
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
    auto nodes = makeNodes(toAct == Player::FIRST ? Player::SECOND : Player::FIRST, bids);
    bids.pop_back();
    for (int i = 0; i < diceSides * diceSides; i++) {
      children[i].push_back(nodes[i]);
    }
    labels.push_back(bid.toString());
  }
  vector<Node*> res;
  vector<InformationSet*> is;
  for (int i = 0; i < diceSides; i++)
    is.push_back(new InformationSet(children[0].size()));

  for (int i = 0; i < diceSides * diceSides; i++) {
    int isIdx = toAct == Player::FIRST ? i % diceSides : i / diceSides;
    res.push_back(new Node(toAct, is[isIdx], children[i], labels));
  }
  return res;
}

int main() {
  for (int count = 1; count <= 2; count++) {
    for (int value = 0; value < diceSides; value++) {
      allBids.push_back(Bid(count, value));
    }
  }
  allBids.push_back(Bid(-1, -1));

  vector<Bid> bids;
  auto start = makeNodes(Player::FIRST, bids);

  vector<Node*> spStart;
  for (int i = 0; i < diceSides; i++) {
    vector<Node*> children;
    vector<string> labels;
    for (int j = 0; j < diceSides; j++) {
      children.push_back(start[i + j * 6]);
      ostringstream os;
      os << "SP rolled: " << (j+1);
      labels.push_back(os.str());
    }
    spStart.push_back(new Node(Player::CHANCE_SECOND, nullptr, children, labels));
  }

  vector<string> labels;
  for (int i = 0; i < diceSides; i++) {
      ostringstream os;
      os << "FP rolled: " << (i+1);
      labels.push_back(os.str());
  }

  Node root(Player::CHANCE_FIRST, nullptr, spStart, labels);

  for (int i = 0; i < 2000; i++) {
    if (i % 25 == 0) {
      cout << i << " value: " << root.computeValue(Player::FIRST) << " "
           << root.computeValue(Player::SECOND) << endl;
    }
    if (i == 1000) {
      root.clearCumulativeStrategy();
    }
    runRound(&root);
  }

  cout << "value: " << root.computeValue(Player::FIRST) << " "
    << root.computeValue(Player::SECOND) << endl;

  cout << endl << endl << endl;
  while (true) {
    playInstance(&root);
    cout << endl;
  }
}
