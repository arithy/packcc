# Testing packcc

## How to run

For running test cases, we assume you have `bats-core`.

If you don't have it installed, you can do so using your package manager or from its tarball:
```
$ curl -L -o bats-core-1.2.1.tar.gz https://github.com/bats-core/bats-core/archive/v1.2.1.tar.gz &&
  tar zxvf bats-core-1.2.1.tar.gz &&
  cd bats-core-1.2.1 &&
  sudo ./install.sh /usr/local
```

After installing `bats-core-1.2.1`, you can run the tests using `tests/test.sh` script:
```
$ ./test.sh
 ✓ Testing basic.d - generation
 ✓ Testing basic.d - compilation
 ✓ Testing basic.d - run
 ...
 ✓ Testing strings.d - generation
 ✓ Testing strings.d - compilation
 ✓ Testing strings.d - run
 ✓ Testing strings.d - run [utf8]

19 tests, 0 failures, 1 skipped
```

The script passes all its arguments to `bats`, for example to run only some tests,
you can call it with `--filter <regexp>`. To see all the available arguments, execute `tests/test.sh --help`

The behavior of the `test.sh` can also be influenced by environment variables:
 - `PACKCC` - Path to compiled packcc binary. If unset, the script will compile it before running the tests.
 - `CC` - Compiler to use to compile `packcc` (if necessary) and tested programs. Defaults to `cc`.

## How to write generic test cases

To create a new tests, just follow these simple steps:

1. Create a directory with suitable name, e.g.: `tests/sequence.d`.
2. Create a grammar file called `input.peg` in this directory.
3. Create one or more input files. The files must match mask `input*.txt`.
4. Create a file with expected output for each of the inputs. The names must match the input,
   just replace "input" with "expected". E.g.: for `input-1.txt`, there must be `expected-1.txt`

Each test automatically performs three or more test cases:

1. Generates a parser from the `input.peg` grammar.
2. Compiles the generated parser
3. Runs the parser with specified inputs, comparing the output with the contents of expected files.

## How to write a customized tests

Sometimes the auto-generated test is not exactly what you need. In such case, you can simply create your own:

1. Create a directory with suitable name, e.g.: `tests/sequence.d`.
2. Create one or more `*.bats` files in this directory.
3. Specify custom tests in the bats files.

The test script will notice the customized files and will not generate a generic one.
However, you can still reuse as much of the common code as you want simply by loading `tests/utils.sh`
and calling the prepared functions. See [calc.d/calc.bats](calc.d/calc.bats) as an example.

## How to skip a test case

*Note: This paragraph applies only to automatically generated test cases. For customized tests,
just add `skip` directive to your* `*.bats` *file as needed.*

Sometimes it is useful to skip a test, for example to avoid an input that triggers known bug
that has not yet been fixed. To do so, simply rename the input file to `input*.skip.txt`.

If you want to skip all test cases in directory, rename the grammar file to `input.skip.peg`.
