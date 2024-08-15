# Miscellaneous Tools

## Overview

In this directory, miscellaneous tools shown below are stored.

## Tools

### `unicode_general_category.py`

#### Synopsis

A Python script to generate a PEG file defining rules to match Unicode characters with the respective _general category properties_.
It needs internet access for fetching Unicode data from https://www.unicode.org/ .

The Python module `requests` is required.

#### Usage

~~~sh
$ python unicode_general_category.py > ../import/char/unicode_general_category.peg
~~~

### `unicode_derived_core.py`

#### Synopsis

A Python script to generate a PEG file defining rules to match Unicode characters with the respective _derived core properties_.
It needs internet access for fetching Unicode data from https://www.unicode.org/ .

The Python module `requests` is required.

#### Usage

~~~sh
$ python unicode_derived_core.py > ../import/char/unicode_derived_core.peg
~~~
