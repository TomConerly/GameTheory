#include "play.h"

#include <cassert>
#include <iostream>
#include <random>

using namespace std;

static int genRandom(int low, int high) {
  static random_device rd;
  static default_random_engine e1(rd());
  uniform_int_distribution<int> ud(low, high);
  return ud(e1);
}

static double genRandom() {
  static random_device rd;
  static default_random_engine e1(rd());
  return generate_canonical<double, 64>(e1);
}

int playInstance(const Node* root) {
  if (genRandom(0, 1) == 0) {
    return playInstance(Player::FIRST, root);
  } else {
    return playInstance(Player::SECOND, root);
  }
}

static int play(Player human, const Node* at) {
  if (at->_children.size() == 0) {
    return human == Player::FIRST ? at->_firstPlayerUtility
                                  : at->_secondPlayerUtility;
  }
  if (isChance(at->_player)) {
    int option = genRandom(0, at->_children.size() - 1);
    if (at->_player == Player::CHANCE_PUBLIC) {
      cout << "Chance picked: " << at->_labels[option] << endl;
      return play(human, at->_children[option].get());
    } else if ((at->_player == Player::CHANCE_FIRST &&
                human == Player::FIRST) ||
               (at->_player == Player::CHANCE_SECOND &&
                human == Player::SECOND)) {
      cout << "You were dealt: " << at->_labels[option] << endl;
      return play(human, at->_children[option].get());
    } else {
      cout << "Your opponent was dealt something hidden." << endl;
      int utility = play(human, at->_children[option].get());
      cout << "Your opponent was dealt: " << at->_labels[option] << endl;
      return utility;
    }
  } else if (at->_player == human) {
    cout << "Your options are: ";
    for (int i = 0; i < at->_children.size(); i++) {
      cout << at->_labels[i];
      if (i + 1 < at->_children.size())
        cout << ", ";
    }
    cout << endl;
    string option;
    while (true) {
      cout << "Pick one: ";
      getline(cin, option);
      for (int i = 0; i < at->_children.size(); i++) {
        if (option == at->_labels[i]) {
          return play(human, at->_children[i].get());
        }
      }
      cout << "That wasn't one of the options. Please pick again." << endl;
    }
  } else {
    auto is = at->_informationSet;
    double rand = genRandom();
    for (int i = 0; i < is->_currentStrategy.size(); i++) {
      double prob = is->_currentStrategy[i];
      if (rand < prob) {
        cout << "Opponent played: " << at->_labels[i] << endl;
        return play(human, at->_children[i].get());
      } else {
        rand -= prob;
      }
    }
    assert(false);
  }
}

int playInstance(Player human, const Node* root) {
  int utility = play(human, root);
  cout << "Your utility was: " << utility << endl;
  return utility;
}
