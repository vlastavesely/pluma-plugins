#include <libpeas/peas-activatable.h>
#include <pluma/pluma-window.h>
#include <stdbool.h>
#include <iast.h>
#include "plugin.h"

enum {
	PROP_WINDOW = 1
};

struct PlumaIastPlugin {
	PeasExtensionBase parent_instance;
	PlumaWindow *window;
	GtkActionGroup *action_group;
	unsigned int ui_id;
};

struct PlumaIastPluginClass {
	PeasExtensionBaseClass parent_class;
};

static void peas_activatable_iface_init(PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(PlumaIastPlugin, pluma_iast_plugin,
			PEAS_TYPE_EXTENSION_BASE, 0,
			G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_TYPE_ACTIVATABLE,
				peas_activatable_iface_init));

static char *buffer_selected_text(GtkTextBuffer *buffer)
{
	GtkTextIter start, end;

	if (!gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
		return NULL;
	}

	return gtk_text_buffer_get_text(buffer, &start, &end, false);
}

static void replace_selected_text(GtkTextBuffer *buffer, const char *text)
{
	gtk_text_buffer_begin_user_action(buffer);
	gtk_text_buffer_delete_selection(buffer, true, true);
	gtk_text_buffer_insert_at_cursor(buffer, text, strlen(text));
	gtk_text_buffer_end_user_action(buffer);
}

#define SWITCH(a, b) {free(a); a = b;}

/*
 * Convert IAST, Velthuis or Harvard-Kyoto to Devanagari.
 */
static void convert_devanagari(GtkAction *action, PlumaWindow *window)
{
	GtkTextBuffer *buffer;
	char *in, *out;

	buffer = (GtkTextBuffer *) pluma_window_get_active_document(window);
	in = buffer_selected_text(buffer);

	if (encode_velthuis_to_iast(in, &out) != 0)
		return;

	SWITCH(in, out);
	if (encode_harvard_kyoto_to_iast(in, &out) != 0)
		return;

	SWITCH(in, out);
	if (transliterate_latin_to_devanagari(in, &out) != 0)
		return;

	free(in);

	if (out) {
		replace_selected_text(buffer, out);
		free(out);
	}
}

/*
 * Convert Devanagari, Velthuis or Harvard-Kyoto to IAST.
 */
static void convert_iast(GtkAction *action, PlumaWindow *window)
{
	GtkTextBuffer *buffer;
	char *in, *out;

	buffer = (GtkTextBuffer *) pluma_window_get_active_document(window);
	in = buffer_selected_text(buffer);

	if (transliterate_devanagari_to_latin(in, &out) != 0)
		return;

	SWITCH(in, out);
	if (encode_velthuis_to_iast(in, &out) != 0)
		return;

	SWITCH(in, out);
	if (encode_harvard_kyoto_to_iast(in, &out) != 0)
		return;

	free(in);

	if (out) {
		replace_selected_text(buffer, out);
		free(out);
	}
}

static const GtkActionEntry action_entries[] = {
	{"Iast", NULL, "Devanagari Conversion"},
	{"ConvertDeva", NULL, "Convert to Devanagari", "<control><shift>d",
		"Convert the selected text to Devanagari",
		G_CALLBACK(convert_devanagari)},
	{"ConvertIast", NULL, "Convert to IAST", "<control><shift>i",
		"Convert the selected text to IAST",
		G_CALLBACK(convert_iast)}
};

static const char *menu =
	"<ui>"
	"  <menubar name='MenuBar'>"
	"    <menu name='EditMenu' action='Edit'>"
	"      <placeholder name='EditOps_6'>"
	"        <menu action='Iast'>"
	"          <menuitem action='ConvertDeva'/>"
	"          <menuitem action='ConvertIast'/>"
	"        </menu>"
	"      </placeholder>"
	"    </menu>"
	"  </menubar>"
	"</ui>";

static void update_ui(PlumaIastPlugin *plugin)
{
	PlumaWindow *window;
	GtkTextView *view;
	GtkAction *action;
	bool sensitive = false;

	window = plugin->window;
	view = (GtkTextView *) pluma_window_get_active_view(window);

	if (view != NULL) {
		GtkTextBuffer *buffer;

		buffer = gtk_text_view_get_buffer(view);
		sensitive = (gtk_text_view_get_editable(view) &&
			     gtk_text_buffer_get_has_selection(buffer));
	}

	action = gtk_action_group_get_action(plugin->action_group, "Iast");
	gtk_action_set_sensitive(action, sensitive);
}

static void plugin_activate(PeasActivatable *activatable)
{
	PlumaIastPlugin *plugin;
	PlumaWindow *window;
	GtkUIManager *manager;
	GError *error = NULL;

	plugin = (PlumaIastPlugin *) activatable;
	window = plugin->window;
	manager = pluma_window_get_ui_manager(window);

	plugin->action_group = gtk_action_group_new("PlumaIastPluginActions");
	gtk_action_group_add_actions(plugin->action_group, action_entries,
				     G_N_ELEMENTS(action_entries), window);
	gtk_ui_manager_insert_action_group(manager, plugin->action_group, -1);

	plugin->ui_id = gtk_ui_manager_add_ui_from_string(manager, menu, -1, &error);
	if (plugin->ui_id == 0) {
		g_warning("%s", error->message);
		return;
	}

	update_ui(plugin);
}

static void plugin_deactivate(PeasActivatable *activatable)
{
	PlumaIastPlugin *plugin;
	GtkUIManager *manager;

	plugin = (PlumaIastPlugin *) activatable;
	manager = pluma_window_get_ui_manager(plugin->window);

	gtk_ui_manager_remove_ui(manager, plugin->ui_id);
	gtk_ui_manager_remove_action_group(manager, plugin->action_group);
}

static void plugin_update_state(PeasActivatable *activatable)
{
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) activatable;
	update_ui(plugin);
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) object;

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
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) object;

	switch (prop_id) {
	case PROP_WINDOW:
		g_value_set_object(value, plugin->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void pluma_iast_plugin_init(PlumaIastPlugin *plugin)
{
}

static void plugin_dispose(GObject *object)
{
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) object;
	if (plugin->window) {
		g_object_unref(plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS(pluma_iast_plugin_parent_class)->dispose(object);
}

static void pluma_iast_plugin_class_init(PlumaIastPluginClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->dispose = plugin_dispose;

	g_object_class_override_property(object_class, PROP_WINDOW, "window");
}

static void pluma_iast_plugin_class_finalize(PlumaIastPluginClass *class)
{
}

static void peas_activatable_iface_init(PeasActivatableInterface *iface)
{
	iface->activate = plugin_activate;
	iface->deactivate = plugin_deactivate;
	iface->update_state = plugin_update_state;
}

G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module)
{
	pluma_iast_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
			PEAS_TYPE_ACTIVATABLE, pluma_iast_plugin_get_type());
}
