#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <curl/curl.h>

static void
sqlite3_url_result_curl_error(sqlite3_context *ctx, CURLUcode err)
{
#define HANDLE_STATUS(s, msg)\
    case s:\
    sqlite3_result_error(ctx, msg, -1);\
    return;

    switch(err) {
        case CURLUE_OK:
            return;
        case CURLUE_OUT_OF_MEMORY:
            sqlite3_result_error_nomem(ctx);
            return;

        HANDLE_STATUS(CURLUE_BAD_HANDLE, "bad handle");
        HANDLE_STATUS(CURLUE_BAD_PARTPOINTER, "bad part pointer");
        HANDLE_STATUS(CURLUE_MALFORMED_INPUT, "malformed input");
        HANDLE_STATUS(CURLUE_BAD_PORT_NUMBER, "bad port number");
        HANDLE_STATUS(CURLUE_UNSUPPORTED_SCHEME, "unsuppported scheme");
        HANDLE_STATUS(CURLUE_URLDECODE, "urldecode");
        HANDLE_STATUS(CURLUE_USER_NOT_ALLOWED, "user not allowed");
        HANDLE_STATUS(CURLUE_UNKNOWN_PART, "unknown part");
        HANDLE_STATUS(CURLUE_NO_SCHEME, "no scheme");
        HANDLE_STATUS(CURLUE_NO_USER, "no user");
        HANDLE_STATUS(CURLUE_NO_PASSWORD, "no password");
        HANDLE_STATUS(CURLUE_NO_OPTIONS, "no options");
        HANDLE_STATUS(CURLUE_NO_HOST, "no host");
        HANDLE_STATUS(CURLUE_NO_PORT, "no port");
        HANDLE_STATUS(CURLUE_NO_QUERY, "no query");
        HANDLE_STATUS(CURLUE_NO_FRAGMENT, "no fragment");
    }
#undef HANDLE_STATUS
}

static void
sqlite3_url_scheme(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    const char *url;
    char *url_part;
    CURLUcode status;
    CURLU *h;

    // XXX check type/return value?
    url = sqlite3_value_text(args[0]);

    h = curl_url();

    if(h == NULL) {
        sqlite3_result_error_nomem(ctx);
        return;
    }

    status = curl_url_set(h, CURLUPART_URL, url, 0);
    if(status != CURLUE_OK) {
        curl_url_cleanup(h);
        sqlite3_url_result_curl_error(ctx, status);
        return;
    }

    status = curl_url_get(h, CURLUPART_SCHEME, &url_part, CURLU_DEFAULT_PORT);
    if(status != CURLUE_OK) {
        curl_url_cleanup(h);
        sqlite3_url_result_curl_error(ctx, status);
        return;
    }

    sqlite3_result_text(ctx, url_part, -1, curl_free); // XXX SQLITE_TRANSIENT?

    curl_url_cleanup(h);
}

static void
sqlite3_url_user(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    const char *url;
    char *url_part;
    CURLUcode status;
    CURLU *h;

    // XXX check type/return value?
    url = sqlite3_value_text(args[0]);

    h = curl_url();

    if(h == NULL) {
        sqlite3_result_error_nomem(ctx);
        return;
    }

    status = curl_url_set(h, CURLUPART_URL, url, 0);
    if(status != CURLUE_OK) {
        curl_url_cleanup(h);
        sqlite3_url_result_curl_error(ctx, status);
        return;
    }

    status = curl_url_get(h, CURLUPART_USER, &url_part, CURLU_DEFAULT_PORT);
    if(status == CURLUE_NO_USER) {
        sqlite3_result_null(ctx);
    } else if(status != CURLUE_OK) {
        curl_url_cleanup(h);
        sqlite3_url_result_curl_error(ctx, status);
        return;
    } else {
        sqlite3_result_text(ctx, url_part, -1, curl_free); // XXX SQLITE_TRANSIENT?
    }

    curl_url_cleanup(h);
}

int
sqlite3_url_init(sqlite3 *db, char **err_out, const sqlite3_api_routines *api)
{
    SQLITE_EXTENSION_INIT2(api);

    sqlite3_create_function(db, "url_scheme", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_scheme, NULL, NULL);
    sqlite3_create_function(db, "url_user", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_user, NULL, NULL);

    return SQLITE_OK;
}
