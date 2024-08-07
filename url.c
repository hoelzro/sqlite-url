/*
 * Copyright 2020 Rob Hoelz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

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
        case CURLUE_MALFORMED_INPUT:
            sqlite3_result_null(ctx);
            return;
        case CURLUE_OUT_OF_MEMORY:
            sqlite3_result_error_nomem(ctx);
            return;

        HANDLE_STATUS(CURLUE_BAD_HANDLE, "bad handle");
        HANDLE_STATUS(CURLUE_BAD_PARTPOINTER, "bad part pointer");
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
sqlite3_url_part(sqlite3_context *ctx, int nargs, sqlite3_value **args, CURLUPart what, CURLUcode missing_err)
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

    status = curl_url_set(h, CURLUPART_URL, url, CURLU_NON_SUPPORT_SCHEME);
    if(status != CURLUE_OK) {
        curl_url_cleanup(h);
        sqlite3_url_result_curl_error(ctx, status);
        return;
    }

    status = curl_url_get(h, what, &url_part, CURLU_DEFAULT_PORT);
    if(status == missing_err) {
        sqlite3_result_null(ctx);
    } else if(status != CURLUE_OK) {
        curl_url_cleanup(h);
        sqlite3_url_result_curl_error(ctx, status);
        return;
    } else {
        sqlite3_result_text(ctx, url_part, -1, curl_free);
    }

    curl_url_cleanup(h);
}

static void
sqlite3_url_scheme(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_SCHEME, CURLUE_NO_SCHEME);
}

static void
sqlite3_url_user(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_USER, CURLUE_NO_USER);
}

static void
sqlite3_url_password(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_PASSWORD, CURLUE_NO_PASSWORD);
}

static void
sqlite3_url_options(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_OPTIONS, CURLUE_NO_OPTIONS);
}

static void
sqlite3_url_host(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_HOST, CURLUE_NO_HOST);
}

static void
sqlite3_url_port(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_PORT, CURLUE_NO_PORT);
}

static void
sqlite3_url_path(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_PATH, -1);
}

static void
sqlite3_url_query(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_QUERY, CURLUE_NO_QUERY);
}

static void
sqlite3_url_fragment(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_FRAGMENT, CURLUE_NO_FRAGMENT);
}

static void
sqlite3_url_zoneid(sqlite3_context *ctx, int nargs, sqlite3_value **args)
{
    sqlite3_url_part(ctx, nargs, args, CURLUPART_ZONEID, -1);
}

int
sqlite3_url_init(sqlite3 *db, char **err_out, const sqlite3_api_routines *api)
{
    SQLITE_EXTENSION_INIT2(api);

    sqlite3_create_function(db, "url_scheme", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_scheme, NULL, NULL);
    sqlite3_create_function(db, "url_user", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_user, NULL, NULL);
    sqlite3_create_function(db, "url_password", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_password, NULL, NULL);
    sqlite3_create_function(db, "url_options", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_options, NULL, NULL);
    sqlite3_create_function(db, "url_host", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_host, NULL, NULL);
    sqlite3_create_function(db, "url_port", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_port, NULL, NULL);
    sqlite3_create_function(db, "url_path", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_path, NULL, NULL);
    sqlite3_create_function(db, "url_query", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_query, NULL, NULL);
    sqlite3_create_function(db, "url_fragment", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_fragment, NULL, NULL);
    sqlite3_create_function(db, "url_zoneid", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, sqlite3_url_zoneid, NULL, NULL);

    return SQLITE_OK;
}
