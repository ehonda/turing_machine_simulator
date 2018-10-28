#pragma once

#include <list>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace turing_machine_sim::turing_machine {

// -----------------------------------------------------------
// Transition function definitions
using State = std::string;
const State HALT = "HALT";

using Symbol = char;
constexpr Symbol BLANK = ' ';

struct TransitionFunctionKey {
	Symbol tapeSymbol;
	State currentState;
};

struct compare_tf_keys_less_than {
	bool operator()(const TransitionFunctionKey& l,
		const TransitionFunctionKey& r) const noexcept;
};

enum class Shift {
	L,
	R,
	N
};

struct TransitionFunctionValue {
	Symbol writeSymbol;
	State nextState;
	Shift shift;
};

using TransitionFunction = std::map<
	TransitionFunctionKey, TransitionFunctionValue, compare_tf_keys_less_than>;
// -----------------------------------------------------------

// -----------------------------------------------------------
// Tape class

// TODO: REFACTOR TAPE INTO OWN HEADER FILE
class TuringMachine;

class Tape {
public:
	friend std::ostream& operator<<(std::ostream&, const Tape&);
	friend std::ostream& operator<<(std::ostream&, const TuringMachine&);

	Tape();
	Tape(const std::list<Symbol>& input);

	bool empty() const noexcept;
	bool hasEqualContent(const Tape& other) const noexcept;

	void moveHead(Shift shift);
	const Symbol& read() const noexcept;
	void write(const Symbol& symbol);

private:
	using TapeIterator = std::list<Symbol>::iterator;
	using TapeConstIterator = std::list<Symbol>::const_iterator;

	std::pair<TapeConstIterator, TapeConstIterator>
		getFirstAndLastNonBlanks() const noexcept;

	std::list<Symbol> tape_;
	TapeIterator tapeHead_;
};

std::ostream& operator<<(std::ostream& os, const Tape& tape);
// -----------------------------------------------------------

// -----------------------------------------------------------
// Turing Machine class
class TuringMachine {
public:
	friend std::ostream& operator<<(std::ostream&, const TuringMachine&);

	TuringMachine();
	TuringMachine(const State& initialState, 
		const TransitionFunction& transitionFunction);
	TuringMachine(const State& initialState,
		const TransitionFunction& transitionFunction,
		const std::list<Symbol>& input);

	const Tape& getTape() const noexcept;
	std::string getAutotoolDescription() const;

	bool didHalt() const noexcept;
	void iterate();

private:
	Tape tape_;
	State startingState_;
	State state_;
	TransitionFunction transitionFunction_;
};

std::ostream& operator<<(std::ostream& os, const TuringMachine& tm);
TransitionFunction makeTransitionFunction(
	const std::vector<std::string>& rules);
TuringMachine loadTuringMachine(std::string_view path);
// -----------------------------------------------------------


}