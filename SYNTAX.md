# Proposed Syntax

These are notes that may not represent the final implementation of this project.

## Storage types

- Bounded types
  - A type bound to/owned by some storage (ie. a variable or container)
  - `Obj x = CreateObj();`
    - `x` is a bounded `Obj` type
- Unbounded types
  - Represents a temporary object that was created but not bound to any
    storage. This is untracked memory that must be eventually tracked and
    bound to some storage.
  - Values returned by functions are unbounded types
    - `CreateObj();`
      - The new object returned by `CreateObj()` is unbounded and not assigned
        to any storage.
  - Literals are unbounded types
    - `2`, `1.0`, `'c'`, `"abc"`
- References
  - `int &`, `Obj &`
  - These types operator just like other types but are not freed once the
    storage bounded to them goes out of scope.

### Transferring storage

Storage of data can be transferred via assignment or passing as a function argument.
I will use the term `assignment` or `assign` to refer to actual assignment or passing
as a function argument.

#### Bounded types

Bounded types can be assigned from other bounded types or unbounded types.
Bounded types cannot be assigned from references because we would be trying to
take ownership from something that does not originally have any ownership in
anything.

```
Obj x;
Obj y = x;  // y owns the data under x; x is now unusable after this point
            // unless it is reassigned data
x = CreateObj();  // ok
Obj y_ref = y;
x = y_ref;  // Illegal
```

#### Unbounded types

Unbounded types are not assignable since assignment implies creating permanent
storage but unbounded types, by definition, have no storage.

```
Obj x;
2 = x;  // Illegal
CreateObj() = &x;  // Illegal
```

#### References

References can only be assigned from bounded types or references.
References cannot be made from unbounded types because unbounded types do not
have any permanent storage that we can reference.

```
Obj x;
Obj& x_ref = x;  // ok
Obj &y_ref = x_ref;  // ok
x_ref = CreateObj();  // Illegal since we are trying to make a reference to
                      // some data that is not stored in any permanent place

// Proper way to use CreateObj()
Obj new_obj = CreateObj();
x_ref = new_obj;
```

## Function Semantics

All functions return either unbounded types or references.

Bounded objects returned by functions are the only time when bouned types
are implicitely converted to unbounded types.

```
Obj func() {
  Obj x;
  return x;
}

Obj x = func();
```

References can only be returned if the object being referenced is not owned
by the function itself (ie. the reference must point to some global variable
or a class member).

```
Obj func() {
  Obj x;
  Obj &x_ref = x;
  return x_ref;  // Illegal
}

class A {
  Obj &get() {
    return x_;  // ok
  }

  Obj x_;
};
```

## Ownership

All objects are owned either by the scope they are created in or another
object. I don't know rust, but ownership will probably be very similar to
that of rust.

Assignment and passing arguments transfers ownership.

```
void func() {
  // x is created in and owned by func's scope
  Obj x = CreateObj();
  Obj y = CreateObj();

  other_func(x, y);  // x is borrowed; y ownership is given to other_func()
  // use of y after this is now illegal

  // other_func(x, y);  // Illegal
  Obj z = CreateObj();

  other_func(x, z.clone());  // z can still be used since we passed a new copy
                             // of z instead which is freed by other_func()

  Obj a = return_func(x, z.clone());  // a is a new object that will be freed
                                      // at the end of this function

  // x is automatically freed once it goes out of scope
}

void other_func(Obj &x, Obj y) {
  // x borrowed from caller
  // other_func does not own x
  // other_func now owns y

  // y freed at end of function
}

Obj return_func(Obj &x, Obj y) {
  return CreateObj();  // Ownership is transferred to the caller; y is automatically freed
}
```
