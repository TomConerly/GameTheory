#include "cfr.h"
#include "play.h"

using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    return 1;
  }
  auto root = readFromFile(argv[1]);
  cout << "value: " << root->computeValue(Player::FIRST) << " "
       << root->computeValue(Player::SECOND) << endl;

  while (true) {
    playInstance(root.get());
    cout << endl;
  }
}
