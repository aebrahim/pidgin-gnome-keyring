# pidgin-gnome-keyring

Pidgin usually stores passwords as plaintext. This plugin instead saves all
passwords to the system keyring, which some would argue is a more secure form
of password storage.

After the plugin is enabled, whenever an account with a pidgin-stored password
signs on, its password will automatically be saved to the keyring and removed
from the plaintext accounts.xml file.

The plugin is available as a ppa for Ubuntu: ```ppa:pidgin-gnome-keyring/ppa```
