#define PLUGIN_ID "core-gnome-keyring"

#ifndef VERSION
#define VERSION "experimental"
#endif

#define PLUGIN_NAME "GNOME Password Keyring"
#define PLUGIN_SUMMARY "Save passwords to the systems keyring instead of "\
                       "as plaintext"
#define PLUGIN_DESCRIPTION "This plugin should prevent writing passwords to "\
                           "the plaintext accounts.xml file. Instead, "\
                           "passwords will be securely stored in the "\
                           "system keyring."

#define PLUGIN_AUTHOR "Ali Ebrahim <ali.ebrahim314@gmail.com>"
#define PLUGIN_WEBSITE "https://github.com/aebrahim/pidgin-gnome-keyring/"
