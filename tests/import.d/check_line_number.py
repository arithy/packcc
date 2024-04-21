#!/usr/bin/python3

# Copyright (c) 2024 Arihiro Yoshida. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import sys
import os
import re

def main():
    args = sys.argv
    if len(args) < 3:
        print('Too few arguments')
        sys.exit(1)
    optp = args.pop(1) if args[1] == '--only-pre' else ''
    word = args.pop(1)
    path = args.pop(1)
    with open(path, 'r') as file:
        text = file.read().split('\n')
    for i, s in enumerate(text):
        if s.find(word) >= 0:
            break
    if i >= len(text):
        print('Keyword not found')
        sys.exit(2)
    if optp == '':
        if i == 0 or i == len(text) - 1:
            print('Keyword found in invalid line')
            sys.exit(2)
        m = re.search(r'^#line ([0-9]+) "(.*)"$', text[i + 1])
        if m is None:
            print('#line directive not found one line after keyword')
            sys.exit(2)
        if int(m.group(1)) - 1 != i + 1:
            print('#line directive with inconsistent line number')
            sys.exit(2)
        if m.group(2) != os.path.abspath(path):
            print('#line directive with inconsistent file name')
            sys.exit(2)
    else:
        if i == 0:
            print('Keyword found in invalid line')
            sys.exit(2)
    m = re.search(r'^#line ([0-9]+) "(.*)"$', text[i - 1])
    if m is None:
        print('#line directive not found one line before keyword')
        sys.exit(2)
    with open(m.group(2), 'r') as file:
        text = file.read().split('\n')
    j = int(m.group(1)) - 1
    if j < 0 or j >= len(text):
        print('#line directive with invalid line number')
        sys.exit(2)
    if text[j].find(word) < 0:
        print('#line directive with inconsistent line number')
        sys.exit(2)

if __name__ == '__main__':
    main()
