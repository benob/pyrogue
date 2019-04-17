#include <stdio.h>

#define USE_MINIZ

#ifdef USE_MINIZ
#include "miniz.h"
#else
#include <zip.h>
#endif

#include <SDL2/SDL.h>

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
#define REVERSED_MARKER "__DAOLYAP__"

enum { RESOURCE_DIR, RESOURCE_ZIP };

static char* resource_path = NULL;
static int resource_type = RESOURCE_DIR;
#ifdef USE_MINIZ
mz_zip_archive* resource_zip = NULL;
#else
static zip_t* resource_zip = NULL;
#endif

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
}

// TODO: print error messages
#ifdef USE_MINIZ
static char* load_file_zip(mz_zip_archive* archive, const char* filename, uint32_t* size) {
	size_t uncompressed_size;
	char* data = mz_zip_reader_extract_file_to_heap(archive, filename, &uncompressed_size, 0);
	*size = uncompressed_size;
	return data;
#else
static char* load_file_zip(zip_t* archive, const char* filename, uint32_t* size) {
	struct zip_stat info;
	if(archive == NULL) return NULL;
	if(-1 == zip_stat(archive, filename, 0, &info)) {
		fprintf(stderr, "%s: %s\n", filename, zip_error_strerror(zip_get_error(archive)));
		return NULL;
	}
	zip_file_t *fp = zip_fopen(archive, filename, 0);
	if(fp == NULL) {
		fprintf(stderr, "%s: %s\n", filename, zip_error_strerror(zip_get_error(archive)));
		return NULL;
	}
	*size = info.size;

	char* data = malloc(*size);
	if(zip_fread(fp, data, *size) != *size) {
		fprintf(stderr, "%s: %s\n", filename, zip_error_strerror(zip_get_error(archive)));
		free(data);
		zip_fclose(fp);
		return NULL;
	}
	zip_fclose(fp);
	return data;
#endif
}

int fs_open_resources(const char* _path) {
	const char* path = normalize_path(_path);
	if(resource_path != NULL) free(resource_path);
	resource_path = strdup(path);
	if(!strcmp(path + strlen(path) - 4, ".zip")) {
		resource_type = RESOURCE_ZIP;
#ifdef USE_MINIZ
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
#else
		int ze;
		if(resource_zip != NULL) zip_close(resource_zip);
		resource_zip = zip_open(resource_path, ZIP_RDONLY, &ze);
		if(resource_zip == NULL) {
			zip_error_t error;
			zip_error_init_with_code(&error, ze);
			fprintf(stderr, "%s: %s\n", resource_path, zip_error_strerror(&error));
			zip_error_fini(&error);
			return 0;
		}
#endif
	} else if(!strcmp(path + strlen(path) - 1, STR_PATH_SEPARATOR)) {
		resource_type = RESOURCE_DIR;
	} else {
		resource_type = RESOURCE_ZIP;
		uint32_t data_size;
		char* data = load_file(path, &data_size);
		if(data == NULL) return 0;
		uint32_t marker_length = strlen(REVERSED_MARKER);
		char marker[marker_length + 1];
		for(int i = 0; i < marker_length; i++) marker[i] = REVERSED_MARKER[marker_length - i - 1];
		marker[marker_length] = 0;
		for(uint32_t offset = data_size - marker_length - 1; offset > 0; offset--) {
			if(!memcmp(data + offset, marker, marker_length)) {
				//fprintf(stderr, "marker found at %u\n", offset);
#ifdef USE_MINIZ
				if(resource_zip != NULL) mz_zip_reader_end(resource_zip);
				resource_zip = calloc(sizeof(mz_zip_archive), 1);
				if(!mz_zip_reader_init_mem(resource_zip, data + offset + marker_length, data_size - offset - marker_length, 0)) {
					fprintf(stderr, "%s: invalid zip\n", resource_path);
					return 0;
				}
				return 1;
#else
				zip_error_t error;
				zip_source_t* src = zip_source_buffer_create(data + offset + marker_length, data_size - offset - marker_length, 1, &error);

				if(src == NULL) {
					fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
					free(data);
					zip_error_fini(&error);
					return 0;
				}
				if ((resource_zip = zip_open_from_source(src, 0, &error)) == NULL) {
					fprintf(stderr, "can't open zip from source: %s\n", zip_error_strerror(&error));
					zip_source_free(src);
					zip_error_fini(&error);
					return 0;
				}
				return 1;
#endif
			}
		}
		//fprintf(stderr, "marker not found in '%s'\n", path);
		return 0;
	}
	return 1;
}

static void __attribute__((destructor)) _fs_cleanup() {
	//if(resource_zip != NULL) zip_close(resource_zip);
#ifdef USE_MINIZ
	if(resource_zip != NULL) mz_zip_reader_end(resource_zip);
#endif
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
		//printf("_path = %s, path = %s, resource_path = %s, filename = %s\n", _path, path, resource_path, filename);
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

int fs_extract_native(const char* self_exe, const char* dir, const char* target) {
	fprintf(stderr, "not implemented yet\n");
	// extract resources from executable
}

int fs_export_native(const char* self_exe, const char* dir, const char* target) {
	fprintf(stderr, "not implemented yet\n");
	// 0) check that game.py exists in dir
	// 1) create zip with contents of dir
	// 2) append zip to argv[0]
}
