
[![Ubuntu CI](https://github.com/fabiosvm/powerc/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/fabiosvm/powerc/actions/workflows/ubuntu.yml) [![macOS CI](https://github.com/fabiosvm/powerc/actions/workflows/macos.yml/badge.svg)](https://github.com/fabiosvm/powerc/actions/workflows/macos.yml) [![Windows CI](https://github.com/fabiosvm/powerc/actions/workflows/windows.yml/badge.svg)](https://github.com/fabiosvm/powerc/actions/workflows/windows.yml)

# The PowerC Programming Language

PowerC is a high-level programming language that transpiles to C.

> **Note:** This project is a work in progress.

## How does it look like?

Here is a classic fibonacci function in PowerC:

```mojo
fn fib(n: Int) -> Int {
  if n < 2 {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}
```

## What features does this language aim to include?

The main features of the language are:

- Cross-Platform
- C-Like Syntax
- Strong Type Checking
- Sound null safety
- Mutable Value Semantics
- UTF-8 Strings
- Closures
- Structs
- Methods
- Interfaces
- Generics (Monomorphization)
- Automatic Reference Counting

> **Note:** This list will increase as the project evolves.

## What features are not planned for this language?

Some features that are not intended to be part of the language are:

- Preprocessor
- Inheritance
- Exceptions
- Tracing Garbage Collection

## Building

### Requirements

In order to build the project, you will need the following:

- [CMake](https://cmake.org)

To facilitate, a build script is provided. So, simply run the build script:

```
./build.sh
```

## Testing

After building, you can run the tests by typing the following command:

```
./test.sh
```

## Compiling an example

Now, you can compile an example by typing the following command:

```
build/powerc examples/hello.pwc
```

> **Note:** Currently, the compiler just prints the AST.

## Cleaning

If you want to clean the project, run the following command:

```
./clean.sh
```

## Grammar

Check the [docs/powerc.ebnf](docs/powerc.ebnf) file for the current PowerC grammar.

## Roadmap

The roadmap for the project is:

- [x] Lexer
- [x] Parser
- [x] Abstract Syntax Tree
- [ ] Symbol Table
- [ ] Type Checker
- [ ] Code Generator
- [ ] Standard Library
- [ ] Self-Hosted Compiler
- [ ] Examples
- [ ] Documentation
- [ ] Benchmarks

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
