 /*
 * Copyright (C) 2008 MP3tunes, LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define _GNU_SOURCE
#include <libsoup/soup.h>
#include <openssl/md5.h>
#include "locker.h"
#include "xml_xpath.h"
#include "md5.h"

G_DEFINE_TYPE(MP3tunesLocker, mp3tunes_locker, G_TYPE_OBJECT);

enum {
    MP3TUNES_LOCKER_0,
    MP3TUNES_LOCKER_PARTNER_TOKEN,
    MP3TUNES_LOCKER_DEVICE_IDENTIFIER,
    MP3TUNES_LOCKER_EMAIL,
    MP3TUNES_LOCKER_PASSWORD,
};

struct _MP3tunesLockerPrivate {
    gchar* partner_token;
    gchar* device_identifier;
    gchar* email;
    gchar* pin;
    gchar* password;
    gchar* session_id;
    gchar* server_api;
    gchar* server_content;
    gchar* server_login;
    SoupSession* soup_session;
    gboolean debug;
};

#define MP3TUNES_LOCKER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MP3TUNES_TYPE_LOCKER, MP3tunesLockerPrivate))

int mp3tunes_locker_login(MP3tunesLocker*);

static void mp3tunes_locker_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
    MP3tunesLocker* self = MP3TUNES_LOCKER(object);
    switch (property_id) {
        case MP3TUNES_LOCKER_PARTNER_TOKEN:
            g_free(self->priv->partner_token);
            self->priv->partner_token = g_value_dup_string(value);
            break;
        case MP3TUNES_LOCKER_DEVICE_IDENTIFIER:
            g_free(self->priv->device_identifier);
            self->priv->device_identifier = g_value_dup_string(value);
            if (strcmp(self->priv->device_identifier, "ERROR") == 0) {
                g_error("MP3tunesLocker Device Identifier not set.");
            }
            break;
        case MP3TUNES_LOCKER_EMAIL:
            g_free(self->priv->email);
            self->priv->email = g_value_dup_string(value);
            break;
        case MP3TUNES_LOCKER_PASSWORD:
            g_free(self->priv->password);
            self->priv->password = g_value_dup_string(value);
            break;
       default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void mp3tunes_locker_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
    MP3tunesLocker* self = MP3TUNES_LOCKER(object);
    switch (property_id) {
        case MP3TUNES_LOCKER_PARTNER_TOKEN:
            g_value_set_string(value, self->priv->partner_token);
            break;
        case MP3TUNES_LOCKER_DEVICE_IDENTIFIER:
            g_value_set_string(value, self->priv->device_identifier);
            break;
        case MP3TUNES_LOCKER_EMAIL:
            g_value_set_string(value, self->priv->email);
            break;
        case MP3TUNES_LOCKER_PASSWORD:
            g_value_set_string(value, self->priv->password);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void mp3tunes_locker_class_init(MP3tunesLockerClass *klass) {
    g_type_class_add_private(klass, sizeof(MP3tunesLockerPrivate));
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    GParamSpec *partner_token_param_spec;
    GParamSpec *device_identifier_param_spec;
    GParamSpec *email_param_spec;
    GParamSpec *password_param_spec;

    gobject_class->set_property = mp3tunes_locker_set_property;
    gobject_class->get_property = mp3tunes_locker_get_property;

    partner_token_param_spec = g_param_spec_string ("partner-token",
                                                    "MP3tunes Partner Token",
                                                    "Set MP3tunes Partner Token",
                                                    "9999999999" /* default value */,
                                                    G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, MP3TUNES_LOCKER_PARTNER_TOKEN, partner_token_param_spec);
    
    device_identifier_param_spec = g_param_spec_string ("device-identifier",
                                                        "Device Identifier",
                                                        "Set MP3tunes Device Identifier",
                                                        "ERROR" /* default value */,
                                                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, MP3TUNES_LOCKER_DEVICE_IDENTIFIER, device_identifier_param_spec);

    email_param_spec = g_param_spec_string ("email",
                                            "MP3tunes Login Email",
                                            "Set MP3tunes Login Email",
                                            "" /* default value */,
                                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, MP3TUNES_LOCKER_EMAIL, email_param_spec);

    password_param_spec = g_param_spec_string ("password",
                                            "MP3tunes Login Password",
                                            "Set MP3tunes Login Password",
                                            "" /* default value */,
                                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, MP3TUNES_LOCKER_PASSWORD, password_param_spec);

}

static void mp3tunes_locker_init(MP3tunesLocker *self) {
    MP3tunesLockerPrivate *priv;
    self->priv = priv = MP3TUNES_LOCKER_GET_PRIVATE(self);

    priv->partner_token = NULL;
    priv->device_identifier = NULL;
    priv->session_id = NULL;

    priv->server_api = getenv("MP3TUNES_SERVER_API");
    if (priv->server_api == NULL) {
        priv->server_api = MP3TUNES_SERVER_API_URL;
    }

    priv->server_content = getenv("MP3TUNES_SERVER_CONTENT");
    if (priv->server_content == NULL) {
        priv->server_content = MP3TUNES_SERVER_CONTENT_URL;
    }

    priv->server_login = getenv("MP3TUNES_SERVER_LOGIN");
    if (priv->server_login == NULL) {
        priv->server_login = MP3TUNES_SERVER_LOGIN_URL;
    }

    priv->debug = FALSE;

    if (getenv("MP3TUNES_DEBUG") != NULL) {
        priv->debug = TRUE;
    }

    priv->soup_session = soup_session_sync_new();
    /* TODO: Enable for a later version of libsoup */
    /*
    GValue user_agent_value = {0};
    g_value_init(&user_agent_value, G_TYPE_STRING);
    g_value_set_string(&user_agent_value, "libmp3tunes/1.0 ");
    g_object_set_property(G_OBJECT(priv->soup_session), "user-agent", &user_agent_value);
    g_value_unset(&user_agent_value);
    */
}

MP3tunesLocker* mp3tunes_locker_new(gchar* partner_token, gchar* device_identifier) {
    return g_object_new(MP3TUNES_TYPE_LOCKER, "partner-token", partner_token, "device-identifier", device_identifier, NULL);
}

MP3tunesLocker* mp3tunes_locker_new_with_email_and_password(gchar* partner_token, gchar* device_identifier, gchar* email, gchar* password) {
    return g_object_new(MP3TUNES_TYPE_LOCKER, "partner-token", partner_token, "device-identifier", device_identifier, "email", email, "password", password, NULL);
}

static SoupMessage* mp3tunes_locker_api_generate_request_valist(MP3tunesLocker *self, int server, gchar* path, gchar* first_name, va_list argp) {
    gchar *server_url;
    gchar *name, *value;
    gchar *encoded_name, *encoded_value;

    switch (server) {
        case MP3TUNES_SERVER_LOGIN:
            server_url = self->priv->server_login;
            break;
        case MP3TUNES_SERVER_CONTENT:
            server_url = self->priv->server_content;
            break;
        case MP3TUNES_SERVER_API:
            server_url = self->priv->server_api;
            break;
        default:
            server_url = NULL;
            break;
    }
    if (server_url == NULL) {
        return NULL;
    }

    GString* url = g_string_new("");

    g_string_printf(url, "http://%s/%s?", server_url, path);
    name = first_name;
    while (name) {
        value = va_arg(argp, char*);

        encoded_name = soup_uri_encode(name, NULL);
        encoded_value = soup_uri_encode(value, NULL);
        g_string_append_printf(url, "%s=%s&", encoded_name, encoded_value);
        g_free(encoded_name);
        g_free(encoded_value);

        name = va_arg(argp, char*);
    }

    if (server != MP3TUNES_SERVER_LOGIN) {
        if (self->priv->session_id == NULL) {
            mp3tunes_locker_login(self);
        } 
        if (server == MP3TUNES_SERVER_API) {
            g_string_append_printf(url, "output=xml&sid=%s&partner_token=%s", self->priv->session_id, self->priv->partner_token);
        } else {
            g_string_append_printf(url, "sid=%s&partner_token=%s", self->priv->session_id, self->priv->partner_token);
        }
    } else {
        g_string_append_printf(url, "output=xml&partner_token=%s", self->priv->partner_token);
    }

    SoupMessage *msg = soup_message_new("GET", g_string_free(url, FALSE));
    
    return msg;

}

/*
static SoupMessage* mp3tunes_locker_api_generate_request(MP3tunesLocker *obj, int server, char* path, char* first_name, ...) {
    va_list argp;
    SoupMessage *msg;
    va_start(argp, first_name);
    msg = mp3tunes_locker_api_generate_request_valist(obj, server, path, first_name, argp);
    va_end(argp);
    return msg;
}
*/

static MP3tunesXMLXPath* mp3tunes_locker_api_simple_fetch(MP3tunesLocker *self, int server, char* path, char* first_name, ...) {
    SoupMessage *msg;
    va_list argp;

    va_start(argp, first_name);

    msg = mp3tunes_locker_api_generate_request_valist(self, server, path, first_name, argp);

    va_end(argp);

    guint response = soup_session_send_message(self->priv->soup_session, msg); 

    if (response != SOUP_STATUS_OK) {
        g_free(msg);
        return NULL;
    }

    if (self->priv->debug) {
        g_print("Fetch result:\n%s\n", msg->response.body);
    }

    MP3tunesXMLXPath* xml_xpath = mp3tunes_xml_xpath_new(g_strdup(msg->response.body));

    g_object_unref(msg);

    return xml_xpath;
}

/*

static xml_xpath_t* mp3tunes_locker_api_post_fetch(mp3tunes_locker_object_t *obj, int server, char* path, char* post_data) {
    request_t *request;
    CURLcode res;
    chunk_t *chunk;

    chunk_init(&chunk);

    request = mp3tunes_locker_api_generate_request_valist(obj, server, path, NULL, NULL);

    curl_easy_setopt( request->curl, CURLOPT_URL, request->url );
    curl_easy_setopt( request->curl, CURLOPT_WRITEFUNCTION, write_chunk_callback );
    curl_easy_setopt( request->curl, CURLOPT_WRITEDATA, (void *)chunk );
    curl_easy_setopt( request->curl, CURLOPT_USERAGENT, "liboboe/1.0" );
    curl_easy_setopt( request->curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt( request->curl, CURLOPT_NOPROGRESS, 1 );

    res = curl_easy_perform(request->curl);
    curl_easy_cleanup(request->curl);

    if (res != CURLE_OK) {
        chunk_deinit(&chunk);
        return NULL;
    }

    if (chunk->data == NULL) {
        return NULL;
    }

    printf("Fetch result:\n%s\n", chunk->data);

    xmlDocPtr document = xmlParseDoc((xmlChar*)chunk->data);

    chunk_deinit(&chunk);

    if (document == NULL) {
        return NULL;
    }

    return xml_xpath_init(document);
}
*/


/*
char* mp3tunes_locker_generate_download_url_from_file_key(mp3tunes_locker_object_t *obj, char *file_key) {
    request_t *request;
    char *path = malloc(256*sizeof(char));
    char *ret;
    snprintf(path, 256, "storage/lockerget/%s", file_key);
    request = mp3tunes_locker_api_generate_request(obj, MP3TUNES_SERVER_CONTENT, path, NULL);
    ret = request->url; request->url = NULL;
    free(path);
    mp3tunes_request_deinit(&request);
    return ret;
}

char* mp3tunes_locker_generate_download_url_from_file_key_and_bitrate(mp3tunes_locker_object_t *obj, char *file_key, char* bitrate) {
    request_t *request;
    char *path = malloc(256*sizeof(char));
    char *ret;
    snprintf(path, 256, "storage/lockerget/%s", file_key);
    request = mp3tunes_locker_api_generate_request(obj, MP3TUNES_SERVER_CONTENT, path, "bitrate", bitrate, NULL);
    ret = request->url; request->url = NULL;
    free(path);
    mp3tunes_request_deinit(&request);
    return ret;
}

*/

int mp3tunes_locker_login(MP3tunesLocker *self) {
    MP3tunesXMLXPath* xml_xpath;
    char *status, *session_id;

    xml_xpath = mp3tunes_locker_api_simple_fetch(self, MP3TUNES_SERVER_LOGIN, "api/v1/login/", "username", self->priv->email, "password", self->priv->password, NULL);

    if (xml_xpath == NULL) {
        return -2;
    }

    status = mp3tunes_xml_xpath_get_string(xml_xpath, "/mp3tunes/status");

    if (status[0] != '1') {
        char* error = mp3tunes_xml_xpath_get_string(xml_xpath, "/mp3tunes/errorMessage");
        /* TODO: Deal with errors using GError */
        g_error(error);
        /*obj->error_message = error;*/
        free(status);
        g_object_unref(xml_xpath);
        return -1;
    }
    free(status);

    session_id = mp3tunes_xml_xpath_get_string(xml_xpath, "/mp3tunes/session_id");
    self->priv->session_id = session_id;
    g_object_unref(xml_xpath);

    return 0;
}

/*

int mp3tunes_locker_session_valid(mp3tunes_locker_object_t *obj) {

    request_t *request;
    CURLcode res;
    chunk_t *chunk;

    chunk_init(&chunk);

    request = mp3tunes_locker_api_generate_request_valist(obj, MP3TUNES_SERVER_API, "api/v1/accountData", NULL, NULL);

    curl_easy_setopt( request->curl, CURLOPT_URL, request->url );
    curl_easy_setopt( request->curl, CURLOPT_WRITEFUNCTION, write_chunk_callback );
    curl_easy_setopt( request->curl, CURLOPT_WRITEDATA, (void *)chunk );
    curl_easy_setopt( request->curl, CURLOPT_NOBODY, 1 );
    curl_easy_setopt( request->curl, CURLOPT_USERAGENT, "liboboe/1.0" );
    curl_easy_setopt( request->curl, CURLOPT_HEADER, 1 );
    curl_easy_setopt( request->curl, CURLOPT_NOPROGRESS, 1 );

    res = curl_easy_perform(request->curl);
    curl_easy_cleanup(request->curl);

    if (res != CURLE_OK) {
        chunk_deinit(&chunk);
        return -1;
    }

    if (chunk->data == NULL) {
        return -1;
    }

    char name[] = "X-MP3tunes-ErrorNo";
    char value[] = "401001";
    char * result;
    result = strstr (chunk->data, name);
    if(result != 0)
    {
        int i;
        i=strcspn(result, "\n");
        char * result1 = ( char * ) malloc( i+1 );
        strncpy(result1, result, i);
        result = strstr (result1, value);
        if(result1 != 0) 
        {
            return -1;
        }
    }

    return 0;
}

int mp3tunes_locker_tracks_search( mp3tunes_locker_object_t *obj, mp3tunes_locker_track_list_t **tracks, char *search) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerSearch", "type", "track", "s", search, NULL);

    mp3tunes_locker_track_list_init(tracks);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/trackList/item");

    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_track_t *track = (mp3tunes_locker_track_t*)malloc(sizeof(mp3tunes_locker_track_t));
        memset(track, 0, sizeof(mp3tunes_locker_track_t));

        track->trackId = xml_xpath_get_integer(xml_xpath_context, "trackId");
        track->trackTitle = xml_xpath_get_string(xml_xpath_context, "trackTitle");
        track->trackNumber = xml_xpath_get_integer(xml_xpath_context, "trackNumber");
        track->trackLength = xml_xpath_get_float(xml_xpath_context, "trackLength");
        track->trackFileName = xml_xpath_get_string(xml_xpath_context, "trackFileName");
        track->trackFileKey = xml_xpath_get_string(xml_xpath_context, "trackFileKey");
        track->trackFileSize = xml_xpath_get_integer(xml_xpath_context, "trackFileSize");
        track->downloadURL = xml_xpath_get_string(xml_xpath_context, "downloadURL");
        track->playURL = xml_xpath_get_string(xml_xpath_context, "playURL");
        track->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
        track->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
        track->albumYear = xml_xpath_get_integer(xml_xpath_context, "albumYear");
        track->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        track->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");

        mp3tunes_locker_track_list_add(tracks, track);
        xml_xpath_deinit(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return 0;
}


*/

static GList* _mp3tunes_locker_tracks(MP3tunesLocker *obj, int artist_id, int album_id, char* playlist_id) {
    MP3tunesXMLXPath* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    gchar* artist_id_s = g_strdup_printf("%d", artist_id);
    gchar* album_id_s = g_strdup_printf("%d", artist_id);
    int i;
    GList* tracks = NULL;

    if (playlist_id == NULL) {
        if (artist_id == -1 && album_id == -1) {
            xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", NULL);
        } else if (artist_id != -1 && album_id == -1) {
            xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", "artist_id", artist_id_s, NULL);
        } else if (artist_id == -1 && album_id != -1) {
            xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", "album_id", album_id_s, NULL);
        } else {
            xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", "artist_id", artist_id_s, "album_id", album_id_s, NULL);
        }
    } else {
        xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", "playlist_id", playlist_id, NULL);
    }

    if (xml_xpath == NULL) {
        return NULL;
    }

    xpath_obj = mp3tunes_xml_xpath_query(xml_xpath, "/mp3tunes/trackList/item");

    if (xpath_obj == NULL) {
        return NULL;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        MP3tunesXMLXPath* xml_xpath_context = mp3tunes_xml_xpath_new_with_context(xml_xpath, node);
        mp3tunes_locker_track_t *track = (mp3tunes_locker_track_t*)g_new0(mp3tunes_locker_track_t, 1);

        track->trackId = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "trackId");
        track->trackTitle = mp3tunes_xml_xpath_get_string(xml_xpath_context, "trackTitle");
        track->trackNumber = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "trackNumber");
        track->trackLength = mp3tunes_xml_xpath_get_float(xml_xpath_context, "trackLength");
        track->trackFileName = mp3tunes_xml_xpath_get_string(xml_xpath_context, "trackFileName");
        track->trackFileKey = mp3tunes_xml_xpath_get_string(xml_xpath_context, "trackFileKey");
        track->trackFileSize = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "trackFileSize");
        track->downloadURL = mp3tunes_xml_xpath_get_string(xml_xpath_context, "downloadURL");
        track->playURL = mp3tunes_xml_xpath_get_string(xml_xpath_context, "playURL");
        track->albumId = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "albumId");
        track->albumTitle = mp3tunes_xml_xpath_get_string(xml_xpath_context, "albumTitle");
        track->albumYear = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "albumYear");
        track->artistName = mp3tunes_xml_xpath_get_string(xml_xpath_context, "artistName");
        track->artistId = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "artistId");

        tracks = g_list_append(tracks, track);
        g_object_unref(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    g_object_unref(xml_xpath);
    return tracks;
}

/*

int mp3tunes_locker_tracks(mp3tunes_locker_object_t *obj, mp3tunes_locker_track_list_t **tracks) {
    return _mp3tunes_locker_tracks(obj, tracks, -1, -1, NULL);
}

int mp3tunes_locker_tracks_with_artist_id(mp3tunes_locker_object_t *obj, mp3tunes_locker_track_list_t **tracks, int artist_id) {
    return _mp3tunes_locker_tracks(obj, tracks, artist_id, -1, NULL);
}

int mp3tunes_locker_tracks_with_album_id(mp3tunes_locker_object_t *obj, mp3tunes_locker_track_list_t **tracks, int album_id) {
    return _mp3tunes_locker_tracks(obj, tracks, -1, album_id, NULL);
}

int mp3tunes_locker_tracks_with_artist_id_and_album_id(mp3tunes_locker_object_t *obj, mp3tunes_locker_track_list_t **tracks, int artist_id, int album_id) {
    return _mp3tunes_locker_tracks(obj, tracks, artist_id, album_id, NULL);
}

*/

GList* mp3tunes_locker_get_tracks_with_playlist_id(MP3tunesLocker *obj, char* playlist_id) {
    return _mp3tunes_locker_tracks(obj, -1, -1, playlist_id);
}

/*
int mp3tunes_locker_tracks_with_file_key( mp3tunes_locker_object_t *obj, char *file_keys, mp3tunes_locker_track_list_t **tracks ) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", "key", file_keys, NULL);

    mp3tunes_locker_track_list_init(tracks);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/trackList/item");

    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_track_t *track = (mp3tunes_locker_track_t*)malloc(sizeof(mp3tunes_locker_track_t));
        memset(track, 0, sizeof(mp3tunes_locker_track_t));

        track->trackId = xml_xpath_get_integer(xml_xpath_context, "trackId");
        track->trackTitle = xml_xpath_get_string(xml_xpath_context, "trackTitle");
        track->trackNumber = xml_xpath_get_integer(xml_xpath_context, "trackNumber");
        track->trackLength = xml_xpath_get_float(xml_xpath_context, "trackLength");
        track->trackFileName = xml_xpath_get_string(xml_xpath_context, "trackFileName");
        track->trackFileKey = xml_xpath_get_string(xml_xpath_context, "trackFileKey");
        track->trackFileSize = xml_xpath_get_integer(xml_xpath_context, "trackFileSize");
        track->downloadURL = xml_xpath_get_string(xml_xpath_context, "downloadURL");
        track->playURL = xml_xpath_get_string(xml_xpath_context, "playURL");
        track->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
        track->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
        track->albumYear = xml_xpath_get_integer(xml_xpath_context, "albumYear");
        track->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        track->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");

        mp3tunes_locker_track_list_add(tracks, track);
        xml_xpath_deinit(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return 0;   

}

int mp3tunes_locker_track_with_file_key( mp3tunes_locker_object_t *obj, char *file_key, mp3tunes_locker_track_t **track ) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", "key", file_key, NULL);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/trackList/item");

    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;
    if ( nodeset->nodeNr == 1) {
        node = nodeset->nodeTab[0];

        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_track_t *t = *track = (mp3tunes_locker_track_t*)malloc(sizeof(mp3tunes_locker_track_t));

        t->trackId = xml_xpath_get_integer(xml_xpath_context, "trackId");
        t->trackTitle = xml_xpath_get_string(xml_xpath_context, "trackTitle");
        t->trackNumber = xml_xpath_get_integer(xml_xpath_context, "trackNumber");
        t->trackLength = xml_xpath_get_float(xml_xpath_context, "trackLength");
        t->trackFileName = xml_xpath_get_string(xml_xpath_context, "trackFileName");
        t->trackFileKey = xml_xpath_get_string(xml_xpath_context, "trackFileKey");
        t->trackFileSize = xml_xpath_get_integer(xml_xpath_context, "trackFileSize");
        t->downloadURL = xml_xpath_get_string(xml_xpath_context, "downloadURL");
        t->playURL = xml_xpath_get_string(xml_xpath_context, "playURL");
        t->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
        t->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
        t->albumYear = xml_xpath_get_integer(xml_xpath_context, "albumYear");
        t->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        t->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");

        xml_xpath_deinit(xml_xpath_context);
        xmlXPathFreeObject(xpath_obj);
        xml_xpath_deinit(xml_xpath);
        return 0;
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return -1;   
}

int mp3tunes_locker_artists(mp3tunes_locker_object_t *obj, mp3tunes_locker_artist_list_t **artists) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "artist", NULL);

    mp3tunes_locker_artist_list_init(artists);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/artistList/item");
    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_artist_t *artist = (mp3tunes_locker_artist_t*)malloc(sizeof(mp3tunes_locker_artist_t));
        memset(artist, 0, sizeof(mp3tunes_locker_artist_t));

        artist->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");
        artist->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        artist->artistSize = xml_xpath_get_integer(xml_xpath_context, "artistSize");
        artist->albumCount = xml_xpath_get_integer(xml_xpath_context, "albumCount");
        artist->trackCount = xml_xpath_get_integer(xml_xpath_context, "trackCount");

        mp3tunes_locker_artist_list_add(artists, artist);
        xml_xpath_deinit(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return 0;
}

int mp3tunes_locker_artists_search( mp3tunes_locker_object_t *obj, mp3tunes_locker_artist_list_t **artists, char *search) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerSearch", "type", "artist", "s", search, NULL);
    mp3tunes_locker_artist_list_init(artists);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/artistList/item");
    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_artist_t *artist = (mp3tunes_locker_artist_t*)malloc(sizeof(mp3tunes_locker_artist_t));
        memset(artist, 0, sizeof(mp3tunes_locker_artist_t));

        artist->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");
        artist->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        artist->artistSize = xml_xpath_get_integer(xml_xpath_context, "artistSize");
        artist->albumCount = xml_xpath_get_integer(xml_xpath_context, "albumCount");
        artist->trackCount = xml_xpath_get_integer(xml_xpath_context, "trackCount");

        mp3tunes_locker_artist_list_add(artists, artist);
        xml_xpath_deinit(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return 0;
}


int mp3tunes_locker_albums_with_artist_id(mp3tunes_locker_object_t *obj, mp3tunes_locker_album_list_t **albums, int artist_id) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;
    char artist_id_string[15];

    if (artist_id == -1) {
        xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "album", NULL);
    } else {
        snprintf(artist_id_string, 15, "%d", artist_id);
        xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "album", "artist_id", artist_id_string, NULL);
    }

    mp3tunes_locker_album_list_init(albums);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/albumList/item");

    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_album_t *album = (mp3tunes_locker_album_t*)malloc(sizeof(mp3tunes_locker_album_t));
        memset(album, 0, sizeof(mp3tunes_locker_album_t));

        album->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
        album->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
        album->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");
        album->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        album->trackCount = xml_xpath_get_integer(xml_xpath_context, "trackCount");
        album->albumSize = xml_xpath_get_integer(xml_xpath_context, "albumSize");
        album->hasArt = xml_xpath_get_integer(xml_xpath_context, "hasArt");

        mp3tunes_locker_album_list_add(albums, album);
        xml_xpath_deinit(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return 0;
}

int mp3tunes_locker_albums(mp3tunes_locker_object_t *obj, mp3tunes_locker_album_list_t **albums) {
    return mp3tunes_locker_albums_with_artist_id(obj, albums, -1);
}

int mp3tunes_locker_albums_search(  mp3tunes_locker_object_t *obj, mp3tunes_locker_album_list_t **albums, char *search) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerSearch", "type", "album", "s", search, NULL);

    mp3tunes_locker_album_list_init(albums);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/albumList/item");

    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_album_t *album = (mp3tunes_locker_album_t*)malloc(sizeof(mp3tunes_locker_album_t));
        memset(album, 0, sizeof(mp3tunes_locker_album_t));

        album->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
        album->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
        album->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");
        album->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        album->trackCount = xml_xpath_get_integer(xml_xpath_context, "trackCount");
        album->albumSize = xml_xpath_get_integer(xml_xpath_context, "albumSize");
        album->hasArt = xml_xpath_get_integer(xml_xpath_context, "hasArt");

        mp3tunes_locker_album_list_add(albums, album);
        xml_xpath_deinit(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return 0;
}
*/

GList* mp3tunes_locker_get_playlists(MP3tunesLocker *self) {
    MP3tunesXMLXPath* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;
    GList* playlists = NULL;

    xml_xpath = mp3tunes_locker_api_simple_fetch(self, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "playlist", NULL);

    if (xml_xpath == NULL) {
        return playlists;
    }

    xpath_obj = mp3tunes_xml_xpath_query(xml_xpath, "/mp3tunes/playlistList/item");

    if (xpath_obj == NULL) {
        return playlists;
    }


    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        MP3tunesXMLXPath* xml_xpath_context = mp3tunes_xml_xpath_new_with_context(xml_xpath, node);
        mp3tunes_locker_playlist_t *playlist = (mp3tunes_locker_playlist_t*)malloc(sizeof(mp3tunes_locker_playlist_t));
        memset(playlist, 0, sizeof(mp3tunes_locker_playlist_t));

        playlist->playlistId = mp3tunes_xml_xpath_get_string(xml_xpath_context, "playlistId");
        playlist->playlistTitle = mp3tunes_xml_xpath_get_string(xml_xpath_context, "playlistTitle");
        playlist->title = mp3tunes_xml_xpath_get_string(xml_xpath_context, "title");
        playlist->fileName = mp3tunes_xml_xpath_get_string(xml_xpath_context, "fileName");
        playlist->fileCount = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "fileCount");
        playlist->playlistSize = mp3tunes_xml_xpath_get_integer(xml_xpath_context, "playlistSize");

        playlists = g_list_append(playlists, playlist);
        g_object_unref(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    g_object_unref(xml_xpath);
    return playlists;
}

/*
int mp3tunes_locker_search(mp3tunes_locker_object_t *obj, mp3tunes_locker_artist_list_t **artists, mp3tunes_locker_album_list_t **albums, mp3tunes_locker_track_list_t **tracks, char *query) {
    xml_xpath_t* xml_xpath;

    char type[20] = "";
    if( artists != NULL ) {
      strcat( type, "artist," );
    }
    if( albums != NULL ) {
      strcat( type, "album," );
    }
    if( tracks != NULL ) {
      strcat( type, "track," );
    }
    if( strlen(type) == 0 ) {
      return -1;
    }

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerSearch", "type", type, "s", query, NULL);

    if(artists != NULL) {
        xmlXPathObjectPtr xpath_obj;
        xmlNodeSetPtr nodeset;
        xmlNodePtr node;
        int i;
        mp3tunes_locker_artist_list_init(artists);

        if (xml_xpath == NULL) {
            return -1;
        }

        xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/artistList/item");
        if (xpath_obj == NULL) {
            return -1;
        }

        nodeset = xpath_obj->nodesetval;

        for (i = 0; i < nodeset->nodeNr; i++) {
            node = nodeset->nodeTab[i];
            xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
            mp3tunes_locker_artist_t *artist = (mp3tunes_locker_artist_t*)malloc(sizeof(mp3tunes_locker_artist_t));
            memset(artist, 0, sizeof(mp3tunes_locker_artist_t));

            artist->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");
            artist->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
            artist->artistSize = xml_xpath_get_integer(xml_xpath_context, "artistSize");
            artist->albumCount = xml_xpath_get_integer(xml_xpath_context, "albumCount");
            artist->trackCount = xml_xpath_get_integer(xml_xpath_context, "trackCount");

            mp3tunes_locker_artist_list_add(artists, artist);
            xml_xpath_deinit(xml_xpath_context);
        }
        xmlXPathFreeObject(xpath_obj);
    }

    if( albums != NULL ) {
        xmlXPathObjectPtr xpath_obj;
        xmlNodeSetPtr nodeset;
        xmlNodePtr node;
        int i;

        mp3tunes_locker_album_list_init(albums);

        xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/albumList/item");

        if (xpath_obj == NULL) {
            return -1;
        }

        nodeset = xpath_obj->nodesetval;

        for (i = 0; i < nodeset->nodeNr; i++) {
            node = nodeset->nodeTab[i];
            xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
            mp3tunes_locker_album_t *album = (mp3tunes_locker_album_t*)malloc(sizeof(mp3tunes_locker_album_t));
            memset(album, 0, sizeof(mp3tunes_locker_album_t));

            album->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
            album->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
            album->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");
            album->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
            album->trackCount = xml_xpath_get_integer(xml_xpath_context, "trackCount");
            album->albumSize = xml_xpath_get_integer(xml_xpath_context, "albumSize");
            album->hasArt = xml_xpath_get_integer(xml_xpath_context, "hasArt");

            mp3tunes_locker_album_list_add(albums, album);
            xml_xpath_deinit(xml_xpath_context);
        }
        xmlXPathFreeObject(xpath_obj);
    }
    if( tracks != NULL) {
        xmlXPathObjectPtr xpath_obj;
        xmlNodeSetPtr nodeset;
        xmlNodePtr node;
        int i;

        mp3tunes_locker_track_list_init(tracks);

        xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/trackList/item");

        if (xpath_obj == NULL) {
            return -1;
        }

        nodeset = xpath_obj->nodesetval;

        for (i = 0; i < nodeset->nodeNr; i++) {
            node = nodeset->nodeTab[i];
            xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
            mp3tunes_locker_track_t *track = (mp3tunes_locker_track_t*)malloc(sizeof(mp3tunes_locker_track_t));
            memset(track, 0, sizeof(mp3tunes_locker_track_t));

            track->trackId = xml_xpath_get_integer(xml_xpath_context, "trackId");
            track->trackTitle = xml_xpath_get_string(xml_xpath_context, "trackTitle");
            track->trackNumber = xml_xpath_get_integer(xml_xpath_context, "trackNumber");
            track->trackLength = xml_xpath_get_float(xml_xpath_context, "trackLength");
            track->trackFileName = xml_xpath_get_string(xml_xpath_context, "trackFileName");
            track->trackFileKey = xml_xpath_get_string(xml_xpath_context, "trackFileKey");
            track->trackFileSize = xml_xpath_get_integer(xml_xpath_context, "trackFileSize");
            track->downloadURL = xml_xpath_get_string(xml_xpath_context, "downloadURL");
            track->playURL = xml_xpath_get_string(xml_xpath_context, "playURL");
            track->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
            track->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
            track->albumYear = xml_xpath_get_integer(xml_xpath_context, "albumYear");
            track->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
            track->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");

            mp3tunes_locker_track_list_add(tracks, track);
            xml_xpath_deinit(xml_xpath_context);
        }
        xmlXPathFreeObject(xpath_obj);
    }
    xml_xpath_deinit(xml_xpath);
    return 0;
}

int mp3tunes_locker_sync_down(mp3tunes_locker_object_t *obj, char* type, char* bytes_local, char* files_local, char* keep_local_files, char* playlist_id) {
    xml_xpath_t* xml_xpath;
    xmlBufferPtr buf;
    xmlTextWriterPtr writer;

    buf = xmlBufferCreate();
    if (buf == NULL) {
        return -1;
    }

    writer = xmlNewTextWriterMemory(buf, 0);

    if (writer == NULL) {
        return -1;
    }

    if (xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL) < 0) {
        return -1;
    }

    if (xmlTextWriterStartElement(writer, BAD_CAST "sync") < 0) {
        return -1;
    }

    if (xmlTextWriterStartElement(writer, BAD_CAST "options") < 0) {
        return -1;
    }

    if (xmlTextWriterStartElement(writer, BAD_CAST "direction") < 0) {
        return -1;
    }

    if (xmlTextWriterWriteAttribute(writer, BAD_CAST "sync_down", BAD_CAST "1") < 0) {
        return -1;
    }

    if (xmlTextWriterEndElement(writer) < 0) {
        return -1;
    }

    if (xmlTextWriterStartElement(writer, BAD_CAST "file_sync") < 0) {
        return -1;
    }

    if (xmlTextWriterWriteAttribute(writer, BAD_CAST "type", BAD_CAST type) < 0) {
        return -1;
    }

    if (xmlTextWriterEndElement(writer) < 0) {
        return -1;
    }

    if (xmlTextWriterStartElement(writer, BAD_CAST "max") < 0) {
        return -1;
    }

    if (bytes_local) {
        if (xmlTextWriterWriteAttribute(writer, BAD_CAST "bytes_local", BAD_CAST bytes_local) < 0) {
            return -1;
        }
    }

    if (files_local) {
        if (xmlTextWriterWriteAttribute(writer, BAD_CAST "files_local", BAD_CAST files_local) < 0) {
            return -1;
        }
    }

    if (keep_local_files) {
        if (xmlTextWriterWriteAttribute(writer, BAD_CAST "keep_local_files", BAD_CAST files_local) < 0) {
            return -1;
        }
    }

    if (xmlTextWriterEndElement(writer) < 0) {
        return -1;
    }

    if (playlist_id) {
        if (xmlTextWriterStartElement(writer, BAD_CAST "playlist") < 0) {
            return -1;
        }

        if (xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST playlist_id) < 0) {
            return -1;
        }

        if (xmlTextWriterEndElement(writer) < 0) {
            return -1;
        }
    }

    if (xmlTextWriterEndDocument(writer) < 0) {
        return -1;
    }

    xmlFreeTextWriter(writer);

    xml_xpath = mp3tunes_locker_api_post_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerSync/", (char*)buf->content);
    printf("Sync:\n%s\n", (const char *) buf->content);

    xmlBufferFree(buf);
    return 0;
}

int mp3tunes_locker_generate_track_from_file_key(mp3tunes_locker_object_t *obj, char *file_key, mp3tunes_locker_track_list_t **tracks ) {
    xml_xpath_t* xml_xpath;
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;

    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_API, "api/v1/lockerData/", "type", "track", "key", file_key, NULL);

    mp3tunes_locker_track_list_init(tracks);

    if (xml_xpath == NULL) {
        return -1;
    }

    xpath_obj = xml_xpath_query(xml_xpath, "/mp3tunes/trackList/item");

    if (xpath_obj == NULL) {
        return -1;
    }

    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        xml_xpath_t* xml_xpath_context = xml_xpath_context_init(xml_xpath, node);
        mp3tunes_locker_track_t *track = (mp3tunes_locker_track_t*)malloc(sizeof(mp3tunes_locker_track_t));
        memset(track, 0, sizeof(mp3tunes_locker_track_t));

        track->trackId = xml_xpath_get_integer(xml_xpath_context, "trackId");
        track->trackTitle = xml_xpath_get_string(xml_xpath_context, "trackTitle");
        track->trackNumber = xml_xpath_get_integer(xml_xpath_context, "trackNumber");
        track->trackLength = xml_xpath_get_float(xml_xpath_context, "trackLength");
        track->trackFileName = xml_xpath_get_string(xml_xpath_context, "trackFileName");
        track->trackFileKey = xml_xpath_get_string(xml_xpath_context, "trackFileKey");
        track->trackFileSize = xml_xpath_get_integer(xml_xpath_context, "trackFileSize");
        track->downloadURL = xml_xpath_get_string(xml_xpath_context, "downloadURL");
        track->playURL = xml_xpath_get_string(xml_xpath_context, "playURL");
        track->albumId = xml_xpath_get_integer(xml_xpath_context, "albumId");
        track->albumTitle = xml_xpath_get_string(xml_xpath_context, "albumTitle");
        track->albumYear = xml_xpath_get_integer(xml_xpath_context, "albumYear");
        track->artistName = xml_xpath_get_string(xml_xpath_context, "artistName");
        track->artistId = xml_xpath_get_integer(xml_xpath_context, "artistId");

        mp3tunes_locker_track_list_add(tracks, track);
        xml_xpath_deinit(xml_xpath_context);
    }
    xmlXPathFreeObject(xpath_obj);
    xml_xpath_deinit(xml_xpath);
    return 0;   

}

char* mp3tunes_locker_generate_filekey(const char *filename) {
  unsigned char sig[MD5_DIGEST_LENGTH];
  char      buffer[4096];
  char*    file_key;
  MD5_CTX     md5;
  int       ret;
  FILE      *stream;

  stream = fopen(filename, "r");
  if (stream == NULL) {
    perror(filename);
    exit(1);
  }
  MD5_Init(&md5);

  while (1) {
    ret = fread(buffer, sizeof(char), sizeof(buffer), stream);
    if (ret <= 0)
      break;
    MD5_Update(&md5, buffer, ret);
  }

  MD5_Final(sig, &md5);

  if (stream != stdin) {
    (void)fclose(stream);
  }

  md5_sig_to_string(sig, buffer, sizeof(buffer));
  file_key = (char*)malloc(4096*sizeof(char));
  strcpy (file_key,buffer);
  return file_key;
}

TODO: Rewrite this more sanely*/
/*
int mp3tunes_locker_upload_track(MP3tunesLocker *obj, char *path) {
    request_t *request;
    CURLcode res;
    FILE * hd_src ;
    int hd ;
    struct stat file_info;
    char* file_key = malloc(4096*sizeof(char));
    file_key = mp3tunes_locker_generate_filekey(path);

    get the file size of the local file
    hd = open(path, O_RDONLY);
    fstat(hd, &file_info);
    close(hd);
    get a FILE * of the same file
    hd_src = fopen(path, "rb");

    create the request url
    char *url = malloc(256*sizeof(char));
    snprintf(url, 256, "storage/lockerput/%s", file_key);
    request = mp3tunes_locker_api_generate_request(obj, MP3TUNES_SERVER_CONTENT, url, NULL);

    curl_easy_setopt( request->curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt( request->curl, CURLOPT_PUT, 1L);
    curl_easy_setopt( request->curl, CURLOPT_URL, request->url);
    curl_easy_setopt( request->curl, CURLOPT_READDATA, hd_src);
    curl_easy_setopt( request->curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
    curl_easy_setopt( request->curl, CURLOPT_USERAGENT, "liboboe/1.0" );
    res = curl_easy_perform(request->curl);
    curl_easy_cleanup(request->curl);

    fclose(hd_src);
    free(url);
    return 0;
}
*/

/* TODO: Rewrite this more sanely too */
/*
int mp3tunes_locker_load_track(MP3tunesLocker *obj, char *url) {
    xml_xpath_t* xml_xpath;
    char *status;
    xml_xpath = mp3tunes_locker_api_simple_fetch(obj, MP3TUNES_SERVER_LOGIN, "api/v0/lockerLoad/", "email", obj->username, "url", url, "sid", obj->session_id, NULL);

    if (xml_xpath == NULL) {
        return -2;
    }

    status = xml_xpath_get_string(xml_xpath, "/mp3tunes/status");

    if (status[0] != '1') {
        char* error = xml_xpath_get_string(xml_xpath, "/mp3tunes/errorMessage");
        obj->error_message = error;
        free(status);
        xml_xpath_deinit(xml_xpath);
        return -1;
    }
    free(status);
    xml_xpath_deinit(xml_xpath);

    return 0;

}
*/
