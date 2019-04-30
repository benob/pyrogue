#include <stdio.h>
#include <sys/stat.h>
#include <SDL.h>
#include "miniz.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#endif

#include "rogue_filesystem.h"

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

// may need to change path separator if developed on a different platform
static const char* normalize_path(const char* path) {
	return path;
	/*static char copy[MAX_PATH_SIZE];
	strncpy(copy, path, MAX_PATH_SIZE);
	for(int i = 0; copy[i]; i++) {
		if(copy[i] == PATH_SEPARATOR_OTHER) copy[i] = PATH_SEPARATOR;
	}
	return copy;*/
}

static char* load_file(const char* filename, uint32_t* size) {
//#ifdef __EMSCRIPTEN__
#if 0
	emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, filename); // Blocks here until the operation is complete.
	char* data = NULL;
  if (fetch->status == 200) {
    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
		*size = fetch->numBytes;
		data = malloc(*size + 1);
		memcpy(data, fetch->data, *size);
		data[*size] = 0;
  } else {
    fprintf(stderr, "ERROR: downloading %s failed, HTTP failure status code: %d; status text: %s.\n", fetch->url, fetch->status, fetch->statusText);
  }
  emscripten_fetch_close(fetch);
	return data;
#else
	FILE* fp = fopen(filename, "rb");
	if(!fp) {
		//printf("cannot open file\n");
		perror(filename);
		return NULL;
	}
	if(fseek(fp, 0, SEEK_END) < 0) {
		//printf("cannot seek to end\n");
		perror(filename);
		return NULL;
	}
	*size = ftell(fp);
	if(*size < 0) {
		//printf("cannot ftell\n");
		perror(filename);
		return NULL;
	}
	char* data = malloc(*size + 1);
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
	if(*size != fread(data, 1, *size, fp)) {
		//printf("cannot read\n");
		free(data);
		perror(filename);
		return NULL;
	}
	data[*size] = 0; // ensure terminal zero
	fclose(fp);
	return data;
#endif
}

// TODO: print error messages
static char* load_file_zip(mz_zip_archive* archive, const char* filename, uint32_t* size) {
	size_t uncompressed_size;
	char* data = mz_zip_reader_extract_file_to_heap(archive, filename, &uncompressed_size, 0);
	*size = uncompressed_size;
	return data;
}

int fs_open_resources(const char* _path) {
	const char* path = normalize_path(_path);
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
	} else if(!strcmp(path + strlen(path) - 1, STR_PATH_SEPARATOR)) {
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
		perror(exe);
		return 0;
	}
	int marker_size = strlen(embed_marker);
	fseek(fp, marker_size, SEEK_END);
	char buffer[marker_size];
	if(marker_size != fread(buffer, 1, marker_size, fp)) {
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
  pref_dir = SDL_GetPrefPath("pyrogue", app_name);
}

char* fs_load_asset(const char* _path, uint32_t* size) {
	const char* path = normalize_path(_path);
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

char* fs_load_pref(const char* _path, uint32_t* size) {
	const char* path = normalize_path(_path);
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
		perror(filename);
		return 0;
	}
	if(size != fwrite(data, 1, size, fp)) {
		perror(filename);
		return 0;
	}
	fclose(fp);
	return 1;
}

