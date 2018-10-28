#include "turing_machine.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <sstream>

#include "boost/algorithm/string.hpp"

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
	// First output row: Tape content
	auto [start, end] = tape.getFirstAndLastNonBlanks();
	std::copy(start, end, std::ostream_iterator<Symbol>(os, ""));
	os << std::endl;

	// Second output row: Head and state

	return os;
}
// -----------------------------------------------------------


// -----------------------------------------------------------
// Turing Machine class
TuringMachine::TuringMachine()
	: TuringMachine(HALT, {}, {})
{
}

TuringMachine::TuringMachine(const State& initialState, 
	const TransitionFunction& transitionFunction)
	: TuringMachine(initialState, transitionFunction, {})
{
}

TuringMachine::TuringMachine(const State& initialState, 
	const TransitionFunction& transitionFunction, 
	const std::list<Symbol>& input)
	: startingState_(initialState), state_(initialState), 
	transitionFunction_(transitionFunction), tape_(input)
{
}

const Tape& TuringMachine::getTape() const noexcept {
	return tape_;
}

// Autotool description helpers
namespace {

std::set<Symbol> getSymbols(const TransitionFunction& tf) {
	std::set<Symbol> symbols;
	for (const auto& rule : tf) {
		symbols.insert(rule.first.tapeSymbol);
		symbols.insert(rule.second.writeSymbol);
	}
	return symbols;
}

std::set<Symbol> getInputAlphabet(const std::set<Symbol>& alphabet) {
	std::set<Symbol> inputAlphabet;
	for (const auto& symbol : alphabet)
		if (boost::algorithm::is_any_of(".01")(symbol))
			inputAlphabet.insert(symbol);
	return inputAlphabet;
}

void appendSymbol(std::stringstream& desc, const Symbol& s) {
	const Symbol BLANK_SUB = '#';
	if (s == BLANK)
		desc << BLANK_SUB;
	else
		desc << s;
}

using StateMap = std::map<State, std::size_t>;

StateMap getStatesSubsitutedByNumbers(const TransitionFunction& tf) {
	std::set<State> states;
	for (const auto& rule : tf) {
		states.insert(rule.first.currentState);
		states.insert(rule.second.nextState);
	}

	StateMap stateMap;
	{
		std::size_t i = 0;
		for (const auto& state : states)
			stateMap.insert({ state, i++ });
	}
	return stateMap;
}

void appendState(std::stringstream& desc, 
	const State& state, 
	const StateMap& stateMap) 
{
	desc << stateMap.at(state);
}

void appendRule(std::stringstream& desc,
	const TransitionFunction::value_type& rule,
	const StateMap& stateMap)
{
	desc << "('";
	appendSymbol(desc, rule.first.tapeSymbol);
	desc << "', ";
	appendState(desc, rule.first.currentState, stateMap);
	desc << ", '";
	appendSymbol(desc, rule.second.writeSymbol);
	desc << "', ";
	appendState(desc, rule.second.nextState, stateMap);
	desc << ", ";
	switch (rule.second.shift) {
	case Shift::R:
		desc << "R";
		break;

	case Shift::L:
		desc << "L";
		break;

	case Shift::N:
		desc << "N";
		break;
	}
	desc << ")";
}

}

std::string TuringMachine::getAutotoolDescription() const {
	std::stringstream desc;
	desc << "Turing\n\t{ ";

	const auto symbols = getSymbols(transitionFunction_);

	// Input alphabet
	desc << "eingabealphabet = mkSet\n\t\t\"";
	{
		const auto inputAlphabet = getInputAlphabet(symbols);
		for (const auto& s : inputAlphabet)
			appendSymbol(desc, s);
	}
	desc << "\"\n\t, ";

	// Working alphabet
	desc << "arbeitsalphabet = mkSet\n\t\t\"";
	for (const auto& s : symbols)
		appendSymbol(desc, s);
	desc << "\"\n\t, ";

	// Blank
	desc << "leerzeichen = '";
	appendSymbol(desc, BLANK);
	desc << "'\n\t, ";


	const auto stateMap = getStatesSubsitutedByNumbers(transitionFunction_);
	
	// State set
	desc << "zustandsmenge = mkSet\n\t\t[";
	for (std::size_t i = 0; i < stateMap.size() - 1; ++i)
		desc << i << ", ";
	desc << stateMap.size() - 1;
	desc << "]\n\t, ";

	// Transition Function
	desc << "tafel = collect [\n\t\t";
	{
		std::size_t counter = 0;
		for (const auto& rule : transitionFunction_) {
			appendRule(desc, rule, stateMap);
			if (counter++ != transitionFunction_.size() - 1)
				desc << ",\n\t\t";
		}
	}
	desc << "]\n\t, ";

	// Starting state
	desc << "startzustand = ";
	appendState(desc, startingState_, stateMap);
	desc << "\n\t,";

	// End state
	desc << " endzustandsmenge = mkSet\n\t\t";
	desc << "[";
	appendState(desc, HALT, stateMap);
	desc << "]\n\t}";

	return desc.str();
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

// Helpers for operator<<
namespace {

void pad(std::ostream& os, std::size_t length) {
	for (std::size_t i = 0; i < length; ++i)
		os << " ";
}

}

std::ostream& operator<<(std::ostream& os, const TuringMachine& tm) {
	// First output row: Tape content
	const auto& tape = tm.tape_;
	std::copy(tape.tape_.cbegin(), tape.tape_.cend(),
		std::ostream_iterator<Symbol>(os, ""));
	os << std::endl;

	// Second output row: Head and state
	const auto distHeadToStart
		= std::distance(tape.tape_.cbegin(), Tape::TapeConstIterator(tape.tapeHead_));
	const auto headPos = std::max(0, static_cast<int>(distHeadToStart));
	pad(os, headPos);
	os << "^" << tm.state_;

	return os;
}

// Helpers for makeTransitionFunction
namespace {

Symbol parseSymbol(std::string_view symbolString) {
	if (symbolString == "BLANK")
		return BLANK;
	else if (symbolString.size() == 1)
		return symbolString[0];

	throw std::invalid_argument("Invalid symbol string");
}

std::pair<TransitionFunctionKey, TransitionFunctionValue>
	makeRule(std::string_view rule)
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

}

TransitionFunction makeTransitionFunction(
	const std::vector<std::string>& rules)
{
	TransitionFunction f;
	for (auto rule : rules)
		f.insert(makeRule(rule));
	return f;
}

TuringMachine loadTuringMachine(std::string_view path) {
	// Open file if existant
	if (!std::filesystem::exists(path))
		throw std::runtime_error("loadTuringMachine: Path does not exist.");
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("loadTuringMachine: Could not open file.");

	// Extract content line by line
	std::vector<std::string> lines;
	{
		std::string currentLine;
		while (std::getline(file, currentLine))
			// Lines starting with # are comments
			if (!currentLine.empty() && currentLine.front() != '#')
				lines.push_back(currentLine);
	}

	// First line is starting state,
	// second line initial config
	// rest are rules and parsed
	// to make the transition table
	const auto startingState = lines.front();
	lines.erase(lines.begin());

	const auto inputString = lines.front();
	std::list<Symbol> input(inputString.cbegin(), inputString.cend());
	lines.erase(lines.begin());

	const auto transitionFunction = makeTransitionFunction(lines);

	return TuringMachine(startingState, transitionFunction, input);
}
// -----------------------------------------------------------

}
