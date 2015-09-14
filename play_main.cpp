#include <gflags/gflags.h>

#include "cfr.h"
#include "play.h"

using namespace std;

DEFINE_string(file, "", "The .game file to play");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_file == "") {
    cout << "--file is required" << endl;
    exit(-1);
  }

  auto root = readFromFile(FLAGS_file);
  cout << "value: " << root->computeValue(Player::FIRST) << " "
       << root->computeValue(Player::SECOND) << endl;

  while (true) {
    playInstance(root.get());
    cout << endl;
  }
}
