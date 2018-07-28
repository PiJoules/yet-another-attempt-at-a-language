## Requirements

- clang 6.0 (see `build.ninja`)

### Testing

- libgtest
  - Setup on Ubuntu (https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/)

## Usage
```
$ ninja  # Build compiler and example hello_world executable 
$ ./hello_world
hello world

# Compiler options
$ ninja compiler
$ ./compiler example/hello_world.lang  # Generate object file that can be linked against libc into an executable
$ ./compiler example/hello_world.lang --lvm-dump  # Dump llvm IR
$ ./compiler example/hello_world.lang --ast-dump  # Dump AST

# Testing
$ ninja check-all  # Run all tests. Requires libgtest

# Code formatting
$ ninja format-all
```

## Goals

- High level language
- No manual memory management
  - Allocation/freeing handled through ownership
- Minimal (ideally no) undefined behavior
- Good readbility
