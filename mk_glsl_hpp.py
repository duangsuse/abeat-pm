from sys import argv
from os import path
import re

RE_CODE = re.compile('''\n// (\w+)\n(.*?)(|$)''', re.DOTALL)
NL = "\n\n"
def genCpp_CharLit(name, text):
  head = text[:text.index("\n// ")+len("\n")]
  def term(stype,s): return f"const char *{(stype+'_'+name).upper()} = R\"({head}{s})\";"
  return NL.join(term(*m[0:2]) for m in RE_CODE.findall(text))

def main(args=argv[1:]):
  code = str()
  for fp in args:
    with open(fp) as f: code += genCpp_CharLit(path.splitext(path.basename(fp))[0], f.read())+NL
  print(code)
if __name__ == "__main__": main()
