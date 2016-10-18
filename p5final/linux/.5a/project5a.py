import os, subprocess, shutil

import toolspath
from testing import Test, BuildTest

call_fs = "fscheck "

def readall(filename):
  f = open(filename, 'r')
  s = f.read()
  f.close()
  return s

class FSBuildTest(BuildTest):
  targets = ['fscheck']

  def run(self):
    self.clean(['fscheck', '*.o'])
    if not self.make(self.targets):
      self.run_util(['gcc', 'fscheck.c', '-o', 'fscheck'])
    self.done()

class FSTest(Test):
  def run(self, command = None, stdout = None, stderr = None, 
          addl_args = []):
    in_path = self.test_path + '/images/' + self.name
    if command == None:
      command = ['./fscheck', in_path]
    
    out_path = self.test_path + '/out/' + self.name
    err_path = self.test_path + '/err/' + self.name
    self.command = call_fs + in_path + \
        "\n and check out the test folder\n " + self.test_path \
        + '/err/' + self.name + \
        "\n to compare your error output with reference outputs. "

    if stdout == None:
      stdout = readall(out_path)

    if stderr == None:
      stderr = readall(err_path)

    self.runexe(command, status=self.status, stderr=stderr, stdout=stdout)
    self.done()


######################### Built-in Commands #########################

class Good(FSTest):
  name = 'good'
  description = 'run on a good filesystem'
  timeout = 10
  status = 0
  point_value = 5

class Nonexistant(FSTest):
  name = 'nonexistant'
  description = 'run on a nonexistant filesystem'
  timout = 10
  status = 1
  point_value = 5

class Badinode(FSTest):
  name = 'badinode'
  description = 'run on a filesystem with a bad type in an inode'
  timout = 10
  status = 1
  point_value = 5

class Badaddr(FSTest):
  name = 'badaddr'
  description = 'run on a filesystem with a bad direct address in an inode'
  timout = 10
  status = 1
  point_value = 5

class Badindir1(FSTest):
  name = 'badindir1'
  description = 'run on a filesystem with a bad indirect address in an inode'
  timout = 10
  status = 1
  point_value = 5

class Badindir2(FSTest):
  name = 'badindir2'
  description = 'run on a filesystem with a bad indirect address in an inode'
  timout = 10
  status = 1
  point_value = 5

class Badroot(FSTest):
  name = 'badroot'
  description = 'run on a filesystem with a root directory in bad location'
  timout = 10
  status = 1
  point_value = 5

class Badroot2(FSTest):
  name = 'badroot2'
  description = 'run on a filesystem with a bad root directory in good location'
  timout = 10
  status = 1
  point_value = 5

class Badfmt(FSTest):
  name = 'badfmt'
  description = 'run on a filesystem without . or .. directories'
  timout = 10
  status = 1
  point_value = 5

class Mismatch(FSTest):
  name = 'mismatch'
  description = 'run on a filesystem with .. pointing to the wrong directory'
  timout = 10
  status = 1
  point_value = 5

class Mrkfree(FSTest):
  name = 'mrkfree'
  description = 'run on a filesystem with an inuse direct block marked free'
  timout = 10
  status = 1
  point_value = 5

class Mrkfreeindir(FSTest):
  name = 'indirfree'
  description = 'run on a filesystem with an inuse indirect block marked free'
  timout = 10
  status = 1
  point_value = 5

class Mrkused(FSTest):
  name = 'mrkused'
  description = 'run on a filesystem with a free block marked used'
  timout = 10
  status = 1
  point_value = 5

class Addronce(FSTest):
  name = 'addronce'
  description = 'run on a filesystem with an address used more than once'
  timout = 10
  status = 1
  point_value = 5

class Imrkused(FSTest):
  name = 'imrkused'
  description = 'run with inode marked used, but not referenced in a directory'
  timout = 10
  status = 1
  point_value = 5

class Imrkfree(FSTest):
  name = 'imrkfree'
  description = 'run with inode marked free, but referenced in a directory'
  timout = 10
  status = 1
  point_value = 5

class Badrefcnt(FSTest):
  name = 'badrefcnt'
  description = 'run on fs which has an inode with a bad reference count'
  timout = 10
  status = 1
  point_value = 5

class Goodrefcnt(FSTest):
  name = 'goodrefcnt'
  description = 'run on fs with only good reference counts'
  timout = 10
  status = 0
  point_value = 5

class Dironce(FSTest):
  name = 'dironce'
  description = 'run on fs with a directory appearing more than once'
  timout = 10
  status = 1
  point_value = 5



#=========================================================================

all_tests = [
  # Easy tests
  Good,
  Goodrefcnt,
  Nonexistant,
  Badroot,

  # Medium/hard tests
  Badinode,
  Badaddr,
  Badindir1,
  Badroot2,
  Badfmt,
  Mismatch,
  Mrkfree,
  Mrkfreeindir,
  Mrkused,
  Addronce,
  Imrkused,
  Imrkfree,
  Badrefcnt,
  Badindir2,
  Dironce
  ]

build_test = FSBuildTest

from testing.runtests import main
main(build_test, all_tests)
