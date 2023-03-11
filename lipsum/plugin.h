#ifndef __PLUMA_LIPSUM_PLUGIN_H
#define __PLUMA_LIPSUM_PLUGIN_H

#include <glib.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

typedef struct PlumaLipsumPlugin       PlumaLipsumPlugin;
typedef struct PlumaLipsumPluginClass  PlumaLipsumPluginClass;

GType pluma_lipsum_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module);

G_END_DECLS

#endif /* __PLUMA_LIPSUM_PLUGIN_H */
