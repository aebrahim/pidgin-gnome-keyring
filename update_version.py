#!/usr/bin/env python
"""updates the line below /* version */ in source code with svnversion"""
import re
import fileinput

base_version = 1

def get_svn_version():
    svnfile = open(".svn/entries", "r")
    version = svnfile.readline().strip()
    svnfile.close()
    return int(version)

# the version number should be one more than the current svn version
version = get_svn_version() + 1
version = str(base_version)+"."+str(version)
source_file = fileinput.input("gnome-keyring.c", inplace = 1)
finder = re.compile("/\* version \*/").findall
found  = False
for line in source_file:
    if found:
        # fileinput redirects stdout to the file
        print '%s"%s",' %(whitespace, version)
        found = False
    else: print line,
    if finder(line) != []:
       found = True
       whitespace = re.findall("^[\s]*", line)
       if len(whitespace) == 0: whitespace = ""
       else: whitespace = whitespace[0]

source_file.close()
