# Testing packcc

## How to run

For running test cases, we assume you have `bats-core-1.2.1`.

If you don't have, you can install it from its tarball:
```
$ curl -L -o bats-core-1.2.1.tar.gz https://github.com/bats-core/bats-core/archive/v1.2.1.tar.gz &&
  tar zxvf bats-core-1.2.1.tar.gz &&
  cd bats-core-1.2.1 &&
  ./install.sh /usr/local
```

After installing `bats-core-1.2.1`, move the top source directory of packcc.
Run `bats` with `tests` directory:

```
$ bats tests
   generate calc1/1   
   target executable: build/gcc/bin/debug/packcc
   target executable: build/gcc/bin/release/packcc
 ✓ generate calc

1 test, 0 failures
```

## How to write a test case

You needs:

* a bats file
* an input peg file
* expected .c file for the peg file
* expected .h file for the peg file

In the bats file, load `utils.bash`.  `utils.bash` provides
`simple_test` function.  Pass the name of the ipnut peg file to
`simple_test` in your test case.
See `calc.bats` as an example.
