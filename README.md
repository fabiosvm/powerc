
# The PowerC Programming Language

PowerC is a high-level programming language that transpiles to C.

> **Note:** This project is a work in progress.

## What are the features that this language intends to have?

The main features of the language are:

- Cross-Platform
- C-Like Syntax
- Strong Type Checking
- Sound null safety
- Mutable Value Semantics
- UTF-8 Strings and runes
- First-Class Functions
- Closure (Capturing Values)
- Structs and Enums
- Methods
- Interfaces
- Templates
- Automatic Reference Counting

> **Note:** This list will increase as the project evolves.

## What is this language not intended to have?

Some features that are not intended to be part of the language are:

- Preprocessor
- Inheritance
- Exceptions
- Function/Method Overloading
- Tracing Garbage Collection

## Requirements

In order to build the project and carry out the tests, you will need the following:

- [Go](https://golang.org) 1.20 or higher
- [make](https://www.gnu.org/software/make)

## Building

To build the project, run the following command:

```
make build
```

## Testing

After building, you can run the tests by typing the following command:

```
make test
```

## Compiling an example

Now, you can compile an example by typing the following command:

```
bin/powerc examples/example1.pwc
```

> **Note:** Currently, the compiler does nothing.

## Cleaning

If you want to clean the project, run the following command:

```
make clean
```

## Roadmap

The roadmap for the project is:

- [ ] Scanner
- [ ] Abstract Syntax Tree
- [ ] Parser
- [ ] Type Checker
- [ ] Code Generator
- [ ] Standard Library
- [ ] Self-Hosted Compiler
- [ ] Examples
- [ ] Documentation
- [ ] Benchmarks

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
