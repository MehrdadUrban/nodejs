#include <node_api.h>
#include <windows.h>
#include <string> 

static void (*module_register)(napi_module*);
static napi_status (*create_string_utf8)(napi_env, const char*, size_t, napi_value *);
static napi_status (*create_string_utf16)(napi_env, const char16_t*, size_t, napi_value *);
static napi_status (*create_function)(napi_env, const char*, size_t, napi_callback, void*, napi_value*);
static napi_status (*set_named_property)(napi_env, napi_value, const char*, napi_value);
static napi_status (*create_int64)(napi_env , int64_t, napi_value*);
static napi_status (*create_error)(napi_env , napi_value, napi_value, napi_value*);
static napi_status (*throw_error)(napi_env , const char*, const char*);
static napi_status (*get_cb_info)(napi_env, napi_callback_info , size_t* , napi_value* ,napi_value* ,void** );
static napi_status (*create_uint32)(napi_env env, uint32_t value, napi_value* result);
static napi_status (*create_object)(napi_env env, napi_value* result);

static void initialize(void) {
    HMODULE NodeJS = GetModuleHandleW(0);
    *(void**)&module_register = (void*) GetProcAddress(NodeJS,"napi_module_register");
    *(void**)&create_string_utf8 = (void*) GetProcAddress(NodeJS,"napi_create_string_utf8");
    *(void**)&create_string_utf16 = (void*) GetProcAddress(NodeJS,"napi_create_string_utf16");
    *(void**)&create_function = (void*) GetProcAddress(NodeJS,"napi_create_function");
    *(void**)&set_named_property = (void*) GetProcAddress(NodeJS,"napi_set_named_property");
    *(void**)&create_int64 = (void*) GetProcAddress(NodeJS,"napi_create_int64");
    *(void**)&create_error = (void*) GetProcAddress(NodeJS,"napi_create_error");
    *(void**)&throw_error = (void*) GetProcAddress(NodeJS,"napi_throw_error");
    *(void**)&get_cb_info = (void*) GetProcAddress(NodeJS,"napi_get_cb_info");
    *(void**)&create_uint32 = (void*) GetProcAddress(NodeJS,"napi_create_uint32");
    *(void**)&create_object = (void*) GetProcAddress(NodeJS,"napi_create_object");
}

void WindowsAPI_Error(napi_env env) {
    DWORD ErrorCode = GetLastError();
    if (ErrorCode!=0) {
        char ErrorMessage[512] = {0};  // Buffer for text.
        DWORD ErrorMessageLen = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL,
                                ErrorCode,
                                0,
                                ErrorMessage,
                                512,
                                NULL );
        if (ErrorMessageLen == 0)
            strcpy(ErrorMessage,"Unknown Error");
        char ErrorCodeText[16];
        sprintf(ErrorCodeText,"%d",ErrorCode);
        throw_error(env,ErrorCodeText,ErrorMessage);
    }
}

napi_value Method(napi_env env, napi_callback_info args)
{
    napi_value msg,code,res;
    wchar_t widestring[256];
    //char utf8string[256];
    DWORD widestringlen = GetModuleFileNameW(0, widestring,256); WindowsAPI_Error(env);
    napi_status status = create_string_utf16(env, (char16_t*)widestring, widestringlen, &msg);
    if (status != napi_ok) return (napi_value)0;
    size_t argc = 8;
    napi_value argv[8];
    napi_value thisArg;
    status = get_cb_info(env, args, &argc, argv, &thisArg, NULL);
    if (status != napi_ok) return (napi_value)0;
    napi_value ret;
    status = create_int64(env, argc, &ret);
    if (status != napi_ok) return (napi_value)0;
    napi_value obj;
    create_object(env, &obj);
    set_named_property(env, obj, "num", ret);
    set_named_property(env, obj, "str", msg);
    
    // size_t utf8stringlen = WideCharToMultiByte(CP_UTF8,0,widestring,widestringlen,utf8string,256,NULL,NULL);
    // CheckError(env);
    //create_int64(env, 122, &code);
    // create_string_utf8(env, "122", NAPI_AUTO_LENGTH , &code);
    // create_string_utf8(env, "First Error", NAPI_AUTO_LENGTH , &msg);
    // napi_status status = create_error(env,code,msg,&res);
    //napi_status status = create_string_utf8(env, utf8string, utf8stringlen, &msg);
    //napi_status status = create_int64(env, (int64_t) &env, &greeting);
    //return status == napi_ok ? msg : (napi_value)0;
    //return res;
    //throw_error(env,"122","New Error From C++");
    //return status == napi_ok ? res : (napi_value)0;
    //CheckError(env);
    return obj;
}

// napi_create_external_arraybuffer(napi_env env,
//                                  void* external_data,
//                                  size_t byte_length,
//                                  napi_finalize finalize_cb,
//                                  void* finalize_hint,
//                                  napi_value* result);


napi_value init(napi_env env, napi_value exports)
{
    napi_value function;

    napi_status status = create_function(env, 0, 0, &Method, 0, &function);

    if (status != napi_ok)
        return (napi_value)0;

    status = set_named_property(env, exports, "hello", function);
    return status == napi_ok ? exports : (napi_value)0;
}

// static void _register_hello(void)__attribute((constructor));
// calls napi_module_register
//NAPI_MODULE(hello, init)
static napi_module _module = {NAPI_MODULE_VERSION,
                           0,
                           "File Name Mehrdad",
                           &init,
                           "Module Name Nazmdar",
                           (void*)0,
                           {0}};

static void _register_hello(void) __attribute((constructor));
static void _register_hello(void)
{
    // TODO: load pointers from calling exe]
    initialize();

    module_register(&_module);
}

