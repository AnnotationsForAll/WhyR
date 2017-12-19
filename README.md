# WhyR

WhyR is a tool to convert programs written in [LLVM IR](http://llvm.org/) into a model for an [SMT solver](https://en.wikipedia.org/wiki/SMT_solver). It translates programs into models, and also allows you to write annotations for the code, which is translated to goals. In addition, annotations can be automatically generated, using WhyR's runtime error annotation generation (aka RTE generation). Currently, [the Why3 language](http://why3.lri.fr/) is WhyR's output format.

## Design Goals

* LLVM is low-level, so WhyR should prefer low-level input.
* WhyR should favor provability over total correctness.
* Even then, total correctness should be possible via simple configuration.
* LLVM is a tool for other tools, not for users. WhyR should have tool-oriented features.

## Current Features

* Takes input as files in LLVM's bitcode (`.bc`) or IR (`.ll`) format
* Why3 model generation for the following LLVM constructs:
  * Most common LLVM instructions: Arithmetic, bitwise operations, casts, branches, calls, memory operations...
  * LLVM data types:
  * Integers of any size
    * Floats, doubles
    * Array types
    * Structure types
    * Vector types
  * PHI nodes
  * Indirect branches and block addresses
  * Memory operations (loading, storing, checking if valid addresses...)
  * Global variables
* Function contracts for function definitions
* Specifications of assignable locations for functions
* Assertions and assumptions for points in the program
* Generation of annotations to ensure no undefined behavior occurs at runtime
* The WhyR Annotation Language (WAR), a higher-level method of input to WhyR

## Installation

See `INSTALL.md` for more information on how to compile and run WhyR.

## How to Use WhyR

Here is a simple program written in LLVM IR that computes the absolute value of a 32-bit int. Let's call this file `abs.ll`:

```
define i32 @abs(i32 %n) {
    %is_neg = icmp slt i32 %n, 0
    %neg = sub nsw i32 0, %n
    %val = select i1 %is_neg, i32 %neg, i32 %n
    ret i32 %val
}
```

We wish to make certain that the result of this program is always the absolute value of n (barring undefined behavior like overflow).

We can make an annotation that verifies that, after the execution of the function, the result is always what we want. We do this by attaching LLVM metadata to the function, with a special metadata tag "whyr.ensures":

```
define i32 @abs(i32 %n) !whyr.ensures !{!{!"eq", !{!"result"}, !{!"ifte", !{!"slt", !{!"arg", !"n"}, i32 0}, !{"neg", !{!"arg", !"n"}}, !{!"arg", !"n"}}}} {
    %is_neg = icmp slt i32 %n, 0
    %neg = sub nsw i32 0, %n
    %val = select i1 %is_neg, i32 %neg, i32 %n
    ret i32 %val
}
```

What does all that mess in the metadata node mean? Essentially, it's a method of encoding LISP-like s-expressions into LLVM metadata. It looks much more readable as this sexpr-like form:

```
(eq (result) (ifte (slt %n 0) (neg %n) %n))
```

This means "the result of this function is equal to -n if n is less than 0, and n otherwise". The expression "slt" corresponds to LLVM's signed-less-than operator, and "ifte" is a conditional expression.

If one runs this program through WhyR, it will generate a file for Why3 to parse. Why3 will then attempt to verify one goal. This goal corresponds to the ensures clause that was added- It is trying to prove that for all possible inputs, the ensures clause holds true.

However, the goal may not prove for all inputs. Why not? This is because there is potential undefined behavior in our program. What happens if %n is the most negative signed integer? Because of how 2's complement arithmetic works, subtracting 0 from %n would cause signed overflow. Since we use a "sub nsw" instruction, @abs will produce undefined behavior when presented with  %n of the smallest integer.

To rectify this issue, we will require that all functions that call @abs do not pass in that value of %n. We can do this with a "whyr.requires" metadata:

```
define i32 @abs(i32 %n) !whyr.requires !{!{!"neq", !{!"arg", !"n"}, !{!"minint", !{!"typeof", !{!"arg", !"n"}}}}} !whyr.ensures !{!{!"eq", !{!"result"}, !{!"ifte", !{!"slt", !{!"arg", !"n"}, i32 0}, !{"neg", !{!"arg", !"n"}}, !{!"arg", !"n"}}}} {
    %is_neg = icmp slt i32 %n, 0
    %neg = sub nsw i32 0, %n
    %val = select i1 %is_neg, i32 %neg, i32 %n
    ret i32 %val
}
```

Our requires clause looks like this in a more readable format:

```
(neq %n (minint (typeof %n)))
```

This states that "n cannot be equal to the minimum possible integer (of n's type, which is type i32)".

And there you go! To run this program through WhyR, just run the following:

```
whyr abs.ll
```

And it will print out the Why3 theory it created. To prove this file, run something like the following:

```
whyr abs.ll | why3 prove -F why -P alt-ergo -
```

## Documentation

There are more features to WhyR then just ensures and requires clauses. Learn more about WhyR at [our wiki](https://github.com/AnnotationsForAll/WhyR/wiki)!

## Planned Features

* The following LLVM features:
 * Exception handling
 * Recursive functions
 * Intrinsic calls
 * Indirect function calls
* A better, more robust memory model
* Support for alternate output formats
* Loop invariants and data invariants
* Logical functions
* Optimized Why3 output

## Contributing

Contributions are welcome! This repository is already set up with files for the [Eclipse](http://www.eclipse.org/home/index.php) IDE; just import this project, and you get a working setup for contributing to WhyR.

## Disclaimer

This material is based upon work supported by the [National Science Foundation](https://nsf.gov/) under [Grant No. ACI-1314674](https://nsf.gov/awardsearch/showAward?AWD_ID=1314674).
Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s)
and do not necessarily reflect the views of the National Science Foundation.
