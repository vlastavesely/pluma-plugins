#include <libpeas/peas-activatable.h>
#include <pluma/pluma-window-activatable.h>
#include <pluma/pluma-window.h>
#include <stdbool.h>
#include "plugin.h"

enum {
	PROP_WINDOW = 1
};

struct PlumaLengthPlugin {
	PeasExtensionBase parent_instance;
	PlumaWindow *window;
	GtkWidget *label;
};

struct PlumaLengthPluginClass {
	PeasExtensionBaseClass parent_class;
};

static void pluma_window_activatable_iface_init(PlumaWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(PlumaLengthPlugin, pluma_length_plugin,
		PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC(PLUMA_TYPE_WINDOW_ACTIVATABLE,
			pluma_window_activatable_iface_init));

static GtkWidget *build_label(GtkWidget *statusbar)
{
	GtkWidget *label;

	label = gtk_label_new("0 chars");
	gtk_label_set_width_chars((GtkLabel *) label, 15);
	gtk_widget_show(label);
	gtk_box_pack_start((GtkBox *) statusbar, label, false, true, 0);

	return label;
}

static void update_label(PlumaDocument *document, GtkLabel *label)
{
	GtkTextBuffer *buffer;
	int length;
	char buf[32] = "";

	buffer = (GtkTextBuffer *) document;
	if (buffer) {
		length = gtk_text_buffer_get_char_count(buffer);
		snprintf(buf, 32, "%'d chars", length);
	}

	gtk_label_set_text(label, buf);
}

static void update_ui(PlumaLengthPlugin *plugin)
{
	PlumaDocument *doc;

	doc = pluma_window_get_active_document(plugin->window);
	update_label(doc, GTK_LABEL(plugin->label));
}

static void initialise_document(PlumaDocument *document,
				PlumaLengthPlugin *plugin)
{
	GtkLabel *label = (GtkLabel *) plugin->label;

	g_signal_connect(document, "changed", (GCallback) update_label, label);
	update_label(document, label);
}

static void finalise_document(PlumaDocument *document,
			      PlumaLengthPlugin *plugin)
{
	g_signal_handlers_disconnect_by_data(document, plugin->label);
}

static void initialise_tab(PlumaWindow *window, PlumaTab *tab,
			   PlumaLengthPlugin *plugin)
{
	initialise_document(pluma_tab_get_document(tab), plugin);
}

static void initialise_all_docs(PlumaWindow *window, PlumaLengthPlugin *plugin)
{
	GList *docs, *doc;

	docs = pluma_window_get_documents(window);
	for (doc = docs; doc && doc->data; doc = doc->next)
		initialise_document(doc->data, plugin);

	g_list_free(docs);
}

static void finalise_all_docs(PlumaWindow *window, PlumaLengthPlugin *plugin)
{
	GList *docs, *doc;

	docs = pluma_window_get_documents(window);
	for (doc = docs; doc && doc->data; doc = doc->next)
		finalise_document(doc->data, plugin);

	g_list_free(docs);
}

static void plugin_activate(PlumaWindowActivatable *activatable)
{
	PlumaLengthPlugin *plugin;
	PlumaWindow *window;
	GtkWidget *statusbar;

	plugin = (PlumaLengthPlugin *) activatable;
	window = (PlumaWindow *) plugin->window;
	statusbar = pluma_window_get_statusbar(window);

	plugin->label = build_label(statusbar);
	initialise_all_docs(window, plugin);

	g_signal_connect(window, "tab-added", (GCallback) initialise_tab,
			 plugin);

	update_ui(plugin);
}

static void plugin_deactivate(PlumaWindowActivatable *activatable)
{
	PlumaLengthPlugin *plugin;

	plugin = (PlumaLengthPlugin *) activatable;
	finalise_all_docs(plugin->window, plugin);

	gtk_widget_destroy(plugin->label);
}

static void plugin_update_state(PlumaWindowActivatable *activatable)
{
	PlumaLengthPlugin *plugin;

	plugin = (PlumaLengthPlugin *) activatable;
	update_ui(plugin);
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	PlumaLengthPlugin *plugin;

	plugin = (PlumaLengthPlugin *) object;

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
	PlumaLengthPlugin *plugin;

	plugin = (PlumaLengthPlugin *) object;

	switch (prop_id) {
	case PROP_WINDOW:
		g_value_set_object(value, plugin->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void pluma_length_plugin_init(PlumaLengthPlugin *plugin)
{
}

static void plugin_dispose(GObject *object)
{
	PlumaLengthPlugin *plugin;

	plugin = (PlumaLengthPlugin *) object;
	if (plugin->window) {
		g_object_unref(plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS(pluma_length_plugin_parent_class)->dispose(object);
}

static void pluma_length_plugin_class_init(PlumaLengthPluginClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->dispose = plugin_dispose;

	g_object_class_override_property(object_class, PROP_WINDOW, "window");
}

static void pluma_length_plugin_class_finalize(PlumaLengthPluginClass *class)
{
}

static void pluma_window_activatable_iface_init(PlumaWindowActivatableInterface *iface)
{
	iface->activate = plugin_activate;
	iface->deactivate = plugin_deactivate;
	iface->update_state = plugin_update_state;
}

G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module)
{
	pluma_length_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
			PLUMA_TYPE_WINDOW_ACTIVATABLE,
			pluma_length_plugin_get_type());
}
