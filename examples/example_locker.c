#include <stdio.h>
#include <stdlib.h>
#include "locker.h"

int main(int argc, char* argv[]) {
    MP3tunesLocker* mp3tunes_locker;

    argc = argc;
    argv = argv;

    g_thread_init(NULL);
    g_type_init();

    mp3tunes_locker = mp3tunes_locker_new_with_email_and_password("9999999999", "ERROR123", "demo@mp3tunes.com", "demo");

    GList* playlists = mp3tunes_locker_get_playlists(mp3tunes_locker);

    GList* playlist_item = g_list_first(playlists);

    while (playlist_item != NULL) {
        mp3tunes_locker_playlist_t* playlist = (mp3tunes_locker_playlist_t*)playlist_item->data;
        g_print("Playlist ID: %s Title: %s\n", playlist->playlistId, playlist->title);

        GList* tracks = mp3tunes_locker_get_tracks_with_playlist_id(mp3tunes_locker, playlist->playlistId);

        GList* track_item = g_list_first(tracks);

        while (track_item != NULL) {
            mp3tunes_locker_track_t* track = (mp3tunes_locker_track_t*)track_item->data;
            g_print("    Track ID: %d Title %s\n", track->trackId, track->trackTitle);
            track_item = g_list_next(track_item);
        }
        g_list_free(tracks);
        playlist_item = g_list_next(playlist_item);
    }
    g_list_free(playlists);


    GList* artists = mp3tunes_locker_get_artists(mp3tunes_locker);

    GList* artist_item = g_list_first(artists);
    while (artist_item != NULL) {
        mp3tunes_locker_artist_t* artist = (mp3tunes_locker_artist_t*)artist_item->data;
        printf("Artist ID: %d Name: %s\n", artist->artistId, artist->artistName);
        GList* albums = mp3tunes_locker_get_albums_with_artist_id(mp3tunes_locker, artist->artistId);
        
        GList* album_item = g_list_first(albums);
        while (album_item != NULL) {
            mp3tunes_locker_album_t* album = (mp3tunes_locker_album_t*)album_item->data;
            printf("    Album ID: %d Name: %s\n", album->albumId, album->albumTitle);
            GList* tracks = mp3tunes_locker_get_tracks_with_album_id(mp3tunes_locker, album->albumId);

            GList* track_item = g_list_first(tracks);
            while (track_item != NULL) {
                mp3tunes_locker_track_t* track = (mp3tunes_locker_track_t*)track_item->data;
                printf("        Track ID: %d Title: %s\n", track->trackId, track->trackTitle);
                track_item = g_list_next(track_item);
            }
            g_list_free(tracks);

            album_item = g_list_next(album_item);
        } 
        g_list_free(albums);

        artist_item = g_list_next(artist_item);
    }
    g_list_free(artists);
    
    /*
    mp3tunes_locker_sync_down(mp3tunes_locker, "recent", NULL, "3", NULL, NULL);

    mp3tunes_locker_deinit(&mp3tunes_locker);
    */

    return 0;
}
