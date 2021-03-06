#include <stdio.h>
#include <sys/stat.h>
#include <SDL.h>

#include "miniz.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#endif

#include "rogue_filesystem.h"
#include "rogue_util.h"

#define MAX_PATH_SIZE 1024
#ifdef __WIN32__
#define STR_PATH_SEPARATOR "\\"
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_OTHER '/'
#else
#define STR_PATH_SEPARATOR "/"
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_OTHER '\\'
#endif

// marker is reversed to prevent it from showing in the regular executable
char* embed_marker = "__PYROGUE_EMBED__";

enum { RESOURCE_DIR, RESOURCE_ZIP };

static char* resource_path = NULL;
static int resource_type = RESOURCE_DIR;
mz_zip_archive* resource_zip = NULL;

static char* load_file(const char* filename, uint32_t* size) {
	FILE* fp = fopen(filename, "rb");
	if(!fp) {
		//printf("cannot open file\n");
		rl_error("cannot open file '%s'", filename);
		perror(filename);
		return NULL;
	}
	if(fseek(fp, 0, SEEK_END) < 0) {
		//printf("cannot seek to end\n");
		perror(filename);
		return NULL;
	}
	uint32_t length = ftell(fp);
	if(length < 0) {
		//printf("cannot ftell\n");
		perror(filename);
		return NULL;
	}
	char* data = malloc(length + 1);
	if(data == NULL) {
		perror("malloc");
		return NULL;
	}
	if(fseek(fp, 0, SEEK_SET) < 0) {
		free(data);
		//printf("cannot seek to start\n");
		perror(filename);
		return NULL;
	}
	if(length != fread(data, 1, length, fp)) {
		//printf("cannot read\n");
		free(data);
		perror(filename);
		return NULL;
	}
	data[length] = 0; // ensure terminal zero
	fclose(fp);
	if(size != NULL) *size = length;
	return data;
}

// TODO: print error messages
static char* load_file_zip(mz_zip_archive* archive, const char* filename, uint32_t* size) {
	size_t uncompressed_size;
	const char* data = mz_zip_reader_extract_file_to_heap(archive, filename, &uncompressed_size, 0);
	if(data == NULL) return NULL;
	char* string_data = malloc(uncompressed_size + 1);
	memcpy(string_data, data, uncompressed_size);
	string_data[uncompressed_size] = '\0';
	if(size != NULL) *size = uncompressed_size;
	return string_data;
}

int fs_open_resources(const char* path) {
	if(resource_path != NULL) free(resource_path);
	resource_path = strdup(path);
	if(!strcmp(path + strlen(path) - 4, ".zip")) {
		resource_type = RESOURCE_ZIP;
		if(resource_zip != NULL) mz_zip_reader_end(resource_zip);
		resource_zip = calloc(sizeof(mz_zip_archive), 1);
		uint32_t zip_data_size;
		char* zip_data = load_file(resource_path, &zip_data_size);
		if(zip_data == NULL) {
			fprintf(stderr, "%s: cannot read file\n", resource_path);
			return 0;
		}
		if(!mz_zip_reader_init_mem(resource_zip, zip_data, zip_data_size, 0)) {
			fprintf(stderr, "%s: invalid zip\n", resource_path);
			return 0;
		}
		//free(zip_data);
	} else if(path[strlen(path) - 1] == '/' || path[strlen(path) - 1] == '\\') {
		// TODO: should test for actual existance of directory
		resource_type = RESOURCE_DIR;
	} else {
		resource_type = RESOURCE_ZIP;
		uint32_t data_size;
		char* data = load_file(path, &data_size);
		if(data == NULL) return 0;
		int marker_size = strlen(embed_marker);
		if(!strncmp(data + data_size - marker_size, embed_marker, marker_size)) {
			uint32_t offset = *(uint32_t*)(data + data_size - marker_size - sizeof(uint32_t));
			uint32_t size = data_size - offset - marker_size - sizeof(uint32_t);
			if(resource_zip != NULL) mz_zip_reader_end(resource_zip);
			resource_zip = calloc(sizeof(mz_zip_archive), 1);
			if(!mz_zip_reader_init_mem(resource_zip, data + offset, size, 0)) {
				fprintf(stderr, "%s: invalid zip\n", resource_path);
				free(data);
				return 0;
			}
			//free(data);
			return 1;
		}
		//free(data);
		return 0;
	}
	return 1;
}

int fs_has_embed(const char* exe) {
	FILE* fp = fopen(exe, "rb");
	if(!fp) {
		rl_error("cannot open '%s'", exe);
		//perror(exe);
		return 0;
	}
	int marker_size = strlen(embed_marker);
	fseek(fp, marker_size, SEEK_END);
	char buffer[marker_size];
	if(marker_size != fread(buffer, 1, marker_size, fp)) {
		rl_error("cannot read %d bytes from '%s'", marker_size, exe);
		perror(exe);
		fclose(fp);
		return 0;
	}
	fclose(fp);
	if(!strncmp(buffer, embed_marker, marker_size)) {
		return 1;
	}
	return 0;
}

int fs_add_embed(const char* exe, const char* zip, const char* target) {
	uint32_t exe_size, zip_size;
	char* exe_content = load_file(exe, &exe_size);
	if(exe_content == NULL) return 0;
	char* zip_content = load_file(zip, &zip_size);
	if(zip_content == NULL) {
		free(exe_content); 
		return 0;
	}
	FILE* fp = fopen(target, "wb");
	if(exe_size != fwrite(exe_content, 1, exe_size, fp)) {
		perror(target);
		fclose(fp);
		free(exe_content);
		free(zip_content);
		return 0;
	}
	if(zip_size != fwrite(zip_content, 1, zip_size, fp)) {
		perror(target);
		fclose(fp);
		free(exe_content);
		free(zip_content);
		return 0;
	}
	if(sizeof(uint32_t) != fwrite(&exe_size, 1, sizeof(uint32_t), fp)) {
		perror(target);
		fclose(fp);
		free(exe_content);
		free(zip_content);
		return 0;
	}
	if(strlen(embed_marker) != fwrite(embed_marker, 1, strlen(embed_marker), fp)) {
		perror(target);
		fclose(fp);
		free(exe_content);
		free(zip_content);
		return 0;
	}
	fclose(fp);
	free(exe_content);
	free(zip_content);
#ifndef __WIN32__
	chmod(target, 0755);
#endif
	fprintf(stderr, "created '%s' with embedded zip '%s'.\n", target, zip);
	return 1;
}

int fs_extract_embed(const char* exe, const char* target) {
	uint32_t data_size;
	char* data = load_file(exe, &data_size);
	if(data == NULL) return 0;
	int marker_size = strlen(embed_marker);
	if(!strncmp(data + data_size - marker_size, embed_marker, marker_size)) {
		uint32_t offset = *(uint32_t*)(data + data_size - marker_size - sizeof(uint32_t));
		uint32_t size = data_size - offset - marker_size - sizeof(uint32_t);
		FILE* fp = fopen(target, "wb");
		if(!fp) {
			perror(target);
			free(data);
			return 0;
		}
		if(size != fwrite(data + offset, 1, size, fp)) {
			perror(target);
			fclose(fp);
			free(data);
			return 0;
		}
		fclose(fp);
		free(data);
		fprintf(stderr, "extracted embedded zip to '%s'.\n", target);
		return 1;
	}
	fprintf(stderr, "no embedded zip found.\n");
	return 0;
}

static void __attribute__((destructor)) _fs_cleanup() {
	if(resource_zip != NULL) mz_zip_reader_end(resource_zip);
}

static char* pref_dir = NULL;

void fs_set_app_name(const char* app_name) {
	// replace forbidden characters (based on linux and windows restrictions)
	char* name = strdup(app_name);
	char* forbidden = "<>:\"/\\|?*";
	for(int i = 0; name[i]; i++)
		for(int j = 0; forbidden[j]; j++)
			if(name[i] == forbidden[j]) name[i] = '_';
  pref_dir = SDL_GetPrefPath("pyrogue", name);
	free(name);
}

char* fs_load_asset(const char* path, uint32_t* size) {
	if(resource_type == RESOURCE_DIR) {
		char filename[MAX_PATH_SIZE];
		if(resource_path == NULL) snprintf(filename, MAX_PATH_SIZE, "%s", path);
		else snprintf(filename, MAX_PATH_SIZE, "%s%c%s", resource_path, PATH_SEPARATOR, path);
		return load_file(filename, size);
	} else if (resource_type == RESOURCE_ZIP) {
		return load_file_zip(resource_zip, path, size);
	} else {
		fprintf(stderr, "unsupported resource type\n");
		return NULL;
	}
}

int fs_asset_is_file(const char* path) {
	if(resource_type == RESOURCE_DIR) {
		char filename[MAX_PATH_SIZE];
		if(resource_path == NULL) snprintf(filename, MAX_PATH_SIZE, "%s", path);
		else snprintf(filename, MAX_PATH_SIZE, "%s%c%s", resource_path, PATH_SEPARATOR, path);
		struct stat info;
    if(stat(filename, &info) < 0) return 0;
		return S_ISREG(info.st_mode);
	} else if (resource_type == RESOURCE_ZIP) {
		mz_uint32 file_index = mz_zip_reader_locate_file(resource_zip, path, NULL, 0);
		if(file_index < 0) return 0;
		mz_zip_archive_file_stat info;
		if(!mz_zip_reader_file_stat(resource_zip, file_index, &info)) return 0;
		return !info.m_is_directory;
	} else {
		fprintf(stderr, "unsupported resource type\n");
		return 0;
	}
}
int fs_asset_is_directory(const char* _path) {
	static char* path = NULL;
	int path_length = strlen(_path);
	// TODO: might not work on windows (path separator is zip creator-dependent)
	if(_path[path_length - 1] != '/') {
		path = realloc(path, path_length + 2);
		memcpy(path, _path, path_length);
		path[path_length] = '/';
		path[path_length + 1] = '\0';
	} else {
		path = realloc(path, path_length + 1);
		memcpy(path, _path, path_length + 1);
	}
	if(resource_type == RESOURCE_DIR) {
		char filename[MAX_PATH_SIZE];
		if(resource_path == NULL) snprintf(filename, MAX_PATH_SIZE, "%s", path);
		else snprintf(filename, MAX_PATH_SIZE, "%s%c%s", resource_path, PATH_SEPARATOR, path);
		struct stat info;
    if(stat(filename, &info) < 0) return 0;
    return S_ISDIR(info.st_mode);
	} else if (resource_type == RESOURCE_ZIP) {
		mz_uint32 file_index = mz_zip_reader_locate_file(resource_zip, path, NULL, 0);
		if(file_index < 0) return 0;
		mz_zip_archive_file_stat info;
		if(!mz_zip_reader_file_stat(resource_zip, file_index, &info)) return 0;
		return info.m_is_directory;
	} else {
		fprintf(stderr, "unsupported resource type\n");
		return 0;
	}
}

char* fs_load_pref(const char* path, uint32_t* size) {
	char filename[MAX_PATH_SIZE];
	if(pref_dir == NULL) snprintf(filename, MAX_PATH_SIZE, "%s", path);
	else snprintf(filename, MAX_PATH_SIZE, "%s%c%s", pref_dir, PATH_SEPARATOR, path);
	return load_file(filename, size);
}

int fs_save_pref(const char* path, const char* data, uint32_t size) {
	char filename[MAX_PATH_SIZE];
	if(pref_dir == NULL) snprintf(filename, MAX_PATH_SIZE, "%s", path);
	else snprintf(filename, MAX_PATH_SIZE, "%s%c%s", pref_dir, PATH_SEPARATOR, path);
	FILE* fp = fopen(filename, "wb");
	if(!fp) {
		rl_error("cannot save pref file '%s'", path);
		perror(filename);
		return 0;
	}
	if(size != fwrite(data, 1, size, fp)) {
		rl_error("cannot write %d bytes to '%s'", size, path);
		perror(filename);
		return 0;
	}
	fclose(fp);
	return 1;
}

