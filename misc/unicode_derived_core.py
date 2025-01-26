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

import os
import requests
import re

ucd_url = 'https://www.unicode.org/Public/UCD/latest/ucd/DerivedCoreProperties.txt'
ucd_dp_list = [ # Grapheme_Link is deprecated; Indic_Conjunct_Break is unsupported.
    'Math',
    'Alphabetic',
    'Lowercase',
    'Uppercase',
    'Cased',
    'Case_Ignorable',
    'Changes_When_Lowercased',
    'Changes_When_Uppercased',
    'Changes_When_Titlecased',
    'Changes_When_Casefolded',
    'Changes_When_Casemapped',
    'ID_Start',
    'ID_Continue',
    'XID_Start',
    'XID_Continue',
    'Default_Ignorable_Code_Point',
    'Grapheme_Extend',
    'Grapheme_Base'
]

def get_unicode_data():
    res = requests.get(ucd_url, stream=True)
    res.raise_for_status()
    txt = ''
    for chunk in res.iter_content(chunk_size=1024*1024):
        txt += chunk.decode()
    return txt

def escape_as_utf16_hex(hex):
    code = int(hex, 16)
    if code > 0x10ffff:
        raise ValueError
    return f'\\u{code:04x}' if code <= 0xffff else f'\\u{0xd800 | ((code - 0x10000) >> 10):04x}\\u{0xdc00 | (code & 0x3ff):04x}'

def generate_rules(dat):
    str = (
        '# This file was generated using the script \'' + os.path.basename(__file__) + '\'.\n'
        '\n'
        '# This file is hereby placed in the public domain.\n'
        '#\n'
        '# THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS\n'
        '# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n'
        '# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n'
        '# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE\n'
        '# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n'
        '# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\n'
        '# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR\n'
        '# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n'
        '# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE\n'
        '# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,\n'
        '# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n'
        '\n'
    )
    for dp in ucd_dp_list:
        if dp not in dat:
            continue
        cc = ''
        cs = ''
        cp = ''
        for c in dat[dp]:
            if cs == '':
                cs = c[0]
            elif int(c[0], 16) - int(cp, 16) != 1:
                cc += escape_as_utf16_hex(cs)
                if cs != cp:
                    cc += '-' + escape_as_utf16_hex(cp)
                cs = c[0]
            cp = c[1]
        if cs != '':
            cc += escape_as_utf16_hex(cs)
            if cs != cp:
                cc += '-' + escape_as_utf16_hex(cp)
        str += 'Unicode_' + dp + ' <- [' + cc + ']\n'
    return str

def main():
    pat = re.compile(r'^([0-9a-fA-F]+)(?:\.\.([0-9a-fA-F]+))?\s*;\s*([0-9a-zA-Z_]+)')
    dat = {}
    for ent in get_unicode_data().splitlines():
        res = pat.search(ent)
        if res is None:
            continue
        pro = res.group(3)
        if pro not in dat:
            dat[pro] = []
        dat[pro] += [(res.group(1), res.group(1) if res.group(2) is None else res.group(2))]
    print(generate_rules(dat), end='')

if __name__ == '__main__':
    main()
