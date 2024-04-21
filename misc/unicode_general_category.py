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

ucd_url = 'https://www.unicode.org/Public/14.0.0/ucd/UnicodeData.txt'
ucd_gc_list = [
    'Lu', 'Ll', 'Lt', 'Lm', 'Lo', 'Mn', 'Mc', 'Me', 'Nd', 'Nl', 'No',
    'Pc', 'Pd', 'Ps', 'Pe', 'Pi', 'Pf', 'Po', 'Sm', 'Sc', 'Sk', 'So',
    'Zs', 'Zl', 'Zp', 'Cc', 'Cf', 'Cs', 'Co', 'Cn'
]
ucd_gc_dict = {
    'Lu': 'Uppercase_Letter',
    'Ll': 'Lowercase_Letter',
    'Lt': 'Titlecase_Letter',
    'Lm': 'Modifier_Letter',
    'Lo': 'Other_Letter',
    'Mn': 'Nonspacing_Mark',
    'Mc': 'Spacing_Mark',
    'Me': 'Enclosing_Mark',
    'Nd': 'Decimal_Number',
    'Nl': 'Letter_Number',
    'No': 'Other_Number',
    'Pc': 'Connector_Punctuation',
    'Pd': 'Dash_Punctuation',
    'Ps': 'Open_Punctuation',
    'Pe': 'Close_Punctuation',
    'Pi': 'Initial_Punctuation',
    'Pf': 'Final_Punctuation',
    'Po': 'Other_Punctuation',
    'Sm': 'Math_Symbol',
    'Sc': 'Currency_Symbol',
    'Sk': 'Modifier_Symbol',
    'So': 'Other_Symbol',
    'Zs': 'Space_Separator',
    'Zl': 'Line_Separator',
    'Zp': 'Paragraph_Separator',
    'Cc': 'Control',
    'Cf': 'Format',
    'Cs': 'Surrogate',
    'Co': 'Private_Use',
    'Cn': 'Unassigned'
}

def get_unicode_data():
    res = requests.get(ucd_url, stream=True)
    res.raise_for_status()
    txt = ''
    for chunk in res.iter_content(chunk_size=1024*1024):
        txt += chunk.decode()
    return txt

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
        'Unicode_Letter <- Unicode_Cased_Letter / Unicode_Modifier_Letter / Unicode_Other_Letter\n'
        'Unicode_Cased_Letter <- Unicode_Uppercase_Letter / Unicode_Lowercase_Letter / Unicode_Titlecase_Letter\n'
        'Unicode_Mark <- Unicode_Nonspacing_Mark / Unicode_Spacing_Mark / Unicode_Enclosing_Mark\n'
        'Unicode_Number <- Unicode_Decimal_Number / Unicode_Letter_Number / Unicode_Other_Number\n'
        'Unicode_Punctuation <- Unicode_Connector_Punctuation / Unicode_Dash_Punctuation / Unicode_Open_Punctuation / Unicode_Close_Punctuation / Unicode_Initial_Punctuation / Unicode_Final_Punctuation / Unicode_Other_Punctuation\n'
        'Unicode_Symbol <- Unicode_Math_Symbol / Unicode_Currency_Symbol / Unicode_Modifier_Symbol / Unicode_Other_Symbol\n'
        'Unicode_Separator <- Unicode_Space_Separator / Unicode_Line_Separator / Unicode_Paragraph_Separator\n'
        'Unicode_Other <- Unicode_Control / Unicode_Format / Unicode_Surrogate / Unicode_Private_Use\n' # The category 'Unassigned' is excluded because currently it has no character.
        '\n'
    )
    for gc in ucd_gc_list:
        if gc not in dat:
            continue
        cc = ''
        cs = ''
        cp = ''
        for c in dat[gc]:
            if cs == '':
                cs = c
            elif int(c, 16) - int(cp, 16) != 1:
                cc += '\\u' + cs
                if cs != cp:
                    cc += '-\\u' + cp
                cs = c
            cp = c
        if cs != '':
            cc += '\\u' + cs
            if cs != cp:
                cc += '-\\u' + cp
        str += 'Unicode_' + ucd_gc_dict[gc] + ' <- [' + cc + ']\n'
    return str

def main():
    pat = re.compile(r'^([0-9a-fA-F]+);[^;]*;([0-9a-zA-Z_]+);')
    dat = {}
    for ent in get_unicode_data().splitlines():
        res = pat.search(ent)
        cat = res.group(2)
        if cat not in dat:
            dat[cat] = []
        dat[cat] += [res.group(1)]
    print(generate_rules(dat), end='')

if __name__ == '__main__':
    main()
