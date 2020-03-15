#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <curl/curl.h>

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
        sqlite3_result_error(ctx, curl_easy_strerror(status), -1);
        return;
    }

    status = curl_url_get(h, CURLUPART_SCHEME, &url_part, CURLU_DEFAULT_PORT);
    if(status != CURLUE_OK) {
        curl_url_cleanup(h);
        sqlite3_result_error(ctx, curl_easy_strerror(status), -1);
        return;
    }

    sqlite3_result_text(ctx, url_part, -1, curl_free); // XXX SQLITE_TRANSIENT?

    curl_url_cleanup(h);
}

int
sqlite3_url_init(sqlite3 *db, char **err_out, const sqlite3_api_routines *api)
{
    SQLITE_EXTENSION_INIT2(api);

    sqlite3_create_function(db, "url_scheme", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_scheme, NULL, NULL);

    return SQLITE_OK;
}
