#include <stdio.h>
#include <stdlib.h>
#include "locker.h"

void download_handler(MP3tunesLocker* self, mp3tunes_locker_download_t* download, void* void_null) {
    self = self;
    void_null = void_null;
    g_print("New download: %s\n", download->filekey);
}

int main(int argc, char* argv[]) {
    MP3tunesLocker* mp3tunes_locker;

    argc = argc;
    argv = argv;

    g_thread_init(NULL);
    g_type_init();

    mp3tunes_locker = mp3tunes_locker_new_with_email_and_password("9999999999", "ERROR123", "demo@mp3tunes.com", "demo");

    g_signal_connect(mp3tunes_locker, "download", G_CALLBACK(download_handler), NULL);

    mp3tunes_locker_sync_down(mp3tunes_locker, "recent", NULL, "3", NULL, NULL);

    return 0;
}
