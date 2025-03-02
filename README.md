# RegEx
Implementation of a conversion of regular expressions into finite automata

Regular expressions and finite automata both express the same set of languages, the regular languages. These two notions are equivalent regarding to the computational power. There are algorithms for converting regular expressions into finite automata that accepts exactly the same language as the original regular expressions. There are also algorithms for the conversions in the opposite direction.

Nondeterministic finite automaton (NFA) is defined according to the lectures. It will be represented as a structure automaton::NFA in your program. The struct consists of the following fields:

the set of states (automaton::NFA::m_States), one initial state (automaton::NFA::m_InitialState) and the set of final states (automaton::NFA::m_FinalStates),
the alphabet (automaton::NFA::m_Alphabet) a
the set of transitions (transition function; automaton::NFA::m_Transitions).
Struct automaton::NFA does not validate the automaton in any way according to the NFA definition. It is your responsibility to cover all the constraints stated in the definition (nonempty set of states, valid initial state, valid transition functions, ...). An invalid automaton is going to be rejected by the tester and your result will be flagged as invalid output.

Individual regular expressions are represented by their own classes:

regexp::Empty represents an empty regular expression (∅),
regexp::Epsilon represents an empty word (ε),
regexp::Symbol represents one symbol of the regular expression,
regexp::Alternation represents an alternation of two regular expressions using binary operator +, there are valid regular expressions on the both left hand side and right hand side of the operator,
regexp::Concatenation represents an concatenation of two regular expressions using binary operator ., there are valid regular expressions on the both left hand side and right hand side of the operator,
regexp::Iteration represents an iteration of a regular expression using unary operator *, the child is a valid regular expression.
And finally, the regular expression itself is represented as regexp::RegExp, which is an std::variant containing the six forementioned classes wrapped in an std::shared_ptr for implementation reasons.
