## Requirements

- clang 6.0 (see `build.ninja`)

### Testing

- libgtest
  - Setup on Ubuntu (https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/)

## Usage
```
$ ninja  # Build example hello_world executable 
$ ninja check-all  # Run all tests. Requires libgtest
```
