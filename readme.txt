A library for doing linear algebra in C++.

The need for this library came about when working on the Tetris AI project I'm
currently doing.  There's a lot of matrix maths involved in training a neural
network and hence I started writing a matrix class.  The idea is to try and make
it as much like an STL container as possible so that all of the algorithms, 
etc... will work with it and to test my knowledge of the standard library.

I've tried hard to make sure things that can be constexpr, (conditionally)
noexcept, etc... are (where appropriate), because of this I found that I
couldn't use the standard algorithms in a lot of places that would have made the
code implementation easier (e.g. in operator== I couldn't use std::equal as
there are no conditions under which it is noexcept but I feel things could be
better than that).

For now it seems like a matrix class and a few associated functions are all I
need for now, the only things I can conceive of for the future are vectors but
these just seem to be an awkward special case of a matrix.
