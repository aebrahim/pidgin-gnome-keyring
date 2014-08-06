#!/usr/bin/env python
"""helps automate building the package"""
import shutil
import os
from subprocess import check_output
from pygit2 import Repository, GIT_SORT_TIME
import xml.dom.minidom
import re
import time

# declare strings up here
ubuntunames = ["lucid", "precise", "quantal", "saucy", "trusty", "utopic"]
author_name = "Ali Ebrahim"
email = "ali.ebrahim314@gmail.com"
basename = "pidgin-gnome-keyring"
installdir = "/usr/lib/purple-2/"

version = check_output(["git", "describe"]).strip()

for ubuntuname in ubuntunames:
    version_str = version + "~" + ubuntuname

    package_version_str = version_str + "-1"

    dirname = basename + "-" + version_str
    tarname = basename + "_" + version_str + ".orig"
    debname = dirname + "/debian/"
    print debname

    if os.path.isdir(dirname):
       os.system("rm -rf " + dirname) 
    os.mkdir(dirname)
    os.mkdir(debname)
    shutil.copy2("gnome-keyring.c", dirname)
    # copy everything in Makefile except for the install part at the end
    shutil.copy2("Makefile", dirname + "/Makefile")
    with open(dirname + "/VERSION", "w") as outfile:
		outfile.write(version)

    #shutil.make_archive(tarname, "gztar", root_dir=dirname)
    # run dh_make
    os.system("cd %s; dh_make -s -c gpl2 --createorig -y -a -e %s" % (dirname, email))
    # remove extra files and copy source files
    os.system("cd %s; rm -rf *.ex *.EX README*" %debname)
    shutil.copy2("deb_control", debname+"control")
    shutil.copy2("deb_copyright", debname+"copyright")
    shutil.copy2("deb_rules", debname+"rules")
    # make the dirs and install files
    os.system("echo '%s' > %sdirs" %(installdir, debname))
    os.system("echo 'gnome-keyring.so %s' > %sinstall" \
            %(installdir, debname))
    # write the changelog
    changelog = open(debname + "changelog", "w")
    repo = Repository(".")
    for commit in repo.walk(repo.head.target, GIT_SORT_TIME):
		changelog.write("%s (%s) %s; urgency=low\n\n" % (basename, package_version_str, ubuntuname))
		for commit_line in commit.message.split("\n"):
			if len(commit_line) > 0:
				changelog.write("  " + commit_line + "\n")
		changelog.write("\n")
		date = time.strftime("%a, %d %b %Y %X", time.gmtime(commit.commit_time))
		offset = "%+0.04d" % (commit.commit_time_offset / 60 * 1000)
		changelog.write(" -- %s <%s>  %s %s\n" % (author_name, email, date, offset))
    changelog.close()
    # call debuild
    os.system("cd %s; debuild -S -sa" % dirname)

