#include <libwebsockets.h>
#include <string.h>

static int bad = 1, status;
static struct lws *client_wsi;
static struct lws_context *context;

static char _access_token[1024];
static char _product_name[1024];
static char _organization_id[1024];

static int
the_callback(struct lws *wsi, enum lws_callback_reasons reason,
	      void *user, void *in, size_t len)
{
    printf("callback_http\n");

	switch (reason) {

	/* because we are protocols[0] ... */
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CLIENT_CONNECTION_ERROR: %s\n",
			 in ? (char *)in : "(null)");
		client_wsi = NULL;
		break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        lwsl_hexdump_notice(in, len);
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        printf("established\n");

        int flags = 0 ? LWS_WRITE_BINARY : LWS_WRITE_TEXT;

        /* notice we allowed for LWS_PRE in the payload already */

        char msg[LWS_PRE + 4096];
        int len = sprintf((char*)(msg+LWS_PRE), "{\"access_token\": \"%s\", \"type\": \"start\", \"sampling_rate\": 8000, \"product_name\": \"%s\", \"organization_id\": \"%s\", \"model_alias\": \"model_batoner_japanese\"}", _access_token, _product_name, _organization_id);
        //printf("msg=%s len=%i\n", (char*)(msg+LWS_PRE), len);

        int m = lws_write(wsi, (unsigned char*)(msg + LWS_PRE), len, (enum lws_write_protocol)flags);
        if (m < len) {
            lwsl_err("ERROR %d writing to ws socket\n", m);
            exit(-1);
        }
        break;

	case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
		client_wsi = NULL;
		bad = status != 200;
		lws_cancel_service(lws_get_context(wsi)); /* abort poll wait */
		break;

	default:
		break;
	}

	return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] = {
    { "lws-minimal-client", the_callback, 0, 0, 0, NULL, 0 },
    LWS_PROTOCOL_LIST_TERM
};


void ws_client_start(char *access_token, char *product_name, char *organization_id) {
    strcpy(_access_token, access_token);
    strcpy(_product_name, product_name);
    strcpy(_organization_id, organization_id);

	struct lws_context_creation_info info;
    struct lws_client_connect_info i;

	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE; // | LLL_DEBUG;
		   /*
		    * For LLL_ verbosity above NOTICE to be built into lws,
		    * lws must have been configured and built with
		    * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE
		    *
		    * | LLL_INFO   | LLL_PARSER  | LLL_HEADER | LLL_EXT |
		    *   LLL_CLIENT | LLL_LATENCY | LLL_DEBUG
		    */ ;

	lws_set_log_level(logs, NULL);

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
	info.protocols = protocols;

	/*
	 * since we know this lws context is only ever going to be used with
	 * one client wsis / fds / sockets at a time, let lws know it doesn't
	 * have to use the default allocations for fd tables up to ulimit -n.
	 * It will just allocate for 1 internal and 1 (+ 1 http2 nwsi) that we
	 * will use.
	 */
	info.fd_limit_per_thread = 1 + 1 + 1;

	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		exit(1);
	}

	memset(&i, 0, sizeof i); /* otherwise uninitialized garbage */

	i.context = context;

	i.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED;
    i.port = 443;
	i.address = "realtime.stt.batoner.works";
	i.path = "real-time-decode/ws/";
	i.host = i.address;
	i.origin = i.address;
    i.ssl_connection = LCCSCF_USE_SSL;
    i.protocol = protocols[0].name;
    i.local_protocol_name = "lws-minimal-client";

	/* currently custom headers receive only works with h1 */
	i.alpn = "http/1.1";

	i.pwsi = &client_wsi;
	lws_client_connect_via_info(&i);

}

void ws_client_loop() {
	lws_service(context, -1);
}

void ws_client_stop() {
	lws_context_destroy(context);
	lwsl_user("Completed: %s\n", bad ? "failed" : "OK");
}
