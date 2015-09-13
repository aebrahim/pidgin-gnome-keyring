#include "keyring_plugin.h"
#include "keyring_plugin_defs.h"

#include <libsecret/secret.h>
#include <glib.h>
#include <string.h>

#include <debug.h>

/* function prototypes */

static void keyring_password_store_cb(GObject *source, GAsyncResult *res,
        gpointer data);


/* store a password in the keyring (asyncronously) */
void keyring_password_store(PurpleAccount *account) {
    gchar *password = account->password;
    secret_password_store(
            SECRET_SCHEMA_COMPAT_NETWORK,
            purple_prefs_get_string("/plugins/core/gnome-keyring/keyring_name"),
            "pidgin account password",
            password, NULL,
            keyring_password_store_cb, account,

            "user", account->username,
            "protocol", account->protocol_id,
            NULL);
}

/* Complete asyncronous keyring storage */
static void keyring_password_store_cb(GObject *source, GAsyncResult *res,
        gpointer data) {
    GError *error = NULL;
    PurpleAccount *account = (PurpleAccount *)data;

    secret_password_store_finish(res, &error);
    if (error != NULL) {
        purple_debug_info(PLUGIN_ID,
                "store_finish error in store_cb: %s\n",
                error->message);
        g_error_free(error);
    }
    else {
        purple_debug_info(PLUGIN_ID,
                "successfully saved password for %s\n",
                account->username);
    }
}

void keyring_password_get(PurpleAccount *account) {
    gchar *password;
    GError *error = NULL;
    password = secret_password_lookup_sync(SECRET_SCHEMA_COMPAT_NETWORK,
            NULL, &error, "user", account->username,
            "protocol", account->protocol_id, NULL);
    if (error != NULL) {
        purple_debug_info(PLUGIN_ID, "password retrieval error for %s: %s\n",
                error->message, account->password);
        g_error_free(error);
    }
    else if (password == NULL) {
        purple_debug_info(PLUGIN_ID, "no password found for %s\n",
                account->password);
    }
    else {
        purple_account_set_password(account, password);
        purple_debug_info(PLUGIN_ID, "successfully retrieved password\n");
        g_free(password);
    }
}


PurplePluginPrefFrame * get_pref_frame(PurplePlugin *plugin) {
    PurplePluginPrefFrame *frame = purple_plugin_pref_frame_new();
    gchar *label = g_strdup_printf("Clear plaintext passwords from memory");
    PurplePluginPref *pref = purple_plugin_pref_new_with_name_and_label(
            "/plugins/core/gnome-keyring/clear_memory",
            label);
    purple_plugin_pref_frame_add(frame, pref);
    purple_plugin_pref_frame_add(frame,
		purple_plugin_pref_new_with_name_and_label(
			"/plugins/core/gnome-keyring/keyring_name", "Keyring name"
		)
    );
    return frame;
}


void init_keyring_plugin(PurplePlugin *plugin) {                       
    purple_prefs_add_none("/plugins/core/gnome-keyring");
    purple_prefs_add_string("/plugins/core/gnome-keyring/keyring_name", SECRET_COLLECTION_DEFAULT);
}

