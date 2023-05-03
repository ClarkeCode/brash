#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <map>
#include <sstream>
#include <iomanip>

struct ProgramInput {
	bool showHelp;
	std::string helpMessage;
	bool inputError;
	std::string errorMessage;
	
	bool modeLexing;
	bool modeCompilation;
	bool modeDisassembly;
	bool modeCompAndDisassemble;
	bool modeInterpret;

	bool behaviourSilent;

	std::string infile;
	std::string outfile;

	struct ProgramOption {
		std::string description;
		std::string missingInputText;
		std::function<bool()> processOption;
	};

	ProgramInput(int argc, char* argv[]) {
		std::map<std::string, ProgramOption> options = {
			{
				"-l", {"Tokenize specified file", "No file specified", [&,this]() { if (argc == 0) return false; modeLexing = true; infile = argv[0]; argc--; argv++; return true; }}
			},
			{
				"-c", {"Compile specified file", "No file specified", [&,this]() { if (argc == 0) return false; modeCompilation = true; infile = argv[0]; argc--; argv++; return true; }}
			},
			{
				"-o", {"Direct output to specified file", "No file specified", [&,this]() { if (argc == 0) return false; outfile = argv[0]; argc--; argv++; return true; }}
			},
			{
				"-d", {"Compile specified file and immediately disassemble", "No file specified", [&,this]() { if (argc == 0) return false; modeCompAndDisassemble = true; infile = argv[0]; argc--; argv++; return true; }}
			},
			{
				"-D", {"Disassemble specified Brash object file", "No file specified", [&,this]() { if (argc == 0) return false; modeCompilation = true; infile = argv[0]; argc--; argv++; return true; }}
			},
			{
				"-s", {"Silence; disable standard output", "", [&,this]() { behaviourSilent = true; return true; }}
			},
			{
				"-h", {"Show help message", "", [&,this]() { showHelp = true; return true; }}
			}
		};

		std::stringstream ss;
		ss << "Usage: brash [options]" << std::endl << std::endl << "Options:" << std::endl;
		for (decltype(options)::iterator it = options.begin(); it != options.end(); it++) {
			ss << std::setw(6) << it->first << std::string(4, ' ') << it->second.description << std::endl;
		}
		helpMessage = ss.str();

		argc--; argv++;
		while (argc > 0) {
			std::string currentArg = argv[0];
			if (options.find(currentArg) != options.end()) {
				argc--; argv++;
				if (!options[currentArg].processOption()) {
					inputError = true;
					errorMessage = options[currentArg].missingInputText;
					break;
				}
			}
		}
	}
};

int main(int argc, char* argv[]) {
	ProgramInput pinput(argc, argv);
	if (pinput.showHelp) {
		std::cout << pinput.helpMessage;
		return EXIT_SUCCESS;
	}
	if (pinput.inputError) {
		std::cout << pinput.errorMessage;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
