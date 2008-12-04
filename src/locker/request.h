#ifndef __MP3TUNES_XML_XPATH__
#define __MP3TUNES_XML_XPATH__

#include <glib-object.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>

#define MP3TUNES_TYPE_XML_REQUEST                  (mp3tunes_xml_request_get_type ())
#define MP3TUNES_XML_REQUEST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MP3TUNES_TYPE_XML_REQUEST, MP3tunesXMLRequest))
#define MP3TUNES_IS_XML_REQUEST(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MP3TUNES_TYPE_XML_REQUEST))
#define MP3TUNES_XML_REQUEST_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MP3TUNES_TYPE_XML_REQUEST, MP3tunesXMLRequestClass))
#define MP3TUNES_IS_XML_REQUEST_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MP3TUNES_TYPE_XML_REQUEST))
#define MP3TUNES_XML_REQUEST_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MP3TUNES_TYPE_XML_REQUEST, MP3tunesXMLRequestClass))

typedef struct _MP3tunesXMLRequest MP3tunesXMLRequest;
typedef struct _MP3tunesXMLRequestClass MP3tunesXMLRequestClass;

struct _MP3tunesXMLRequest {
    GObject parent_instance;
    xmlDocPtr document;
    xmlXPathContextPtr xpath_ctx;
    GQueue* context_queue;
};

struct _MP3tunesXMLRequestClass {
    GObjectClass parent_class;
};

GType mp3tunes_xml_request_get_type(void);

typedef void(*xpath_foreach)(MP3tunesXMLRequest*, void**);

MP3tunesXMLRequest* mp3tunes_xml_request_new(gchar* url);
void mp3tunes_xml_request_push_context(MP3tunesXMLRequest* parent, xmlNodePtr node);
void mp3tunes_xml_request_pop_context(MP3tunesXMLRequest* parent);
void mp3tunes_xml_request_xpath_query_foreach(MP3tunesXMLRequest *self, gchar* xpath_expression, xpath_foreach callback_function, void** data);
xmlXPathObjectPtr mp3tunes_xml_request_xpath_query(MP3tunesXMLRequest *self, gchar* xpath_expression);
gchar* mp3tunes_xml_request_xpath_get_string(MP3tunesXMLRequest* self, gchar* xpath_expression);
gint mp3tunes_xml_request_xpath_get_integer(MP3tunesXMLRequest* self, gchar* xpath_expression);
gfloat mp3tunes_xml_request_xpath_get_float(MP3tunesXMLRequest* self, gchar* xpath_expression);


#endif
