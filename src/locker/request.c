#include "request.h"

G_DEFINE_TYPE(MP3tunesXMLRequest, mp3tunes_xml_request, G_TYPE_OBJECT);

static void mp3tunes_xml_request_class_init(MP3tunesXMLRequestClass *klass) {
    klass = klass;
}

static void mp3tunes_xml_request_init(MP3tunesXMLRequest *self) {
    self->document = NULL;
    self->xpath_ctx = NULL;
    self->context_queue = g_queue_new();
}

MP3tunesXMLRequest* mp3tunes_xml_request_new(gchar* document_text) {
    MP3tunesXMLRequest *self = g_object_new(MP3TUNES_TYPE_XML_REQUEST, NULL);
    self->document = xmlParseDoc((xmlChar*)document_text);
    self->xpath_ctx = xmlXPathNewContext(self->document);
    if(self->xpath_ctx == NULL) {
        xmlFreeDoc(self->document);
        return NULL;
    }

    return self;
}

void mp3tunes_xml_request_push_context(MP3tunesXMLRequest* self, xmlNodePtr node) {
    g_queue_push_head(self->context_queue, self->xpath_ctx->node);
    self->xpath_ctx->node = node;
}

void mp3tunes_xml_request_pop_context(MP3tunesXMLRequest* self) {
    xmlNodePtr node = (xmlNodePtr)g_queue_pop_head(self->context_queue);
    self->xpath_ctx->node = node;
}

xmlXPathObjectPtr mp3tunes_xml_request_xpath_query(MP3tunesXMLRequest *self, gchar* xpath_expression) {
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

void mp3tunes_xml_request_xpath_query_foreach(MP3tunesXMLRequest *self, gchar* xpath_expression, xpath_foreach callback_function, void** data) {
    xmlXPathObjectPtr xpath_obj;
    xmlNodeSetPtr nodeset;
    xmlNodePtr node;
    int i;
    
    xpath_obj = mp3tunes_xml_request_xpath_query(self, xpath_expression);

    if (xpath_obj == NULL) {
        return;
    }
    
    nodeset = xpath_obj->nodesetval;

    for (i = 0; i < nodeset->nodeNr; i++) {
        node = nodeset->nodeTab[i];
        mp3tunes_xml_request_push_context(self, node);
        callback_function(self, data);
        mp3tunes_xml_request_pop_context(self);
    }
    xmlXPathFreeObject(xpath_obj);
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

gchar* mp3tunes_xml_request_xpath_get_string(MP3tunesXMLRequest *self, gchar* xpath_expression) {
    xmlXPathObjectPtr xpath_obj;
    char* result = NULL;

    xpath_obj = mp3tunes_xml_request_xpath_query(self, xpath_expression);

    result = xml_get_text_from_nodeset(xpath_obj->nodesetval);

    xmlXPathFreeObject(xpath_obj);

    return result;
}

gint mp3tunes_xml_request_xpath_get_integer(MP3tunesXMLRequest *xml_xpath, gchar* xpath_expression) {
    gint result = 0;
    gchar* str = mp3tunes_xml_request_xpath_get_string(xml_xpath, xpath_expression);
    if (str != NULL) {
        result = atoi(str);
    }
    free(str);
    return result;
}

gfloat mp3tunes_xml_request_xpath_get_float(MP3tunesXMLRequest *xml_xpath, gchar* xpath_expression) {
    float result = 0.0;
    char* str = mp3tunes_xml_request_xpath_get_string(xml_xpath, xpath_expression);
    if (str != NULL) {
        result = atof(str);
    }
    free(str);
    return result;
}
