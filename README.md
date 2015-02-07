# pidgin-gnome-keyring

Pidgin usually stores passwords as plaintext. This plugin instead saves all
passwords to the system keyring, which some would argue is a more secure form
of password storage.

After the plugin is enabled, whenever an account with a pidgin-stored password
signs on, its password will automatically be saved to the keyring and removed
from the plaintext accounts.xml file.

The plugin is available as a ppa for Ubuntu: ```ppa:pidgin-gnome-keyring/ppa```

## Building from source
You will need the libpurple and libsecret development libraries, along with
pkg-config. On Ubuntu, install these with
```sudo apt-get install pkg-config libsecret-1-dev libpurple-dev```

Afterwards, use git clone to download the source (the version is set by git),
and run ```make``` to compile. For installation:
 - local ($HOME/.purple/plugins): ```make install_local```
 - global (/usr/lib/purple-2): ```sudo make install```
