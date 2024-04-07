#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "lexer.hpp"
#include "compiler.hpp"

#include "util.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	Lexer lex("test.txt");
	lex.process();

	{
		ofstream fs("test.lex");
		lex.dump(fs);
	}

	Compiler comp(lex);
	comp.process();

	comp.dump(cout);

	auto bb = makeNode<int>(5);
	bb->hangLeft(4);
	bb->hangRight(6);
	bb = bb->supplantLeft(2);
	bb->supplantRight(99)->getRoot()->toGraphviz(cout);
	bb->getRoot()->deleteNodes();

	return EXIT_SUCCESS;
}
