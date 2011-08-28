#!/usr/bin/env python
"""helps automate building the package"""
import shutil
import os
import update_version
import xml.dom.minidom
import re
import time

version_str = str(update_version.base_version)
version_str += "."
version_str += str(update_version.get_svn_version())

ubuntuname = "natty"
authorname = "Ali Ebrahim"

basename = "pidgin-gnome-keyring"
dirname = basename + "-" + version_str
tarname = basename + "_" + version_str + ".orig"
debname = dirname + "/debian/"

if os.path.isdir(dirname):
   os.system("rm -rf " + dirname) 
os.mkdir(dirname)
shutil.copy2("gnome-keyring.c", dirname)
shutil.copy2("Makefile", dirname)
shutil.make_archive(tarname, "gztar", root_dir=dirname)
os.system("cd %s; dh_make -m" %dirname)
os.system("cd %s; rm -rf *.ex *.EX README*" %debname)
shutil.copy2("deb_control", debname+"control")
shutil.copy2("deb_copyright", debname+"copyright")
# generate the change log in the appropriate format
os.system("svn log --xml > log.xml")
logfile = open("log.xml", "r")
logxml = xml.dom.minidom.parse(logfile)
logfile.close()
os.system("rm log.xml")
logentry = logxml.firstChild.firstChild.nextSibling
changelog = open(debname+"changelog", "w")
print  >> changelog, "%s (%s-1) %s; urgency=low"\
           %(basename, version_str, ubuntuname)
print >>changelog, "" 
msg = logentry.getElementsByTagName("msg")[0].firstChild.nodeValue
msg = msg.strip()
for line in msg.split("\n"):
    line = line.strip()
    if len(line) > 0: print >>changelog, "  " + line
print >>changelog, "" 
email = logentry.getElementsByTagName("author")[0].firstChild.nodeValue
date = logentry.getElementsByTagName("date")[0].firstChild.nodeValue
date = date.replace(re.findall("\.[0-9]*Z$", date)[0], "")
date = time.strptime(date, "%Y-%m-%dT%H:%M:%S")
date= time.strftime("%a, %d %b %Y %X", date)
print >>changelog, " -- %s <%s>  %s +0000" %(authorname, email, date)
changelog.close()
os.system("cd %s; debuild -S" %dirname)


