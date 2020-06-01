import os.path
import zipfile
import tempfile
import random
import string
import shutil
import re

from os.path  import basename

class Unpacker(object):
   #--------------------------------------------------------------------------
   # 
   #--------------------------------------------------------------------------
   def entropy(self, length):
       return ''.join(random.choice('abcdefghijklmnopqrstuvwxyz') for i in range (length))

   #--------------------------------------------------------------------------
   # 
   #--------------------------------------------------------------------------
   def unpack_zipfile(self, file):

        if not os.path.isfile(file):
            raise Exception("Error: file, not found!")

        # Create unique working direction into which the zip file is expanded
        self.unzip_dir = "{0}/{1}_{2}".format(tempfile.gettempdir(), os.path.splitext(basename(file))[0], self.entropy(6))

        datfilename = ""
        binfilename = ""

        with zipfile.ZipFile(file, 'r') as zip:
            files = [item.filename for item in zip.infolist()]
            datfilename = [m.group(0) for f in files for m in [re.search('.*\.dat', f)] if m].pop()
            binfilename = [m.group(0) for f in files for m in [re.search('.*\.bin', f)] if m].pop()

            zip.extractall(r'{0}'.format(self.unzip_dir))

        datfile = "{0}/{1}".format(self.unzip_dir, datfilename)
        binfile = "{0}/{1}".format(self.unzip_dir, binfilename)

        # print "DAT file: " + datfile
        # print "BIN file: " + binfile

        return binfile, datfile

   #--------------------------------------------------------------------------
   # 
   #--------------------------------------------------------------------------
   def delete(self):
       # delete self.unzip_dir and its contents
       shutil.rmtree(self.unzip_dir)
