#include "turing_machine.h"

#include <algorithm>

namespace turing_machine_sim::turing_machine {

// -----------------------------------------------------------
// Transition function definitions
bool operator<(const TransitionFunctionKey& l,
	const TransitionFunctionKey& r)
{
	return std::tie(l.tapeSymbol, l.currentState)
		< std::tie(r.tapeSymbol, r.currentState);
}
// -----------------------------------------------------------


// -----------------------------------------------------------
// Tape class
Tape::Tape() {
	tapeHead_ = tape_.begin();
}

bool Tape::empty() const noexcept {
	return tape_.size() == 0
		|| std::all_of(tape_.cbegin(), tape_.cend(),
			[](const Symbol& s) { return s == BLANK; });
}

const std::list<Symbol>& Tape::getContent() const noexcept {
	return tape_;
}

void Tape::moveHead(Shift shift) {
	switch (shift) {
	case Shift::L:
		if (tapeHead_ == tape_.begin())
			tapeHead_ = tape_.insert(tapeHead_, BLANK);
		else
			--tapeHead_;
		break;

	case Shift::R:
		if (tapeHead_ == tape_.end())
			tapeHead_ = tape_.insert(tapeHead_, BLANK);
		else
			++tapeHead_;
		break;
	}
}

const Symbol& Tape::read() const noexcept {
	if (tapeHead_ == tape_.end())
		return BLANK;
	return *tapeHead_;
}

void Tape::write(const Symbol& symbol) {
	if (tapeHead_ == tape_.end())
		tapeHead_ = tape_.insert(tapeHead_, symbol);
	else
		*tapeHead_ = symbol;
}
// -----------------------------------------------------------


// -----------------------------------------------------------
// Turing Machine class
TuringMachine::TuringMachine()
	: TuringMachine(HALT, {})
{
}

TuringMachine::TuringMachine(const State& initialState, 
	const TransitionFunction& transitionFunction)
	: state_(initialState), transitionFunction_(transitionFunction)
{
}

const Tape& TuringMachine::getTape() const noexcept {
	return tape_;
}

bool TuringMachine::didHalt() const noexcept {
	return state_ == HALT;
}

void TuringMachine::iterate() {
	if (!didHalt()) {
		TransitionFunctionKey key = { tape_.read(), state_ };
		const auto keyValueIt = transitionFunction_.find(key);
		if (keyValueIt != transitionFunction_.cend()) {
			const auto value = keyValueIt->second;
			tape_.write(value.writeSymbol);
			tape_.moveHead(value.shift);
			state_ = value.nextState;
		}
		else {
			state_ = HALT;
		}
	}
}

// -----------------------------------------------------------

}
