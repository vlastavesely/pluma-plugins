#include <libpeas/peas-activatable.h>
#include <pluma/pluma-window.h>
#include "plugin.h"

enum {
	PROP_OBJECT = 1
};

struct PlumaIastPlugin {
	PeasExtensionBase parent_instance;
	PlumaWindow *window;
};

struct PlumaIastPluginClass {
	PeasExtensionBaseClass parent_class;
};

static void peas_activatable_iface_init(PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(PlumaIastPlugin, pluma_iast_plugin,
			PEAS_TYPE_EXTENSION_BASE, 0,
			G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_TYPE_ACTIVATABLE,
				peas_activatable_iface_init));

static void plugin_activate(PeasActivatable *activatable)
{
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) activatable;

	puts("Initialise ...");
}

static void plugin_deactivate(PeasActivatable *activatable)
{
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) activatable;

	puts("Finalise ...");
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) object;

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
	PlumaIastPlugin *plugin;

	plugin = (PlumaIastPlugin *) object;

	switch (prop_id) {
	case PROP_OBJECT:
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

	g_object_class_override_property(object_class, PROP_OBJECT, "object");
}

static void pluma_iast_plugin_class_finalize(PlumaIastPluginClass *class)
{
}

static void peas_activatable_iface_init(PeasActivatableInterface *iface)
{
	iface->activate = plugin_activate;
	iface->deactivate = plugin_deactivate;
}

G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module)
{
	pluma_iast_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
			PEAS_TYPE_ACTIVATABLE, pluma_iast_plugin_get_type());
}
