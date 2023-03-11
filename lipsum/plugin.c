#include <libpeas/peas-activatable.h>
#include <pluma/pluma-window.h>
#include <stdbool.h>
#include "plugin.h"

enum {
	PROP_OBJECT = 1
};

struct PlumaLipsumPlugin {
	PeasExtensionBase parent_instance;
	PlumaWindow *window;
	GtkActionGroup *action_group;
	unsigned int ui_id;
};

struct PlumaLipsumPluginClass {
	PeasExtensionBaseClass parent_class;
};

static void peas_activatable_iface_init(PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(PlumaLipsumPlugin, pluma_lipsum_plugin,
			PEAS_TYPE_EXTENSION_BASE, 0,
			G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_TYPE_ACTIVATABLE,
				peas_activatable_iface_init));

#define LIPSUM_FILE "/usr/share/pluma/plugins/lipsum/lipsum.txt"

static char *build_lorem_ipsum(PlumaDocument *document)
{
	char **lines, *line, *in = NULL, *lang_id = NULL, *format = "%s";
	unsigned int i;
	GtkSourceLanguage *language;
	GString *str;
	GError *error = NULL;

	if (!g_file_get_contents(LIPSUM_FILE, &in, NULL, &error)) {
		return NULL;
	}

	language = pluma_document_get_language(document);
	if (language != NULL) {
		g_object_get(language, "id", &lang_id, NULL);
	}

	if (lang_id && strcmp(lang_id, "html") == 0) {
		format = "<p>%s</p>";
	}

	str = g_string_new(NULL);
	lines = g_strsplit(in, "\n\n", -1);
	free(in);

	for (i = 0; lines[i]; i++) {
		line = g_strdup_printf(format, lines[i]);
		g_string_append(str, i ? "\n\n" : "");
		g_string_append(str, line);
		free(line);
	}

	g_strfreev(lines);

	return g_string_free(str, false);
}

static void insert_lipsum(GtkAction *action, PlumaWindow *window)
{
	GtkTextBuffer *buffer;
	char *text;

	buffer = (GtkTextBuffer *) pluma_window_get_active_document(window);
	text = build_lorem_ipsum(pluma_window_get_active_document(window));
	if (text == NULL) {
		g_warning("Failed to generate the Lorem Ipsum text.");
		return;
	}

	gtk_text_buffer_insert_at_cursor(buffer, text, strlen(text));
	free(text);
}

static const GtkActionEntry action_entries[] = {
	{"Lipsum", NULL, "Insert Lorem Ipsum", NULL,
		"Inserts Lorem Ipsum", G_CALLBACK(insert_lipsum)}
};

static const char *menu =
	"<ui>"
	"  <menubar name='MenuBar'>"
	"    <menu name='EditMenu' action='Edit'>"
	"      <placeholder name='EditOps_4'>"
	"        <menuitem action='Lipsum'/>"
	"      </placeholder>"
	"    </menu>"
	"  </menubar>"
	"</ui>";

static void update_ui(PlumaLipsumPlugin *plugin)
{
	PlumaWindow *window;
	GtkTextView *view;
	GtkAction *action;
	bool sensitive = false;

	window = plugin->window;
	view = (GtkTextView *) pluma_window_get_active_view(window);

	if (view != NULL) {
		sensitive = gtk_text_view_get_editable(view);
	}

	action = gtk_action_group_get_action(plugin->action_group, "Lipsum");
	gtk_action_set_sensitive(action, sensitive);
}

static void plugin_activate(PeasActivatable *activatable)
{
	PlumaLipsumPlugin *plugin;
	PlumaWindow *window;
	GtkUIManager *manager;
	GError *error = NULL;

	plugin = (PlumaLipsumPlugin *) activatable;
	window = plugin->window;
	manager = pluma_window_get_ui_manager(window);

	plugin->action_group = gtk_action_group_new("PlumaLipsumPluginActions");
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
	PlumaLipsumPlugin *plugin;
	GtkUIManager *manager;

	plugin = (PlumaLipsumPlugin *) activatable;
	manager = pluma_window_get_ui_manager(plugin->window);

	gtk_ui_manager_remove_ui(manager, plugin->ui_id);
	gtk_ui_manager_remove_action_group(manager, plugin->action_group);
}

static void plugin_update_state(PeasActivatable *activatable)
{
	PlumaLipsumPlugin *plugin;

	plugin = (PlumaLipsumPlugin *) activatable;
	update_ui(plugin);
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	PlumaLipsumPlugin *plugin;

	plugin = (PlumaLipsumPlugin *) object;

	switch (prop_id) {
	case PROP_OBJECT:
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
	PlumaLipsumPlugin *plugin;

	plugin = (PlumaLipsumPlugin *) object;

	switch (prop_id) {
	case PROP_OBJECT:
		g_value_set_object(value, plugin->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void pluma_lipsum_plugin_init(PlumaLipsumPlugin *plugin)
{
}

static void plugin_dispose(GObject *object)
{
	PlumaLipsumPlugin *plugin;

	plugin = (PlumaLipsumPlugin *) object;
	if (plugin->window) {
		g_object_unref(plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS(pluma_lipsum_plugin_parent_class)->dispose(object);
}

static void pluma_lipsum_plugin_class_init(PlumaLipsumPluginClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->dispose = plugin_dispose;

	g_object_class_override_property(object_class, PROP_OBJECT, "object");
}

static void pluma_lipsum_plugin_class_finalize(PlumaLipsumPluginClass *class)
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
	pluma_lipsum_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
			PEAS_TYPE_ACTIVATABLE, pluma_lipsum_plugin_get_type());
}
