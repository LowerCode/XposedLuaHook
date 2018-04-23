
#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <string.h>
#include <alloca.h>
#include <malloc.h>
#include <stdbool.h>
#include "include/inlineHook.h"

#define TAG "jx"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

#define PACKAGE_NAME "com.fangkuai.n1" // 目标应用的包名
#define TARGET_SO "/data/data/com.fangkuai.n1/lib/libgame.so" // 目标应用的libcocos2dlua.so


int (*origin_luaL_loadbuffer)(void *lua_state, char *buff, size_t size, char *name) = NULL;
void free(void* __ptr);



int my_luaL_loadbuffer(void *lua_state, char *buff, size_t size, char *name) {
    LOGD("lua size: %d, name: %s", (uint32_t) size, name);  // 打印lua脚本的大小和名称

    if (name != NULL) {
        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            if (8 < name_len <= 100) {
                char *base_dir = (char *) "/sdcard/hookLua/";
                int i = 0;
                while (i < name_len) {
                    if (name_t[i] == '/') {
                        name_t[i] = '.';
                    }
                    i++;
                }
                if (strstr(name_t, ".lua")) {
                    sprintf(full_name, "%s%s", base_dir, name_t);
                    //lua脚本保存
                        file = fopen(full_name, "wb");
                        if(file!=NULL){
                          fwrite(buff,1,size,file);
                            fclose(file);
                            free(name_t);
                        }



                    //lua脚本hook加载
                    /*file = fopen(full_name, "r");
                    if (file != NULL) {
                        LOGD("[Tencent]-------path-----%s", full_name);
                        fseek(file, 0, SEEK_END);
                        size_t new_size = ftell(file);
                        fseek(file, 0, SEEK_SET);
                        char *new_buff = (char *) alloca(new_size + 1);
                        fread(new_buff, new_size, 1, file);
                        fclose(file);
                        return origin_luaL_loadbuffer(lua_state, buff, size, name);
                    }*/




                }
            }
        }
    }

    return origin_luaL_loadbuffer(lua_state, buff, size, name);
}

JNIEXPORT jint JNICALL  JNI_OnLoad(JavaVM* vm,void* reserved){
    LOGD("JNI_OnLoad enter");

    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOGD("GetEnv OK");

        char so_name[128] = {0};
        sprintf(so_name, TARGET_SO, PACKAGE_NAME);
        void *handle = dlopen(so_name, RTLD_NOW);
        if (handle) {
            LOGD("dlopen() return %08x", (uint32_t) handle);
            void *luabuffer = dlsym(handle, "luaL_loadbuffer");       //luaL_loadbuffer  
            if (luabuffer) {
                LOGD("luaL_loadbuffer function address:%08X", (uint32_t)luabuffer );
                if (ELE7EN_OK == registerInlineHook((uint32_t) luabuffer,(uint32_t)my_luaL_loadbuffer, (uint32_t **) &origin_luaL_loadbuffer)) {
                    LOGD("registerInlineHook luaL_loadbuffer success");
                    if (ELE7EN_OK == inlineHook((uint32_t) luabuffer)) {
                        LOGD("inlineHook luaL_loadbuffer success");
                    } else {
                        LOGD("inlineHook luaL_loadbuffer failure");
                    }
                } else {
                    LOGD("registerInlineHook luaL_loadbuffer failure");
                }
            } else {
                LOGD("dlsym() failure");
            }
        } else {
            LOGD("dlopen() failure");
        }
    } else {
        LOGD("GetEnv failure");
    }

    LOGD("JNI_OnLoad leave");
    return JNI_VERSION_1_6;
}
