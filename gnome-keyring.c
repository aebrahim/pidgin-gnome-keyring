#define PURPLE_PLUGINS

#include <plugin.h>
#include <version.h>

#include <account.h>
#include <signal.h>
#include <core.h>

#include <gnome-keyring.h>
#include <glib.h>
#include <string.h>

/* function prototypes */
static void keyring_password_store(PurpleAccount *account, char *password);
static void sign_in_cb(PurpleAccount *account, gpointer data);
static void keyring_password_find_cb(GnomeKeyringResult res,
        const gchar *password, gpointer user_data);
static void keyring_password_store_cb(GnomeKeyringResult res, gpointer data);
static void connecting_cb(PurpleAccount *account, gpointer data);
static void memory_clearing_function(PurpleAccount *account);
static PurplePluginPrefFrame * get_pref_frame(PurplePlugin *plugin);

/* function definitions */

/* function called when the plugin starts */
static gboolean plugin_load(PurplePlugin *plugin) {
    GList *accountsList;
    void *accountshandle = purple_accounts_get_handle();
    /* notFound will be a list of accounts not found 
     * in the keyring */
    GList *notFound = NULL;
    GList *notFound_iter;
    
    /* The first thing to do is set all the passwords.
     * This part is purposely written to be locking. If pidgin
     * tries to connect without a password it will result in annoying
     * prompts */
    for (accountsList = purple_accounts_get_all();
         accountsList != NULL;
         accountsList = accountsList->next) {
        PurpleAccount *account = (PurpleAccount *)accountsList->data;
        gchar *password;
        GnomeKeyringResult res;
        /* if the password exists in the keyring, set it in pidgin */
        res = gnome_keyring_find_password_sync(GNOME_KEYRING_NETWORK_PASSWORD,
                &password, "user", account->username,
                "protocol", account->protocol_id, NULL);
        if (res == GNOME_KEYRING_RESULT_OK) {
            /* set the account to not remember passwords */
            purple_account_set_remember_password(account, FALSE);
            /* temporarily set a fake password, then the real one */
            purple_account_set_password(account, "fakedoopdeedoop");
            purple_account_set_password(account, password);
        }
        else {
            /* add to the list of accounts not found in the keyring */
            notFound = g_list_append(notFound, account);
        }
    }
    /* for the acccounts which were not found in the keyring */
    for (notFound_iter = g_list_first(notFound);
         notFound_iter != NULL;
         notFound_iter = notFound_iter->next) {
        PurpleAccount *account = (PurpleAccount *)notFound_iter->data;
        /* if the password was saved by libpurple before then 
         * save it in the keyring, and tell libpurple to forget it */
        if (purple_account_get_remember_password(account)) {
            gchar *password = g_strdup(account->password);
            keyring_password_store(account, password);
            purple_account_set_remember_password(account, FALSE);
            /* temporarily set a fake password, then the real one again */
            purple_account_set_password(account, "fakedoopdeedoop");
            purple_account_set_password(account, password);
        }
    }
    /* done with the notFound, so free it */
    g_list_free(notFound);

    /* create a signal which monitors whenever an account signs in,
     * so that the callback function can store/update the password */
    purple_signal_connect(accountshandle, "account-signed-on", plugin,
            PURPLE_CALLBACK(sign_in_cb), NULL); 
    /* create a signal which monitors whenever an account tries to connect
     * so that the callback can make sure the password is set in pidgin */
    purple_signal_connect(accountshandle, "account-connecting", plugin,
            PURPLE_CALLBACK(connecting_cb), NULL);
    /* at this point, the plugin is set up */
    return TRUE;
}


/* callback to whenever an account is signed in */
static void sign_in_cb(PurpleAccount *account, gpointer data) {
    /* Get the password.
     * The callback will check to see if it is already
     * saved in the keyring.
     * This will be run every time an account signs in. */
    gnome_keyring_find_password(GNOME_KEYRING_NETWORK_PASSWORD,
            keyring_password_find_cb,
            account, NULL,

            "user", account->username,
            "protocol", account->protocol_id,
            NULL);
}

/* callback to gnome keyring password finder
 * Will sync the password between the keyring and pidgin */
static void keyring_password_find_cb(GnomeKeyringResult res,
        const gchar *password, gpointer user_data) {
    PurpleAccount *account = (PurpleAccount *)user_data;
    gboolean remember = purple_account_get_remember_password(account);
    /* set the purple account to not remember passwords */
    purple_account_set_remember_password(account, FALSE);
    /* if the password was not found in the keyring
     * and the password exists in pidgin
     * and the password was set to be remembered
     */
    if (res != GNOME_KEYRING_RESULT_OK &&
        account->password != NULL
        && remember) {
        /* copy it from pidgin to the keyring */
        keyring_password_store(account, account->password);
        return;
    }
   /* if the stored passwords do not match */
    if (res == GNOME_KEYRING_RESULT_OK && account->password != NULL) {
        if (strcmp(password, account->password) != 0) {
            /* update the keyring with the pidgin password */
            keyring_password_store(account, account->password);
            return;
        }
    }
    /* if this code is excecuted, it means that keyring_password_store was
     * not called, so the memory_clearing_function needs to be called now
     */
    memory_clearing_function(account);
}

/* store a password in the keyring */
static void keyring_password_store(PurpleAccount *account,
                                   char *password) {
    gnome_keyring_store_password(
            GNOME_KEYRING_NETWORK_PASSWORD,
            GNOME_KEYRING_DEFAULT,
            "pidgin account password",
            password, keyring_password_store_cb,
            account, NULL,
            
            "user", account->username,
            "protocol", account->protocol_id,
            NULL);
}

/* this is mainly here for stability issues because the program may
 * crash if there is no callback function.
 * It does not actually do anything */
static void keyring_password_store_cb(GnomeKeyringResult res, gpointer data) {
    PurpleAccount *account = (PurpleAccount *)data;
    memory_clearing_function(account);
    return;
}

static void memory_clearing_function(PurpleAccount *account) {
    gboolean clear_memory = purple_prefs_get_bool(
                            "/plugins/core/gnome-keyring/clear_memory");
    if (clear_memory) {
        if (account->password != NULL) {
            free(account->password);
            account->password = NULL;
        }
    }
}

/* callback to whenever a function tries to connect
 * this needs to ensure that there is a password
 * this may happen if the password was disabled, then later re-enabled */
static void connecting_cb(PurpleAccount *account, gpointer data) {
    if (account->password == NULL) {
        gchar *password;
        GnomeKeyringResult res = gnome_keyring_find_password_sync(
            GNOME_KEYRING_NETWORK_PASSWORD, &password,
            "user", account->username,
            "protocol", account->protocol_id, NULL);
        if (res == GNOME_KEYRING_RESULT_OK) {
            purple_account_set_password(account, password);
        }
    }
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

static PurplePluginUiInfo prefs_info = {
    get_pref_frame, 0, NULL, NULL, NULL, NULL, NULL
};

static PurplePluginPrefFrame * get_pref_frame(PurplePlugin *plugin) {
    PurplePluginPrefFrame *frame = purple_plugin_pref_frame_new();
    gchar *label = g_strdup_printf("Should passwords be wiped from pidgin's"
               " memory?\nNote: enabling this setting might break things,\n"
               "as some functions might need the password to be in memory.");
    PurplePluginPref *pref = purple_plugin_pref_new_with_name_and_label(
            "/plugins/core/gnome-keyring/clear_memory",
            label);
    purple_plugin_pref_frame_add(frame, pref);
    return frame;
}


static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC, PURPLE_MAJOR_VERSION, PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_HIGHEST,
    
    "core-gnome-keyring",
    "Gnome Keyring",
    /* version */
    "1.20",

    "Save passwords to the gnome keyring instead of as plaintext",
    "Save passwords to the gnome keyring instead of as plaintext",
    "Ali Ebrahim",
    "http://code.google.com/p/pidgin-gnome-keyring/",     
    
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
    
static void init_plugin(PurplePlugin *plugin) {                       
    purple_prefs_add_none("/plugins/core/gnome-keyring");
    purple_prefs_add_bool("/plugins/core/gnome-keyring/clear_memory", FALSE);
}

PURPLE_INIT_PLUGIN(gnome-keyring, init_plugin, info)
