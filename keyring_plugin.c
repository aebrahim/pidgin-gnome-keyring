#define PURPLE_PLUGINS
#include "keyring_plugin.h"
#include "keyring_plugin_defs.h"

#include <glib.h>

#include <version.h>
#include <signal.h>
#include <core.h>
#include <debug.h>


/* function prototypes */

static void password_store(PurpleAccount *account);
static void password_get(PurpleAccount *account);
/* libpurple signal callbacks */
static void sign_in_cb(PurpleAccount *account, gpointer data);
static void connecting_cb(PurpleAccount *account, gpointer data);


/* function definitions */

static void password_store(PurpleAccount *account) {
    /* only save if the plugin was set to remember */
    if (purple_account_get_remember_password(account)) {
        keyring_password_store(account);
        purple_account_set_remember_password(account, FALSE);
        purple_debug_info(PLUGIN_ID, "attempted password save for %s\n",
                account->username);
    }
    else {
        purple_debug_info(PLUGIN_ID, "not attempting password save for %s\n",
                account->username);
    }
}
static void password_get(PurpleAccount *account) {
    if (account->password == NULL) {
        purple_debug_info(PLUGIN_ID, "attempting password retrieval for %s\n",
                account->username);
        keyring_password_get(account);
    }
    else {
        purple_debug_info(PLUGIN_ID, "%s appears to already have a password\n",
                account->username);
    }

}

/* function called when the plugin starts */
static gboolean plugin_load(PurplePlugin *plugin) {
    GList *accountsList;
    void *accountshandle = purple_accounts_get_handle();

    /* First connect to signals */
    purple_signal_connect(accountshandle, "account-signed-on", plugin,
            PURPLE_CALLBACK(sign_in_cb), NULL);
    purple_signal_connect(accountshandle, "account-connecting", plugin,
            PURPLE_CALLBACK(connecting_cb), NULL);

    /* for existing accounts, ensure the password is saved */
    for (accountsList = purple_accounts_get_all();
         accountsList != NULL;
         accountsList = accountsList->next) {
        PurpleAccount *account = (PurpleAccount *)accountsList->data;
        if (account->password != NULL) {
            password_store(account);
        }
    }

    return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin) {
    /* disconnect from signals */
    void *accounts_handle = purple_accounts_get_handle();
    purple_signal_disconnect(accounts_handle, "account-signed-on",
                             plugin, NULL);
    purple_signal_disconnect(accounts_handle, "account-connecting",
                             plugin, NULL);
    return TRUE;
}

/* callback for whenever an account is signed in */
static void sign_in_cb(PurpleAccount *account, gpointer data) {
    password_store(account);
}

/* callback for whenever an account tries to connect.
 * This should make sure the account contains a correct password.
 */
void connecting_cb(PurpleAccount *account, gpointer data) {
    password_get(account);
}

static PurplePluginUiInfo prefs_info = {
    get_pref_frame, 0, NULL, NULL, NULL, NULL, NULL
};

static void init_plugin(PurplePlugin *plugin) {
    return init_keyring_plugin(plugin);
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC, PURPLE_MAJOR_VERSION, PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_HIGHEST,

    /* obtained from keyring_plugin_defs.h */
    PLUGIN_ID,
    PLUGIN_NAME,
    VERSION,
    PLUGIN_SUMMARY,
    PLUGIN_DESCRIPTION,
    PLUGIN_AUTHOR,
    PLUGIN_WEBSITE,

    plugin_load,
    plugin_unload,
    NULL,
    NULL,
    NULL,
    &prefs_info,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

PURPLE_INIT_PLUGIN(PLUGIN_ID, init_plugin, info)
