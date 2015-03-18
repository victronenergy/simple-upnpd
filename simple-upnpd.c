#include <libgupnp/gupnp.h>
#include <stdlib.h>
#include <gmodule.h>

static gchar *xmlFileName = "description.xml";

static GOptionEntry entries[] =
{
	{ "xml", 'x', 0, G_OPTION_ARG_FILENAME, &xmlFileName, "upnp description file", 0 },
	{ NULL }
};

int main(int argc, char **argv)
{
	GOptionContext *optionContext;
	GMainLoop *main_loop;
	GError *error = NULL;
	GUPnPContext *context;
	GUPnPRootDevice *dev;

#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif

	optionContext = g_option_context_new (NULL);
	g_option_context_add_main_entries(optionContext, entries, NULL);
	if (!g_option_context_parse(optionContext, &argc, &argv, &error)) {
		g_print("option parsing failed: %s\n", error->message);
		exit(1);
	}

	/* Create the UPnP context */
	context = gupnp_context_new(NULL, NULL, 0, &error);
	if (error) {
		g_printerr("Error creating the GUPnP context: %s\n", error->message);
		g_error_free(error);
		return EXIT_FAILURE;
	}

	/* Create root device */
	dev = gupnp_root_device_new(context, xmlFileName, "");
	if (dev == 0) {
		g_print("creating device failed\n");
		return EXIT_FAILURE;
	}

	gupnp_root_device_set_available(dev, TRUE);

	g_print("Attaching to IP/Host %s on port %d\n", gupnp_context_get_host_ip(context),
			gupnp_context_get_port(context));

	/* Run the main loop */
	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	/* Cleanup */
	g_main_loop_unref(main_loop);
	g_object_unref(dev);
	g_object_unref(context);

	return EXIT_SUCCESS;
}
