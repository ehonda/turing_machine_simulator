#include <algorithm>
#include <iostream>
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
	void expectTapeContent(const Tape& tape, const std::string& content) const {
		const std::list<Symbol> expectedContent(content.cbegin(), content.cend());
		const Tape expectedTape(expectedContent);
		EXPECT_TRUE(tape.hasEqualContent(expectedTape));
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

TEST_F(TuringMachineTest, tape_content_equality_test) {
	// Trailing/Leading blank
	Tape t({ BLANK, '1', BLANK, '1', BLANK });
	Tape s({ '1', BLANK, '1' });
	EXPECT_TRUE(t.hasEqualContent(s));
	EXPECT_TRUE(s.hasEqualContent(t));

	// Equal (subrange)content but one tape is longer
	t = Tape({ '1', BLANK, '1', '1' });
	s = Tape({ '1', BLANK, '1' });
	EXPECT_FALSE(t.hasEqualContent(s));
	EXPECT_FALSE(s.hasEqualContent(t));
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
}

TEST_F(TuringMachineTest, binary_to_unary_converter) {
	TuringMachine tm("i", makeTransitionFunction({
		"(0, i, 0, i, R)",
		"(1, i, 1, i, R)",
		"(BLANK, i, x, w_b, L)",

		"(0, w_b, 1, w_b, L)",
		"(1, w_b, 0, g_u, R)",
		"(BLANK, w_b, BLANK, c, R)",
		"(1, g_u, 1, g_u, R)",
		"(x, g_u, x, w_u, R)",

		"(1, w_u, 1, w_u, R)",
		"(BLANK, w_u, 1, g_b, L)",
		"(1, g_b, 1, g_b, L)",
		"(x, g_b, x, w_b, L)",

		"(1, c, BLANK, c, R)",
		"(x, c, BLANK, HALT, R)"
		}),
		{'1', '0', '1'});

	while (!tm.didHalt()) {
		//std::cout << tm.getTape() << std::endl;
		std::cout << tm << std::endl;
		tm.iterate();
	}
	//std::cout << tm.getTape() << std::endl;
	std::cout << tm << std::endl;

}

}