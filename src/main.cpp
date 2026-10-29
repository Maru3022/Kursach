#include "interface.h"
#include <cstdlib>
#include <ctime>
int main() { std::srand(static_cast<unsigned>(std::time(nullptr))); return runInterface(); }
