#include <stdio.h>
#include <stdlib.h>
#include "locker.h"

int main(int argc, char* argv[]) {
    MP3tunesLocker* mp3tunes_locker;

    /*
    mp3tunes_locker_playlist_list_t *playlist_list;
    mp3tunes_locker_list_item_t *playlist_item;
    mp3tunes_locker_playlist_t *playlist;
    */

    /*
    mp3tunes_locker_artist_list_t *artists_list;
    mp3tunes_locker_list_item_t *artist_item;
    mp3tunes_locker_artist_t *artist;
    */

    /*
    mp3tunes_locker_album_list_t *albums_list;
    mp3tunes_locker_list_item_t *album_item;
    mp3tunes_locker_album_t *album;
    
    mp3tunes_locker_track_list_t *tracks_list;
    mp3tunes_locker_list_item_t *track_item;
    mp3tunes_locker_track_t *track;
    */

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
        playlist_item = g_list_next(playlist_item);
    }


    /*
    playlist_item = playlist_list->first;
    while (playlist_item != NULL) {
        playlist = (mp3tunes_locker_playlist_t*)playlist_item->value;
        printf("Playlist ID: %s Title: %s\n", playlist->playlistId, playlist->title);
        mp3tunes_locker_tracks_with_playlist_id(mp3tunes_locker, &tracks_list, playlist->playlistId);

        track_item = tracks_list->first;
        while (track_item != NULL) {
            track = (mp3tunes_locker_track_t*)track_item->value;
            printf("    Track ID: %d Title %s\n", track->trackId, track->trackTitle);
            track_item = track_item->next;
        }
        mp3tunes_locker_track_list_deinit(&tracks_list);
        
        playlist_item = playlist_item->next;
    }
    mp3tunes_locker_playlist_list_deinit(&playlist_list);
    */


    /*
    mp3tunes_locker_artists(mp3tunes_locker, &artists_list);

    artist_item = artists_list->first;
    while (artist_item != NULL) {
        artist = (mp3tunes_locker_artist_t*)artist_item->value;
        printf("Artist ID: %d Name: %s\n", artist->artistId, artist->artistName);
        */

        /*
        mp3tunes_locker_albums_with_artist_id(mp3tunes_locker, &albums_list, artist->artistId);
        
        album_item = albums_list->first;
        while (album_item != NULL) {
            album = (mp3tunes_locker_album_t*)album_item->value;
            printf("    Album ID: %d Name: %s\n", album->albumId, album->albumTitle);
            mp3tunes_locker_tracks_with_album_id(mp3tunes_locker, &tracks_list, album->albumId);

            track_item = tracks_list->first;
            while (track_item != NULL) {
                track = (mp3tunes_locker_track_t*)track_item->value;
                printf("        Track ID: %d Title: %s\n", track->trackId, track->trackTitle);
                track_item = track_item->next;
            }
            mp3tunes_locker_track_list_deinit(&tracks_list);

            album_item = album_item->next;
        } 
        mp3tunes_locker_album_list_deinit(&albums_list);
        */

    /*
        artist_item = artist_item->next;
    }
    mp3tunes_locker_artist_list_deinit(&artists_list);
    
    mp3tunes_locker_sync_down(mp3tunes_locker, "recent", NULL, "3", NULL, NULL);

    mp3tunes_locker_deinit(&mp3tunes_locker);
    */

    return 0;
}
