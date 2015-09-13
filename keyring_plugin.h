#include <account.h>
#include <plugin.h>

void keyring_password_store(PurpleAccount *account);
void keyring_password_get(PurpleAccount *account);

PurplePluginPrefFrame * get_pref_frame(PurplePlugin *plugin);
void init_keyring_plugin(PurplePlugin *plugin);
