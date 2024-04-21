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
    if len(args) < 4:
        print('Too few arguments')
        sys.exit(1)
    root = args.pop(1)
    path = args.pop(1)
    more = args.pop(1)
    os.makedirs(os.path.dirname(root + '/' + path), exist_ok=True)
    id = re.sub(r'[^_a-zA-Z0-9]', '_', re.sub(r'\.peg$', '', os.path.basename(path)))
    with open(root + '/template.peg', 'r') as file:
        text = file.read()
    text = text.replace('${ID}', id + '_')
    text = text.replace('${MORE}', '    / ' + more + '_FILE' if more != '' else '')
    for i in range(4):
        imp = args.pop(1) if len(args) > 1 else ''
        text = text.replace('${IMPORT_' + str(i) + '}', '%import "' + imp + '"' if imp != '' else '')
    with open(root + '/' + path, 'w', newline='\n') as file:
        file.write(text)

if __name__ == '__main__':
    main()
