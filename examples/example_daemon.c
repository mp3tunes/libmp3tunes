#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "harmony.h"

/* Linux Specific Headers */
#include <sys/vfs.h>
#include <errno.h>

GMainLoop *main_loop;

void error_signal_handler(MP3tunesHarmony* harmony, gpointer null_pointer);
void state_change_signal_handler(MP3tunesHarmony* harmony, guint32 state,  gpointer null_pointer);
void download_signal_handler(MP3tunesHarmony* harmony, gpointer null_pointer);

/* Error signal handler. 
 *
 * This signal is emitted whenever there is a user fixable error from inside the
 * library. Most of these errors are from inside of the Jabber library.
 *
 * Whenever this signal handler is called, harmony->error will be set to a valid
 * GError pointer. The message field of that structure will contain the error
 * and should be displayed to the user and the connection reset by calling
 * mp3tunes_harmony_disconnect and a reconnection user initiated.
 */
void error_signal_handler(MP3tunesHarmony* harmony, gpointer null_pointer) {
    GError *err;
    null_pointer = null_pointer;
    g_error("Error: %s\n", harmony->error->message);
    mp3tunes_harmony_disconnect(harmony, &err);
    if (err) {
        g_error("Error disconnecting: %s\n", err->message);
        /* If there is an error disconnecting something has probably gone
         * very wrong and reconnection should not be attempted till the user
         * re-initiates it */
        return;
    }
}

/* State change signal handler.
 *
 * This signal is emitted whenever the state of the connection changes during
 * the Harmony authentication process. The state variable will be set to one of
 * the values of harmony_state_t with the following meanings.
 *
 * MP3TUNES_HARMONY_STATE_DISCONNECTED:
 *     The connection to the server has been disconnected. Occurs a couple of
 *     times during the authentication process. Nothing to act on unless was
 *     already in the CONNECTED state.
 *
 * MP3TUNES_HARMONY_STATE_CONNECTED:
 *     The connection completed successfully. All to be done from here is to
 *     associate the download handler if needed and wait for download messages.
 *
 * MP3TUNES_HARMONY_STATE_WAITING_FOR_PIN:
 *     The client has authenticated and is waiting for the response to a
 *     harmonyPin message. Unless there is a problem with the Conductor this
 *     state should be left almost immediately and moved back into disconnected
 *     before going to WAITING_FOR_EMAIL. Useful for having a progress message
 *     during authentication.
 *
 * MP3TUNES_HARMONY_STATE_WAITING_FOR_EMAIL:
 *     The client has authenticated and is waiting for a response to the
 *     harmonyEmail message. In this state, the action to take is to display to
 *     the user the pin, found by calling mp3tunes_harmony_get_pin, and request
 *     for them to log into mp3tunes.com and have them add the pin to their
 *     devices tab. Upon receiving the reply to this you will be authenticated.
 */

void state_change_signal_handler(MP3tunesHarmony* harmony, guint32 state,  gpointer null_pointer) {
    null_pointer = null_pointer;
    switch (state) {
        case MP3TUNES_HARMONY_STATE_DISCONNECTED:
            g_print("Disconnected.\n");
            /* Do nothing here */
            break;
        case MP3TUNES_HARMONY_STATE_CONNECTED:
            g_print("Connected! Waiting for download requests!\n");
            /* At this point, it would be best to store the pin, if you haven't
             * already, and the email in some somewhat permenant storage for
             * when reauthenticating.
             */
            break;
        case MP3TUNES_HARMONY_STATE_WAITING_FOR_PIN:
            g_print("Connection in process!\n");
            /* At this point, just update the user status. */
            break;
        case MP3TUNES_HARMONY_STATE_WAITING_FOR_EMAIL:
            g_print("Please login to mp3tunes.com and add the pin '%s' to your devices.\n", mp3tunes_harmony_get_pin(harmony));
            /* At this point, it would be best to store the pin in case the
             * network connection drops. As well, display to the user a status
             * message to have them perform the website authentication action. 
             */
            break;
    } 
}

void download_pending_signal_handler(MP3tunesHarmony* harmony, gpointer void_mp3tunes_harmony_download, gpointer null_pointer) {
    mp3tunes_harmony_download_t *download = (mp3tunes_harmony_download_t*)void_mp3tunes_harmony_download;
    harmony = harmony;
    null_pointer = null_pointer;
    g_print("Got message about %s by %s on %s\n", download->track_title, download->artist_name, download->album_title);
}

void download_ready_signal_handler(MP3tunesHarmony* harmony, gpointer void_mp3tunes_harmony_download, gpointer null_pointer) {
    mp3tunes_harmony_download_t *download = (mp3tunes_harmony_download_t*)void_mp3tunes_harmony_download;
    harmony = harmony;
    null_pointer = null_pointer;
    g_print("Downloading %s by %s on %s from URL: %s.\n", download->track_title, download->artist_name, download->album_title, download->url);
    if (strcmp(download->file_key, "dummy_file_key_5") == 0) {
        g_main_loop_quit(main_loop);
    }
    mp3tunes_harmony_download_deinit(&download);
}

int main(int argc, char** argv) {
    /* Harmony object. */
    MP3tunesHarmony* harmony;

    /* GError object for errors in connection */
    GError *err = NULL;

    /* Storage for the device identifier. */
    char* identifier;

    /* Linux specific variable for getting total and available sizes for the
     * file system
     */

    struct statfs fsstats;
    unsigned long long total_bytes;
    unsigned long long available_bytes;
   
    /* g_type_init required for using the GObjects for Harmony. */
    g_type_init();

    if (argc != 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    identifier = argv[1];

    /* Initializer for harmony. */
    harmony = mp3tunes_harmony_new();

    /* Set the error signal handler. */
    g_signal_connect(harmony, "error", G_CALLBACK(error_signal_handler), NULL);

    /* Set the state change signal handler. */
    g_signal_connect(harmony, "state_change", G_CALLBACK(state_change_signal_handler), NULL);

    /* Set the download signal handler. */
    g_signal_connect(harmony, "download-ready", G_CALLBACK(download_ready_signal_handler), NULL);
    
    g_signal_connect(harmony, "download-pending", G_CALLBACK(download_pending_signal_handler), NULL);
    
    /* Set the device identifier.
     *
     * 000000000001 does a complete login without issue and sends a sample
     * harmonyDownload message as well.
     * 
     * 000000000002 only goes as far as HARMONY_WAITING_FOR_EMAIL
     *
     * 000000000003 
     */
    
    mp3tunes_harmony_set_identifier(harmony, identifier);

    mp3tunes_harmony_set_device_attribute(harmony, "device-description", "Example Daemon");

    /* Linux specific method for getting total and available amount of disk
     * space */
    if (statfs(".", &fsstats) != 0) {
        perror("statfs failed");
        return 1;
    }
    total_bytes = fsstats.f_bsize * fsstats.f_blocks;
    available_bytes = fsstats.f_bsize * fsstats.f_bavail;
    mp3tunes_harmony_set_device_attribute(harmony, "total-bytes", &total_bytes);
    mp3tunes_harmony_set_device_attribute(harmony, "available-bytes", &available_bytes);

    /* Configure main loop */
    main_loop = g_main_loop_new(NULL, FALSE);

    /* Start the connection */
    mp3tunes_harmony_connect(harmony, &err);
    /* Check for errors on the connection */
    if (err) {
        g_error("Error: %s\n", err->message);
    }

    /* Run the main loop */
    g_main_loop_run(main_loop);

    return 0;
}

