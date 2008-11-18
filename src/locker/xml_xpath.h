#ifndef __MP3TUNES_XML_XPATH__
#define __MP3TUNES_XML_XPATH__

#include <glib-object.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>

#define MP3TUNES_TYPE_XML_XPATH                  (mp3tunes_xml_xpath_get_type ())
#define MP3TUNES_XML_XPATH(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MP3TUNES_TYPE_XML_XPATH, MP3tunesXMLXPath))
#define MP3TUNES_IS_XML_XPATH(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MP3TUNES_TYPE_XML_XPATH))
#define MP3TUNES_XML_XPATH_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MP3TUNES_TYPE_XML_XPATH, MP3tunesXMLXPathClass))
#define MP3TUNES_IS_XML_XPATH_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), MP3TUNES_TYPE_XML_XPATH))
#define MP3TUNES_XML_XPATH_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MP3TUNES_TYPE_XML_XPATH, MP3tunesXMLXPathClass))

typedef struct _MP3tunesXMLXPath MP3tunesXMLXPath;
typedef struct _MP3tunesXMLXPathClass MP3tunesXMLXPathClass;

struct _MP3tunesXMLXPath {
    GObject parent_instance;
    xmlDocPtr document;
    xmlXPathContextPtr xpath_ctx;
    xmlNodePtr context;
};

struct _MP3tunesXMLXPathClass {
    GObjectClass parent_class;
};

GType mp3tunes_xml_xpath_get_type(void);

MP3tunesXMLXPath* mp3tunes_xml_xpath_new(gchar* document_text);
MP3tunesXMLXPath* mp3tunes_xml_xpath_new_with_context(MP3tunesXMLXPath* parent, xmlNodePtr node);
xmlXPathObjectPtr mp3tunes_xml_xpath_query(MP3tunesXMLXPath *self, gchar* xpath_expression);
gchar* mp3tunes_xml_xpath_get_string(MP3tunesXMLXPath* self, gchar* xpath_expression);
gint mp3tunes_xml_xpath_get_integer(MP3tunesXMLXPath* self, gchar* xpath_expression);

#endif
