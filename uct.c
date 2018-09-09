#include <node_api.h>
#include <stdio.h>

napi_value uct();
napi_value Init(napi_env, napi_value exports);

napi_value uct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value action_js;

    char *action = "7C";

    status = napi_create_string_utf8(env, action, NAPI_AUTO_LENGTH, &action_js);
    return action_js;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value func;

    status = napi_create_function(env, NULL, 0, uct, NULL, &func);
    if (status != napi_ok) return NULL;

    status = napi_set_named_property(env, exports, "uct", func);
    if (status != napi_ok) return NULL;

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);
