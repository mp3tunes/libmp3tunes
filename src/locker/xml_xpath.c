#include "xml_xpath.h"

G_DEFINE_TYPE(MP3tunesXMLXPath, mp3tunes_xml_xpath, G_TYPE_OBJECT);

static void mp3tunes_xml_xpath_class_init(MP3tunesXMLXPathClass *klass) {
    klass = klass;
}

static void mp3tunes_xml_xpath_init(MP3tunesXMLXPath *self) {
    self->document = NULL;
    self->xpath_ctx = NULL;
    self->context = NULL;
}

MP3tunesXMLXPath* mp3tunes_xml_xpath_new(gchar* document_text) {
    MP3tunesXMLXPath *result = g_object_new(MP3TUNES_TYPE_XML_XPATH, NULL);
    result->document = xmlParseDoc((xmlChar*)document_text);
    result->xpath_ctx = xmlXPathNewContext(result->document);
    if(result->xpath_ctx == NULL) {
        xmlFreeDoc(result->document);
        return NULL;
    }
    result->context = NULL;

    return result;
}

MP3tunesXMLXPath* mp3tunes_xml_xpath_new_with_context(MP3tunesXMLXPath* parent, xmlNodePtr node) {
    MP3tunesXMLXPath *result = g_object_new(MP3TUNES_TYPE_XML_XPATH, NULL);
    result->document = parent->document;
    result->xpath_ctx = xmlXPathNewContext(result->document);
    if(result->xpath_ctx == NULL) {
        xmlFreeDoc(result->document);
        return NULL;
    }
    result->xpath_ctx->node = node;
    result->context = node;

    return result;
}

xmlXPathObjectPtr mp3tunes_xml_xpath_query(MP3tunesXMLXPath *self, gchar* xpath_expression) {
    xmlXPathObjectPtr xpath_obj;

    xpath_obj = xmlXPathEvalExpression((xmlChar*)xpath_expression, self->xpath_ctx);
    if (xpath_obj == NULL) {
        return NULL;
    }
    if (xpath_obj->type != XPATH_NODESET) {
        xmlXPathFreeObject(xpath_obj);
        return NULL;
    }
    return xpath_obj;
}

static gchar* xml_get_text_from_nodeset(xmlNodeSetPtr nodeset) {
    xmlNodePtr node;
    xmlNodePtr child;
    int total_nodes;
    gchar* result = NULL;
    total_nodes = (nodeset) ? nodeset->nodeNr : 0;

    if (total_nodes != 1) {
        return NULL;
    }

    if (nodeset->nodeTab[0]->type != XML_ELEMENT_NODE) {
        return NULL;
    }

    node = nodeset->nodeTab[0];
    child = node->children;
    while (child && (XML_TEXT_NODE != child->type))
        child = child->next;
    if (child && (XML_TEXT_NODE == child->type)) {
        result = g_strdup((char*)child->content);
    }
    return result;
}

gchar* mp3tunes_xml_xpath_get_string(MP3tunesXMLXPath *self, gchar* xpath_expression) {
    xmlXPathObjectPtr xpath_obj;
    char* result = NULL;

    xpath_obj = mp3tunes_xml_xpath_query(self, xpath_expression);

    result = xml_get_text_from_nodeset(xpath_obj->nodesetval);

    xmlXPathFreeObject(xpath_obj);

    return result;
}

gint mp3tunes_xml_xpath_get_integer(MP3tunesXMLXPath *xml_xpath, char* xpath_expression) {
    gint result = 0;
    gchar* str = mp3tunes_xml_xpath_get_string(xml_xpath, xpath_expression);
    if (str != NULL) {
        result = atoi(str);
    }
    free(str);
    return result;
}

float mp3tunes_xml_xpath_get_float(MP3tunesXMLXPath *xml_xpath, char* xpath_expression) {
    float result = 0.0;
    char* str = mp3tunes_xml_xpath_get_string(xml_xpath, xpath_expression);
    if (str != NULL) {
        result = atof(str);
    }
    free(str);
    return result;
}
