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

	{
		ofstream fs("test.lex");
		lex.dump(fs);
	}

	return EXIT_SUCCESS;
}
