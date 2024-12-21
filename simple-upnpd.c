#include <libgupnp/gupnp.h>
#include <libsoup/soup.h>

#include <stdlib.h>
#include <gmodule.h>
#include <string.h>

#if !defined(GUPNP_VERSION_MAJOR) || !defined(GUPNP_VERSION_MINOR)
#error gupnp version unknown
#endif

#define GUPNP_CHECK_VERSION(major,minor,micro)    \
	(GUPNP_VERSION_MAJOR > (major) || \
	(GUPNP_VERSION_MAJOR == (major) && GUPNP_VERSION_MINOR > (minor)) || \
	(GUPNP_VERSION_MAJOR == (major) && GUPNP_VERSION_MINOR == (minor) && \
	GUPNP_VERSION_MICRO >= (micro)))

static gchar *xmlFileName = "description.xml";
static int debug;
static GHashTable *cp_hash;

static GOptionEntry entries[] =
{
	{ "debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "debug, don't fork", 0 },
	{ "xml", 'x', 0, G_OPTION_ARG_FILENAME, &xmlFileName, "upnp description file", 0 },
	{ NULL }
};

#if GUPNP_CHECK_VERSION(1,2,0)
static const char *gupnp_context_get_host_ip(GUPnPContext *context)
{
	return gssdp_client_get_host_ip(GSSDP_CLIENT(context));
}
#endif

#if SOUP_CHECK_VERSION(3,0,0)
static void soup_callback(SoupServer *server, SoupServerMessage *msg, const char *path,
					GHashTable *query, gpointer user_data)
#else
static void soup_callback(SoupServer *server, SoupMessage *msg, const char *path,
					GHashTable *query, SoupClientContext *client, gpointer user_data)
#endif
{
	GUPnPContext *context = user_data;
	const char *ip = gupnp_context_get_host_ip(context);
	gchar *url;

	url = g_strdup_printf("http://%s", ip);
#if SOUP_CHECK_VERSION(3,0,0)
	soup_server_message_set_redirect(msg, 301, url);
#else
	soup_message_set_redirect(msg, 301, url);
#endif
	g_free(url);
}

static gboolean context_equal(GUPnPContext *context1, GUPnPContext *context2)
{
	return g_ascii_strcasecmp(gupnp_context_get_host_ip(context1),
								gupnp_context_get_host_ip(context2)) == 0;
}

static void on_context_available(GUPnPContextManager *context_manager,
									GUPnPContext *context, gpointer *user_data)
{
	GUPnPRootDevice *dev;

	g_print("Context available IP/Host %s\n", gupnp_context_get_host_ip(context));

	/* Create root device */
#if GUPNP_CHECK_VERSION(1,2,0)
	dev = gupnp_root_device_new(context, xmlFileName, "", NULL);
#else
	dev = gupnp_root_device_new(context, xmlFileName, "");
#endif
	if (dev == 0) {
		g_print("creating device failed\n");
		exit(EXIT_FAILURE);
	}
	g_hash_table_insert(cp_hash, g_object_ref(context), dev);
	gupnp_root_device_set_available(dev, TRUE);

	soup_server_add_handler(gupnp_context_get_server(context), "/", soup_callback, context, NULL);
}

static void on_context_unavailable(GUPnPContextManager *context_manager, GUPnPContext *context, gpointer *user_data)
{
	g_print("Dettaching from IP/Host %s\n", gupnp_context_get_host_ip(context));

	g_hash_table_remove(cp_hash, context);
}

int main(int argc, char **argv)
{
	GOptionContext *optionContext;
	GMainLoop *main_loop;
	GError *error = NULL;
	GUPnPContextManager *context_manager;
	pid_t pid;

#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif

	optionContext = g_option_context_new (NULL);
	g_option_context_add_main_entries(optionContext, entries, NULL);
	if (!g_option_context_parse(optionContext, &argc, &argv, &error)) {
		g_print("option parsing failed: %s\n", error->message);
		exit(1);
	}

	cp_hash = g_hash_table_new_full(g_direct_hash, (GEqualFunc) context_equal,
									g_object_unref, g_object_unref);

#if GUPNP_CHECK_VERSION(1,2,0)
	context_manager = gupnp_context_manager_create(0); // Since 0.17.2
#else
	context_manager = gupnp_context_manager_new(NULL, 0); // Since 0.13.0
#endif
	g_assert(context_manager != NULL);
	g_signal_connect(context_manager, "context-available", G_CALLBACK(on_context_available), NULL);
	g_signal_connect(context_manager, "context-unavailable", G_CALLBACK(on_context_unavailable), NULL);

	if (!debug) {
		pid = fork();
		if (pid < 0) {
			g_print("Failed to fork\n");
			return EXIT_FAILURE;
		}

		if (pid != 0)
			return EXIT_SUCCESS;

		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
	}

	/* Run the main loop */
	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	/* Cleanup */
	g_main_loop_unref(main_loop);

	return EXIT_SUCCESS;
}
