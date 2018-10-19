#include <algorithm>
#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>

#include "boost/algorithm/string.hpp"

#include "gtest/gtest.h"

#include "turing_machine.h"

using namespace turing_machine_sim;
using namespace turing_machine;

namespace turing_machine_sim_test {

class TuringMachineTest : public testing::Test {
protected:
	Symbol parseSymbol(std::string_view symbolString) const {
		if (symbolString == "BLANK")
			return BLANK;
		else if (symbolString.size() == 1)
			return symbolString[0];
		
		throw std::invalid_argument("Invalid symbol string");
	}

	std::pair<TransitionFunctionKey, TransitionFunctionValue>
		makeRule(std::string_view rule) const 
	{
		const std::string ERROR = "Invalid rule string";

		// Preprocess string
		{
			const auto trim_pos = rule.find('(');
			if (trim_pos > rule.size())
				throw std::invalid_argument(ERROR);
			rule.remove_prefix(trim_pos + 1);
		}
		{
			const auto trim_pos = rule.find(')');
			if (trim_pos > rule.size())
				throw std::invalid_argument(ERROR);
			rule.remove_suffix(rule.size() - trim_pos);
		}

		// Extract rule tokens
		std::vector<std::string> ruleTokens;
		{
			std::string strRule(rule);
			boost::algorithm::split(ruleTokens, strRule, boost::is_any_of(","));
		}
		if (ruleTokens.size() != 5)
			throw std::invalid_argument(ERROR);
		std::transform(ruleTokens.begin(), ruleTokens.end(), ruleTokens.begin(),
			[](const std::string& s) { return boost::algorithm::trim_copy(s); });

		// Make key
		TransitionFunctionKey key;
		key.tapeSymbol = parseSymbol(ruleTokens[0]);
		key.currentState = ruleTokens[1];

		auto b = !boost::is_any_of("LRN")(ruleTokens[4][0]);

		// Make value
		if (ruleTokens[4].size() != 1
			|| !boost::is_any_of("LRN")(ruleTokens[4][0]))
		{
			throw std::invalid_argument(ERROR);
		}

		TransitionFunctionValue value;
		value.writeSymbol = parseSymbol(ruleTokens[2]);
		value.nextState = ruleTokens[3];
		value.shift = 
			ruleTokens[4][0] == 'L' ? Shift::L :
				(ruleTokens[4][0] == 'R' ? Shift::R : Shift::N);

		return { key, value };
	}

	TransitionFunction makeTransitionFunction(
		const std::vector<std::string_view>& rules) const
	{
		TransitionFunction f;
		for (auto rule : rules)
			f.insert(makeRule(rule));
		return f;
	}

	void expectTapeContent(const Tape& tape, const std::string& content) const {
		const std::list<Symbol> expectedContent(content.cbegin(), content.cend());
		EXPECT_EQ(tape.getContent(), expectedContent);
	}
};

TEST_F(TuringMachineTest, tape_of_all_blanks_is_empty) {
	Tape t;
	EXPECT_TRUE(t.empty());

	t.write(BLANK);
	t.moveHead(Shift::R);
	t.write(BLANK);
	EXPECT_TRUE(t.empty());
}

TEST_F(TuringMachineTest, reading_empty_tape_gives_blank) {
	Tape t;
	const auto value = t.read();
	EXPECT_EQ(value, BLANK);
}

TEST_F(TuringMachineTest, write_one_1_and_halt) {
	TuringMachine tm("s", makeTransitionFunction({ "(BLANK, s, 1, HALT, R)" }));
	
	while (!tm.didHalt())
		tm.iterate();

	expectTapeContent(tm.getTape(), "1");
}

TEST_F(TuringMachineTest, run_2_state_2_symbol_busy_beaver) {
	// Busy beaver from https://de.wikipedia.org/wiki/Flei%C3%9Figer_Biber#Flei%C3%9Figer_Biber_mit_2_Zust%C3%A4nden
	TuringMachine tm("q_0", makeTransitionFunction({
		"(BLANK,	q_0, 1, q_1, R)",
		"(1,		q_0, 1, q_1, L)",
		"(BLANK,	q_1, 1, q_0, L)",
		"(1,		q_1, 1, HALT, R)"
		}));

	// The busy beaver takes the tape through the following steps
	//		 _
	//		 ^
	//
	//		 1_
	//		  ^
	//
	//		 11
	//		 ^
	//
	//		 _11
	//		 ^
	//
	//		_111
	//		^
	//
	//		1111
	//		 ^
	//
	//		1111
	//		  ^

	const std::vector<std::string> expectedTapes
		= { "", "1", "11", "11", "111", "1111", "1111" };
	for (std::size_t i = 0; i < expectedTapes.size(); ++i) {
		expectTapeContent(tm.getTape(), expectedTapes[i]);
		tm.iterate();
	}

	expectTapeContent(tm.getTape(), "1111");

	/*const std::list<Symbol> expectedContent = { '1', '1', '1', '1' };
	EXPECT_EQ(tm.getTape().getContent(), expectedContent);*/
}

}