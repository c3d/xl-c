# XL
### An extensible programming language

XL is a very simple and small programming language specifically
designed to be integrated in larger projects.

XL is also a somewhat special language because it is minimalistic yet
usable. It was designed to be extensible by programmers, and as a
result, most of the features that you expect to be built-ins in most
languages (like control or data structures) are really defined by the
library in XL. As a matter of fact, XL is defined using a *single*
operator.

## The magic rewrite operator

The rewrite operator in XL is written `->`. Something like `A -> B`
reads as *if you see A in a program, rewrite it as B*. Depending on
the context, this rewrite operators can be used to define:

* Variables

          X -> 0
     
* Functions

          distance A, B -> sqrt((B-A)^2)

* Functions can be defined using special cases and recursion

          0! -> 1
          N! -> N * (N-1)!
 
* Operators, which can also include special cases

          0 + B -> B
          A + 0 -> A
          A + B -> (A-1) + (B+1)
     
* Combinations of operators

          A in B..C -> A >= B and A <= C

* Control structures

          loop Body -> Body; loop Body
          if true then TrueBody else FalseBody -> TrueBody
          if false then TrueBody else FalseBody -> FalseBody
          while Condition loop Body ->
              if Condition then
                  Body
                  while Condition loop Body


## Field-tested

XL was field-tested at a relatively large scale in the Tao3D project, where
it serves as a dynamic document description language for a real-time
3D rendering engine. XL also serves as the foundation for ELFE, which
is a version of XL indented to allow quick experiments with the Internet
of things.


## Under re-construction

It is based on these experiences that the current rewrite is taking
place. The new implementation will take advantage of several years of
experience with the older implementations. It will be written in C,
rely on LLVM only as an option, if ever, and should be noticeably
faster.

For the moment, this new implementation does not work at all, though.

