A library for doing linear algebra in C++.

The need for this library came about when working on the Tetris AI project I'm
currently doing.  There's a lot of matrix maths involved in training a neural
network and hence I started writing a matrix class.  The idea is to try and make
it as much like an STL container as possible so that all of the algorithms, 
etc... will work with it and to test my knowledge of the standard library.  At
the moment, other common linear algebra constructs like row and column vectors
are just specialisations of the matrix class, this can make the interface a
little awkward but the implementation stays very simple.  This may be something
I change in the future however.

I'm also trying hard to make sure things that can be constexpr, noexcept
(conditionally), no_discard, etc... where appropriate, because of this I found
that I couldn't use the standard algorithms in a lot of places that would have
made the code implenentation easier (e.g. in operator== I couldn't use
std::equal as there are no conditions under which it is noexcept but I feel
things could be better than that).

The next step is to write unit tests in Catch as unit testing is something I
want to practice more and to ensure any use of the library is as correct as
possible then there should be as many tests as possible trying to break the
implementation.
