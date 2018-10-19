#pragma once

#include <list>
#include <map>
#include <string>

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

bool operator<(const TransitionFunctionKey& l,
	const TransitionFunctionKey& r);

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
	TransitionFunctionKey, TransitionFunctionValue>;
// -----------------------------------------------------------

// -----------------------------------------------------------
// Tape class
class Tape {
public:
	Tape();

	bool empty() const noexcept;
	const std::list<Symbol>& getContent() const noexcept;
	void moveHead(Shift shift);
	const Symbol& read() const noexcept;
	void write(const Symbol& symbol);

private:
	std::list<Symbol> tape_;
	std::list<Symbol>::iterator tapeHead_;
};
// -----------------------------------------------------------

// -----------------------------------------------------------
// Turing Machine class
class TuringMachine {
public:
	TuringMachine();
	TuringMachine(const State& initialState, 
		const TransitionFunction& transitionFunction);

	const Tape& getTape() const noexcept;
	bool didHalt() const noexcept;
	void iterate();

private:
	Tape tape_;
	State state_;
	TransitionFunction transitionFunction_;
};

// -----------------------------------------------------------


}