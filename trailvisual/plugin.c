#include <libpeas/peas-activatable.h>
#include <pluma/pluma-window.h>
#include <stdbool.h>
#include "plugin.h"

enum {
	PROP_WINDOW = 1
};

struct PlumaTrailVisualPlugin {
	PeasExtensionBase parent_instance;
	PlumaWindow *window;
};

struct PlumaTrailVisualPluginClass {
	PeasExtensionBaseClass parent_class;
};

static void peas_activatable_iface_init(PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(PlumaTrailVisualPlugin, pluma_trail_visual_plugin,
			PEAS_TYPE_EXTENSION_BASE, 0,
			G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_TYPE_ACTIVATABLE,
				peas_activatable_iface_init));

static bool tag_style_is_visible(GtkTextTag *tag)
{
	int background_set, underline_set;

	g_object_get((GObject *) tag, "background-set", &background_set,
		"underline-set", &underline_set, NULL);

	return background_set | underline_set;
}

static void tag_set_rgb_background(GtkTextTag *tag, double r, double g, double b)
{
	GObject *object = (GObject *) tag;
	GdkRGBA c;

	c.red = r;
	c.green = g;
	c.blue = b;
	c.alpha = 1.0;

	g_object_set(object, "background-rgba", &c, NULL);
}

static void tag_load_style_from_scheme(GtkSourceBuffer *buffer, GtkTextTag *tag)
{
	GtkSourceStyleScheme *scheme;
	GtkSourceStyle *style;

	scheme = gtk_source_buffer_get_style_scheme(buffer);
	if (scheme) {
		style = gtk_source_style_scheme_get_style(scheme, "def:error");
		gtk_source_style_apply(style, tag);
	}

	if (!tag_style_is_visible(tag)) {
		tag_set_rgb_background(tag, 1, 0, 0);
	}
}

static GtkTextTag *create_text_tag(GtkTextBuffer *buffer)
{
	GtkTextTag *tag;

	tag = gtk_text_buffer_create_tag(buffer, "trailvisual", NULL);
	tag_load_style_from_scheme(GTK_SOURCE_BUFFER(buffer), tag);

	return tag;
}

static void find_trailing_whitespaces_start(GtkTextIter *end)
{
	GtkTextIter prev;
	unsigned int c;

	while (!gtk_text_iter_starts_line(end)) {
		prev = *end;
		gtk_text_iter_backward_char(&prev);

		c = gtk_text_iter_get_char(&prev);
		if (!g_unichar_isspace(c))
			break;

		*end = prev;
	}
}

static void highlight_trailing_spaces_on_line(GtkTextBuffer *buffer,
					      GtkTextIter *line,
					      GtkTextTag *tag)
{
	GtkTextIter start = *line, end = *line;

	gtk_text_iter_forward_to_line_end(&end);
	gtk_text_iter_set_line_offset(&start, 0);
	gtk_text_buffer_remove_tag(buffer, tag, &start, &end);

	start = end;
	find_trailing_whitespaces_start(&start);
	gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
}

static void highlight_updated(GtkTextBuffer *buffer, GtkTextIter *start,
			      GtkTextIter *end, GtkTextTag *tag)
{
	GtkTextIter iter = *start;

	do {
		highlight_trailing_spaces_on_line(buffer, &iter, tag);
		gtk_text_iter_forward_lines(&iter, 1);
	} while (gtk_text_iter_compare(&iter, end) < 0);
}

static void style_changed(GtkSourceBuffer *buffer, GParamSpec *pspec,
			  GtkTextTag *tag)
{
	tag_load_style_from_scheme(buffer, tag);
}

static void initialise_document(PlumaDocument *document,
				PlumaTrailVisualPlugin *plugin)
{
	GtkTextBuffer *buffer;
	GtkTextTag *tag;

	buffer = (GtkTextBuffer *) document;
	if (buffer == NULL)
		return;

	tag = create_text_tag(buffer);

	g_signal_connect(G_OBJECT(document), "highlight-updated",
			 (GCallback) highlight_updated, tag);
	g_signal_connect(G_OBJECT(document), "notify::style-scheme",
			 (GCallback) style_changed, tag);
}

static void finalise_document(PlumaDocument *document,
			      PlumaTrailVisualPlugin *plugin)
{
	GtkTextBuffer *buffer;
	GtkTextTagTable *table;
	GtkTextTag *tag;

	buffer = (GtkTextBuffer *) document;
	table = gtk_text_buffer_get_tag_table(buffer);
	tag = gtk_text_tag_table_lookup(table, "trailvisual");

	if (!tag) {
		return;
	}

	g_signal_handlers_disconnect_by_data(buffer, tag);
	gtk_text_tag_table_remove(table, tag);
}

static void initialise_all_docs(PlumaWindow *window, PlumaTrailVisualPlugin *plugin)
{
	GList *docs, *doc;

	docs = pluma_window_get_documents(window);
	for (doc = docs; doc && doc->data; doc = doc->next)
		initialise_document(doc->data, plugin);

	g_list_free(docs);
}

static void finalise_all_docs(PlumaWindow *window, PlumaTrailVisualPlugin *plugin)
{
	GList *docs, *doc;

	docs = pluma_window_get_documents(window);
	for (doc = docs; doc && doc->data; doc = doc->next)
		finalise_document(doc->data, plugin);

	g_list_free(docs);
}

static void initialise_tab(PlumaWindow *window, PlumaTab *tab,
			   PlumaTrailVisualPlugin *plugin)
{
	initialise_document(pluma_tab_get_document(tab), plugin);
}

static void plugin_activate(PeasActivatable *activatable)
{
	PlumaTrailVisualPlugin *plugin;
	PlumaWindow *window;

	plugin = (PlumaTrailVisualPlugin *) activatable;
	window = (PlumaWindow *) plugin->window;

	initialise_all_docs(window, plugin);

	g_signal_connect(window, "tab-added", (GCallback) initialise_tab,
			 plugin);
}

static void plugin_deactivate(PeasActivatable *activatable)
{
	PlumaTrailVisualPlugin *plugin;

	plugin = (PlumaTrailVisualPlugin *) activatable;
	finalise_all_docs(plugin->window, plugin);
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	PlumaTrailVisualPlugin *plugin;

	plugin = (PlumaTrailVisualPlugin *) object;

	switch (prop_id) {
	case PROP_WINDOW:
		plugin->window = g_value_dup_object(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void get_property(GObject *object, unsigned int prop_id, GValue *value,
			 GParamSpec *pspec)
{
	PlumaTrailVisualPlugin *plugin;

	plugin = (PlumaTrailVisualPlugin *) object;

	switch (prop_id) {
	case PROP_WINDOW:
		g_value_set_object(value, plugin->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void pluma_trail_visual_plugin_init(PlumaTrailVisualPlugin *plugin)
{
}

static void plugin_dispose(GObject *object)
{
	PlumaTrailVisualPlugin *plugin;

	plugin = (PlumaTrailVisualPlugin *) object;
	if (plugin->window) {
		g_object_unref(plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS(pluma_trail_visual_plugin_parent_class)->dispose(object);
}

static void pluma_trail_visual_plugin_class_init(PlumaTrailVisualPluginClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->dispose = plugin_dispose;

	g_object_class_override_property(object_class, PROP_WINDOW, "window");
}

static void pluma_trail_visual_plugin_class_finalize(PlumaTrailVisualPluginClass *class)
{
}

static void peas_activatable_iface_init(PeasActivatableInterface *iface)
{
	iface->activate = plugin_activate;
	iface->deactivate = plugin_deactivate;
}

G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module)
{
	pluma_trail_visual_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module, PEAS_TYPE_ACTIVATABLE,
			pluma_trail_visual_plugin_get_type());
}
