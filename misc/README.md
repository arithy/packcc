# Miscellaneous Tool

## Overview

In this directory, currently one miscellaneous tool shown below is stored.

## Tool

### `unicode_general_category.py`

#### Synopsis

A Python script to generate a PEG file defining rules to match specific Unicode characters.
It needs internet access for fetching Unicode data from https://www.unicode.org/ .

The Python module `requests` is required.

#### Usage

~~~sh
$ python unicode_general_category.py > ../import/char/unicode_general_category.peg
~~~
