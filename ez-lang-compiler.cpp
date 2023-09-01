#include "compiler.h"
#include "compiler-visualize.hpp"

int main()
{
	std::string test;
	std::getline(std::cin, test);

	auto tokenlist = compiler::tokenizer::tokenize(test);

	size_t iter = 0;	
	
	auto parse = compiler::statement_parser::parse_if_branch(tokenlist, iter);

	return 0;
}