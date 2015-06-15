// - Extensión para el nautilus "crear_lanzador.c" version 1.0 -
// Muestra la opción "Crear un lanzador..." en el menú contextual
// o en el menú "Archivo" del Nautilus. Al hacer clic sobre dicha
// opción se ejecuta el script "/usr/bin/crear_lanzador".

// Para compilar ejecutar:

// sudo apt-get install libnautilus-extension1a libnautilus-extension-dev
// gcc -c crear_lanzador.c -o crear_lanzador.o -fPIC $(pkg-config libnautilus-extension --cflags)
// gcc -shared crear_lanzador.o -o crear_lanzador.so $(pkg-config libnautilus-extension --libs)
// chmod 644 crear_lanzador.so
// sudo cp crear_lanzador.so /usr/lib/nautilus/extensions-3.0/
// nautilus -q
// nautilus

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-menu-provider.h>

#define CREARL_TYPE_CONTEXT_MENU (crearl_context_menu_get_type ())
#define CREARL_CONTEXT_MENU(o)   (G_TYPE_CHECK_INSTANCE_CAST ((o), CREARL_TYPE_CONTEXT_MENU))

//*****************************************************************************
// Declaración de variables
typedef struct {
    GObject parent;
} CrearlContextMenu;

typedef struct {
    GObjectClass parent_class;
} CrearlContextMenuClass;

static GType crearl_type = 0;
static GObjectClass *parent_class = NULL;

//*****************************************************************************
// Declaracion de las cabeceras de las funciones
static void
crearl_context_menu_init (CrearlContextMenu *self);

static void
crearl_context_menu_class_init (CrearlContextMenuClass *class);

static void
menu_provider_iface_init (NautilusMenuProviderIface *iface);

static GList*
crearl_context_menu_get_background_items (NautilusMenuProvider *provider,
					GtkWidget *window,
                                	NautilusFileInfo *current_folder);
static void
crearl_context_menu_activate (NautilusMenuItem *item,
			    gpointer user_data);

//*****************************************************************************
static GType
crearl_context_menu_get_type (void)
{
    return crearl_type;
}
//*****************************************************************************
static void
crearl_context_menu_register_type (GTypeModule *module)
{
    static const GTypeInfo info = {
	sizeof (CrearlContextMenuClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) crearl_context_menu_class_init,
	NULL,
	NULL,
	sizeof (CrearlContextMenu),
	0,
	(GInstanceInitFunc) crearl_context_menu_init
    };

    static const GInterfaceInfo menu_provider_iface_info = {
	(GInterfaceInitFunc)menu_provider_iface_init,
	NULL,
	NULL
    };

    crearl_type = g_type_module_register_type (module,
					       G_TYPE_OBJECT,
					       "CrearlContextMenu",
					       &info, 0);

    // Interface
    g_type_module_add_interface (module,
				 crearl_type,
				 NAUTILUS_TYPE_MENU_PROVIDER,
				 &menu_provider_iface_info);

}

//*****************************************************************************

static void
crearl_context_menu_class_init (CrearlContextMenuClass *class)
{
    parent_class = g_type_class_peek_parent (class);
}

//*****************************************************************************

static void menu_provider_iface_init (NautilusMenuProviderIface *iface)
{
    iface->get_background_items = crearl_context_menu_get_background_items;
}

//*****************************************************************************

static void
crearl_context_menu_init (CrearlContextMenu *self)
{
}

//*****************************************************************************

// Esta función se ejecuta cuando se hace clic derecho sobre el fondo
// de una carpeta.
static GList *
crearl_context_menu_get_background_items (NautilusMenuProvider *provider,
			 		  GtkWidget *window,
					  NautilusFileInfo *current_folder)
{
    GList *items = NULL;
    NautilusMenuItem *item;

    // Sólo se muestra el menú en el escritorio y carpetas normales,
    // se omiten carpetas especiales como network, trash, Equipo, etc... 
    {
      gchar *uri_scheme = NULL;

      uri_scheme = nautilus_file_info_get_uri_scheme (current_folder);
      if (strncmp (uri_scheme, "x-nautilus-desktop", 18) && // el escritorio
	  strncmp (uri_scheme, "file", 4) )                 // carpetas normales
	{
	  g_free (uri_scheme);
	  return NULL;
	}
      g_free (uri_scheme);
    }

    // Crea la nueva entrada del menu
    item = nautilus_menu_item_new ("Crearl::crear_lanzador",
				   "Crear un lanzador...",
				   "Ejecuta el comando \"gnome-desktop-item-edit\" para crear un fichero \".desktop\"",
				   "gnome-panel-launcher");
    g_object_set_data (G_OBJECT (item), "file", current_folder);
    g_signal_connect (item,
		      "activate",
		      G_CALLBACK (crearl_context_menu_activate), // Esta es la función que se ejecuta al hacer clic en "Crear un lanzador..."
		      window);

    items = g_list_append (items, item);

    return items;
}

//*****************************************************************************

static void
crearl_context_menu_activate (NautilusMenuItem *item,
			      gpointer user_data)
{
    NautilusFileInfo *file = NULL;
    GFile *rutaGFile       = NULL;

    file = g_object_get_data (G_OBJECT (item), "file");
    rutaGFile=nautilus_file_info_get_location (file);

    gchar *uri_scheme = NULL;
    uri_scheme = nautilus_file_info_get_uri_scheme (file);
    if (!strncmp (uri_scheme, "x-nautilus-desktop", 18))
    {
	// Es el escritorio, se pasa "x-nautilus-desktop" como ruta y
        // se deja al script crear_lanzador encargado de encontrar la
        // ruta correcta del escritorio.
        //g_message("uri_scheme: %s", uri_scheme);
    }
    else if (!strncmp (uri_scheme, "file", 4))
    {
	g_free(uri_scheme);
	uri_scheme=g_file_get_path(rutaGFile); // Se debe liberar con g_free
    }

    // Ejecuta el script shell crear_lanzador de forma no bloqueante para el nautilus.
    if (! g_spawn_command_line_async (g_strdup_printf ("crear_lanzador %s", uri_scheme), NULL) ) 
    { 
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new_with_markup (NULL, 0,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_CLOSE,
                                                     "<big><b>"
                                                     "ERROR."
                                                     "</b></big>\n\n"
                                                     "No se ha podido ejecutar el comando \"crear_lanzador\".");
	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
    }

    g_free (uri_scheme);

}

//*****************************************************************************
/* --- extension interface --- */

void
nautilus_module_initialize (GTypeModule *module)
{
    crearl_context_menu_register_type (module);
}

//*****************************************************************************

void
nautilus_module_shutdown (void)
{
}

//*****************************************************************************

void
nautilus_module_list_types (const GType **types,
			    int *num_types)
{
    static GType type_list[1];

    type_list[0] = CREARL_TYPE_CONTEXT_MENU;
    *types = type_list;
    *num_types = G_N_ELEMENTS (type_list);
}

//*****************************************************************************
