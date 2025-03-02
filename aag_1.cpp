#ifndef __PROGTEST__
#include "sample.h"

namespace regexp {

Alternation::Alternation(RegExp left, RegExp right)
    : m_left(std::move(left))
    , m_right(std::move(right))
{
}
Concatenation::Concatenation(RegExp left, RegExp right)
    : m_left(std::move(left))
    , m_right(std::move(right))
{
}
Iteration::Iteration(RegExp node)
    : m_node(std::move(node))
{
}
Symbol::Symbol(alphabet::Symbol symb)
    : m_symbol(std::move(symb))
{
}


#if 1
/** @brief Example usage of std::variant visitor using overloaded struct and lambda functions */
void to_string(const regexp::RegExp& r, std::ostream& os)
{
    std::visit(overloaded{
                   [&os](const std::shared_ptr<regexp::Alternation>& arg) { os << '('; to_string(arg->m_left, os); os << '+'; to_string(arg->m_right, os); os << ')'; },
                   [&os](const std::shared_ptr<regexp::Concatenation>& arg) { os << '('; to_string(arg->m_left, os); os << ' '; to_string(arg->m_right, os); os << ')'; },
                   [&os](const std::shared_ptr<regexp::Iteration>& arg) { os << '('; to_string(arg->m_node, os); os << ")*"; },
                   [&os](const std::shared_ptr<regexp::Symbol>& arg) { os << arg->m_symbol; },
                   [&os](const std::shared_ptr<regexp::Epsilon>& arg) { os << "#E"; },
                   [&os](const std::shared_ptr<regexp::Empty>& arg) { os << "#0"; },
               },
               r);
}
#else
/** @brief Example usage of std::visitor without overloaded struct with lambdas, using std::holds_alternative and std::get instead */
void to_string(const regexp::RegExp& r, std::ostream& os)
{
    if (std::holds_alternative<std::shared_ptr<regexp::Alternation>>(r)) {
        const auto& node = std::get<std::shared_ptr<regexp::Alternation>>(r);
        os << '(';
        to_string(node->m_left, os);
        os << ' ';
        to_string(node->m_right, os);
        os << ')';
    } else if (std::holds_alternative<std::shared_ptr<regexp::Concatenation>>(r)) {
        const auto& node = std::get<std::shared_ptr<regexp::Concatenation>>(r);
        os << '(';
        to_string(node->m_left, os);
        os << ' ';
        to_string(node->m_right, os);
        os << ')';
    } else if (std::holds_alternative<std::shared_ptr<regexp::Iteration>>(r)) {
        const auto& node = std::get<std::shared_ptr<regexp::Iteration>>(r);
        os << '(';
        to_string(node->m_node, os);
        os << ")*";
    } else if (std::holds_alternative<std::shared_ptr<regexp::Symbol>>(r)) {
        os << std::get<std::shared_ptr<regexp::Symbol>>(r)->m_symbol;
    } else if (std::holds_alternative<std::shared_ptr<regexp::Epsilon>>(r)) {
        os << "#E";
    } else if (std::holds_alternative<std::shared_ptr<regexp::Empty>>(r)) {
        os << "#0";
    } else {
        __builtin_unreachable();
    }
}
#endif

/** @brief Convenience function for converting RegExps into the ALT string format (usable in the ALT tool) */
std::ostream& operator<<(std::ostream& os, const regexp::RegExp& regexp)
{
    to_string(regexp, os);
    return os;
}
}

namespace automaton {
/** @brief Convenience function for converting NFAs into the ALT string format (usable in the ALT tool) */
std::ostream& operator<<(std::ostream& os, const automaton::NFA& nfa)
{
    os << "NFA ";
    for (const auto& symb : nfa.m_Alphabet)
        os << symb << " ";
    os << '\n';

    for (const auto& state : nfa.m_States) {
        os << (nfa.m_InitialState == state ? ">" : " ") << (nfa.m_FinalStates.count(state) ? "<" : " ") << state;

        for (const auto& symbol : nfa.m_Alphabet) {
            if (auto iter = nfa.m_Transitions.find({state, symbol}); iter != nfa.m_Transitions.end()) {
                os << " ";
                for (auto i = iter->second.begin(); i != iter->second.end(); ++i) {
                    os << (i != iter->second.begin() ? "|" : "") << *i;
                }
            } else {
                os << " -";
            }
        }
        os << std::endl;
    }
    return os;
}

bool operator==(const automaton::NFA& a, const automaton::NFA& b)
{
    return std::tie(a.m_States, a.m_Alphabet, a.m_Transitions, a.m_InitialState, a.m_FinalStates) == std::tie(b.m_States, b.m_Alphabet, b.m_Transitions, b.m_InitialState, b.m_FinalStates);
}
}
#endif

void mapping(const regexp::RegExp& r, std::map<regexp::Symbol *, int> &symToIdx, std::vector<alphabet::Symbol> &symbols) {
    std::visit(overloaded{
        [&symToIdx, &symbols](const std::shared_ptr<regexp::Alternation>& arg) { mapping(arg->m_left, symToIdx, symbols); mapping(arg->m_right, symToIdx, symbols); },
        [&symToIdx, &symbols](const std::shared_ptr<regexp::Concatenation>& arg) { mapping(arg->m_left, symToIdx, symbols); mapping(arg->m_right, symToIdx, symbols); },
        [&symToIdx, &symbols](const std::shared_ptr<regexp::Iteration>& arg) { mapping(arg->m_node, symToIdx, symbols); },
        [&symToIdx, &symbols](const std::shared_ptr<regexp::Symbol>& arg) { symToIdx[arg.get()] = (int)symbols.size(); symbols.push_back(arg->m_symbol); },
        [&symToIdx, &symbols](const std::shared_ptr<regexp::Epsilon>& arg) {  },
        [&symToIdx, &symbols](const std::shared_ptr<regexp::Empty>& arg) {  },
               },
               r);
}

int epsilonCheck(const regexp::RegExp& r) {
    return std::visit(overloaded{
        [](const std::shared_ptr<regexp::Alternation>& arg) { return (int)(epsilonCheck(arg->m_left) || epsilonCheck(arg->m_right)); },
        [](const std::shared_ptr<regexp::Concatenation>& arg) { return (int)(epsilonCheck(arg->m_left) && epsilonCheck(arg->m_right)); },
        [](const std::shared_ptr<regexp::Iteration>& arg) { return 1; },
        [](const std::shared_ptr<regexp::Symbol>& arg) { return 0; },
        [](const std::shared_ptr<regexp::Epsilon>& arg) { return 1; },
        [](const std::shared_ptr<regexp::Empty>& arg) { return 0; },
               },
               r);
}

std::set<int> getFirst(const regexp::RegExp& r, std::map<regexp::Symbol *, int> &symToIdx) {
    return std::visit(overloaded{
        [&symToIdx](const std::shared_ptr<regexp::Alternation>& arg) {
            auto l = getFirst(arg->m_left, symToIdx);
            auto r = getFirst(arg->m_right, symToIdx);
            l.insert(r.begin(), r.end());
            return l; },
        [&symToIdx](const std::shared_ptr<regexp::Concatenation>& arg) {
            auto l = getFirst(arg->m_left, symToIdx);
            auto r = getFirst(arg->m_right, symToIdx);
            if (epsilonCheck(arg->m_left)) {
                l.insert(r.begin(), r.end());
            }
            return l;
        },
        [&symToIdx](const std::shared_ptr<regexp::Iteration>& arg) { return getFirst(arg->m_node, symToIdx); },
        [&symToIdx](const std::shared_ptr<regexp::Symbol>& arg) { std::set<int> res; res.insert(symToIdx[arg.get()]); return res; },
        [&symToIdx](const std::shared_ptr<regexp::Epsilon>& arg) { return std::set<int>(); },
        [&symToIdx](const std::shared_ptr<regexp::Empty>& arg) { return std::set<int>(); },
               },
               r);
}

std::set<int> getLast(const regexp::RegExp& r, std::map<regexp::Symbol *, int> &symToIdx) {
    return std::visit(overloaded{
        [&symToIdx](const std::shared_ptr<regexp::Alternation>& arg) {
            auto l = getLast(arg->m_left, symToIdx);
            auto r = getLast(arg->m_right, symToIdx);
            l.insert(r.begin(), r.end());
            return l; },
        [&symToIdx](const std::shared_ptr<regexp::Concatenation>& arg) {
            auto l = getLast(arg->m_left, symToIdx);
            auto r = getLast(arg->m_right, symToIdx);
            if (epsilonCheck(arg->m_right)) {
                r.insert(l.begin(), l.end());
            }
            return r;
        },
        [&symToIdx](const std::shared_ptr<regexp::Iteration>& arg) { return getLast(arg->m_node, symToIdx); },
        [&symToIdx](const std::shared_ptr<regexp::Symbol>& arg) { std::set<int> res; res.insert(symToIdx[arg.get()]); return res; },
        [&symToIdx](const std::shared_ptr<regexp::Epsilon>& arg) { return std::set<int>(); },
        [&symToIdx](const std::shared_ptr<regexp::Empty>& arg) { return std::set<int>(); },
               },
               r);
}

std::set<std::pair<int, int>> makePairs(const std::set<int> &f, const std::set<int> &s) {
    std::set<std::pair<int, int>> res;
    for (auto &ff : f) {
        for (auto &ss : s) {
            res.insert({ff, ss});
        }
    }
    return res;
}

std::set<std::pair<int, int>> getNeighbours(const regexp::RegExp& r, std::map<regexp::Symbol *, int> &symToIdx) {
    return std::visit(overloaded{
        [&symToIdx](const std::shared_ptr<regexp::Alternation>& arg) {
            auto l = getNeighbours(arg->m_left, symToIdx);
            auto r = getNeighbours(arg->m_right, symToIdx);
            l.insert(r.begin(), r.end());
            return l;
        },
        [&symToIdx](const std::shared_ptr<regexp::Concatenation>& arg) {
            auto l = getNeighbours(arg->m_left, symToIdx);
            auto r = getNeighbours(arg->m_right, symToIdx);
            auto tmp = makePairs(getLast(arg->m_left, symToIdx), getFirst(arg->m_right, symToIdx));
            l.insert(tmp.begin(), tmp.end());
            l.insert(r.begin(), r.end());
            return l;
        },
        [&symToIdx](const std::shared_ptr<regexp::Iteration>& arg) {
            auto res = getNeighbours(arg->m_node, symToIdx);
            auto tmp = makePairs(getLast(arg->m_node, symToIdx), getFirst(arg->m_node, symToIdx));
            res.insert(tmp.begin(), tmp.end());
            return res;
        },
        [&symToIdx](const std::shared_ptr<regexp::Symbol>& arg) { return std::set<std::pair<int, int>>(); },
        [&symToIdx](const std::shared_ptr<regexp::Epsilon>& arg) { return std::set<std::pair<int, int>>(); },
        [&symToIdx](const std::shared_ptr<regexp::Empty>& arg) { return std::set<std::pair<int, int>>(); },
               },
               r);
}



automaton::NFA convert(const regexp::RegExp& regexp)
{
    std::map<regexp::Symbol *, int> symToIdx;
    std::vector<alphabet::Symbol> symbols;

    mapping(regexp, symToIdx, symbols);

    auto startSet = getFirst(regexp, symToIdx);
    auto endSet = getLast(regexp, symToIdx);
    auto neighbours = getNeighbours(regexp, symToIdx);

    automaton::NFA ans;
    for (int i = 0; i < (int)symbols.size(); ++i) {
        ans.m_States.insert(i + 1);
    }
    ans.m_States.insert(0);

//    std::set<State> m_States;
//    std::set<alphabet::Symbol> m_Alphabet;

//    std::map<std::pair<State, alphabet::Symbol>, std::set<State>> m_Transitions;
//    State m_InitialState;
//    std::set<State> m_FinalStates;

    for (auto & ch : symbols) {
        ans.m_Alphabet.insert(ch);
    }

    for (auto &it : startSet) {
        ans.m_Transitions[{0, symbols[it]}].insert(it + 1);
    }

    for (auto &it : neighbours) {
        ans.m_Transitions[{it.first + 1, symbols[it.second]}].insert(it.second + 1);
    }

    ans.m_InitialState = 0;

    for (auto &it : endSet) {
        ans.m_FinalStates.insert(it + 1);
    }

    if (epsilonCheck(regexp)) {
        ans.m_FinalStates.insert(0);
    }

    return ans;
}

#ifndef __PROGTEST__
regexp::RegExp tests[] = {
    std::make_shared<regexp::Iteration>(
        std::make_shared<regexp::Concatenation>(
            std::make_shared<regexp::Iteration>(
                std::make_shared<regexp::Alternation>(
                    std::make_shared<regexp::Symbol>('a'),
                    std::make_shared<regexp::Symbol>('b'))),
            std::make_shared<regexp::Concatenation>(
                std::make_shared<regexp::Symbol>('a'),
                std::make_shared<regexp::Concatenation>(
                    std::make_shared<regexp::Symbol>('b'),
                    std::make_shared<regexp::Iteration>(
                        std::make_shared<regexp::Alternation>(
                            std::make_shared<regexp::Symbol>('a'),
                            std::make_shared<regexp::Symbol>('b'))))))),

    std::make_shared<regexp::Iteration>(
        std::make_shared<regexp::Alternation>(
            std::make_shared<regexp::Alternation>(
                std::make_shared<regexp::Symbol>('a'),
                std::make_shared<regexp::Epsilon>()),
            std::make_shared<regexp::Alternation>(
                std::make_shared<regexp::Symbol>('b'),
                std::make_shared<regexp::Empty>()))),

    std::make_shared<regexp::Concatenation>(
        std::make_shared<regexp::Iteration>(
            std::make_shared<regexp::Concatenation>(
                std::make_shared<regexp::Symbol>('a'),
                std::make_shared<regexp::Concatenation>(
                    std::make_shared<regexp::Alternation>(
                        std::make_shared<regexp::Empty>(),
                        std::make_shared<regexp::Epsilon>()),
                    std::make_shared<regexp::Concatenation>(
                        std::make_shared<regexp::Iteration>(
                            std::make_shared<regexp::Symbol>('b')),
                        std::make_shared<regexp::Symbol>('a'))))),
        std::make_shared<regexp::Alternation>(
            std::make_shared<regexp::Concatenation>(
                std::make_shared<regexp::Symbol>('b'),
                std::make_shared<regexp::Iteration>(
                    std::make_shared<regexp::Symbol>('c'))),
            std::make_shared<regexp::Concatenation>(
                std::make_shared<regexp::Iteration>(
                    std::make_shared<regexp::Symbol>('a')),
                std::make_shared<regexp::Alternation>(
                    std::make_shared<regexp::Epsilon>(),
                    std::make_shared<regexp::Concatenation>(
                        std::make_shared<regexp::Symbol>('b'),
                        std::make_shared<regexp::Empty>()))))),
};

// Your solution might give another (equivalent) answers. See task details.
automaton::NFA results[] = {
    {{0, 1, 2, 3, 4, 5, 6},
     {'a', 'b'},
     {
         {{0, 'a'}, {1, 3}},
         {{0, 'b'}, {2}},
         {{1, 'a'}, {1, 3}},
         {{1, 'b'}, {2}},
         {{2, 'a'}, {1, 3}},
         {{2, 'b'}, {2}},
         {{3, 'b'}, {4}},
         {{4, 'a'}, {1, 3, 5}},
         {{4, 'b'}, {2, 6}},
         {{5, 'a'}, {1, 3, 5}},
         {{5, 'b'}, {2, 6}},
         {{6, 'a'}, {1, 3, 5}},
         {{6, 'b'}, {2, 6}},
     },
     0,
     {0, 4, 5, 6}},

    {{0, 1, 2},
     {'a', 'b'},
     {
         {{0, 'a'}, {1}},
         {{0, 'b'}, {2}},
         {{1, 'a'}, {1}},
         {{1, 'b'}, {2}},
         {{2, 'a'}, {1}},
         {{2, 'b'}, {2}},
     },
     0,
     {0, 1, 2}},

    {{0, 1, 2, 3, 4, 5, 6, 7},
     {'a', 'b', 'c'},
     {
         {{0, 'a'}, {1, 6}},
         {{0, 'b'}, {4, 7}},
         {{1, 'a'}, {3}},
         {{1, 'b'}, {2}},
         {{2, 'a'}, {3}},
         {{2, 'b'}, {2}},
         {{3, 'a'}, {1, 6}},
         {{3, 'b'}, {4, 7}},
         {{4, 'c'}, {5}},
         {{5, 'c'}, {5}},
         {{6, 'a'}, {6}},
         {{6, 'b'}, {7}},
     },
     0,
     {0, 3, 4, 5, 6}},
};

int main()
{
    assert(convert(tests[0]) == results[0]);
    assert(convert(tests[1]) == results[1]);
    assert(convert(tests[2]) == results[2]);
}
#endif
