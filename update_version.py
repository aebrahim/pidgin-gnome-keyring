#!/usr/bin/env python
"""updates the line below /* version */ in source code with svnversion"""
import re
import fileinput
import os

base_version = 1

def get_svn_version():
    res=os.popen("svnversion")
    return res.read().strip()

if __name__ == "__main__":
    svn_version = get_svn_version()
    # if there has been an update, increment
    if svn_version[-1].lower() == "m":
        svn_version = str(int(svn_version[0:-1]) + 1)
    version = str(base_version)+"."+svn_version
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
