#include <iostream>
#include <string>
#include <dirent.h>
#include <vector>
#include <limits>
#include "CPU.h"

std::string getGamePath(void) {
  std::string path;
  std::cout << "Enter game path: ";
  std::FILE *game = NULL;
  std::cin.clear();
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  while (game == NULL) {
    std::cin.clear();
    std::getline(std::cin, path);
    game = std::fopen(path.c_str(), "rb");
    if (game == NULL) {
      std::cout << path << " doesn't exist\n";
    }
  }
  std::fclose(game);
  return path;
}

int runGame(std::string path) {
  Chip8 emulator = Chip8();
  bool loaded = emulator.loadGame(path.c_str());
  if (loaded) {
    emulator.runEmu();
  } else {
    std::cout << "Couldn't load " << path << "\n";
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc > 1) {
    return runGame(argv[1]);
  }
  DIR *root = opendir("./");
  if (root == NULL) {
    std::cout << "Couldn't search the current directory\n";
    return runGame(getGamePath());
  }
  std::cout << "Searching for games...\n";
  struct dirent *entry;
  std::vector<std::string> found_games;
  while ((entry = readdir(root)) != NULL) {
    std::string path = entry->d_name;
    int index = path.rfind(".");
    std::string extension = path.substr(index + 1);
    if (extension == "c8" || extension == "ch8") {
      found_games.push_back(path);
    }
  }
  if (found_games.size() == 0) {
    std::cout << "No games found\n";
    return runGame(getGamePath());
  }
  std::cout << "Games found:\n";
  int i = 1;
  for (auto &game : found_games) {
    std::cout << (i++) << ". " << game << "\n";
  }
  std::cout << "Choose a game (1 - " << i - 1 << ") or press enter to input a game path\n";
  int input;
  std::cin >> input;
  if (input >= 1 && input < i) {
    return runGame(found_games[input - 1]);
  }
  return runGame(getGamePath());
}