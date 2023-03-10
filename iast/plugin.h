#ifndef __PLUMA_IAST_PLUGIN_H
#define __PLUMA_IAST_PLUGIN_H

#include <glib.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

typedef struct PlumaIastPlugin       PlumaIastPlugin;
typedef struct PlumaIastPluginClass  PlumaIastPluginClass;

GType pluma_iast_plugin_get_type(void) G_GNUC_CONST;
G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module);

G_END_DECLS

#endif /* __PLUMA_IAST_PLUGIN_H */
