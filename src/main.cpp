#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "lexer.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	Lexer lex("test.txt");
	lex.process();

	for (auto tk : lex.tokenSequence) {
		cout << tk << endl;
	}

	return EXIT_SUCCESS;
}
