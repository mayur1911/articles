#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <chrono>
#include <fstream>
#include <string_view>
#include <sstream>
#include <numeric>
#include "simpleperf.h"

using namespace std::literals;

const std::string_view LoremIpsumStrv{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
"sed do eiusmod tempor incididuntsuperlongwordsuper ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
"quis nostrud exercitation ullamco laboris nisi ut aliquipsuperlongword ex ea commodo consequat. Duis aute "
"irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
"Excepteur sint occaecat cupidatatsuperlongword non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." };

int main(int argc, const char** argv)
{
	std::string testString{ LoremIpsumStrv };

	if (argc == 1)
	{
		std::cout << "searchers.exe filename iterations pattern_len\n";
		return 0;
	}

	if (argc > 1 && "nofile"s != argv[1])
	{
		std::ifstream inFile(argv[1]);

		std::stringstream strStream;
		strStream << inFile.rdbuf();
		testString = strStream.str();
	}

	std::cout << "string length: " << testString.length() << '\n';

	const size_t ITERS = argc > 2 ? atoi(argv[2]) : 1000;
	std::cout << "test iterations: " << ITERS << '\n';

	const size_t tempLen = argc > 3 ? atoi(argv[3]) : testString.length() / 4;
	const size_t PATTERN_LEN = tempLen > testString.length() ? testString.length()  : tempLen;
	std::cout << "pattern length: " << PATTERN_LEN << '\n';

	const auto needle = testString.substr(testString.length() - PATTERN_LEN - 1, PATTERN_LEN);

	RunAndMeasure("string::find", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::size_t found = testString.find(needle);
			if (found == std::string::npos)
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("default searcher", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testString.begin(), testString.end(),
				std::default_searcher(
					needle.begin(), needle.end()));
			if (it == testString.end())
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_searcher init only", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::boyer_moore_searcher b(needle.begin(), needle.end());
			DoNotOptimizeAway(&b);
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_searcher", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testString.begin(), testString.end(),
				std::boyer_moore_searcher(
					needle.begin(), needle.end()));
			if (it == testString.end())
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_horspool_searcher init only", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::boyer_moore_horspool_searcher b(needle.begin(), needle.end());
			DoNotOptimizeAway(&b);
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_horspool_searcher", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testString.begin(), testString.end(),
				std::boyer_moore_horspool_searcher(
					needle.begin(), needle.end()));
			if (it == testString.end())
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	std::vector<int> testVector(1000000);
	std::iota(testVector.begin(), testVector.end(), 0);
	std::vector vecNeedle(testVector.end() - 1000, testVector.end());

	RunAndMeasure("vector of ints default", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testVector.begin(), testVector.end(),
				std::default_searcher(
					vecNeedle.begin(), vecNeedle.end()));
			if (it == testVector.end())
				std::cout << "The pattern " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_horspool_searcher vec init only", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::boyer_moore_horspool_searcher b(vecNeedle.begin(), vecNeedle.end());
			DoNotOptimizeAway(&b);
		}
		return 0;
	});

	RunAndMeasure("vector of ints", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testVector.begin(), testVector.end(),
				std::boyer_moore_horspool_searcher(
					vecNeedle.begin(), vecNeedle.end()));
			if (it == testVector.end())
				std::cout << "The pattern " << needle << " not found\n";
		}
		return 0;
	});
}