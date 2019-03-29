#ifndef __FS_H__
#define __FS_H__

#include <stdint.h>

int fs_open_resources(const char* path);
void fs_set_app_name(const char* app_name);
char* fs_load_asset(const char* path, uint32_t* size);
char* fs_load_pref(const char* path, uint32_t* size);
int fs_save_pref(const char* path, const char* data, uint32_t size);

#endif
