#include "turing_machine.h"

#include <algorithm>
#include <iterator>

namespace turing_machine_sim::turing_machine {

// -----------------------------------------------------------
// Transition function definitions
bool compare_tf_keys_less_than::operator()(
	const TransitionFunctionKey& l, 
	const TransitionFunctionKey& r) const noexcept
{
	return std::tie(l.tapeSymbol, l.currentState)
		< std::tie(r.tapeSymbol, r.currentState);
}
// -----------------------------------------------------------


// -----------------------------------------------------------
// Tape class
Tape::Tape()
	: Tape{ { } }
{	
}

Tape::Tape(const std::list<Symbol>& input)
	: tape_(input)
{
	tapeHead_ = tape_.begin();
}

bool Tape::empty() const noexcept {
	return tape_.size() == 0
		|| std::all_of(tape_.cbegin(), tape_.cend(),
			[](const Symbol& s) { return s == BLANK; });
}

bool Tape::hasEqualContent(const Tape& other) const noexcept {
	auto [start, end] = getFirstAndLastNonBlanks();
	auto [startOther, endOther] = other.getFirstAndLastNonBlanks();
	return std::equal(start, end, startOther, endOther);
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

std::pair<Tape::TapeConstIterator, Tape::TapeConstIterator>
	Tape::getFirstAndLastNonBlanks() const noexcept
{
	const auto predNotBlank
		= [](const Symbol& s) { return s != BLANK; };

	const auto start = std::find_if(tape_.cbegin(),
		tape_.cend(), predNotBlank);
	const auto end = std::find_if(tape_.crbegin(),
		tape_.crend(), predNotBlank).base();

	return { start, end };
}

std::ostream& operator<<(std::ostream& os, const Tape& tape) {
	auto [start, end] = tape.getFirstAndLastNonBlanks();
	//std::copy(start, end, std::back_inserter(os));
	std::copy(start, end, std::ostream_iterator<char>(os, ""));
	return os;
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

		// Check if there is a rule specified for the tape
		// configuration and the machines state and halt
		// if there isnt
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

std::ostream& operator<<(std::ostream& os, const TuringMachine& tm)
{
	return os;
	// TODO: insert return statement here
}

}
