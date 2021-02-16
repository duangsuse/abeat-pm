from re import sub, finditer # for half tokenize
def catExec(argv, code):
  from subprocess import Popen, PIPE
  return Popen(argv, stdin=PIPE, stdout=PIPE).communicate(code.encode())[0].decode()
def prints(s, i, i1): print("%d..%d" %(i,i1-1), s[i:i1])

def writeHpp(fdst, code, pair=("{", "}"), cpp=["cpp", "-DPKGED"]):
  includes = []
  hp = catExec(cpp, sub('''#\s*include.*''', lambda m: includes.append(m.group(0)) or "", code)) # don't expand includes
  hps = list(hp); i_rm = 0
  for m in finditer("(class .*{)|(struct .*{)", hp):
    i0 = m.end(); i1 = pairedIndex(hp, i0-1, pair) # { exclusive
    if i1 == -1: continue
    prints(hp, m.start(), i0)
    ii = i0; iEnd = ii+(i1-i0) # old:  for mm in finditer("{", hp[i0:i1]): ii0 = i0+mm.start(); ii1 = pairedIndex(hp, ii0, pair)
    while ii != iEnd:
      ii1 = pairedIndex(hp, ii, pair) # search for level-1 {} s
      if ii1 == ii: ii+=1; continue
      elif ii1 == -1: break
      ii1 += 1 # } inclusive
      prints(hp, ii, ii1)
      if hp.startswith("{1;", ii): ii = ii1; continue # feat! inline member func
      iBeg = ii-i_rm; ii = ii1 # move view.
      if iBeg < 0: continue
      iC = iBeg # feat! adjust ctor( ):
      while iC > 1:
        if hps[iC]==":" and hps[iC-1]==")": iBeg = iC
        elif hps[iC]=="}": break
        iC -= 1
      hps[iBeg] = ";"
      iBeg += 1
      del hps[iBeg:ii1-i_rm]; i_rm += (ii1-(iBeg+i_rm)) # {...}
  w = fdst.write
  w("#pragma once\n")
  def hasFRef(s): # excludes real include, no translate cause it's complicated :(
    for m in finditer('''"(.*)"''', s):
      if path.isfile(m.group(1)): return True
    return False
  prefix = "\"pkged/"
  w("\n".join(sub(prefix, "\"", fp) for fp in includes if (prefix in fp) or not hasFRef(fp) )) # translate pkged include relpath
  w("\n// generated!\n\n")
  w(sub('''# \d+ ".*\n''', "", "".join(hps)).lstrip()) # remove cpp lineno
  fdst.close()
def pairedIndex(s, i, pair):
  count = 0; (sA, sB) = pair
  while i < len(s):
    count += (1 if s[i] == sA else -1 if s[i] == sB else 0); i += 1
    if count == 0: return i-1
  return -1

from sys import argv
from os import path
def subExtName(s, fp, sep="."): return fp[:fp.index(sep)+1]+s
def main(args = argv[1:]):
  for fp in args:
    with open(fp, "r") as f: writeHpp(open(path.join("pkged", path.basename(subExtName("hpp", fp)) ), "w+"), f.read())

if __name__ == "__main__": main()