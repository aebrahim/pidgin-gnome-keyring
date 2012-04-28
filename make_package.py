#!/usr/bin/env python
"""helps automate building the package"""
import shutil
import os
import update_version
import xml.dom.minidom
import re
import time

# declare strings up here
ubuntunames = ["lucid", "natty", "oneiric", "precise"]
authorname = "Ali Ebrahim"
basename = "pidgin-gnome-keyring"
installdir = "/usr/lib/purple-2/"

# get the svn version
svn_version = str(update_version.get_svn_version())
if svn_version[-1].lower() == "m":
    svn_version = svn_version[0:-1]

# make a blank file
with open("blank_file", "w") as outfile:
    outfile.write("\n")

for ubuntuname in ubuntunames:
    version_str = str(update_version.base_version)
    version_str += "."
    version_str += svn_version
    version_str += "~%s" % ubuntuname

    package_version_str = version_str + "-1"

    dirname = basename + "-" + version_str
    tarname = basename + "_" + version_str + ".orig"
    debname = dirname + "/debian/"

    if os.path.isdir(dirname):
       os.system("rm -rf " + dirname) 
    os.mkdir(dirname)
    shutil.copy2("gnome-keyring.c", dirname)
    # copy everything in Makefile except for the install part at the end
    makefile_src = open("Makefile","r")
    makefile_dest = open(dirname+"/Makefile","w")
    for line in makefile_src:
        if re.match("install:", line) is not None:
            break
        makefile_dest.write(line)
    makefile_src.close(); makefile_dest.close()

    shutil.make_archive(tarname, "gztar", root_dir=dirname)
    # run dh_make
    os.system("cd %s; dh_make -m -c gpl2 < ../blank_file" %dirname)
    # remove extra files and copy source files
    os.system("cd %s; rm -rf *.ex *.EX README*" %debname)
    shutil.copy2("deb_control", debname+"control")
    shutil.copy2("deb_copyright", debname+"copyright")
    # make the dirs and install files
    os.system("echo '%s' > %sdirs" %(installdir, debname))
    os.system("echo 'debian/../gnome-keyring.so %s' > %sinstall" \
            %(installdir, debname))

    # generate the change log in the appropriate format
    os.system("svn log --xml > log.xml")
    logfile = open("log.xml", "r")
    logxml = xml.dom.minidom.parse(logfile)
    logfile.close()
    os.system("rm log.xml")
    changelog = open(debname+"changelog", "w")
    for i, logentry in enumerate(logxml.firstChild.getElementsByTagName("logentry")):
        if i > 0:
            continue
        print  >> changelog, "%s (%s) %s; urgency=low"\
                   %(basename, package_version_str, ubuntuname)
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
    # call debuild
    os.system("cd %s; debuild -S" %dirname)


