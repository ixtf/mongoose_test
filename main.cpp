#include "mongoose/mongoose.h"
#include <functional>

static int kSigNum = 0;
static mg_mgr kMgr;

inline static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);
    kSigNum = sig_num;
}

static mg_serve_http_opts kHttpServerOpts;
static mg_str kMethodGET = MG_MK_STR("GET");
static mg_str kMethodPOST = MG_MK_STR("POST");
static mg_str kApiUri = MG_MK_STR("/api");

inline static int mg_str_equal(const mg_str *s1, const mg_str *s2) {
    return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}

inline static void serve_400(mg_connection *nc) {
    mg_printf(nc, "%s",
              "HTTP/1.0 400 Bad Request\r\n"
              "Content-Length: 0\r\n\r\n");
}

inline static void serve_404(mg_connection *nc) {
    mg_printf(nc, "%s",
              "HTTP/1.0 404 Not Found\r\n"
              "Content-Length: 0\r\n\r\n");
}

static void serve_api(mg_connection *nc, const http_message *hm);

static void serve_websocket(mg_connection *nc, const websocket_message *wm);

static void ev_handler(mg_connection *nc, int ev, void *ev_data) {
    switch (ev) {
        case MG_EV_WEBSOCKET_FRAME: {
            websocket_message *wm = (websocket_message *) ev_data;
            serve_websocket(nc, wm);
            break;
        }
        case MG_EV_HTTP_REQUEST: {
            http_message *hm = (http_message *) ev_data;
            if (mg_str_equal(&kMethodGET, &hm->method)) {
                mg_serve_http(nc, hm, kHttpServerOpts);
            } else if (mg_str_equal(&kMethodPOST, &hm->method)) {
                if (mg_str_equal(&kApiUri, &hm->uri)) {
                    serve_api(nc, hm);
                } else {
                    serve_400(nc);
                }
            } else {
                serve_400(nc);
            }
            break;
        }
        default: {
            break;
        }
    }
}

int main(void) {
    mg_mgr_init(&kMgr, NULL);
    mg_connection *nc = mg_bind(&kMgr, "8080", ev_handler);
    mg_set_protocol_http_websocket(nc);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    while (kSigNum == 0) {
        mg_mgr_poll(&kMgr, 1000);
    }

    mg_mgr_free(&kMgr);
    printf("Exiting on signal %d\n", kSigNum);

    return 0;
}

static void serve_api(mg_connection *nc, const http_message *hm) {
//    mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
//    mg_printf_http_chunk(nc, "test");
//    mg_printf_http_chunk(nc, "", 0);

//    mg_printf(nc, "%s",
//              "HTTP/1.1 500 Server Error\r\n"
//              "Content-Length: 0\r\n\r\n");

    std::string data = "{\"key\":\"value\"}";
    mg_printf(nc,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n\r\n%s",
              (int) data.size(), data.c_str());
}

static void serve_websocket(mg_connection *nc, const websocket_message *wm) {
    mg_connection *c;
    char buf[500];
    char addr[32];
    mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

    for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
        if (c == nc) {
            continue;
        }
        mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
    }
}
