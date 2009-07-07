
/* Generated data (by glib-mkenums) */

#include <glib-object.h>
#include "vinagre-enums.h"


/* enumerations from "../vinagre/vinagre-connection.h" */
#include "../vinagre/vinagre-connection.h"
static const GEnumValue _vinagre_connection_protocol_values[] = {
  { VINAGRE_CONNECTION_PROTOCOL_VNC, "VINAGRE_CONNECTION_PROTOCOL_VNC", "vnc" },
  { VINAGRE_CONNECTION_PROTOCOL_RDP, "VINAGRE_CONNECTION_PROTOCOL_RDP", "rdp" },
  { VINAGRE_CONNECTION_PROTOCOL_INVALID, "VINAGRE_CONNECTION_PROTOCOL_INVALID", "invalid" },
  { 0, NULL, NULL }
};

GType
vinagre_connection_protocol_get_type (void)
{
  static GType type = 0;

  if (!type)
    type = g_enum_register_static ("VinagreConnectionProtocol", _vinagre_connection_protocol_values);

  return type;
}


/* Generated data ends here */

