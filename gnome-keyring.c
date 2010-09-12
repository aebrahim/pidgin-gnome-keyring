#define PURPLE_PLUGINS

#include <plugin.h>
#include <version.h>

#include <account.h>
#include <signal.h>
#include <core.h>

#include <gnome-keyring.h>
#include <glib.h>

struct container {
    gboolean reenable;
    PurpleAccount *account;
};


/* function prototypes */
static void password_sync(struct container *cont);
static void password_find_cb(GnomeKeyringResult res,
        const gchar *password, gpointer user_data);
static void password_store(PurpleAccount *account, char *password);
static void password_store_cb(GnomeKeyringResult res, gpointer user_data);
static void sign_in_cb(PurpleAccount *account, gpointer data);
static void password_ensure_save_cb(GnomeKeyringResult res,
        const gchar *password, gpointer user_data);

/* function definitions */

/* function called when the plugin starts */
static gboolean plugin_load(PurplePlugin *plugin){

    GList *accountsList;
    void *accountshandle = purple_accounts_get_handle();
    
    /* for every account */
    for (accountsList = purple_accounts_get_all();
         accountsList != NULL;
         accountsList = accountsList->next) {
        PurpleAccount *account = (PurpleAccount *)accountsList->data;
        const char *ui = purple_core_get_ui();
        struct container *cont = (struct container *)malloc(sizeof(struct container));
        cont->account = account;
        cont->reenable = purple_account_get_enabled(account, ui);
            
        purple_account_set_enabled(account, ui, FALSE);
        /* set the account to not remember passwords */
        purple_account_set_remember_password(account, FALSE);
        
        /* synchronize the passwords for each account */
        password_sync(cont);
        
    }
    /* create a signal which monitors whenever an account signs in,
     * so that the callback function can synchronize accounts
     */
    purple_signal_connect(accountshandle, "account-signed-on", plugin,
            PURPLE_CALLBACK(sign_in_cb), NULL); 
    return TRUE;
}

/* synchronize passwords */
static void password_sync(struct container *cont) {
    gnome_keyring_find_password(GNOME_KEYRING_NETWORK_PASSWORD,
            password_find_cb,
            cont, NULL,

            "user", cont->account->username,
            "protocol", cont->account->protocol_id,
            NULL);
}

/* callback to find_password from sync function
 * actually performs the synchronization
 */
static void password_find_cb(GnomeKeyringResult res,
        const gchar *password, gpointer user_data) {
    struct container *cont = (struct container *)user_data;
    PurpleAccount *account = cont->account;
    /* if the password was found on the keyring
     * and the password does not exist in pidgin
     */
    /* if the account is enabled, briefly disable it, then re-enable it */
    const char *ui = purple_core_get_ui();

    if (res == GNOME_KEYRING_RESULT_OK && account->password == NULL) {
        /* copy it from the keyring to pidgin */
        purple_account_set_password(account, password);
    }
    /* if the password was not found in the keyring
     * and the password exists in pidgin
     */
    if (res != GNOME_KEYRING_RESULT_OK && account->password != NULL) {
        /* copy it from pidgin to the keyring */
        password_store(account, account->password);
    }
    if (cont->reenable) purple_account_set_enabled(account, ui, TRUE);
}

/* store a password in the keyring */
static void password_store(PurpleAccount *account, char *password) {
    gnome_keyring_store_password(
            GNOME_KEYRING_NETWORK_PASSWORD,
            GNOME_KEYRING_DEFAULT,
            "pidgin account password",
            password,
            password_store_cb,
            NULL, NULL,
            
            "user", account->username,
            "protocol", account->protocol_id,
            NULL);
}

static void password_store_cb(GnomeKeyringResult res, gpointer user_data) {
    return;
}

/* callback to whenever an account is signed in */
static void sign_in_cb(PurpleAccount *account, gpointer data) {
    /* whenever an account is signed in, make sure the password is saved */
    gnome_keyring_find_password(GNOME_KEYRING_NETWORK_PASSWORD,
            password_ensure_save_cb,
            account, NULL,

            "user", account->username,
            "protocol", account->protocol_id,
            NULL);
}

static void password_ensure_save_cb(GnomeKeyringResult res,
        const gchar *password, gpointer user_data) {
    PurpleAccount *account = (PurpleAccount *)user_data;
    /* if the password was not found in the keyring
     * and the password exists in pidgin
     */
    if (res != GNOME_KEYRING_RESULT_OK && account->password != NULL) {
        /* copy it from pidgin to the keyring */
        password_store(account, account->password);
        return;
    }
}


static gboolean plugin_unload(PurplePlugin *plugin) {
    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_HIGHEST,
    
    "core-gnome-keyring",
    "Gnome Keyring",
    "1.02",

    "Save passwords to gnome keyring instead of as plaintext",
    "Save passwords to gnome keyring instead of as plaintext",
    "noobster721@gmail.com",
    "http://code.google.com/p/pidgin-gnome-keyring/",     
    
    plugin_load,                   
    plugin_unload,                          
    NULL,                          
                                   
    NULL,                          
    NULL,                          
    NULL,                        
    NULL,                   
    NULL,                          
    NULL,                          
    NULL,                          
    NULL                           
};                               
    
static void                        
init_plugin(PurplePlugin *plugin)
{                                  
}

PURPLE_INIT_PLUGIN(gnome-keyring, init_plugin, info)
