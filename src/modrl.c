/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/parsenum.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/objint.h"

#if PYROGUE_MOD_RL

#include "rogue.h"

/************** rogue_random *********************/

STATIC mp_obj_t mod_rl_random_next() {
	mp_uint_t result = rl_random_next();
	return mp_obj_new_int_from_uint(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rl_random_next_obj, mod_rl_random_next);

STATIC mp_obj_t mod_rl_random_int(mp_obj_t a_in, mp_obj_t b_in) {
	mp_int_t a = mp_obj_get_int(a_in);
	mp_int_t b = mp_obj_get_int(b_in);
	mp_int_t result = rl_random_int(a, b);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_random_int_obj, mod_rl_random_int);

STATIC mp_obj_t mod_rl_random() {
	mp_float_t result = rl_random();
	return mp_obj_new_float(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rl_random_obj, mod_rl_random);

STATIC mp_obj_t mod_rl_roll(mp_obj_t obj) {
	size_t len;
	const char *buf = mp_obj_str_get_data(obj, &len);
	mp_int_t result = rl_roll(buf);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_roll_obj, mod_rl_roll);

STATIC mp_obj_t mod_rl_set_seed(mp_obj_t seed_in) {
	mp_uint_t seed = mp_obj_get_int(seed_in);
	mp_uint_t result = rl_set_seed(seed);
	return mp_obj_new_int_from_uint(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_set_seed_obj, mod_rl_set_seed);

STATIC mp_obj_t mod_rl_get_seed() {
	mp_uint_t result = rl_get_seed();
	return mp_obj_new_int_from_uint(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rl_get_seed_obj, mod_rl_get_seed);

/************** rogue_filesystem *********************/

STATIC mp_obj_t mod_fs_open_resources(mp_obj_t path_in) {
	size_t len;
	const char *path = mp_obj_str_get_data(path_in, &len);
	mp_int_t result = fs_open_resources(path);
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_fs_open_resources_obj, mod_fs_open_resources);

STATIC mp_obj_t mod_fs_set_app_name(mp_obj_t app_name_in) {
	size_t len;
	const char *app_name = mp_obj_str_get_data(app_name_in, &len);
	fs_set_app_name(app_name);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_fs_set_app_name_obj, mod_fs_set_app_name);

STATIC mp_obj_t mod_fs_load_asset(mp_obj_t path_in) {
	size_t len;
	const char *path = mp_obj_str_get_data(path_in, &len);
	uint32_t size;
	unsigned char* result = (unsigned char*) fs_load_asset(path, &size);
	if(result == NULL) return mp_const_none;
	return mp_obj_new_bytes(result, size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_fs_load_asset_obj, mod_fs_load_asset);

STATIC mp_obj_t mod_fs_load_pref(mp_obj_t path_in) {
	size_t len;
	const char *path = mp_obj_str_get_data(path_in, &len);
	uint32_t size;
	unsigned char* result = (unsigned char*) fs_load_pref(path, &size);
	if(result == NULL) return mp_const_none;
	return mp_obj_new_bytes(result, size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_fs_load_pref_obj, mod_fs_load_pref);

STATIC mp_obj_t mod_fs_save_pref(mp_obj_t path_in, mp_obj_t data_in) {
	size_t path_len;
	const char *path = mp_obj_str_get_data(path_in, &path_len);
	size_t data_len;
	const char *data = mp_obj_str_get_data(data_in, &data_len);
	mp_int_t result = fs_save_pref(path, data, data_len);
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_fs_save_pref_obj, mod_fs_save_pref);

/************* rogue_array ********************/

extern const mp_obj_type_t mp_type_rl_array;

typedef struct {
	mp_obj_base_t base;
	array_t* array;
} mp_obj_rl_array_t;

STATIC mp_obj_t mod_rl_array_get(mp_obj_t self_in, mp_obj_t i_in, mp_obj_t j_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t i = mp_obj_get_int(i_in);
	mp_int_t j = mp_obj_get_int(j_in);
	mp_int_t result = rl_array_get(self->array, i, j);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_rl_array_get_obj, mod_rl_array_get);

STATIC mp_obj_t mod_rl_array_set(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t i = mp_obj_get_int(args[1]);
	mp_int_t j = mp_obj_get_int(args[2]);
	mp_int_t value = mp_obj_get_int(args[3]);
	rl_array_set(self->array, i, j, value);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_set_obj, 4, 4, mod_rl_array_set);

STATIC mp_obj_t mod_rl_array_free(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	rl_array_free(self->array);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_free_obj, mod_rl_array_free);

STATIC void mod_rl_array_print(const mp_print_t *print, mp_obj_t o_in, mp_print_kind_t kind) {
	mp_obj_rl_array_t *o = MP_OBJ_TO_PTR(o_in);
	int width = rl_array_width(o->array);
	int height = rl_array_height(o->array);
	mp_print_str(print, "Array[");
	for(int j = 0; j < height; j++) {
		mp_print_str(print, "[");
		for(int i = 0; i < width; i++) {
			if(i > 0) mp_print_str(print, ", ");
			mp_int_t value = rl_array_get(o->array, i, j);
			mp_printf(print, "%d", value);
		}
		if(j < height - 1) mp_print_str(print, "], ");
		else mp_print_str(print, "]");
	}
	mp_print_str(print, "]");
}

STATIC mp_obj_t mod_rl_array_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_obj_tuple_t* index = MP_OBJ_TO_PTR(index_in);
	if(index->len == 2) {
		mp_int_t i = mp_obj_get_int(index->items[0]);
		mp_int_t j = mp_obj_get_int(index->items[1]);
		if(value_in == MP_OBJ_SENTINEL) {
			mp_int_t result = rl_array_get(self->array, i, j);
			return mp_obj_new_int(result);
		} else {
			mp_int_t value = mp_obj_get_int(value_in);
			rl_array_set(self->array, i, j, value);
			return mp_const_none;
		}
	} else {
		mp_raise_msg(&mp_type_IndexError, "rl_array subscripts only supports 2-dim indexing");
	}
	return MP_OBJ_NULL;
}

STATIC mp_obj_t mod_rl_array_width(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t width = rl_array_width(self->array);
	return mp_obj_new_int(width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_width_obj, mod_rl_array_width);

STATIC mp_obj_t mod_rl_array_height(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t height = rl_array_height(self->array);
	return mp_obj_new_int(height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_height_obj, mod_rl_array_height);

STATIC mp_obj_t mod_rl_array_fill(mp_obj_t self_in, mp_obj_t value_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t value = mp_obj_get_int(value_in);
	rl_array_fill(self->array, value);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_array_fill_obj, mod_rl_array_fill);

STATIC mp_obj_t mod_rl_array_replace(mp_obj_t self_in, mp_obj_t value_in, mp_obj_t replacement_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t value = mp_obj_get_int(value_in);
	mp_int_t replacement = mp_obj_get_int(replacement_in);
	rl_array_replace(self->array, value, replacement);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_rl_array_replace_obj, mod_rl_array_replace);

STATIC mp_obj_t mod_rl_array_random_int(mp_obj_t self_in, mp_obj_t a_in, mp_obj_t b_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t a = mp_obj_get_int(a_in);
	mp_int_t b = mp_obj_get_int(b_in);
	rl_array_random_int(self->array, a, b);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_rl_array_random_int_obj, mod_rl_array_random_int);

STATIC mp_obj_t mod_rl_array_random(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	rl_array_random(self->array);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_random_obj, mod_rl_array_random);

STATIC mp_obj_t mod_rl_array_print_ascii(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	size_t len;
	const char *mapping = mp_obj_str_get_data(args[1], &len);
	const char *fg = NULL;
	if(n_args > 2) fg = mp_obj_str_get_data(args[2], &len);
	const char *bg = NULL;
	if(n_args > 3) bg = mp_obj_str_get_data(args[3], &len);
	tty_print_array(self->array, mapping, fg, bg);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_print_ascii_obj, 2, 4, mod_rl_array_print_ascii);

STATIC mp_obj_t mod_rl_array_line(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x1 = mp_obj_get_int(args[1]);
	mp_int_t y1 = mp_obj_get_int(args[2]);
	mp_int_t x2 = mp_obj_get_int(args[3]);
	mp_int_t y2 = mp_obj_get_int(args[4]);
	mp_int_t value = 1;
	if(n_args > 5) value = mp_obj_get_int(args[5]);
	rl_array_line(self->array, x1, y1, x2, y2, value);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_line_obj, 5, 6, mod_rl_array_line);

STATIC mp_obj_t mod_rl_array_rect(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x1 = mp_obj_get_int(args[1]);
	mp_int_t y1 = mp_obj_get_int(args[2]);
	mp_uint_t w = mp_obj_get_int(args[3]);
	mp_uint_t h = mp_obj_get_int(args[4]);
	mp_int_t value = 1;
	if(n_args > 5) value = mp_obj_get_int(args[5]);
	rl_array_rect(self->array, x1, y1, w, h, value);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_rect_obj, 5, 6, mod_rl_array_rect);

STATIC mp_obj_t mod_rl_array_can_see(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x1 = mp_obj_get_int(args[1]);
	mp_int_t y1 = mp_obj_get_int(args[2]);
	mp_int_t x2 = mp_obj_get_int(args[3]);
	mp_int_t y2 = mp_obj_get_int(args[4]);
	mp_int_t blocking = 1;
	if(n_args > 5) blocking = mp_obj_get_int(args[5]);
	mp_int_t result = rl_array_can_see(self->array, x1, y1, x2, y2, blocking);
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_can_see_obj, 5, 6, mod_rl_array_can_see);

STATIC mp_obj_t mod_rl_array_field_of_view(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t xc = mp_obj_get_int(args[1]);
	mp_int_t yc = mp_obj_get_int(args[2]);
	mp_int_t radius = mp_obj_get_int(args[3]);
	mp_int_t blocking = 1;
	if(n_args > 4) blocking = mp_obj_get_int(args[4]);
	mp_int_t light_walls = 0;
	if(n_args > 5) light_walls = mp_obj_get_int(args[5]);
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = rl_array_field_of_view(self->array, xc, yc, radius, blocking, light_walls);
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_field_of_view_obj, 4, 6, mod_rl_array_field_of_view);

STATIC mp_obj_t mod_rl_array_dijkstra(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	rl_array_dijkstra(self->array);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_dijkstra_obj, mod_rl_array_dijkstra);

STATIC mp_obj_t mod_rl_array_shortest_path(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x1 = mp_obj_get_int(args[1]);
	mp_int_t y1 = mp_obj_get_int(args[2]);
	mp_int_t x2 = mp_obj_get_int(args[3]);
	mp_int_t y2 = mp_obj_get_int(args[4]);
	mp_int_t blocking = 1;
	if(n_args > 5) blocking = mp_obj_get_int(args[5]);
	path_t* path = rl_array_shortest_path(self->array, x1, y1, x2, y2, blocking);
	if(path == NULL) {
		mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(0, NULL));
		return tuple;
	} else {
		mp_obj_tuple_t *result = MP_OBJ_TO_PTR(mp_obj_new_tuple(path->size, NULL));
		for(int i = 0; i < path->size; i++) {
			mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
			tuple->items[0] = mp_obj_new_int(path->points[i].x);
			tuple->items[1] = mp_obj_new_int(path->points[i].y);
			result->items[i] = tuple;
		}
		free(path);
		return result;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_shortest_path_obj, 5, 6, mod_rl_array_shortest_path);

STATIC mp_obj_t mod_rl_array_add(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t value = mp_obj_get_int(args[1]);
	mp_int_t blocking = VALUE_MAX;
	if(n_args > 2) blocking = mp_obj_get_int(args[2]);
	rl_array_add(self->array, value, blocking);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_add_obj, 2, 3, mod_rl_array_add);

STATIC mp_obj_t mod_rl_array_mul(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t value = mp_obj_get_int(args[1]);
	mp_int_t blocking = VALUE_MAX;
	if(n_args > 2) blocking = mp_obj_get_int(args[2]);
	rl_array_mul(self->array, value, blocking);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_mul_obj, 2, 3, mod_rl_array_mul);

STATIC mp_obj_t mod_rl_array_min(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t blocking = VALUE_MAX;
	if(n_args > 1) blocking = mp_obj_get_int(args[1]);
	mp_int_t result = rl_array_min(self->array, blocking);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_min_obj, 1, 2, mod_rl_array_min);

STATIC mp_obj_t mod_rl_array_max(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t blocking = VALUE_MAX;
	if(n_args > 1) blocking = mp_obj_get_int(args[1]);
	mp_int_t result = rl_array_max(self->array, blocking);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_max_obj, 1, 2, mod_rl_array_max);

STATIC mp_obj_t mod_rl_array_argmin(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t blocking = VALUE_MAX;
	if(n_args > 1) blocking = mp_obj_get_int(args[1]);
	point_t result = rl_array_argmin(self->array, blocking);
	mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
	tuple->items[0] = mp_obj_new_int(result.x);
	tuple->items[1] = mp_obj_new_int(result.y);
	return tuple;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_argmin_obj, 1, 2, mod_rl_array_argmin);

STATIC mp_obj_t mod_rl_array_argmax(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t blocking = VALUE_MAX;
	if(n_args > 1) blocking = mp_obj_get_int(args[1]);
	point_t result = rl_array_argmax(self->array, blocking);
	mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
	tuple->items[0] = mp_obj_new_int(result.x);
	tuple->items[1] = mp_obj_new_int(result.y);
	return tuple;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_argmax_obj, 1, 2, mod_rl_array_argmax);

STATIC mp_obj_t mod_rl_array_find_random(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t needle = mp_obj_get_int(args[1]);
	mp_int_t tries = 100;
	if(n_args > 2) tries = mp_obj_get_int(args[2]);
	int rx = -1, ry = -1;
	rl_array_find_random(self->array, needle, tries, &rx, &ry);
	mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
	tuple->items[0] = mp_obj_new_int(rx);
	tuple->items[1] = mp_obj_new_int(ry);
	return tuple;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_find_random_obj, 2, 3, mod_rl_array_find_random);

STATIC mp_obj_t mod_rl_array_place_random(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t needle = mp_obj_get_int(args[1]);
	mp_int_t value = mp_obj_get_int(args[2]);
	mp_int_t tries = 100;
	if(n_args > 3) tries = mp_obj_get_int(args[3]);
	int rx = -1, ry = -1;
	rl_array_place_random(self->array, needle, value, tries, &rx, &ry);
	mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
	tuple->items[0] = mp_obj_new_int(rx);
	tuple->items[1] = mp_obj_new_int(ry);
	return tuple;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_place_random_obj, 2, 3, mod_rl_array_place_random);

STATIC mp_obj_t mod_rl_array_copy(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self);
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = rl_array_copy(self->array);
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_copy_obj, mod_rl_array_copy);

STATIC const mp_rom_map_elem_t mod_rl_array_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mod_rl_array_free_obj) },
	{ MP_ROM_QSTR(MP_QSTR_free), MP_ROM_PTR(&mod_rl_array_free_obj) },
	{ MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&mod_rl_array_get_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&mod_rl_array_set_obj) },
	{ MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&mod_rl_array_width_obj) },
	{ MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&mod_rl_array_height_obj) },
	{ MP_ROM_QSTR(MP_QSTR_print_ascii), MP_ROM_PTR(&mod_rl_array_print_ascii_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&mod_rl_array_fill_obj) },
	{ MP_ROM_QSTR(MP_QSTR_replace), MP_ROM_PTR(&mod_rl_array_replace_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_int), MP_ROM_PTR(&mod_rl_array_random_int_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mod_rl_array_random_obj) },
	{ MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&mod_rl_array_line_obj) },
	{ MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&mod_rl_array_rect_obj) },
	{ MP_ROM_QSTR(MP_QSTR_can_see), MP_ROM_PTR(&mod_rl_array_can_see_obj) },
	{ MP_ROM_QSTR(MP_QSTR_field_of_view), MP_ROM_PTR(&mod_rl_array_field_of_view_obj) },
	{ MP_ROM_QSTR(MP_QSTR_dijkstra), MP_ROM_PTR(&mod_rl_array_dijkstra_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shortest_path), MP_ROM_PTR(&mod_rl_array_shortest_path_obj) },
	{ MP_ROM_QSTR(MP_QSTR_add), MP_ROM_PTR(&mod_rl_array_add_obj) },
	{ MP_ROM_QSTR(MP_QSTR_mul), MP_ROM_PTR(&mod_rl_array_mul_obj) },
	{ MP_ROM_QSTR(MP_QSTR_min), MP_ROM_PTR(&mod_rl_array_min_obj) },
	{ MP_ROM_QSTR(MP_QSTR_max), MP_ROM_PTR(&mod_rl_array_max_obj) },
	{ MP_ROM_QSTR(MP_QSTR_argmin), MP_ROM_PTR(&mod_rl_array_argmin_obj) },
	{ MP_ROM_QSTR(MP_QSTR_argmax), MP_ROM_PTR(&mod_rl_array_argmax_obj) },
	{ MP_ROM_QSTR(MP_QSTR_find_random), MP_ROM_PTR(&mod_rl_array_find_random_obj) },
	{ MP_ROM_QSTR(MP_QSTR_place_random), MP_ROM_PTR(&mod_rl_array_place_random_obj) },
	{ MP_ROM_QSTR(MP_QSTR_copy), MP_ROM_PTR(&mod_rl_array_copy_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mod_rl_array_locals_dict, mod_rl_array_locals_dict_table);

STATIC mp_obj_t mod_rl_array_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	(void)type_in;
	mp_arg_check_num(n_args, n_kw, 2, 2, false);
	mp_int_t width = mp_obj_get_int(args[0]);
	mp_int_t height = mp_obj_get_int(args[1]);
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = rl_array_new(width, height);
	return MP_OBJ_FROM_PTR(output);
}

const mp_obj_type_t mp_type_rl_array = {
	{ &mp_type_type },
	.name = MP_QSTR_rl_array,
	.print = mod_rl_array_print,
	.make_new = mod_rl_array_make_new,
	//.unary_op = list_unary_op,
	//.binary_op = list_binary_op,
	.subscr = mod_rl_array_subscr,
	//.getiter = list_getiter,
	.locals_dict = (mp_obj_dict_t*)&mod_rl_array_locals_dict,
};

/************* rogue_display ******************/

STATIC mp_obj_t mod_td_init(mp_obj_t title_in, mp_obj_t width_in, mp_obj_t height_in) {
	size_t len;
	const char *title = mp_obj_str_get_data(title_in, &len);
	mp_int_t width = mp_obj_get_int(width_in);
	mp_int_t height = mp_obj_get_int(height_in);
	mp_int_t result = td_init(title, width, height);
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_td_init_obj, mod_td_init);

STATIC mp_obj_t mod_td_load_font(size_t n_args, const mp_obj_t *args) {
	size_t len;
	const char *font_path = mp_obj_str_get_data(args[0], &len);
	mp_int_t size = mp_obj_get_int(args[1]);
	mp_int_t line_height = 0;
	if(n_args > 2) line_height = mp_obj_get_int(args[2]);
	td_load_font(font_path, size, line_height);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_load_font_obj, 2, 3, mod_td_load_font);

STATIC mp_obj_t mod_td_load_image(size_t n_args, const mp_obj_t *args) {
	mp_int_t image = mp_obj_get_int(args[0]);
	size_t len;
	const char *path = mp_obj_str_get_data(args[1], &len);
	mp_int_t tile_width = 8;
	if(n_args > 2) tile_width = mp_obj_get_int(args[2]);
	mp_int_t tile_height = 8;
	if(n_args > 3) tile_height = mp_obj_get_int(args[3]);
	mp_int_t result = td_load_image(image, path, tile_width, tile_height);
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_load_image_obj, 2, 4, mod_td_load_image);

STATIC mp_obj_t mod_td_draw_image(mp_obj_t image_in, mp_obj_t x_in, mp_obj_t y_in) {
	mp_int_t image = mp_obj_get_int(image_in);
	mp_int_t x = mp_obj_get_int(x_in);
	mp_int_t y = mp_obj_get_int(y_in);
	td_draw_image(image, x, y);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_td_draw_image_obj, mod_td_draw_image);

STATIC mp_obj_t mod_td_draw_tile(size_t n_args, const mp_obj_t *args) {
	mp_int_t image = mp_obj_get_int(args[0]);
	mp_int_t x = mp_obj_get_int(args[1]);
	mp_int_t y = mp_obj_get_int(args[2]);
	mp_int_t tile = mp_obj_get_int(args[3]);
	td_draw_tile(image, x, y, tile);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_draw_tile_obj, 4, 4, mod_td_draw_tile);

STATIC mp_obj_t mod_td_colorize_tile(size_t n_args, const mp_obj_t *args) {
	mp_int_t image = mp_obj_get_int(args[0]);
	mp_int_t x = mp_obj_get_int(args[1]);
	mp_int_t y = mp_obj_get_int(args[2]);
	mp_int_t tile = mp_obj_get_int(args[3]);
	mp_uint_t fg = 0;
	if(n_args > 4) fg = mp_obj_get_int(args[4]);
	mp_uint_t bg = 0;
	if(n_args > 5) bg = mp_obj_get_int(args[5]);
	td_colorize_tile(image, x, y, tile, fg, bg);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_colorize_tile_obj, 4, 6, mod_td_colorize_tile);

//void td_draw_array(int index, array_t* a, int x, int y, int x_shift, int y_shift, int info_size, int* info_mapping, uint32_t* info_fg, uint32_t* info_bg);
mp_obj_t mod_td_draw_array(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	static const mp_arg_t allowed_args[] = {
		/*{ MP_QSTR_array, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
		{ MP_QSTR_x, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },
		{ MP_QSTR_y, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },*/
		{ MP_QSTR_image, MP_ARG_INT, {.u_int = 0} },
		{ MP_QSTR_x_shift, MP_ARG_INT, {.u_int = 0} },
		{ MP_QSTR_y_shift, MP_ARG_INT, {.u_int = 0} },
		{ MP_QSTR_mapping, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
		{ MP_QSTR_fg, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
		{ MP_QSTR_bg, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
	};

	// parse args
	struct {
		mp_arg_val_t /*array, x, y,*/ image, x_shift, y_shift, mapping, fg, bg;
	} args;

	mp_arg_parse_all(0, pos_args + 3, kw_args,
			MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

	mp_check_self(mp_obj_is_type(pos_args[0], &mp_type_rl_array));
	mp_obj_rl_array_t *array = MP_OBJ_TO_PTR(pos_args[0]);
	//mp_obj_rl_array_t* array = mp_instance_cast_to_native_base(pos_args[0], &mp_type_rl_array);
	mp_int_t x = mp_obj_get_int(pos_args[1]);
	mp_int_t y = mp_obj_get_int(pos_args[2]);

	int* mapping = NULL;
	uint32_t* fg = NULL;
	uint32_t* bg = NULL;

	int size = 0;
	if(args.mapping.u_obj != mp_const_none) {
		size = mp_obj_get_int(mp_obj_len(args.mapping.u_obj));
		mapping = malloc(size * sizeof(int));
		mp_obj_t iterable = mp_getiter(args.mapping.u_obj, NULL);
		mp_obj_t item;
		int i = 0;
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			mapping[i++] = mp_obj_get_int(item);
		}
	}

	if(args.fg.u_obj != mp_const_none) {
		size = mp_obj_get_int(mp_obj_len(args.fg.u_obj));
		fg = malloc(size * sizeof(int));
		mp_obj_t iterable = mp_getiter(args.fg.u_obj, NULL);
		mp_obj_t item;
		int i = 0;
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			fg[i++] = mp_obj_get_int(item);
		}
	}

	if(args.bg.u_obj != mp_const_none) {
		size = mp_obj_get_int(mp_obj_len(args.bg.u_obj));
		bg = malloc(size * sizeof(int));
		mp_obj_t iterable = mp_getiter(args.bg.u_obj, NULL);
		mp_obj_t item;
		int i = 0;
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			bg[i++] = mp_obj_get_int(item);
		}
	}

	td_draw_array(args.image.u_int, array->array, x, y, args.x_shift.u_int, args.y_shift.u_int, size, mapping, fg, bg);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_td_draw_array_obj, 3, mod_td_draw_array);

STATIC mp_obj_t mod_td_set_buffer(mp_obj_t buffer_in) {
	mp_int_t buffer = mp_obj_get_int(buffer_in);
	td_set_buffer(buffer);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_set_buffer_obj, mod_td_set_buffer);

STATIC mp_obj_t mod_td_print_text(size_t n_args, const mp_obj_t *args) {
	mp_int_t x = mp_obj_get_int(args[0]);
	mp_int_t y = mp_obj_get_int(args[1]);
	size_t len;
	const char *text = mp_obj_str_get_data(args[2], &len);
	mp_uint_t color = 0xffffffff;
	if(n_args > 3) color = mp_obj_get_int(args[3]);
	mp_int_t align = TD_ALIGN_LEFT;
	if(n_args > 4) align = mp_obj_get_int(args[4]);
	td_print_text(x, y, text, color, align);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_print_text_obj, 3, 5, mod_td_print_text);

STATIC mp_obj_t mod_td_size_text(mp_obj_t text_in) {
	int width, height;
	size_t len;
	const char *text = mp_obj_str_get_data(text_in, &len);
	td_size_text(text, &width, &height);
	mp_obj_tuple_t *result = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
	result->items[0] = mp_obj_new_int(width);
	result->items[1] = mp_obj_new_int(height);
	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_size_text_obj, mod_td_size_text);

STATIC mp_obj_t mod_td_fill_rect(size_t n_args, const mp_obj_t *args) {
	mp_int_t x = mp_obj_get_int(args[0]);
	mp_int_t y = mp_obj_get_int(args[1]);
	mp_int_t w = mp_obj_get_int(args[2]);
	mp_int_t h = mp_obj_get_int(args[3]);
	mp_uint_t color = 0xffffffff;
	if(n_args > 4) color = mp_obj_get_int(args[4]);
	td_fill_rect(x, y, w, h, color);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_fill_rect_obj, 4, 5, mod_td_fill_rect);

STATIC mp_obj_t mod_td_draw_rect(size_t n_args, const mp_obj_t *args) {
	mp_int_t x = mp_obj_get_int(args[0]);
	mp_int_t y = mp_obj_get_int(args[1]);
	mp_int_t w = mp_obj_get_int(args[2]);
	mp_int_t h = mp_obj_get_int(args[3]);
	mp_uint_t color = 0xffffffff;
	if(n_args > 4) color = mp_obj_get_int(args[4]);
	td_draw_rect(x, y, w, h, color);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_draw_rect_obj, 4, 5, mod_td_draw_rect);

void td_draw_points(td_point_t* points, int num, uint32_t color);
STATIC mp_obj_t mod_td_draw_points(mp_obj_t points_in, mp_obj_t color_in) {
	mp_int_t size = mp_obj_get_int(mp_obj_len(points_in));
	td_point_t* points = malloc(size * sizeof(point_t));
	for(int i = 0; i < size; i++) {
		mp_obj_t* tuple_in = mp_obj_subscr(points_in, mp_obj_new_int(i), MP_OBJ_SENTINEL);
		if(!mp_obj_is_type(tuple_in, &mp_type_tuple)) mp_raise_msg(&mp_type_TypeError, "expecting list of tuples");
		mp_obj_tuple_t* tuple = MP_OBJ_TO_PTR(tuple_in);
		if(tuple->len != 2) mp_raise_msg(&mp_type_ValueError, "expecting tuples of size 2");

		points[i].x = mp_obj_get_int(tuple->items[0]);
		points[i].y = mp_obj_get_int(tuple->items[1]);
	}
	mp_uint_t color = mp_obj_get_int(color_in);
	td_draw_points(points, size, color);
	free(points);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_td_draw_points_obj, mod_td_draw_points);

STATIC mp_obj_t mod_td_still_running() {
	mp_int_t result = td_still_running();
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_still_running_obj, mod_td_still_running);

STATIC mp_obj_t mod_td_wait_key() {
	mp_int_t result = td_wait_key();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_wait_key_obj, mod_td_wait_key);

STATIC mp_obj_t mod_td_wait_event(size_t n_args, const mp_obj_t *args) {
	mp_int_t include_mouse = 0;
	if(n_args > 0) include_mouse = mp_obj_is_true(args[0]);
	mp_int_t result = td_wait_event(include_mouse);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_wait_event_obj, 0, 1, mod_td_wait_event);

STATIC mp_obj_t mod_td_poll_event() {
	mp_int_t result = td_poll_event();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_poll_event_obj, mod_td_poll_event);

STATIC mp_obj_t mod_td_present() {
	td_present();
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_present_obj, mod_td_present);

STATIC mp_obj_t mod_td_delay(mp_obj_t ms_in) {
	mp_uint_t ms = mp_obj_get_int(ms_in);
	td_delay(ms);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_delay_obj, mod_td_delay);

STATIC mp_obj_t mod_td_clear() {
	td_clear();
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_clear_obj, mod_td_clear);

STATIC mp_obj_t mod_td_quit() {
	td_quit();
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_quit_obj, mod_td_quit);

STATIC mp_obj_t mod_td_color(size_t n_args, const mp_obj_t *args) {
	unsigned char r = (unsigned char) mp_obj_get_int(args[0]);
	unsigned char g = (unsigned char) mp_obj_get_int(args[1]);
	unsigned char b = (unsigned char) mp_obj_get_int(args[2]);
	unsigned char a = 0xff;
	if(n_args > 3) a = (unsigned char) mp_obj_get_int(args[3]);
	uint32_t result = td_color_rgba(r, g, b, a);
	return mp_obj_new_int_from_uint(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_color_obj, 3, 4, mod_td_color);

STATIC mp_obj_t mod_td_hsv_color(size_t n_args, const mp_obj_t *args) {
	unsigned char h = (unsigned char) mp_obj_get_int(args[0]);
	unsigned char s = (unsigned char) mp_obj_get_int(args[1]);
	unsigned char v = (unsigned char) mp_obj_get_int(args[2]);
	unsigned char a = 0xff;
	if(n_args > 3) a = (unsigned char) mp_obj_get_int(args[3]);
	uint32_t result = td_hsv_color(h, s, v, a);
	return mp_obj_new_int_from_uint(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_hsv_color_obj, 3, 4, mod_td_hsv_color);

STATIC mp_obj_t mod_td_mouse_x() {
	mp_int_t result = td_mouse_x();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_mouse_x_obj, mod_td_mouse_x);

STATIC mp_obj_t mod_td_mouse_y() {
	mp_int_t result = td_mouse_y();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_mouse_y_obj, mod_td_mouse_y);

STATIC mp_obj_t mod_td_mouse_button() {
	mp_int_t result = td_mouse_button();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_mouse_button_obj, mod_td_mouse_button);

STATIC mp_obj_t mod_rl_walk_line_start(size_t n_args, const mp_obj_t *args) {
	mp_int_t x1 = mp_obj_get_int(args[0]);
	mp_int_t y1 = mp_obj_get_int(args[1]);
	mp_int_t x2 = mp_obj_get_int(args[2]);
	mp_int_t y2 = mp_obj_get_int(args[3]);
	rl_walk_line_start(x1, y1, x2, y2);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_walk_line_start_obj, 4, 4, mod_rl_walk_line_start);

STATIC mp_obj_t mod_rl_walk_line_next() {
	int x, y;
	mp_int_t status = rl_walk_line_next(&x, &y);
	if(status) {
		mp_obj_tuple_t *result = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
		result->items[0] = mp_obj_new_int(x);
		result->items[1] = mp_obj_new_int(y);
		return result;
	} else {
		return mp_const_none;
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rl_walk_line_next_obj, mod_rl_walk_line_next);

STATIC const mp_rom_map_elem_t mp_module_rl_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rl) },
	/************* rogue_random *******************/
	{ MP_ROM_QSTR(MP_QSTR_random_next), MP_ROM_PTR(&mod_rl_random_next_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_int), MP_ROM_PTR(&mod_rl_random_int_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mod_rl_random_obj) },
	{ MP_ROM_QSTR(MP_QSTR_roll), MP_ROM_PTR(&mod_rl_roll_obj) },
	{ MP_ROM_QSTR(MP_QSTR_get_seed), MP_ROM_PTR(&mod_rl_get_seed_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_seed), MP_ROM_PTR(&mod_rl_set_seed_obj) },
	/************* rogue_filesystem ***************/
	{ MP_ROM_QSTR(MP_QSTR_open_resources), MP_ROM_PTR(&mod_fs_open_resources_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_app_name), MP_ROM_PTR(&mod_fs_set_app_name_obj) },
	{ MP_ROM_QSTR(MP_QSTR_load_asset), MP_ROM_PTR(&mod_fs_load_asset_obj) },
	{ MP_ROM_QSTR(MP_QSTR_load_pref), MP_ROM_PTR(&mod_fs_load_pref_obj) },
	{ MP_ROM_QSTR(MP_QSTR_save_pref), MP_ROM_PTR(&mod_fs_save_pref_obj) },
	/************* rogue_array ********************/
	{ MP_ROM_QSTR(MP_QSTR_array), MP_ROM_PTR(&mp_type_rl_array) },
	/************* rogue_display ******************/
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mod_td_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_load_font), MP_ROM_PTR(&mod_td_load_font_obj) },
	{ MP_ROM_QSTR(MP_QSTR_load_image), MP_ROM_PTR(&mod_td_load_image_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_image), MP_ROM_PTR(&mod_td_draw_image_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_tile), MP_ROM_PTR(&mod_td_draw_tile_obj) },
	{ MP_ROM_QSTR(MP_QSTR_colorize_tile), MP_ROM_PTR(&mod_td_colorize_tile_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_array), MP_ROM_PTR(&mod_td_draw_array_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_buffer), MP_ROM_PTR(&mod_td_set_buffer_obj) },
	{ MP_ROM_QSTR(MP_QSTR_print_text), MP_ROM_PTR(&mod_td_print_text_obj) },
	{ MP_ROM_QSTR(MP_QSTR_size_text), MP_ROM_PTR(&mod_td_size_text_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fill_rect), MP_ROM_PTR(&mod_td_fill_rect_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_rect), MP_ROM_PTR(&mod_td_draw_rect_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_points), MP_ROM_PTR(&mod_td_draw_points_obj) },
	{ MP_ROM_QSTR(MP_QSTR_still_running), MP_ROM_PTR(&mod_td_still_running_obj) },
	{ MP_ROM_QSTR(MP_QSTR_wait_key), MP_ROM_PTR(&mod_td_wait_key_obj) },
	{ MP_ROM_QSTR(MP_QSTR_wait_event), MP_ROM_PTR(&mod_td_wait_event_obj) },
	{ MP_ROM_QSTR(MP_QSTR_poll_event), MP_ROM_PTR(&mod_td_poll_event_obj) },
	{ MP_ROM_QSTR(MP_QSTR_present), MP_ROM_PTR(&mod_td_present_obj) },
	{ MP_ROM_QSTR(MP_QSTR_delay), MP_ROM_PTR(&mod_td_delay_obj) },
	{ MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&mod_td_clear_obj) },
	{ MP_ROM_QSTR(MP_QSTR_quit), MP_ROM_PTR(&mod_td_quit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_color), MP_ROM_PTR(&mod_td_color_obj) },
	{ MP_ROM_QSTR(MP_QSTR_hsv_color), MP_ROM_PTR(&mod_td_hsv_color_obj) },
	{ MP_ROM_QSTR(MP_QSTR_mouse_x), MP_ROM_PTR(&mod_td_mouse_x_obj) },
	{ MP_ROM_QSTR(MP_QSTR_mouse_y), MP_ROM_PTR(&mod_td_mouse_y_obj) },
	{ MP_ROM_QSTR(MP_QSTR_mouse_button), MP_ROM_PTR(&mod_td_mouse_button_obj) },
	/************** utils ******************/
	{ MP_ROM_QSTR(MP_QSTR_walk_line_start), MP_ROM_PTR(&mod_rl_walk_line_start_obj) },
	{ MP_ROM_QSTR(MP_QSTR_walk_line_next), MP_ROM_PTR(&mod_rl_walk_line_next_obj) },
	/*************** constants **************/
	// text alignment
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_LEFT), MP_ROM_INT(TD_ALIGN_LEFT) },
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_RIGHT), MP_ROM_INT(TD_ALIGN_RIGHT) },
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_CENTER), MP_ROM_INT(TD_ALIGN_CENTER) },

	// events
	{ MP_ROM_QSTR(MP_QSTR_QUIT), MP_ROM_INT(TD_QUIT) },
	{ MP_ROM_QSTR(MP_QSTR_MOUSE), MP_ROM_INT(TD_MOUSE) },
	{ MP_ROM_QSTR(MP_QSTR_REDRAW), MP_ROM_INT(TD_REDRAW) },
	{ MP_ROM_QSTR(MP_QSTR_ESCAPE), MP_ROM_INT(TD_ESCAPE) },
	{ MP_ROM_QSTR(MP_QSTR_LEFT), MP_ROM_INT(TD_LEFT) },
	{ MP_ROM_QSTR(MP_QSTR_RIGHT), MP_ROM_INT(TD_RIGHT) },
	{ MP_ROM_QSTR(MP_QSTR_UP), MP_ROM_INT(TD_UP) },
	{ MP_ROM_QSTR(MP_QSTR_DOWN), MP_ROM_INT(TD_DOWN) },

	// pico8 colors
  { MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_INT(td_color_rgba(0, 0, 0, 0)) },
  { MP_ROM_QSTR(MP_QSTR_DARKBLUE), MP_ROM_INT(td_color_rgba(29, 43, 83, 255)) },
  { MP_ROM_QSTR(MP_QSTR_DARKPURPLE), MP_ROM_INT(td_color_rgba(126, 37, 83, 255)) },
  { MP_ROM_QSTR(MP_QSTR_DARKGREEN), MP_ROM_INT(td_color_rgba(0, 135, 81, 255)) },
  { MP_ROM_QSTR(MP_QSTR_BROWN), MP_ROM_INT(td_color_rgba(171, 82, 54, 255)) },
  { MP_ROM_QSTR(MP_QSTR_DARKGRAY), MP_ROM_INT(td_color_rgba(95, 87, 79, 255)) },
  { MP_ROM_QSTR(MP_QSTR_LIGHTGRAY), MP_ROM_INT(td_color_rgba(194, 195, 199, 255)) },
  { MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_INT(td_color_rgba(255, 241, 232, 255)) },
  { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(td_color_rgba(255, 0, 77, 255)) },
  { MP_ROM_QSTR(MP_QSTR_ORANGE), MP_ROM_INT(td_color_rgba(255, 163, 0, 255)) },
  { MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_INT(td_color_rgba(255, 236, 39, 255)) },
  { MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(td_color_rgba(0, 228, 54, 255)) },
  { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(td_color_rgba(41, 173, 255, 255)) },
  { MP_ROM_QSTR(MP_QSTR_INDIGO), MP_ROM_INT(td_color_rgba(131, 118, 156, 255)) },
  { MP_ROM_QSTR(MP_QSTR_PINK), MP_ROM_INT(td_color_rgba(255, 119, 168, 255)) },
  { MP_ROM_QSTR(MP_QSTR_PEACH), MP_ROM_INT(td_color_rgba(255, 204, 170, 255)) },

};

STATIC MP_DEFINE_CONST_DICT(mp_module_rl_globals, mp_module_rl_globals_table);

const mp_obj_module_t mp_module_rl = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&mp_module_rl_globals,
};

#endif //PYROGUE_MOD_RL
