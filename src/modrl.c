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
#include <string.h>

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/parsenum.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/objint.h"
#include "py/objstr.h"

#if PYROGUE_MOD_RL

#include "rogue.h"

/************** rogue_random *********************/

STATIC mp_obj_t mod_rl_random_next() {
	mp_uint_t result = rl_random_next();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rl_random_next_obj, mod_rl_random_next);

STATIC mp_obj_t mod_rl_random_int(mp_obj_t a_in, mp_obj_t b_in) {
	mp_int_t a = mp_obj_get_int(a_in);
	mp_int_t b = mp_obj_get_int(b_in);
	mp_int_t result = rl_random_int(a, b);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_random_int_obj, mod_rl_random_int);

STATIC mp_obj_t mod_rl_random_3d(mp_obj_t x_in, mp_obj_t y_in, mp_obj_t z_in) {
	mp_int_t x = mp_obj_get_int(x_in);
	mp_int_t y = mp_obj_get_int(y_in);
	mp_int_t z = mp_obj_get_int(z_in);
	mp_int_t result = rl_random_3d(x, y, z);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_rl_random_3d_obj, mod_rl_random_3d);

STATIC mp_obj_t mod_rl_random_2d(mp_obj_t x_in, mp_obj_t y_in) {
	mp_int_t x = mp_obj_get_int(x_in);
	mp_int_t y = mp_obj_get_int(y_in);
	mp_int_t result = rl_random_2d(x, y);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_random_2d_obj, mod_rl_random_2d);

STATIC mp_obj_t mod_rl_random_1d(mp_obj_t x_in) {
	mp_int_t x = mp_obj_get_int(x_in);
	mp_int_t result = rl_random_1d(x);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_random_1d_obj, mod_rl_random_1d);

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

STATIC mp_obj_t mod_rl_set_seed(size_t n_args, const mp_obj_t *args) {
	mp_uint_t seed = 0;
	if(n_args > 0) mp_obj_get_int(args[0]);
	rl_set_seed(seed);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_set_seed_obj, 0, 1, mod_rl_set_seed);

STATIC mp_obj_t mod_rl_get_seed() {
	mp_uint_t result = rl_get_seed();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rl_get_seed_obj, mod_rl_get_seed);

// TODO: weighted choice
STATIC mp_obj_t mod_rl_random_choice(mp_obj_t list) {
	mp_int_t length = mp_obj_get_int(mp_obj_len(list));
	mp_int_t index = rl_random_int(0, length - 1);
	mp_obj_t item = mp_obj_subscr(list, mp_obj_new_int(index), MP_OBJ_SENTINEL); 
	return item;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_random_choice_obj, mod_rl_random_choice);

STATIC mp_obj_t mod_rl_shuffle(mp_obj_t list) {
	mp_int_t length = mp_obj_get_int(mp_obj_len(list));
	for(int i = 0; i < length; i++) {
		mp_int_t j = rl_random_int(0, length - 1);
		mp_obj_t obj_i = mp_obj_new_int(i);
		mp_obj_t obj_j = mp_obj_new_int(j);
		mp_obj_t a = mp_obj_subscr(list, obj_i, MP_OBJ_SENTINEL); 
		mp_obj_t b = mp_obj_subscr(list, obj_j, MP_OBJ_SENTINEL); 
		mp_obj_subscr(list, obj_j, b); 
		mp_obj_subscr(list, obj_i, a); 
	}
	return list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_shuffle_obj, mod_rl_shuffle);

/************** rogue_filesystem *********************/

/*STATIC mp_obj_t mod_fs_open_resources(mp_obj_t path_in) {
	size_t len;
	const char *path = mp_obj_str_get_data(path_in, &len);
	mp_int_t result = fs_open_resources(path);
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_fs_open_resources_obj, mod_fs_open_resources);*/

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
	unsigned char* data = (unsigned char*) fs_load_asset(path, &size);
	if(data == NULL) return mp_const_none;
	mp_obj_t result = mp_obj_new_bytes(data, size);
	free(data);
	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_fs_load_asset_obj, mod_fs_load_asset);

STATIC mp_obj_t mod_fs_load_pref(mp_obj_t path_in) {
	size_t len;
	const char *path = mp_obj_str_get_data(path_in, &len);
	uint32_t size;
	unsigned char* data = (unsigned char*) fs_load_pref(path, &size);
	if(data == NULL) return mp_const_none;
	mp_obj_t result = mp_obj_new_bytes(data, size);
	free(data);
	return result;
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

/************* font **************************/

extern const mp_obj_type_t mp_type_rl_font;

typedef struct {
	mp_obj_base_t base;
	font_t* font;
} mp_obj_rl_font_t;

STATIC mp_obj_t mod_rl_font_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	(void)type_in;
	mp_arg_check_num(n_args, n_kw, 2, 2, false);
	size_t len;
	const char *filename = mp_obj_str_get_data(args[0], &len);
	mp_float_t size = mp_obj_get_float(args[1]);
	mp_obj_rl_font_t* output = m_new_obj_with_finaliser(mp_obj_rl_font_t);
	output->base.type = &mp_type_rl_font;
	output->font = td_load_font(filename, size);
	return MP_OBJ_FROM_PTR(output);
}

STATIC mp_obj_t mod_rl_font_free(mp_obj_t self_in) {
	mp_obj_rl_font_t *self = MP_OBJ_TO_PTR(self_in);
	td_free_font(self->font);
	self->font = NULL;
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_font_free_obj, mod_rl_font_free);

// Implements load, store and delete attribute.
//
// dest[0] = MP_OBJ_NULL means load
//  return: for fail, do nothing
//          for attr, dest[0] = value
//          for method, dest[0] = method, dest[1] = self
//
// dest[0,1] = {MP_OBJ_SENTINEL, MP_OBJ_NULL} means delete
// dest[0,1] = {MP_OBJ_SENTINEL, object} means store
//  return: for fail, do nothing
//          for success set dest[0] = MP_OBJ_NULL
STATIC void mod_rl_font_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
	mp_obj_rl_font_t *self = MP_OBJ_TO_PTR(self_in);
	if(self->font == NULL) return;
	if (attr == MP_QSTR_size) {
		if(dest[0] == MP_OBJ_NULL) 
			dest[0] = mp_obj_new_float(self->font->size);
	} else if (attr == MP_QSTR_line_height) {
		if(dest[0] == MP_OBJ_NULL) 
			dest[0] = mp_obj_new_float(self->font->line_height);
	} else if(attr == MP_QSTR___del__) { // method
		if(dest[0] == MP_OBJ_NULL) {
			dest[0] = MP_OBJ_FROM_PTR(&mod_rl_font_free_obj);
			dest[1] = self_in;
		}
	}
}

const mp_obj_type_t mp_type_rl_font = {
	{ &mp_type_type },
	.name = MP_QSTR_Font,
	.make_new = mod_rl_font_make_new,
	.attr = mod_rl_font_attr,
};

/************* image **************************/

extern const mp_obj_type_t mp_type_rl_image;

typedef struct {
	mp_obj_base_t base;
	image_t* image;
} mp_obj_rl_image_t;

STATIC mp_obj_t mod_rl_image_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	(void)type_in;
	mp_arg_check_num(n_args, n_kw, 1, 3, false);
	size_t len;
	const char *filename = mp_obj_str_get_data(args[0], &len);
	mp_int_t tile_width = 8;
	if(n_args > 1) tile_width = mp_obj_get_int(args[1]);
	mp_int_t tile_height = 8;
	if(n_args > 2) tile_height = mp_obj_get_int(args[2]);
	mp_obj_rl_image_t* output = m_new_obj_with_finaliser(mp_obj_rl_image_t);
	output->base.type = &mp_type_rl_image;
	output->image = td_load_image(filename, tile_width, tile_height);
	return MP_OBJ_FROM_PTR(output);
}

STATIC mp_obj_t mod_rl_image_free(mp_obj_t self_in) {
	mp_obj_rl_image_t *self = MP_OBJ_TO_PTR(self_in);
	td_free_image(self->image);
	self->image = NULL;
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_image_free_obj, mod_rl_image_free);

// Implements load, store and delete attribute.
//
// dest[0] = MP_OBJ_NULL means load
//  return: for fail, do nothing
//          for attr, dest[0] = value
//          for method, dest[0] = method, dest[1] = self
//
// dest[0,1] = {MP_OBJ_SENTINEL, MP_OBJ_NULL} means delete
// dest[0,1] = {MP_OBJ_SENTINEL, object} means store
//  return: for fail, do nothing
//          for success set dest[0] = MP_OBJ_NULL
STATIC void mod_rl_image_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
	mp_obj_rl_image_t *self = MP_OBJ_TO_PTR(self_in);
	if(self->image == NULL) return;
	if (attr == MP_QSTR_width) {
		if(dest[0] == MP_OBJ_NULL) 
			dest[0] = mp_obj_new_int(self->image->width);
	} else if (attr == MP_QSTR_height) {
		if(dest[0] == MP_OBJ_NULL) 
			dest[0] = mp_obj_new_int(self->image->height);
	} else if (attr == MP_QSTR_tile_width) {
		if(dest[0] == MP_OBJ_NULL) 
			dest[0] = mp_obj_new_int(self->image->tile_width);
		else if(dest[0] == MP_OBJ_SENTINEL && dest[1] != MP_OBJ_NULL) {
			self->image->tile_width = mp_obj_get_int(dest[1]);
			self->image->tiles_per_line = self->image->width / self->image->tile_width;
			dest[0] = MP_OBJ_NULL;
		}
	} else if (attr == MP_QSTR_tile_height) {
		if(dest[0] == MP_OBJ_NULL) 
			dest[0] = mp_obj_new_int(self->image->tile_height);
		else if(dest[0] == MP_OBJ_SENTINEL && dest[1] != MP_OBJ_NULL) {
			self->image->tile_height = mp_obj_get_int(dest[1]);
			dest[0] = MP_OBJ_NULL;
		}
	} else if(attr == MP_QSTR___del__) { // method
		if(dest[0] == MP_OBJ_NULL) {
			dest[0] = MP_OBJ_FROM_PTR(&mod_rl_image_free_obj);
			dest[1] = self_in;
		}
	}
}


/*STATIC const mp_rom_map_elem_t mod_rl_image_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mod_rl_image_free_obj) },
};*/

//STATIC MP_DEFINE_CONST_DICT(mod_rl_image_locals_dict, mod_rl_image_locals_dict_table);

const mp_obj_type_t mp_type_rl_image = {
	{ &mp_type_type },
	.name = MP_QSTR_Image,
	.make_new = mod_rl_image_make_new,
	.attr = mod_rl_image_attr,
	//.locals_dict = (mp_obj_dict_t*)&mod_rl_image_locals_dict,
};

/************* rogue_array ********************/

extern const mp_obj_type_t mp_type_rl_array;

typedef struct {
	mp_obj_base_t base;
	array_t* array;
} mp_obj_rl_array_t;

STATIC mp_obj_t mod_rl_array_to_string(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	char* string = rl_array_to_string(self->array);
	mp_obj_t result = mp_obj_new_str_copy(&mp_type_str, (unsigned char*) string, strlen(string));
	free(string);
	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_to_string_obj, mod_rl_array_to_string);

STATIC mp_obj_t mod_rl_array_from_string(mp_obj_t string_in) {
	size_t len;
	const char *string = mp_obj_str_get_data(string_in, &len);
	array_t* array = rl_array_from_string(string);
	if(array == NULL) return mp_const_none;
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = array;
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_from_string_obj, mod_rl_array_from_string);

STATIC mp_obj_t mod_rl_array_from_list(mp_obj_t list) {
	mp_int_t height = mp_obj_get_int(mp_obj_len(list));
	if(height == 0)
		mp_raise_msg(&mp_type_IndexError, "len(list) == 0");
	mp_obj_t row = mp_obj_subscr(list, mp_obj_new_int(0), MP_OBJ_SENTINEL); 
	mp_int_t width = mp_obj_get_int(mp_obj_len(row));
	if(width == 0)
		mp_raise_msg(&mp_type_IndexError, "len(list[0]) == 0");

	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = rl_array_new(width, height);

	for(int j = 0; j < height; j++) {
		row = mp_obj_subscr(list, mp_obj_new_int(j), MP_OBJ_SENTINEL); 
		for(int i = 0; i < width; i++) {
			mp_obj_t item = mp_obj_subscr(row, mp_obj_new_int(i), MP_OBJ_SENTINEL); 
			rl_array_value(output->array, i, j) = mp_obj_get_int(item);
		}
	}

	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_from_list_obj, mod_rl_array_from_list);

/*STATIC mp_obj_t mod_rl_array_get(mp_obj_t self_in, mp_obj_t i_in, mp_obj_t j_in) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_set_obj, 4, 4, mod_rl_array_set);*/

/*STATIC mp_obj_t mod_rl_array_free(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	rl_array_free(self->array);
	fprintf(stderr, "__del__ called\n");
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_free_obj, mod_rl_array_free);*/

STATIC mp_obj_t mod_rl_array_equals(mp_obj_t self_in, mp_obj_t other_in) {
	mp_obj_rl_array_t* self = MP_OBJ_TO_PTR(self_in);
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	if(mp_obj_is_type(other_in, &mp_type_rl_array)) {
		mp_obj_rl_array_t* other = MP_OBJ_TO_PTR(other_in);
		output->array = rl_array_left_equal_other(self->array, other->array);
	} else {
		output->array = rl_array_left_equal_value(self->array, mp_obj_get_int(other_in));
	}
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_array_equals_obj, mod_rl_array_equals);

STATIC mp_obj_t mod_rl_array_not_equals(mp_obj_t self_in, mp_obj_t other_in) {
	mp_obj_rl_array_t* self = MP_OBJ_TO_PTR(self_in);
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	if(mp_obj_is_type(other_in, &mp_type_rl_array)) {
		mp_obj_rl_array_t* other = MP_OBJ_TO_PTR(other_in);
		output->array = rl_array_left_not_equal_other(self->array, other->array);
	} else {
		output->array = rl_array_left_not_equal_value(self->array, mp_obj_get_int(other_in));
	}
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_array_not_equals_obj, mod_rl_array_not_equals);

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
	if(!mp_obj_is_type(index_in, &mp_type_tuple))
		mp_raise_msg(&mp_type_TypeError, "subscript index should be a tuple");
	mp_obj_tuple_t* index = MP_OBJ_TO_PTR(index_in);
	if(index->len == 2) {
		if(mp_obj_is_type(index->items[0], &mp_type_slice) || mp_obj_is_type(index->items[1], &mp_type_slice)) {
			// support for making a view from slices
			mp_bound_slice_t i, j;
			if(mp_obj_is_type(index->items[0], &mp_type_slice))
				mp_seq_get_fast_slice_indexes(self->array->width, index->items[0], &i);
			else {
				mp_int_t start = mp_obj_get_int(index->items[0]);
				if(start < 0) start += self->array->width;
				i.start = start;
				i.stop = start + 1;
				i.step = 1;
			}
			if(mp_obj_is_type(index->items[1], &mp_type_slice))
				mp_seq_get_fast_slice_indexes(self->array->height, index->items[1], &j);
			else {
				mp_int_t start = mp_obj_get_int(index->items[1]);
				if(start < 0) start += self->array->width;
				j.start = start;
				j.stop = start + 1;
				j.step = 1;
			}
			if(i.step != 1 || j.step != 1)
				mp_raise_msg(&mp_type_IndexError, "only slices of step 1 supported");
			if(value_in == MP_OBJ_SENTINEL) { // get
				mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
				output->base.type = &mp_type_rl_array;
				output->array = rl_array_view(self->array, i.start, j.start, i.stop - i.start, j.stop - j.start);
				return MP_OBJ_FROM_PTR(output);
			} else { // set
				array_t* view = rl_array_view(self->array, i.start, j.start, i.stop - i.start, j.stop - j.start);
				if(mp_obj_is_type(value_in, &mp_type_rl_array)) {
					mp_obj_rl_array_t* other = MP_OBJ_TO_PTR(value_in);
					rl_array_copy_to(other->array, view, NULL);
					rl_array_free(view);
				} else {
					mp_int_t value = mp_obj_get_int(value_in);
					rl_array_fill(self->array, value);
				}
				return mp_const_none;
			}
		} else {
			// regular indexing with integers
			mp_int_t i = mp_obj_get_int(index->items[0]);
			mp_int_t j = mp_obj_get_int(index->items[1]);
			if(i < 0) i += self->array->width;
			if(j < 0) j += self->array->height;
			if(value_in == MP_OBJ_SENTINEL) { // get
				mp_int_t result = rl_array_get(self->array, i, j);
				return mp_obj_new_int(result);
			} else { // set
				mp_int_t value = mp_obj_get_int(value_in);
				rl_array_set(self->array, i, j, value);
				return mp_const_none;
			}
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

STATIC mp_obj_t mod_rl_array_random_2d(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t x = mp_obj_get_int(x_in);
	mp_int_t y = mp_obj_get_int(y_in);
	rl_array_random_2d(self->array, x, y);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_rl_array_random_2d_obj, mod_rl_array_random_2d);

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

STATIC mp_obj_t mod_rl_array_draw_line(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x1 = mp_obj_get_int(args[1]);
	mp_int_t y1 = mp_obj_get_int(args[2]);
	mp_int_t x2 = mp_obj_get_int(args[3]);
	mp_int_t y2 = mp_obj_get_int(args[4]);
	mp_int_t value = 1;
	if(n_args > 5) value = mp_obj_get_int(args[5]);
	rl_array_draw_line(self->array, x1, y1, x2, y2, value);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_draw_line_obj, 5, 6, mod_rl_array_draw_line);

STATIC mp_obj_t mod_rl_array_draw_rect(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x1 = mp_obj_get_int(args[1]);
	mp_int_t y1 = mp_obj_get_int(args[2]);
	mp_uint_t w = mp_obj_get_int(args[3]);
	mp_uint_t h = mp_obj_get_int(args[4]);
	mp_int_t value = 1;
	if(n_args > 5) value = mp_obj_get_int(args[5]);
	rl_array_draw_rect(self->array, x1, y1, w, h, value);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_draw_rect_obj, 5, 6, mod_rl_array_draw_rect);

STATIC mp_obj_t mod_rl_array_fill_rect(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x1 = mp_obj_get_int(args[1]);
	mp_int_t y1 = mp_obj_get_int(args[2]);
	mp_uint_t w = mp_obj_get_int(args[3]);
	mp_uint_t h = mp_obj_get_int(args[4]);
	mp_int_t value = 1;
	if(n_args > 5) value = mp_obj_get_int(args[5]);
	rl_array_fill_rect(self->array, x1, y1, w, h, value);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_fill_rect_obj, 5, 6, mod_rl_array_fill_rect);

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

STATIC mp_obj_t mod_rl_array_view(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(args[0], &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x = mp_obj_get_int(args[1]);
	mp_int_t y = mp_obj_get_int(args[2]);
	mp_int_t w = mp_obj_get_int(args[3]);
	mp_int_t h = mp_obj_get_int(args[4]);

	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = rl_array_view(self->array, x, y, w, h);
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_view_obj, 5, 5, mod_rl_array_view);

STATIC mp_obj_t mod_rl_array_copy(size_t n_args, const mp_obj_t *args) {
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	array_t* mask = NULL;
	if(n_args > 1) {
		if(!mp_obj_is_type(args[1], &mp_type_rl_array)) 
			mp_raise_msg(&mp_type_TypeError, "mask should be of type array");
		mp_obj_rl_array_t* obj_mask = MP_OBJ_TO_PTR(args[1]);
		mask = obj_mask->array;
	}
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = rl_array_copy(self->array, mask);
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_copy_obj, 1, 2, mod_rl_array_copy);

STATIC mp_obj_t mod_rl_array_copy_to(size_t n_args, const mp_obj_t *args) {
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	if(!mp_obj_is_type(args[1], &mp_type_rl_array)) 
		mp_raise_msg(&mp_type_TypeError, "dest should be of type array");
	mp_obj_rl_array_t *dest = MP_OBJ_TO_PTR(args[1]);
	array_t* mask = NULL;
	if(n_args > 2) {
		if(!mp_obj_is_type(args[2], &mp_type_rl_array)) 
			mp_raise_msg(&mp_type_TypeError, "mask should be of type array");
		mp_obj_rl_array_t* obj_mask = MP_OBJ_TO_PTR(args[2]);
		mask = obj_mask->array;
	}
	rl_array_copy_to(self->array, dest->array, mask);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_copy_to_obj, 2, 3, mod_rl_array_copy_to);

STATIC mp_obj_t mod_rl_array_cell_automaton(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(args[0], &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	size_t len;
	const char *definition = mp_obj_str_get_data(args[1], &len);
	mp_int_t wrap = 0;
	if(n_args > 2) wrap = mp_obj_get_int(args[2]);
  rl_array_cell_automaton(self->array, definition, wrap);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_cell_automaton_obj, 2, 3, mod_rl_array_cell_automaton);

STATIC mp_obj_t mod_rl_array_count(mp_obj_t self_in, mp_obj_t value_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t value = mp_obj_get_int(value_in);
	mp_int_t result = rl_array_count(self->array, value);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_array_count_obj, mod_rl_array_count);

STATIC mp_obj_t mod_rl_array_sum(mp_obj_t self_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	mp_int_t result = rl_array_sum(self->array);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_sum_obj, mod_rl_array_sum);

STATIC mp_obj_t mod_rl_array_apply_kernel(mp_obj_t self_in, mp_obj_t kernel_in) {
	mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	if(!mp_obj_is_type(kernel_in, &mp_type_rl_array))
		mp_raise_msg(&mp_type_TypeError, "expected array");
	mp_obj_rl_array_t *kernel = MP_OBJ_TO_PTR(kernel_in);

	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = rl_array_apply_kernel(self->array, kernel->array);
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rl_array_apply_kernel_obj, mod_rl_array_apply_kernel);

STATIC mp_obj_t mod_rl_array_flood_fill(size_t n_args, const mp_obj_t *args) {
	mp_check_self(mp_obj_is_type(args[0], &mp_type_rl_array));
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(args[0]);
	mp_int_t x = mp_obj_get_int(args[1]);
	mp_int_t y = mp_obj_get_int(args[2]);
	mp_int_t value = mp_obj_get_int(args[3]);
	mp_int_t use_diagonals = 0;
	if(n_args > 4) use_diagonals = mp_obj_is_true(args[4]);
	mp_int_t result = rl_array_flood_fill(self->array, x, y, value, use_diagonals);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_array_flood_fill_obj, 4, 5, mod_rl_array_flood_fill);

STATIC const mp_rom_map_elem_t mod_rl_array_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_to_string), MP_ROM_PTR(&mod_rl_array_to_string_obj) },
	{ MP_ROM_QSTR(MP_QSTR_view), MP_ROM_PTR(&mod_rl_array_view_obj) },
	/*{ MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&mod_rl_array_get_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&mod_rl_array_set_obj) },*/
	{ MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&mod_rl_array_width_obj) },
	{ MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&mod_rl_array_height_obj) },
	{ MP_ROM_QSTR(MP_QSTR_print_ascii), MP_ROM_PTR(&mod_rl_array_print_ascii_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&mod_rl_array_fill_obj) },
	{ MP_ROM_QSTR(MP_QSTR_replace), MP_ROM_PTR(&mod_rl_array_replace_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_int), MP_ROM_PTR(&mod_rl_array_random_int_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_2d), MP_ROM_PTR(&mod_rl_array_random_2d_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mod_rl_array_random_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_line), MP_ROM_PTR(&mod_rl_array_draw_line_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_rect), MP_ROM_PTR(&mod_rl_array_draw_rect_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fill_rect), MP_ROM_PTR(&mod_rl_array_fill_rect_obj) },
	{ MP_ROM_QSTR(MP_QSTR_can_see), MP_ROM_PTR(&mod_rl_array_can_see_obj) },
	{ MP_ROM_QSTR(MP_QSTR_field_of_view), MP_ROM_PTR(&mod_rl_array_field_of_view_obj) },
	{ MP_ROM_QSTR(MP_QSTR_dijkstra), MP_ROM_PTR(&mod_rl_array_dijkstra_obj) },
	{ MP_ROM_QSTR(MP_QSTR_cell_automaton), MP_ROM_PTR(&mod_rl_array_cell_automaton_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shortest_path), MP_ROM_PTR(&mod_rl_array_shortest_path_obj) },
	{ MP_ROM_QSTR(MP_QSTR_min), MP_ROM_PTR(&mod_rl_array_min_obj) },
	{ MP_ROM_QSTR(MP_QSTR_max), MP_ROM_PTR(&mod_rl_array_max_obj) },
	{ MP_ROM_QSTR(MP_QSTR_argmin), MP_ROM_PTR(&mod_rl_array_argmin_obj) },
	{ MP_ROM_QSTR(MP_QSTR_argmax), MP_ROM_PTR(&mod_rl_array_argmax_obj) },
	{ MP_ROM_QSTR(MP_QSTR_find_random), MP_ROM_PTR(&mod_rl_array_find_random_obj) },
	{ MP_ROM_QSTR(MP_QSTR_copy), MP_ROM_PTR(&mod_rl_array_copy_obj) },
	{ MP_ROM_QSTR(MP_QSTR_copy_to), MP_ROM_PTR(&mod_rl_array_copy_to_obj) },
	{ MP_ROM_QSTR(MP_QSTR_equals), MP_ROM_PTR(&mod_rl_array_equals_obj) },
	{ MP_ROM_QSTR(MP_QSTR_not_equals), MP_ROM_PTR(&mod_rl_array_not_equals_obj) },
	{ MP_ROM_QSTR(MP_QSTR_count), MP_ROM_PTR(&mod_rl_array_count_obj) },
	{ MP_ROM_QSTR(MP_QSTR_sum), MP_ROM_PTR(&mod_rl_array_sum_obj) },
	{ MP_ROM_QSTR(MP_QSTR_apply_kernel), MP_ROM_PTR(&mod_rl_array_apply_kernel_obj) },
	{ MP_ROM_QSTR(MP_QSTR_flood_fill), MP_ROM_PTR(&mod_rl_array_flood_fill_obj) },
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

STATIC mp_obj_t mod_rl_array_unary_op(mp_unary_op_t op, mp_obj_t self_in) {
	mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
	switch (op) {
		case MP_UNARY_OP_POSITIVE: {
			return self_in;
		}
		case MP_UNARY_OP_NEGATIVE: {
			mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
			output->base.type = &mp_type_rl_array;
			output->array = rl_array_unary_minus(self->array);
			return MP_OBJ_FROM_PTR(output);
		}
		case MP_UNARY_OP_INVERT: {
			mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
			output->base.type = &mp_type_rl_array;
			output->array = rl_array_unary_minus(self->array);
			return MP_OBJ_FROM_PTR(output);
		}
		case MP_UNARY_OP_BOOL:
			return mp_obj_new_bool(!rl_array_all_equal(self->array, 0));
		case MP_UNARY_OP_ABS: {
			mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
			output->base.type = &mp_type_rl_array;
			output->array = rl_array_abs(self->array);
			return MP_OBJ_FROM_PTR(output);
	  }
		default: return MP_OBJ_NULL; // op not supported
  }
}

#define mod_rl_array_op_side(MP_OP, side, name) \
        case MP_OP: { \
						mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t); \
						output->base.type = &mp_type_rl_array; \
						if(rhs_is_array) { \
							mp_obj_rl_array_t *rhs = MP_OBJ_TO_PTR(rhs_in); \
              output->array = rl_array_##side##_##name##_other(lhs->array, rhs->array); \
            } else { \
              output->array = rl_array_##side##_##name##_value(lhs->array, mp_obj_get_int(rhs_in)); \
						} \
						return MP_OBJ_FROM_PTR(output); \
        }
#define mod_rl_array_op(OP, name) \
	mod_rl_array_op_side(MP_BINARY_OP_##OP, right, name)

#define mod_rl_array_op_inplace(OP, name) \
	mod_rl_array_op_side(MP_BINARY_OP_##OP, right, name) \
	mod_rl_array_op_side(MP_BINARY_OP_REVERSE_##OP, left, name) \
	mod_rl_array_op_side(MP_BINARY_OP_INPLACE_##OP, right, name)

STATIC mp_obj_t mod_rl_array_binary_op(mp_binary_op_t op, mp_obj_t lhs_in, mp_obj_t rhs_in) {
    mp_obj_rl_array_t *lhs = MP_OBJ_TO_PTR(lhs_in);
    int rhs_is_array = mp_obj_is_type(rhs_in, &mp_type_rl_array);
		//printf("op: %d %p %p\n", op, lhs_in, rhs_in);
    switch (op) {
			// not supported due to bug in micropython
			/*mod_rl_array_op(EQUAL, equal);
			mod_rl_array_op(NOT_EQUAL, not_equal);*/
			mod_rl_array_op(MORE, greater_than);
			mod_rl_array_op(LESS, less_than);
			mod_rl_array_op(MORE_EQUAL, greater_or_equal_than);
			mod_rl_array_op(LESS_EQUAL, less_or_equal_than);
			mod_rl_array_op_inplace(AND, and);
			mod_rl_array_op_inplace(OR, or);
			mod_rl_array_op_inplace(XOR, xor);
			mod_rl_array_op_inplace(LSHIFT, lshift);
			mod_rl_array_op_inplace(RSHIFT, rshift);
			mod_rl_array_op_inplace(ADD, add);
			mod_rl_array_op_inplace(SUBTRACT, sub);
			mod_rl_array_op_inplace(MULTIPLY, mul);
			mod_rl_array_op_inplace(TRUE_DIVIDE, div);
			mod_rl_array_op_inplace(MODULO, mod);
			default:
				return MP_OBJ_NULL; // op not supported
    }
}

const mp_obj_type_t mp_type_rl_array = {
	{ &mp_type_type },
	.name = MP_QSTR_Array,
	.print = mod_rl_array_print,
	.make_new = mod_rl_array_make_new,
	.unary_op = mod_rl_array_unary_op,
	.binary_op = mod_rl_array_binary_op,
	.subscr = mod_rl_array_subscr,
	//.getiter = list_getiter,
	.locals_dict = (mp_obj_dict_t*)&mod_rl_array_locals_dict,
};

/************* rogue_display ******************/

STATIC mp_obj_t mod_td_init_display(mp_obj_t title_in, mp_obj_t width_in, mp_obj_t height_in) {
	size_t len;
	const char *title = mp_obj_str_get_data(title_in, &len);
	mp_int_t width = mp_obj_get_int(width_in);
	mp_int_t height = mp_obj_get_int(height_in);
	mp_int_t result = td_init_display(title, width, height);
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_td_init_display_obj, mod_td_init_display);

STATIC mp_obj_t mod_td_array_to_image(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_array, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
		{ MP_QSTR_tile_width, MP_ARG_INT, {.u_int = 0} },
		{ MP_QSTR_tile_height, MP_ARG_INT, {.u_int = 0} },
		{ MP_QSTR_palette, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
	};
	struct {
		mp_arg_val_t array, tile_width, tile_height, palette;
	} args;

	mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

	mp_check_self(mp_obj_is_type(args.array.u_obj, &mp_type_rl_array));
	mp_obj_rl_array_t *array = MP_OBJ_TO_PTR(args.array.u_obj);
	mp_int_t tile_width = args.tile_width.u_int;
	mp_int_t tile_height = args.tile_height.u_int;
	mp_int_t palette_size = 0;
	uint32_t* palette = NULL;
	if(args.palette.u_obj != mp_const_none) {
		palette_size = mp_obj_get_int(mp_obj_len(args.palette.u_obj));
		palette = malloc(palette_size * sizeof(uint32_t));
		mp_obj_t iterable = mp_getiter(args.palette.u_obj, NULL);
		mp_obj_t item;
		int i = 0;
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			palette[i++] = mp_obj_get_int(item);
		}
	}

	mp_obj_rl_image_t* output = m_new_obj_with_finaliser(mp_obj_rl_image_t);
	output->base.type = &mp_type_rl_image;
	output->image = td_array_to_image(array->array, tile_width, tile_height, palette_size, palette);
	if(palette != NULL) free(palette);
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_td_array_to_image_obj, 1, mod_td_array_to_image);

STATIC mp_obj_t mod_td_image_to_array(mp_obj_t image_in) {
	mp_obj_rl_image_t* image = MP_OBJ_TO_PTR(image_in);
	if(!mp_obj_is_type(image, &mp_type_rl_image) || image->image == NULL)
		mp_raise_msg(&mp_type_TypeError, "invalid image");
	mp_obj_rl_array_t* output = m_new_obj(mp_obj_rl_array_t);
	output->base.type = &mp_type_rl_array;
	output->array = td_image_to_array(image->image);
	return MP_OBJ_FROM_PTR(output);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_image_to_array_obj, mod_td_image_to_array);

STATIC mp_obj_t mod_td_draw_image(mp_obj_t image_in, mp_obj_t x_in, mp_obj_t y_in) {
	mp_obj_rl_image_t *image = MP_OBJ_TO_PTR(image_in);
	if(!mp_obj_is_type(image, &mp_type_rl_image) || image->image == NULL)
		mp_raise_msg(&mp_type_TypeError, "invalid image");
	mp_int_t x = mp_obj_get_int(x_in);
	mp_int_t y = mp_obj_get_int(y_in);
	td_draw_image(image->image, x, y);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_td_draw_image_obj, mod_td_draw_image);

/*STATIC mp_obj_t mod_td_draw_tile(size_t n_args, const mp_obj_t *args) {
	mp_obj_rl_image_t *image = MP_OBJ_TO_PTR(args[0]);
	if(!mp_obj_is_type(image, &mp_type_rl_image) || image->image == NULL)
		mp_raise_msg(&mp_type_TypeError, "invalid image");
	mp_int_t x = mp_obj_get_int(args[1]);
	mp_int_t y = mp_obj_get_int(args[2]);
	mp_int_t tile = mp_obj_get_int(args[3]);
	td_draw_tile(image->image, x, y, tile);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_draw_tile_obj, 4, 4, mod_td_draw_tile);*/

STATIC mp_obj_t mod_td_draw_tile(size_t n_args, const mp_obj_t *args) {
	mp_obj_rl_image_t *image = MP_OBJ_TO_PTR(args[0]);
	if(!mp_obj_is_type(image, &mp_type_rl_image) || image->image == NULL)
		mp_raise_msg(&mp_type_TypeError, "invalid image");
	mp_int_t x = mp_obj_get_int(args[1]);
	mp_int_t y = mp_obj_get_int(args[2]);
	mp_int_t tile = mp_obj_get_int(args[3]);
	mp_uint_t fg = 0;
	if(n_args > 4) fg = mp_obj_get_int(args[4]);
	mp_uint_t bg = 0;
	if(n_args > 5) bg = mp_obj_get_int(args[5]);
	td_colorize_tile(image->image, x, y, tile, fg, bg);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_draw_tile_obj, 4, 6, mod_td_draw_tile);

mp_obj_t mod_td_draw_array(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_array, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
		{ MP_QSTR_x, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },
		{ MP_QSTR_y, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },
		/*{ MP_QSTR_x_shift, MP_ARG_INT, {.u_int = 0} },
		{ MP_QSTR_y_shift, MP_ARG_INT, {.u_int = 0} },*/
		{ MP_QSTR_image, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL} },
		{ MP_QSTR_tile_map, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
		{ MP_QSTR_fg_palette, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
		{ MP_QSTR_bg_palette, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
		{ MP_QSTR_packed, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_false_obj)} },
		/*{ MP_QSTR_tile_mask, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
		{ MP_QSTR_fg_mask, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
		{ MP_QSTR_bg_mask, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },*/
	};

	// parse args
	struct {
		mp_arg_val_t array, x, y, /*x_shift, y_shift,*/ image, tile_map, fg_palette, bg_palette, packed/*, tile_mask, fg_mask, bg_mask*/;
	} args;

	mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

	mp_check_self(mp_obj_is_type(args.array.u_obj, &mp_type_rl_array));
	mp_obj_rl_array_t *array = MP_OBJ_TO_PTR(args.array.u_obj);
	if(!mp_obj_is_type(array, &mp_type_rl_array))
		mp_raise_msg(&mp_type_TypeError, "invalid array");
	mp_obj_rl_image_t *image = MP_OBJ_TO_PTR(args.image.u_obj);
	if(image != NULL && (!mp_obj_is_type(image, &mp_type_rl_image) || image->image == NULL))
		mp_raise_msg(&mp_type_TypeError, "invalid image");
	mp_int_t x = args.x.u_int;
	mp_int_t y = args.y.u_int;

	int* tile_map = NULL;
	int tile_map_size = 0;
	if(args.tile_map.u_obj != mp_const_none) {
		tile_map_size = mp_obj_get_int(mp_obj_len(args.tile_map.u_obj));
		tile_map = malloc(tile_map_size * sizeof(int32_t));
		mp_obj_t iterable = mp_getiter(args.tile_map.u_obj, NULL);
		mp_obj_t item;
		int i = 0;
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			tile_map[i++] = mp_obj_get_int(item);
		}
	}

	uint32_t* fg_palette = NULL;
	int fg_palette_size = 0;
	if(args.fg_palette.u_obj != mp_const_none) {
		fg_palette_size = mp_obj_get_int(mp_obj_len(args.fg_palette.u_obj));
		fg_palette = malloc(fg_palette_size * sizeof(int32_t));
		mp_obj_t iterable = mp_getiter(args.fg_palette.u_obj, NULL);
		mp_obj_t item;
		int i = 0;
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			fg_palette[i++] = mp_obj_get_int(item);
		}
	}

	uint32_t* bg_palette = NULL;
	int bg_palette_size = 0;
	if(args.bg_palette.u_obj != mp_const_none) {
		bg_palette_size = mp_obj_get_int(mp_obj_len(args.bg_palette.u_obj));
		bg_palette = malloc(bg_palette_size * sizeof(int32_t));
		mp_obj_t iterable = mp_getiter(args.bg_palette.u_obj, NULL);
		mp_obj_t item;
		int i = 0;
		while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
			bg_palette[i++] = mp_obj_get_int(item);
		}
	}

  uint32_t tile_mask, fg_mask, bg_mask;
  if(mp_obj_is_true(args.packed.u_obj)) {
    tile_mask = 0x0000ffff;
    fg_mask =   0x00ff0000;
    bg_mask =   0xff000000;
  } else {
    tile_mask = fg_mask = bg_mask = 0xffffffff;
  }

  /*if(args.tile_mask.u_obj != mp_const_none) tile_mask = mp_obj_get_int(args.tile_mask.u_obj);
  if(args.fg_mask.u_obj != mp_const_none) fg_mask = mp_obj_get_int(args.fg_mask.u_obj);
  if(args.bg_mask.u_obj != mp_const_none) bg_mask = mp_obj_get_int(args.bg_mask.u_obj);*/

	td_draw_array(array->array, x, y, /*args.x_shift.u_int, args.y_shift.u_int, */
      0, 0, image->image, 
      tile_map, tile_map_size, tile_mask, 
      fg_palette, fg_palette_size, fg_mask, 
      bg_palette, bg_palette_size, bg_mask);

	if(tile_map != NULL) free(tile_map);
	if(fg_palette != NULL) free(fg_palette);
	if(bg_palette != NULL) free(bg_palette);

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_td_draw_array_obj, 3, mod_td_draw_array);

mp_obj_t mod_td_draw_text(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_font, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
		{ MP_QSTR_x, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },
		{ MP_QSTR_y, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = 0} },
		{ MP_QSTR_text, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL} },
		{ MP_QSTR_color, MP_ARG_INT, {.u_int = 0xffffffff} },
		{ MP_QSTR_align, MP_ARG_INT, {.u_int = TD_ALIGN_LEFT} },
		{ MP_QSTR_line_height, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
	};

	// parse args
	struct {
		mp_arg_val_t font, x, y, text, color, align, line_height;
	} args;

	mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

	mp_int_t x = args.x.u_int;
	mp_int_t y = args.y.u_int;
	size_t len;
	const char *text = mp_obj_str_get_data(args.text.u_obj, &len);
	mp_obj_rl_font_t *font = NULL;
	mp_obj_rl_image_t *image = NULL;
	if(mp_obj_is_type(args.font.u_obj, &mp_type_rl_font)) {
		font = MP_OBJ_TO_PTR(args.font.u_obj);
	} else if(mp_obj_is_type(args.font.u_obj, &mp_type_rl_image)) {
		image = MP_OBJ_TO_PTR(args.font.u_obj);
	} else 
		mp_raise_msg(&mp_type_TypeError, "invalid font");

	mp_float_t line_height = 0;
	if(args.line_height.u_obj != NULL) line_height = mp_obj_get_float(args.line_height.u_obj);

	if(font != NULL)
		td_draw_text(font->font, x, y, text, args.color.u_int, args.align.u_int, line_height);
	else if(image != NULL)
		td_draw_text_from_tiles(image->image, x, y, text, args.color.u_int, args.align.u_int);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_td_draw_text_obj, 3, mod_td_draw_text);

STATIC mp_obj_t mod_td_size_text(mp_obj_t font_in, mp_obj_t text_in) {
	int width, height;
	mp_obj_rl_font_t *font = MP_OBJ_TO_PTR(font_in);
	if(!mp_obj_is_type(font, &mp_type_rl_font))
		mp_raise_msg(&mp_type_TypeError, "invalid font");
	size_t len;
	const char *text = mp_obj_str_get_data(text_in, &len);
	td_size_text(font->font, text, &width, &height);
	mp_obj_tuple_t *result = MP_OBJ_TO_PTR(mp_obj_new_tuple(2, NULL));
	result->items[0] = mp_obj_new_int(width);
	result->items[1] = mp_obj_new_int(height);
	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_td_size_text_obj, mod_td_size_text);

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

STATIC mp_obj_t mod_td_draw_line(size_t n_args, const mp_obj_t *args) {
	mp_int_t x1 = mp_obj_get_int(args[0]);
	mp_int_t y1 = mp_obj_get_int(args[1]);
	mp_int_t x2 = mp_obj_get_int(args[2]);
	mp_int_t y2 = mp_obj_get_int(args[3]);
	mp_uint_t color = 0xffffffff;
	if(n_args > 4) color = mp_obj_get_int(args[4]);
	td_draw_line(x1, y1, x2, y2, color);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_draw_line_obj, 4, 5, mod_td_draw_line);

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

STATIC mp_obj_t mod_td_random_color() {
	uint32_t result = td_random_color();
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_random_color_obj, mod_td_random_color);

STATIC mp_obj_t mod_td_color(size_t n_args, const mp_obj_t *args) {
	if(n_args == 1) {
		size_t len;
		const char *color = mp_obj_str_get_data(args[0], &len);
		uint32_t result = td_hex_color(color);
		return mp_obj_new_int(result);
	} else {
		unsigned char r = (unsigned char) mp_obj_get_int(args[0]);
		unsigned char g = (unsigned char) mp_obj_get_int(args[1]);
		unsigned char b = (unsigned char) mp_obj_get_int(args[2]);
		unsigned char a = 0xff;
		if(n_args > 3) a = (unsigned char) mp_obj_get_int(args[3]);
		uint32_t result = td_color_rgba(r, g, b, a);
		return mp_obj_new_int(result);
	}
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_color_obj, 1, 4, mod_td_color);

STATIC mp_obj_t mod_td_hsv_color(size_t n_args, const mp_obj_t *args) {
	unsigned char h = (unsigned char) mp_obj_get_int(args[0]);
	unsigned char s = (unsigned char) mp_obj_get_int(args[1]);
	unsigned char v = (unsigned char) mp_obj_get_int(args[2]);
	unsigned char a = 0xff;
	if(n_args > 3) a = (unsigned char) mp_obj_get_int(args[3]);
	uint32_t result = td_hsv_color(h, s, v, a);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_hsv_color_obj, 3, 4, mod_td_hsv_color);

STATIC mp_obj_t mod_td_color_components(mp_obj_t color_in) {
	uint32_t color = (uint32_t) mp_obj_get_int(color_in);
	mp_obj_tuple_t *result = MP_OBJ_TO_PTR(mp_obj_new_tuple(4, NULL));
	result->items[0] = mp_obj_new_int(td_color_r(color));
	result->items[1] = mp_obj_new_int(td_color_g(color));
	result->items[2] = mp_obj_new_int(td_color_b(color));
	result->items[3] = mp_obj_new_int(td_color_a(color));
	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_color_components_obj, mod_td_color_components);

/*STATIC mp_obj_t mod_td_color_r(mp_obj_t color_in) {
	uint32_t color = (uint32_t) mp_obj_get_int(color_in);
	uint8_t result = td_color_r(color);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_color_r_obj, mod_td_color_r);

STATIC mp_obj_t mod_td_color_g(mp_obj_t color_in) {
	uint32_t color = (uint32_t) mp_obj_get_int(color_in);
	uint8_t result = td_color_g(color);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_color_g_obj, mod_td_color_g);

STATIC mp_obj_t mod_td_color_b(mp_obj_t color_in) {
	uint32_t color = (uint32_t) mp_obj_get_int(color_in);
	uint8_t result = td_color_b(color);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_color_b_obj, mod_td_color_b);

STATIC mp_obj_t mod_td_color_a(mp_obj_t color_in) {
	uint32_t color = (uint32_t) mp_obj_get_int(color_in);
	uint8_t result = td_color_a(color);
	return mp_obj_new_int(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_td_color_a_obj, mod_td_color_a);*/

STATIC mp_obj_t mod_rl_pack_tile(mp_obj_t tile_in, mp_obj_t fg_in, mp_obj_t bg_in) {
	uint32_t tile = (uint32_t) mp_obj_get_int(tile_in);
	uint32_t fg = (uint32_t) mp_obj_get_int(fg_in);
	uint32_t bg = (uint32_t) mp_obj_get_int(bg_in);
	return mp_obj_new_int((tile & 0xffff) | ((fg & 0xff) << 16) | ((bg & 0xff) << 24));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_rl_pack_tile_obj, mod_rl_pack_tile);

STATIC mp_obj_t mod_rl_unpack_tile(mp_obj_t value_in) {
  uint32_t value = (uint32_t) mp_obj_get_int(value_in);
	mp_obj_tuple_t *result = MP_OBJ_TO_PTR(mp_obj_new_tuple(3, NULL));
	result->items[0] = mp_obj_new_int(value & 0xffff);
	result->items[1] = mp_obj_new_int((value >> 16) & 0xff);
	result->items[2] = mp_obj_new_int((value >> 24) & 0xff);
	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_unpack_tile_obj, mod_rl_unpack_tile);

STATIC mp_obj_t mod_td_mouse() {
	mp_int_t x = td_mouse_x();
	mp_int_t y = td_mouse_y();
	mp_int_t button = td_mouse_button();
	mp_obj_tuple_t *result = MP_OBJ_TO_PTR(mp_obj_new_tuple(3, NULL));
	result->items[0] = mp_obj_new_int(x);
	result->items[1] = mp_obj_new_int(y);
	result->items[2] = mp_obj_new_int(button);
	return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_mouse_obj, mod_td_mouse);

STATIC mp_obj_t mod_td_key() {
  return mp_obj_new_int(td_key());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_key_obj, mod_td_key);

STATIC mp_obj_t mod_td_shift_pressed() {
	mp_int_t result = td_shift_pressed();
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_shift_pressed_obj, mod_td_shift_pressed);

STATIC mp_obj_t mod_td_alt_pressed() {
	mp_int_t result = td_alt_pressed();
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_alt_pressed_obj, mod_td_alt_pressed);

STATIC mp_obj_t mod_td_ctrl_pressed() {
	mp_int_t result = td_ctrl_pressed();
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_ctrl_pressed_obj, mod_td_ctrl_pressed);

STATIC mp_obj_t mod_td_win_pressed() {
	mp_int_t result = td_win_pressed();
	return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_td_win_pressed_obj, mod_td_win_pressed);

static mp_obj_t event_callback;
static void run_callback(int key) {
	mp_call_function_1(event_callback, mp_obj_new_int(key));
}

STATIC mp_obj_t mod_td_run(size_t n_args, const mp_obj_t *args) {
	event_callback = args[0];
	mp_int_t update_filter = TD_ON_EVENT;
	if(n_args > 1) update_filter = mp_obj_get_int(args[1]);
	td_run(run_callback, update_filter);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_td_run_obj, 1, 2, mod_td_run);

/* utils */

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

STATIC mp_obj_t mod_rl_distance(size_t n_args, const mp_obj_t *args) {
	mp_float_t x1 = mp_obj_get_float(args[0]);
	mp_float_t y1 = mp_obj_get_float(args[1]);
	mp_float_t x2 = mp_obj_get_float(args[2]);
	mp_float_t y2 = mp_obj_get_float(args[3]);
	mp_float_t result = rl_distance(x1, y1, x2, y2);
	return mp_obj_new_float(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_rl_distance_obj, 4, 4, mod_rl_distance);

// WARNING: declaring const long intenger requires to use of MICROPY_LONGINT_IMPL_LONGLONG

const mp_obj_int_t mod_rl_INT_MAX = {{&mp_type_int}, INT_MAX};
const mp_obj_int_t mod_rl_INT_MIN = {{&mp_type_int}, INT_MIN};

// pico8 colors
const mp_obj_int_t mod_rl_BLACK = {{&mp_type_int}, (td_color_rgba(0, 0, 0, 255)) };
const mp_obj_int_t mod_rl_DARKBLUE = {{&mp_type_int}, (td_color_rgba(29, 43, 83, 255)) };
const mp_obj_int_t mod_rl_DARKPURPLE = {{&mp_type_int}, (td_color_rgba(126, 37, 83, 255)) };
const mp_obj_int_t mod_rl_DARKGREEN = {{&mp_type_int}, (td_color_rgba(0, 135, 81, 255)) };
const mp_obj_int_t mod_rl_BROWN = {{&mp_type_int}, (td_color_rgba(171, 82, 54, 255)) };
const mp_obj_int_t mod_rl_DARKGRAY = {{&mp_type_int}, (td_color_rgba(95, 87, 79, 255)) };
const mp_obj_int_t mod_rl_LIGHTGRAY = {{&mp_type_int}, (td_color_rgba(194, 195, 199, 255)) };
const mp_obj_int_t mod_rl_WHITE = {{&mp_type_int}, (td_color_rgba(255, 241, 232, 255)) };
const mp_obj_int_t mod_rl_RED = {{&mp_type_int}, (td_color_rgba(255, 0, 77, 255)) };
const mp_obj_int_t mod_rl_ORANGE = {{&mp_type_int}, (td_color_rgba(255, 163, 0, 255)) };
const mp_obj_int_t mod_rl_YELLOW = {{&mp_type_int}, (td_color_rgba(255, 236, 39, 255)) };
const mp_obj_int_t mod_rl_GREEN = {{&mp_type_int}, (td_color_rgba(0, 228, 54, 255)) };
const mp_obj_int_t mod_rl_BLUE = {{&mp_type_int}, (td_color_rgba(41, 173, 255, 255)) };
const mp_obj_int_t mod_rl_INDIGO = {{&mp_type_int}, (td_color_rgba(131, 118, 156, 255)) };
const mp_obj_int_t mod_rl_PINK = {{&mp_type_int}, (td_color_rgba(255, 119, 168, 255)) };
const mp_obj_int_t mod_rl_PEACH = {{&mp_type_int}, (td_color_rgba(255, 204, 170, 255)) };

// keys
const mp_obj_int_t mod_rl_TD_0 = {{&mp_type_int}, (TD_0)};
const mp_obj_int_t mod_rl_TD_1 = {{&mp_type_int}, (TD_1)};
const mp_obj_int_t mod_rl_TD_2 = {{&mp_type_int}, (TD_2)};
const mp_obj_int_t mod_rl_TD_3 = {{&mp_type_int}, (TD_3)};
const mp_obj_int_t mod_rl_TD_4 = {{&mp_type_int}, (TD_4)};
const mp_obj_int_t mod_rl_TD_5 = {{&mp_type_int}, (TD_5)};
const mp_obj_int_t mod_rl_TD_6 = {{&mp_type_int}, (TD_6)};
const mp_obj_int_t mod_rl_TD_7 = {{&mp_type_int}, (TD_7)};
const mp_obj_int_t mod_rl_TD_8 = {{&mp_type_int}, (TD_8)};
const mp_obj_int_t mod_rl_TD_9 = {{&mp_type_int}, (TD_9)};
const mp_obj_int_t mod_rl_TD_AC_BACK = {{&mp_type_int}, (TD_AC_BACK)};
const mp_obj_int_t mod_rl_TD_AC_BOOKMARKS = {{&mp_type_int}, (TD_AC_BOOKMARKS)};
const mp_obj_int_t mod_rl_TD_AC_FORWARD = {{&mp_type_int}, (TD_AC_FORWARD)};
const mp_obj_int_t mod_rl_TD_AC_HOME = {{&mp_type_int}, (TD_AC_HOME)};
const mp_obj_int_t mod_rl_TD_AC_REFRESH = {{&mp_type_int}, (TD_AC_REFRESH)};
const mp_obj_int_t mod_rl_TD_AC_SEARCH = {{&mp_type_int}, (TD_AC_SEARCH)};
const mp_obj_int_t mod_rl_TD_AC_STOP = {{&mp_type_int}, (TD_AC_STOP)};
const mp_obj_int_t mod_rl_TD_AGAIN = {{&mp_type_int}, (TD_AGAIN)};
const mp_obj_int_t mod_rl_TD_ALTERASE = {{&mp_type_int}, (TD_ALTERASE)};
const mp_obj_int_t mod_rl_TD_AMPERSAND = {{&mp_type_int}, (TD_AMPERSAND)};
const mp_obj_int_t mod_rl_TD_APP1 = {{&mp_type_int}, (TD_APP1)};
const mp_obj_int_t mod_rl_TD_APP2 = {{&mp_type_int}, (TD_APP2)};
const mp_obj_int_t mod_rl_TD_APPLICATION = {{&mp_type_int}, (TD_APPLICATION)};
const mp_obj_int_t mod_rl_TD_ASTERISK = {{&mp_type_int}, (TD_ASTERISK)};
const mp_obj_int_t mod_rl_TD_AT = {{&mp_type_int}, (TD_AT)};
const mp_obj_int_t mod_rl_TD_AUDIOFASTFORWARD = {{&mp_type_int}, (TD_AUDIOFASTFORWARD)};
const mp_obj_int_t mod_rl_TD_AUDIOMUTE = {{&mp_type_int}, (TD_AUDIOMUTE)};
const mp_obj_int_t mod_rl_TD_AUDIONEXT = {{&mp_type_int}, (TD_AUDIONEXT)};
const mp_obj_int_t mod_rl_TD_AUDIOPLAY = {{&mp_type_int}, (TD_AUDIOPLAY)};
const mp_obj_int_t mod_rl_TD_AUDIOPREV = {{&mp_type_int}, (TD_AUDIOPREV)};
const mp_obj_int_t mod_rl_TD_AUDIOREWIND = {{&mp_type_int}, (TD_AUDIOREWIND)};
const mp_obj_int_t mod_rl_TD_AUDIOSTOP = {{&mp_type_int}, (TD_AUDIOSTOP)};
const mp_obj_int_t mod_rl_TD_BACKQUOTE = {{&mp_type_int}, (TD_BACKQUOTE)};
const mp_obj_int_t mod_rl_TD_BACKSLASH = {{&mp_type_int}, (TD_BACKSLASH)};
const mp_obj_int_t mod_rl_TD_BACKSPACE = {{&mp_type_int}, (TD_BACKSPACE)};
const mp_obj_int_t mod_rl_TD_BRIGHTNESSDOWN = {{&mp_type_int}, (TD_BRIGHTNESSDOWN)};
const mp_obj_int_t mod_rl_TD_BRIGHTNESSUP = {{&mp_type_int}, (TD_BRIGHTNESSUP)};
const mp_obj_int_t mod_rl_TD_CALCULATOR = {{&mp_type_int}, (TD_CALCULATOR)};
const mp_obj_int_t mod_rl_TD_CANCEL = {{&mp_type_int}, (TD_CANCEL)};
const mp_obj_int_t mod_rl_TD_CAPSLOCK = {{&mp_type_int}, (TD_CAPSLOCK)};
const mp_obj_int_t mod_rl_TD_CARET = {{&mp_type_int}, (TD_CARET)};
const mp_obj_int_t mod_rl_TD_CLEAR = {{&mp_type_int}, (TD_CLEAR)};
const mp_obj_int_t mod_rl_TD_CLEARAGAIN = {{&mp_type_int}, (TD_CLEARAGAIN)};
const mp_obj_int_t mod_rl_TD_COLON = {{&mp_type_int}, (TD_COLON)};
const mp_obj_int_t mod_rl_TD_COMMA = {{&mp_type_int}, (TD_COMMA)};
const mp_obj_int_t mod_rl_TD_COMPUTER = {{&mp_type_int}, (TD_COMPUTER)};
const mp_obj_int_t mod_rl_TD_COPY = {{&mp_type_int}, (TD_COPY)};
const mp_obj_int_t mod_rl_TD_CRSEL = {{&mp_type_int}, (TD_CRSEL)};
const mp_obj_int_t mod_rl_TD_CURRENCYSUBUNIT = {{&mp_type_int}, (TD_CURRENCYSUBUNIT)};
const mp_obj_int_t mod_rl_TD_CURRENCYUNIT = {{&mp_type_int}, (TD_CURRENCYUNIT)};
const mp_obj_int_t mod_rl_TD_CUT = {{&mp_type_int}, (TD_CUT)};
const mp_obj_int_t mod_rl_TD_DECIMALSEPARATOR = {{&mp_type_int}, (TD_DECIMALSEPARATOR)};
const mp_obj_int_t mod_rl_TD_DELETE = {{&mp_type_int}, (TD_DELETE)};
const mp_obj_int_t mod_rl_TD_DISPLAYSWITCH = {{&mp_type_int}, (TD_DISPLAYSWITCH)};
const mp_obj_int_t mod_rl_TD_DOLLAR = {{&mp_type_int}, (TD_DOLLAR)};
const mp_obj_int_t mod_rl_TD_DOWN = {{&mp_type_int}, (TD_DOWN)};
const mp_obj_int_t mod_rl_TD_EJECT = {{&mp_type_int}, (TD_EJECT)};
const mp_obj_int_t mod_rl_TD_END = {{&mp_type_int}, (TD_END)};
const mp_obj_int_t mod_rl_TD_EQUALS = {{&mp_type_int}, (TD_EQUALS)};
const mp_obj_int_t mod_rl_TD_ESCAPE = {{&mp_type_int}, (TD_ESCAPE)};
const mp_obj_int_t mod_rl_TD_EXCLAIM = {{&mp_type_int}, (TD_EXCLAIM)};
const mp_obj_int_t mod_rl_TD_EXECUTE = {{&mp_type_int}, (TD_EXECUTE)};
const mp_obj_int_t mod_rl_TD_EXSEL = {{&mp_type_int}, (TD_EXSEL)};
const mp_obj_int_t mod_rl_TD_F1 = {{&mp_type_int}, (TD_F1)};
const mp_obj_int_t mod_rl_TD_F10 = {{&mp_type_int}, (TD_F10)};
const mp_obj_int_t mod_rl_TD_F11 = {{&mp_type_int}, (TD_F11)};
const mp_obj_int_t mod_rl_TD_F12 = {{&mp_type_int}, (TD_F12)};
const mp_obj_int_t mod_rl_TD_F13 = {{&mp_type_int}, (TD_F13)};
const mp_obj_int_t mod_rl_TD_F14 = {{&mp_type_int}, (TD_F14)};
const mp_obj_int_t mod_rl_TD_F15 = {{&mp_type_int}, (TD_F15)};
const mp_obj_int_t mod_rl_TD_F16 = {{&mp_type_int}, (TD_F16)};
const mp_obj_int_t mod_rl_TD_F17 = {{&mp_type_int}, (TD_F17)};
const mp_obj_int_t mod_rl_TD_F18 = {{&mp_type_int}, (TD_F18)};
const mp_obj_int_t mod_rl_TD_F19 = {{&mp_type_int}, (TD_F19)};
const mp_obj_int_t mod_rl_TD_F2 = {{&mp_type_int}, (TD_F2)};
const mp_obj_int_t mod_rl_TD_F20 = {{&mp_type_int}, (TD_F20)};
const mp_obj_int_t mod_rl_TD_F21 = {{&mp_type_int}, (TD_F21)};
const mp_obj_int_t mod_rl_TD_F22 = {{&mp_type_int}, (TD_F22)};
const mp_obj_int_t mod_rl_TD_F23 = {{&mp_type_int}, (TD_F23)};
const mp_obj_int_t mod_rl_TD_F24 = {{&mp_type_int}, (TD_F24)};
const mp_obj_int_t mod_rl_TD_F3 = {{&mp_type_int}, (TD_F3)};
const mp_obj_int_t mod_rl_TD_F4 = {{&mp_type_int}, (TD_F4)};
const mp_obj_int_t mod_rl_TD_F5 = {{&mp_type_int}, (TD_F5)};
const mp_obj_int_t mod_rl_TD_F6 = {{&mp_type_int}, (TD_F6)};
const mp_obj_int_t mod_rl_TD_F7 = {{&mp_type_int}, (TD_F7)};
const mp_obj_int_t mod_rl_TD_F8 = {{&mp_type_int}, (TD_F8)};
const mp_obj_int_t mod_rl_TD_F9 = {{&mp_type_int}, (TD_F9)};
const mp_obj_int_t mod_rl_TD_FIND = {{&mp_type_int}, (TD_FIND)};
const mp_obj_int_t mod_rl_TD_GREATER = {{&mp_type_int}, (TD_GREATER)};
const mp_obj_int_t mod_rl_TD_HASH = {{&mp_type_int}, (TD_HASH)};
const mp_obj_int_t mod_rl_TD_HELP = {{&mp_type_int}, (TD_HELP)};
const mp_obj_int_t mod_rl_TD_HOME = {{&mp_type_int}, (TD_HOME)};
const mp_obj_int_t mod_rl_TD_INSERT = {{&mp_type_int}, (TD_INSERT)};
const mp_obj_int_t mod_rl_TD_KBDILLUMDOWN = {{&mp_type_int}, (TD_KBDILLUMDOWN)};
const mp_obj_int_t mod_rl_TD_KBDILLUMTOGGLE = {{&mp_type_int}, (TD_KBDILLUMTOGGLE)};
const mp_obj_int_t mod_rl_TD_KBDILLUMUP = {{&mp_type_int}, (TD_KBDILLUMUP)};
const mp_obj_int_t mod_rl_TD_KP_0 = {{&mp_type_int}, (TD_KP_0)};
const mp_obj_int_t mod_rl_TD_KP_00 = {{&mp_type_int}, (TD_KP_00)};
const mp_obj_int_t mod_rl_TD_KP_000 = {{&mp_type_int}, (TD_KP_000)};
const mp_obj_int_t mod_rl_TD_KP_1 = {{&mp_type_int}, (TD_KP_1)};
const mp_obj_int_t mod_rl_TD_KP_2 = {{&mp_type_int}, (TD_KP_2)};
const mp_obj_int_t mod_rl_TD_KP_3 = {{&mp_type_int}, (TD_KP_3)};
const mp_obj_int_t mod_rl_TD_KP_4 = {{&mp_type_int}, (TD_KP_4)};
const mp_obj_int_t mod_rl_TD_KP_5 = {{&mp_type_int}, (TD_KP_5)};
const mp_obj_int_t mod_rl_TD_KP_6 = {{&mp_type_int}, (TD_KP_6)};
const mp_obj_int_t mod_rl_TD_KP_7 = {{&mp_type_int}, (TD_KP_7)};
const mp_obj_int_t mod_rl_TD_KP_8 = {{&mp_type_int}, (TD_KP_8)};
const mp_obj_int_t mod_rl_TD_KP_9 = {{&mp_type_int}, (TD_KP_9)};
const mp_obj_int_t mod_rl_TD_KP_A = {{&mp_type_int}, (TD_KP_A)};
const mp_obj_int_t mod_rl_TD_KP_AMPERSAND = {{&mp_type_int}, (TD_KP_AMPERSAND)};
const mp_obj_int_t mod_rl_TD_KP_AT = {{&mp_type_int}, (TD_KP_AT)};
const mp_obj_int_t mod_rl_TD_KP_B = {{&mp_type_int}, (TD_KP_B)};
const mp_obj_int_t mod_rl_TD_KP_BACKSPACE = {{&mp_type_int}, (TD_KP_BACKSPACE)};
const mp_obj_int_t mod_rl_TD_KP_BINARY = {{&mp_type_int}, (TD_KP_BINARY)};
const mp_obj_int_t mod_rl_TD_KP_C = {{&mp_type_int}, (TD_KP_C)};
const mp_obj_int_t mod_rl_TD_KP_CLEAR = {{&mp_type_int}, (TD_KP_CLEAR)};
const mp_obj_int_t mod_rl_TD_KP_CLEARENTRY = {{&mp_type_int}, (TD_KP_CLEARENTRY)};
const mp_obj_int_t mod_rl_TD_KP_COLON = {{&mp_type_int}, (TD_KP_COLON)};
const mp_obj_int_t mod_rl_TD_KP_COMMA = {{&mp_type_int}, (TD_KP_COMMA)};
const mp_obj_int_t mod_rl_TD_KP_D = {{&mp_type_int}, (TD_KP_D)};
const mp_obj_int_t mod_rl_TD_KP_DBLAMPERSAND = {{&mp_type_int}, (TD_KP_DBLAMPERSAND)};
const mp_obj_int_t mod_rl_TD_KP_DBLVERTICALBAR = {{&mp_type_int}, (TD_KP_DBLVERTICALBAR)};
const mp_obj_int_t mod_rl_TD_KP_DECIMAL = {{&mp_type_int}, (TD_KP_DECIMAL)};
const mp_obj_int_t mod_rl_TD_KP_DIVIDE = {{&mp_type_int}, (TD_KP_DIVIDE)};
const mp_obj_int_t mod_rl_TD_KP_E = {{&mp_type_int}, (TD_KP_E)};
const mp_obj_int_t mod_rl_TD_KP_ENTER = {{&mp_type_int}, (TD_KP_ENTER)};
const mp_obj_int_t mod_rl_TD_KP_EQUALS = {{&mp_type_int}, (TD_KP_EQUALS)};
const mp_obj_int_t mod_rl_TD_KP_EQUALSAS400 = {{&mp_type_int}, (TD_KP_EQUALSAS400)};
const mp_obj_int_t mod_rl_TD_KP_EXCLAM = {{&mp_type_int}, (TD_KP_EXCLAM)};
const mp_obj_int_t mod_rl_TD_KP_F = {{&mp_type_int}, (TD_KP_F)};
const mp_obj_int_t mod_rl_TD_KP_GREATER = {{&mp_type_int}, (TD_KP_GREATER)};
const mp_obj_int_t mod_rl_TD_KP_HASH = {{&mp_type_int}, (TD_KP_HASH)};
const mp_obj_int_t mod_rl_TD_KP_HEXADECIMAL = {{&mp_type_int}, (TD_KP_HEXADECIMAL)};
const mp_obj_int_t mod_rl_TD_KP_LEFTBRACE = {{&mp_type_int}, (TD_KP_LEFTBRACE)};
const mp_obj_int_t mod_rl_TD_KP_LEFTPAREN = {{&mp_type_int}, (TD_KP_LEFTPAREN)};
const mp_obj_int_t mod_rl_TD_KP_LESS = {{&mp_type_int}, (TD_KP_LESS)};
const mp_obj_int_t mod_rl_TD_KP_MEMADD = {{&mp_type_int}, (TD_KP_MEMADD)};
const mp_obj_int_t mod_rl_TD_KP_MEMCLEAR = {{&mp_type_int}, (TD_KP_MEMCLEAR)};
const mp_obj_int_t mod_rl_TD_KP_MEMDIVIDE = {{&mp_type_int}, (TD_KP_MEMDIVIDE)};
const mp_obj_int_t mod_rl_TD_KP_MEMMULTIPLY = {{&mp_type_int}, (TD_KP_MEMMULTIPLY)};
const mp_obj_int_t mod_rl_TD_KP_MEMRECALL = {{&mp_type_int}, (TD_KP_MEMRECALL)};
const mp_obj_int_t mod_rl_TD_KP_MEMSTORE = {{&mp_type_int}, (TD_KP_MEMSTORE)};
const mp_obj_int_t mod_rl_TD_KP_MEMSUBTRACT = {{&mp_type_int}, (TD_KP_MEMSUBTRACT)};
const mp_obj_int_t mod_rl_TD_KP_MINUS = {{&mp_type_int}, (TD_KP_MINUS)};
const mp_obj_int_t mod_rl_TD_KP_MULTIPLY = {{&mp_type_int}, (TD_KP_MULTIPLY)};
const mp_obj_int_t mod_rl_TD_KP_OCTAL = {{&mp_type_int}, (TD_KP_OCTAL)};
const mp_obj_int_t mod_rl_TD_KP_PERCENT = {{&mp_type_int}, (TD_KP_PERCENT)};
const mp_obj_int_t mod_rl_TD_KP_PERIOD = {{&mp_type_int}, (TD_KP_PERIOD)};
const mp_obj_int_t mod_rl_TD_KP_PLUS = {{&mp_type_int}, (TD_KP_PLUS)};
const mp_obj_int_t mod_rl_TD_KP_PLUSMINUS = {{&mp_type_int}, (TD_KP_PLUSMINUS)};
const mp_obj_int_t mod_rl_TD_KP_POWER = {{&mp_type_int}, (TD_KP_POWER)};
const mp_obj_int_t mod_rl_TD_KP_RIGHTBRACE = {{&mp_type_int}, (TD_KP_RIGHTBRACE)};
const mp_obj_int_t mod_rl_TD_KP_RIGHTPAREN = {{&mp_type_int}, (TD_KP_RIGHTPAREN)};
const mp_obj_int_t mod_rl_TD_KP_SPACE = {{&mp_type_int}, (TD_KP_SPACE)};
const mp_obj_int_t mod_rl_TD_KP_TAB = {{&mp_type_int}, (TD_KP_TAB)};
const mp_obj_int_t mod_rl_TD_KP_VERTICALBAR = {{&mp_type_int}, (TD_KP_VERTICALBAR)};
const mp_obj_int_t mod_rl_TD_KP_XOR = {{&mp_type_int}, (TD_KP_XOR)};
const mp_obj_int_t mod_rl_TD_LALT = {{&mp_type_int}, (TD_LALT)};
const mp_obj_int_t mod_rl_TD_LCTRL = {{&mp_type_int}, (TD_LCTRL)};
const mp_obj_int_t mod_rl_TD_LEFT = {{&mp_type_int}, (TD_LEFT)};
const mp_obj_int_t mod_rl_TD_LEFTBRACKET = {{&mp_type_int}, (TD_LEFTBRACKET)};
const mp_obj_int_t mod_rl_TD_LEFTPAREN = {{&mp_type_int}, (TD_LEFTPAREN)};
const mp_obj_int_t mod_rl_TD_LESS = {{&mp_type_int}, (TD_LESS)};
const mp_obj_int_t mod_rl_TD_LGUI = {{&mp_type_int}, (TD_LGUI)};
const mp_obj_int_t mod_rl_TD_LSHIFT = {{&mp_type_int}, (TD_LSHIFT)};
const mp_obj_int_t mod_rl_TD_MAIL = {{&mp_type_int}, (TD_MAIL)};
const mp_obj_int_t mod_rl_TD_MEDIASELECT = {{&mp_type_int}, (TD_MEDIASELECT)};
const mp_obj_int_t mod_rl_TD_MENU = {{&mp_type_int}, (TD_MENU)};
const mp_obj_int_t mod_rl_TD_MINUS = {{&mp_type_int}, (TD_MINUS)};
const mp_obj_int_t mod_rl_TD_MODE = {{&mp_type_int}, (TD_MODE)};
const mp_obj_int_t mod_rl_TD_MUTE = {{&mp_type_int}, (TD_MUTE)};
const mp_obj_int_t mod_rl_TD_NUMLOCKCLEAR = {{&mp_type_int}, (TD_NUMLOCKCLEAR)};
const mp_obj_int_t mod_rl_TD_OPER = {{&mp_type_int}, (TD_OPER)};
const mp_obj_int_t mod_rl_TD_OUT = {{&mp_type_int}, (TD_OUT)};
const mp_obj_int_t mod_rl_TD_PAGEDOWN = {{&mp_type_int}, (TD_PAGEDOWN)};
const mp_obj_int_t mod_rl_TD_PAGEUP = {{&mp_type_int}, (TD_PAGEUP)};
const mp_obj_int_t mod_rl_TD_PASTE = {{&mp_type_int}, (TD_PASTE)};
const mp_obj_int_t mod_rl_TD_PAUSE = {{&mp_type_int}, (TD_PAUSE)};
const mp_obj_int_t mod_rl_TD_PERCENT = {{&mp_type_int}, (TD_PERCENT)};
const mp_obj_int_t mod_rl_TD_PERIOD = {{&mp_type_int}, (TD_PERIOD)};
const mp_obj_int_t mod_rl_TD_PLUS = {{&mp_type_int}, (TD_PLUS)};
const mp_obj_int_t mod_rl_TD_POWER = {{&mp_type_int}, (TD_POWER)};
const mp_obj_int_t mod_rl_TD_PRINTSCREEN = {{&mp_type_int}, (TD_PRINTSCREEN)};
const mp_obj_int_t mod_rl_TD_PRIOR = {{&mp_type_int}, (TD_PRIOR)};
const mp_obj_int_t mod_rl_TD_QUESTION = {{&mp_type_int}, (TD_QUESTION)};
const mp_obj_int_t mod_rl_TD_QUOTE = {{&mp_type_int}, (TD_QUOTE)};
const mp_obj_int_t mod_rl_TD_QUOTEDBL = {{&mp_type_int}, (TD_QUOTEDBL)};
const mp_obj_int_t mod_rl_TD_RALT = {{&mp_type_int}, (TD_RALT)};
const mp_obj_int_t mod_rl_TD_RCTRL = {{&mp_type_int}, (TD_RCTRL)};
const mp_obj_int_t mod_rl_TD_RETURN = {{&mp_type_int}, (TD_RETURN)};
const mp_obj_int_t mod_rl_TD_RETURN2 = {{&mp_type_int}, (TD_RETURN2)};
const mp_obj_int_t mod_rl_TD_RGUI = {{&mp_type_int}, (TD_RGUI)};
const mp_obj_int_t mod_rl_TD_RIGHT = {{&mp_type_int}, (TD_RIGHT)};
const mp_obj_int_t mod_rl_TD_RIGHTBRACKET = {{&mp_type_int}, (TD_RIGHTBRACKET)};
const mp_obj_int_t mod_rl_TD_RIGHTPAREN = {{&mp_type_int}, (TD_RIGHTPAREN)};
const mp_obj_int_t mod_rl_TD_RSHIFT = {{&mp_type_int}, (TD_RSHIFT)};
const mp_obj_int_t mod_rl_TD_SCANCODE_MASK = {{&mp_type_int}, (TD_SCANCODE_MASK)};
const mp_obj_int_t mod_rl_TD_SCROLLLOCK = {{&mp_type_int}, (TD_SCROLLLOCK)};
const mp_obj_int_t mod_rl_TD_SELECT = {{&mp_type_int}, (TD_SELECT)};
const mp_obj_int_t mod_rl_TD_SEMICOLON = {{&mp_type_int}, (TD_SEMICOLON)};
const mp_obj_int_t mod_rl_TD_SEPARATOR = {{&mp_type_int}, (TD_SEPARATOR)};
const mp_obj_int_t mod_rl_TD_SLASH = {{&mp_type_int}, (TD_SLASH)};
const mp_obj_int_t mod_rl_TD_SLEEP = {{&mp_type_int}, (TD_SLEEP)};
const mp_obj_int_t mod_rl_TD_SPACE = {{&mp_type_int}, (TD_SPACE)};
const mp_obj_int_t mod_rl_TD_STOP = {{&mp_type_int}, (TD_STOP)};
const mp_obj_int_t mod_rl_TD_SYSREQ = {{&mp_type_int}, (TD_SYSREQ)};
const mp_obj_int_t mod_rl_TD_TAB = {{&mp_type_int}, (TD_TAB)};
const mp_obj_int_t mod_rl_TD_THOUSANDSSEPARATOR = {{&mp_type_int}, (TD_THOUSANDSSEPARATOR)};
const mp_obj_int_t mod_rl_TD_UNDERSCORE = {{&mp_type_int}, (TD_UNDERSCORE)};
const mp_obj_int_t mod_rl_TD_UNDO = {{&mp_type_int}, (TD_UNDO)};
const mp_obj_int_t mod_rl_TD_UNKNOWN = {{&mp_type_int}, (TD_UNKNOWN)};
const mp_obj_int_t mod_rl_TD_UP = {{&mp_type_int}, (TD_UP)};
const mp_obj_int_t mod_rl_TD_VOLUMEDOWN = {{&mp_type_int}, (TD_VOLUMEDOWN)};
const mp_obj_int_t mod_rl_TD_VOLUMEUP = {{&mp_type_int}, (TD_VOLUMEUP)};
const mp_obj_int_t mod_rl_TD_WWW = {{&mp_type_int}, (TD_WWW)};
const mp_obj_int_t mod_rl_TD_A = {{&mp_type_int}, (TD_A)};
const mp_obj_int_t mod_rl_TD_B = {{&mp_type_int}, (TD_B)};
const mp_obj_int_t mod_rl_TD_C = {{&mp_type_int}, (TD_C)};
const mp_obj_int_t mod_rl_TD_D = {{&mp_type_int}, (TD_D)};
const mp_obj_int_t mod_rl_TD_E = {{&mp_type_int}, (TD_E)};
const mp_obj_int_t mod_rl_TD_F = {{&mp_type_int}, (TD_F)};
const mp_obj_int_t mod_rl_TD_G = {{&mp_type_int}, (TD_G)};
const mp_obj_int_t mod_rl_TD_H = {{&mp_type_int}, (TD_H)};
const mp_obj_int_t mod_rl_TD_I = {{&mp_type_int}, (TD_I)};
const mp_obj_int_t mod_rl_TD_J = {{&mp_type_int}, (TD_J)};
const mp_obj_int_t mod_rl_TD_K = {{&mp_type_int}, (TD_K)};
const mp_obj_int_t mod_rl_TD_L = {{&mp_type_int}, (TD_L)};
const mp_obj_int_t mod_rl_TD_M = {{&mp_type_int}, (TD_M)};
const mp_obj_int_t mod_rl_TD_N = {{&mp_type_int}, (TD_N)};
const mp_obj_int_t mod_rl_TD_O = {{&mp_type_int}, (TD_O)};
const mp_obj_int_t mod_rl_TD_P = {{&mp_type_int}, (TD_P)};
const mp_obj_int_t mod_rl_TD_Q = {{&mp_type_int}, (TD_Q)};
const mp_obj_int_t mod_rl_TD_R = {{&mp_type_int}, (TD_R)};
const mp_obj_int_t mod_rl_TD_S = {{&mp_type_int}, (TD_S)};
const mp_obj_int_t mod_rl_TD_T = {{&mp_type_int}, (TD_T)};
const mp_obj_int_t mod_rl_TD_U = {{&mp_type_int}, (TD_U)};
const mp_obj_int_t mod_rl_TD_V = {{&mp_type_int}, (TD_V)};
const mp_obj_int_t mod_rl_TD_W = {{&mp_type_int}, (TD_W)};
const mp_obj_int_t mod_rl_TD_X = {{&mp_type_int}, (TD_X)};
const mp_obj_int_t mod_rl_TD_Y = {{&mp_type_int}, (TD_Y)};
const mp_obj_int_t mod_rl_TD_Z = {{&mp_type_int}, (TD_Z)};

STATIC const mp_rom_map_elem_t mp_module_rl_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rl) },
	/************* rogue_random *******************/
	{ MP_ROM_QSTR(MP_QSTR_random_next), MP_ROM_PTR(&mod_rl_random_next_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_int), MP_ROM_PTR(&mod_rl_random_int_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_3d), MP_ROM_PTR(&mod_rl_random_3d_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_2d), MP_ROM_PTR(&mod_rl_random_2d_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_1d), MP_ROM_PTR(&mod_rl_random_1d_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mod_rl_random_obj) },
	{ MP_ROM_QSTR(MP_QSTR_roll), MP_ROM_PTR(&mod_rl_roll_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_choice), MP_ROM_PTR(&mod_rl_random_choice_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shuffle), MP_ROM_PTR(&mod_rl_shuffle_obj) },
	{ MP_ROM_QSTR(MP_QSTR_get_seed), MP_ROM_PTR(&mod_rl_get_seed_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_seed), MP_ROM_PTR(&mod_rl_set_seed_obj) },
	/************* rogue_filesystem ***************/
	//{ MP_ROM_QSTR(MP_QSTR_open_resources), MP_ROM_PTR(&mod_fs_open_resources_obj) },
	{ MP_ROM_QSTR(MP_QSTR_set_app_name), MP_ROM_PTR(&mod_fs_set_app_name_obj) },
	{ MP_ROM_QSTR(MP_QSTR_load_asset), MP_ROM_PTR(&mod_fs_load_asset_obj) },
	{ MP_ROM_QSTR(MP_QSTR_load_pref), MP_ROM_PTR(&mod_fs_load_pref_obj) },
	{ MP_ROM_QSTR(MP_QSTR_save_pref), MP_ROM_PTR(&mod_fs_save_pref_obj) },
	/************* font ********************/
	{ MP_ROM_QSTR(MP_QSTR_Font), MP_ROM_PTR(&mp_type_rl_font) },
	/************* image ********************/
	{ MP_ROM_QSTR(MP_QSTR_Image), MP_ROM_PTR(&mp_type_rl_image) },
	/************* rogue_array ********************/
	{ MP_ROM_QSTR(MP_QSTR_Array), MP_ROM_PTR(&mp_type_rl_array) },
	{ MP_ROM_QSTR(MP_QSTR_array_from_string), MP_ROM_PTR(&mod_rl_array_from_string_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_from_list), MP_ROM_PTR(&mod_rl_array_from_list_obj) },
	/************* rogue_display ******************/
	{ MP_ROM_QSTR(MP_QSTR_init_display), MP_ROM_PTR(&mod_td_init_display_obj) },
	{ MP_ROM_QSTR(MP_QSTR_array_to_image), MP_ROM_PTR(&mod_td_array_to_image_obj) },
	{ MP_ROM_QSTR(MP_QSTR_image_to_array), MP_ROM_PTR(&mod_td_image_to_array_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_image), MP_ROM_PTR(&mod_td_draw_image_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_tile), MP_ROM_PTR(&mod_td_draw_tile_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_array), MP_ROM_PTR(&mod_td_draw_array_obj) },
	{ MP_ROM_QSTR(MP_QSTR_pack_tile), MP_ROM_PTR(&mod_rl_pack_tile_obj) },
	{ MP_ROM_QSTR(MP_QSTR_unpack_tile), MP_ROM_PTR(&mod_rl_unpack_tile_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_text), MP_ROM_PTR(&mod_td_draw_text_obj) },
	{ MP_ROM_QSTR(MP_QSTR_size_text), MP_ROM_PTR(&mod_td_size_text_obj) },
	{ MP_ROM_QSTR(MP_QSTR_fill_rect), MP_ROM_PTR(&mod_td_fill_rect_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_rect), MP_ROM_PTR(&mod_td_draw_rect_obj) },
	{ MP_ROM_QSTR(MP_QSTR_draw_line), MP_ROM_PTR(&mod_td_draw_line_obj) },
	{ MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&mod_td_clear_obj) },
	{ MP_ROM_QSTR(MP_QSTR_quit), MP_ROM_PTR(&mod_td_quit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_color), MP_ROM_PTR(&mod_td_color_obj) },
	{ MP_ROM_QSTR(MP_QSTR_hsv_color), MP_ROM_PTR(&mod_td_hsv_color_obj) },
	{ MP_ROM_QSTR(MP_QSTR_color_components), MP_ROM_PTR(&mod_td_color_components_obj) },
	/*{ MP_ROM_QSTR(MP_QSTR_color_r), MP_ROM_PTR(&mod_td_color_r_obj) },
	{ MP_ROM_QSTR(MP_QSTR_color_g), MP_ROM_PTR(&mod_td_color_g_obj) },
	{ MP_ROM_QSTR(MP_QSTR_color_b), MP_ROM_PTR(&mod_td_color_b_obj) },
	{ MP_ROM_QSTR(MP_QSTR_color_a), MP_ROM_PTR(&mod_td_color_a_obj) },*/
	{ MP_ROM_QSTR(MP_QSTR_random_color), MP_ROM_PTR(&mod_td_random_color_obj) },
	{ MP_ROM_QSTR(MP_QSTR_mouse), MP_ROM_PTR(&mod_td_mouse_obj) },
	{ MP_ROM_QSTR(MP_QSTR_key), MP_ROM_PTR(&mod_td_key_obj) },
	{ MP_ROM_QSTR(MP_QSTR_shift_pressed), MP_ROM_PTR(&mod_td_shift_pressed_obj) },
	{ MP_ROM_QSTR(MP_QSTR_alt_pressed), MP_ROM_PTR(&mod_td_alt_pressed_obj) },
	{ MP_ROM_QSTR(MP_QSTR_ctrl_pressed), MP_ROM_PTR(&mod_td_ctrl_pressed_obj) },
	{ MP_ROM_QSTR(MP_QSTR_win_pressed), MP_ROM_PTR(&mod_td_win_pressed_obj) },
	{ MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&mod_td_run_obj) },
	/************** utils ******************/
	{ MP_ROM_QSTR(MP_QSTR_walk_line_start), MP_ROM_PTR(&mod_rl_walk_line_start_obj) },
	{ MP_ROM_QSTR(MP_QSTR_walk_line_next), MP_ROM_PTR(&mod_rl_walk_line_next_obj) },
	{ MP_ROM_QSTR(MP_QSTR_distance), MP_ROM_PTR(&mod_rl_distance_obj) },
	/*************** constants **************/
	// text alignment
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_LEFT), MP_ROM_INT(TD_ALIGN_LEFT) },
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_RIGHT), MP_ROM_INT(TD_ALIGN_RIGHT) },
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_CENTER), MP_ROM_INT(TD_ALIGN_CENTER) },
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_TOP), MP_ROM_INT(TD_ALIGN_TOP) },
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_BOTTOM), MP_ROM_INT(TD_ALIGN_BOTTOM) },
	{ MP_ROM_QSTR(MP_QSTR_ALIGN_MIDDLE), MP_ROM_INT(TD_ALIGN_MIDDLE) },

	// events
	{ MP_ROM_QSTR(MP_QSTR_QUIT), MP_ROM_INT(TD_QUIT) },
	{ MP_ROM_QSTR(MP_QSTR_KEY), MP_ROM_INT(TD_KEY) },
	{ MP_ROM_QSTR(MP_QSTR_MOUSE_MOVED), MP_ROM_INT(TD_MOUSE_MOVED) },
	{ MP_ROM_QSTR(MP_QSTR_MOUSE_DOWN), MP_ROM_INT(TD_MOUSE_DOWN) },
	{ MP_ROM_QSTR(MP_QSTR_MOUSE_UP), MP_ROM_INT(TD_MOUSE_UP) },
	{ MP_ROM_QSTR(MP_QSTR_REDRAW), MP_ROM_INT(TD_REDRAW) },
	// mouse
	{ MP_ROM_QSTR(MP_QSTR_MOUSE_LEFT), MP_ROM_INT(TD_MOUSE_LEFT) },
	{ MP_ROM_QSTR(MP_QSTR_MOUSE_MIDDLE), MP_ROM_INT(TD_MOUSE_MIDDLE) },
	{ MP_ROM_QSTR(MP_QSTR_MOUSE_RIGHT), MP_ROM_INT(TD_MOUSE_RIGHT) },
	// update filters
	{ MP_ROM_QSTR(MP_QSTR_ON_EVENT), MP_ROM_INT(TD_ON_EVENT) },
	{ MP_ROM_QSTR(MP_QSTR_ON_REDRAW), MP_ROM_INT(TD_ON_REDRAW) },
	{ MP_ROM_QSTR(MP_QSTR_ON_KEY), MP_ROM_INT(TD_ON_KEY) },
	{ MP_ROM_QSTR(MP_QSTR_ON_MOUSE), MP_ROM_INT(TD_ON_MOUSE) },

	// math
  { MP_ROM_QSTR(MP_QSTR_INT_MAX), MP_ROM_PTR(&mod_rl_INT_MAX) },
  { MP_ROM_QSTR(MP_QSTR_INT_MIN), MP_ROM_PTR(&mod_rl_INT_MIN) },
	
	// pico8 colors
  { MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_PTR(&mod_rl_BLACK) },
  { MP_ROM_QSTR(MP_QSTR_DARKBLUE), MP_ROM_PTR(&mod_rl_DARKBLUE) },
  { MP_ROM_QSTR(MP_QSTR_DARKPURPLE), MP_ROM_PTR(&mod_rl_DARKPURPLE) },
  { MP_ROM_QSTR(MP_QSTR_DARKGREEN), MP_ROM_PTR(&mod_rl_DARKGREEN) },
  { MP_ROM_QSTR(MP_QSTR_BROWN), MP_ROM_PTR(&mod_rl_BROWN) },
  { MP_ROM_QSTR(MP_QSTR_DARKGRAY), MP_ROM_PTR(&mod_rl_DARKGRAY) },
  { MP_ROM_QSTR(MP_QSTR_LIGHTGRAY), MP_ROM_PTR(&mod_rl_LIGHTGRAY) },
  { MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_PTR(&mod_rl_WHITE) },
  { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_PTR(&mod_rl_RED) },
  { MP_ROM_QSTR(MP_QSTR_ORANGE), MP_ROM_PTR(&mod_rl_ORANGE) },
  { MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_PTR(&mod_rl_YELLOW) },
  { MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_PTR(&mod_rl_GREEN) },
  { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_PTR(&mod_rl_BLUE) },
  { MP_ROM_QSTR(MP_QSTR_INDIGO), MP_ROM_PTR(&mod_rl_INDIGO) },
  { MP_ROM_QSTR(MP_QSTR_PINK), MP_ROM_PTR(&mod_rl_PINK) },
  { MP_ROM_QSTR(MP_QSTR_PEACH), MP_ROM_PTR(&mod_rl_PEACH) },

	// keys
	{ MP_ROM_QSTR(MP_QSTR_0), MP_ROM_PTR(&mod_rl_TD_0) },
	{ MP_ROM_QSTR(MP_QSTR_1), MP_ROM_PTR(&mod_rl_TD_1) },
	{ MP_ROM_QSTR(MP_QSTR_2), MP_ROM_PTR(&mod_rl_TD_2) },
	{ MP_ROM_QSTR(MP_QSTR_3), MP_ROM_PTR(&mod_rl_TD_3) },
	{ MP_ROM_QSTR(MP_QSTR_4), MP_ROM_PTR(&mod_rl_TD_4) },
	{ MP_ROM_QSTR(MP_QSTR_5), MP_ROM_PTR(&mod_rl_TD_5) },
	{ MP_ROM_QSTR(MP_QSTR_6), MP_ROM_PTR(&mod_rl_TD_6) },
	{ MP_ROM_QSTR(MP_QSTR_7), MP_ROM_PTR(&mod_rl_TD_7) },
	{ MP_ROM_QSTR(MP_QSTR_8), MP_ROM_PTR(&mod_rl_TD_8) },
	{ MP_ROM_QSTR(MP_QSTR_9), MP_ROM_PTR(&mod_rl_TD_9) },
	{ MP_ROM_QSTR(MP_QSTR_AC_BACK), MP_ROM_PTR(&mod_rl_TD_AC_BACK) },
	{ MP_ROM_QSTR(MP_QSTR_AC_BOOKMARKS), MP_ROM_PTR(&mod_rl_TD_AC_BOOKMARKS) },
	{ MP_ROM_QSTR(MP_QSTR_AC_FORWARD), MP_ROM_PTR(&mod_rl_TD_AC_FORWARD) },
	{ MP_ROM_QSTR(MP_QSTR_AC_HOME), MP_ROM_PTR(&mod_rl_TD_AC_HOME) },
	{ MP_ROM_QSTR(MP_QSTR_AC_REFRESH), MP_ROM_PTR(&mod_rl_TD_AC_REFRESH) },
	{ MP_ROM_QSTR(MP_QSTR_AC_SEARCH), MP_ROM_PTR(&mod_rl_TD_AC_SEARCH) },
	{ MP_ROM_QSTR(MP_QSTR_AC_STOP), MP_ROM_PTR(&mod_rl_TD_AC_STOP) },
	{ MP_ROM_QSTR(MP_QSTR_AGAIN), MP_ROM_PTR(&mod_rl_TD_AGAIN) },
	{ MP_ROM_QSTR(MP_QSTR_ALTERASE), MP_ROM_PTR(&mod_rl_TD_ALTERASE) },
	{ MP_ROM_QSTR(MP_QSTR_AMPERSAND), MP_ROM_PTR(&mod_rl_TD_AMPERSAND) },
	{ MP_ROM_QSTR(MP_QSTR_APP1), MP_ROM_PTR(&mod_rl_TD_APP1) },
	{ MP_ROM_QSTR(MP_QSTR_APP2), MP_ROM_PTR(&mod_rl_TD_APP2) },
	{ MP_ROM_QSTR(MP_QSTR_APPLICATION), MP_ROM_PTR(&mod_rl_TD_APPLICATION) },
	{ MP_ROM_QSTR(MP_QSTR_ASTERISK), MP_ROM_PTR(&mod_rl_TD_ASTERISK) },
	{ MP_ROM_QSTR(MP_QSTR_AT), MP_ROM_PTR(&mod_rl_TD_AT) },
	{ MP_ROM_QSTR(MP_QSTR_AUDIOFASTFORWARD), MP_ROM_PTR(&mod_rl_TD_AUDIOFASTFORWARD) },
	{ MP_ROM_QSTR(MP_QSTR_AUDIOMUTE), MP_ROM_PTR(&mod_rl_TD_AUDIOMUTE) },
	{ MP_ROM_QSTR(MP_QSTR_AUDIONEXT), MP_ROM_PTR(&mod_rl_TD_AUDIONEXT) },
	{ MP_ROM_QSTR(MP_QSTR_AUDIOPLAY), MP_ROM_PTR(&mod_rl_TD_AUDIOPLAY) },
	{ MP_ROM_QSTR(MP_QSTR_AUDIOPREV), MP_ROM_PTR(&mod_rl_TD_AUDIOPREV) },
	{ MP_ROM_QSTR(MP_QSTR_AUDIOREWIND), MP_ROM_PTR(&mod_rl_TD_AUDIOREWIND) },
	{ MP_ROM_QSTR(MP_QSTR_AUDIOSTOP), MP_ROM_PTR(&mod_rl_TD_AUDIOSTOP) },
	{ MP_ROM_QSTR(MP_QSTR_BACKQUOTE), MP_ROM_PTR(&mod_rl_TD_BACKQUOTE) },
	{ MP_ROM_QSTR(MP_QSTR_BACKSLASH), MP_ROM_PTR(&mod_rl_TD_BACKSLASH) },
	{ MP_ROM_QSTR(MP_QSTR_BACKSPACE), MP_ROM_PTR(&mod_rl_TD_BACKSPACE) },
	{ MP_ROM_QSTR(MP_QSTR_BRIGHTNESSDOWN), MP_ROM_PTR(&mod_rl_TD_BRIGHTNESSDOWN) },
	{ MP_ROM_QSTR(MP_QSTR_BRIGHTNESSUP), MP_ROM_PTR(&mod_rl_TD_BRIGHTNESSUP) },
	{ MP_ROM_QSTR(MP_QSTR_CALCULATOR), MP_ROM_PTR(&mod_rl_TD_CALCULATOR) },
	{ MP_ROM_QSTR(MP_QSTR_CANCEL), MP_ROM_PTR(&mod_rl_TD_CANCEL) },
	{ MP_ROM_QSTR(MP_QSTR_CAPSLOCK), MP_ROM_PTR(&mod_rl_TD_CAPSLOCK) },
	{ MP_ROM_QSTR(MP_QSTR_CARET), MP_ROM_PTR(&mod_rl_TD_CARET) },
	{ MP_ROM_QSTR(MP_QSTR_CLEAR), MP_ROM_PTR(&mod_rl_TD_CLEAR) },
	{ MP_ROM_QSTR(MP_QSTR_CLEARAGAIN), MP_ROM_PTR(&mod_rl_TD_CLEARAGAIN) },
	{ MP_ROM_QSTR(MP_QSTR_COLON), MP_ROM_PTR(&mod_rl_TD_COLON) },
	{ MP_ROM_QSTR(MP_QSTR_COMMA), MP_ROM_PTR(&mod_rl_TD_COMMA) },
	{ MP_ROM_QSTR(MP_QSTR_COMPUTER), MP_ROM_PTR(&mod_rl_TD_COMPUTER) },
	{ MP_ROM_QSTR(MP_QSTR_COPY), MP_ROM_PTR(&mod_rl_TD_COPY) },
	{ MP_ROM_QSTR(MP_QSTR_CRSEL), MP_ROM_PTR(&mod_rl_TD_CRSEL) },
	{ MP_ROM_QSTR(MP_QSTR_CURRENCYSUBUNIT), MP_ROM_PTR(&mod_rl_TD_CURRENCYSUBUNIT) },
	{ MP_ROM_QSTR(MP_QSTR_CURRENCYUNIT), MP_ROM_PTR(&mod_rl_TD_CURRENCYUNIT) },
	{ MP_ROM_QSTR(MP_QSTR_CUT), MP_ROM_PTR(&mod_rl_TD_CUT) },
	{ MP_ROM_QSTR(MP_QSTR_DECIMALSEPARATOR), MP_ROM_PTR(&mod_rl_TD_DECIMALSEPARATOR) },
	{ MP_ROM_QSTR(MP_QSTR_DELETE), MP_ROM_PTR(&mod_rl_TD_DELETE) },
	{ MP_ROM_QSTR(MP_QSTR_DISPLAYSWITCH), MP_ROM_PTR(&mod_rl_TD_DISPLAYSWITCH) },
	{ MP_ROM_QSTR(MP_QSTR_DOLLAR), MP_ROM_PTR(&mod_rl_TD_DOLLAR) },
	{ MP_ROM_QSTR(MP_QSTR_DOWN), MP_ROM_PTR(&mod_rl_TD_DOWN) },
	{ MP_ROM_QSTR(MP_QSTR_EJECT), MP_ROM_PTR(&mod_rl_TD_EJECT) },
	{ MP_ROM_QSTR(MP_QSTR_END), MP_ROM_PTR(&mod_rl_TD_END) },
	{ MP_ROM_QSTR(MP_QSTR_EQUALS), MP_ROM_PTR(&mod_rl_TD_EQUALS) },
	{ MP_ROM_QSTR(MP_QSTR_ESCAPE), MP_ROM_PTR(&mod_rl_TD_ESCAPE) },
	{ MP_ROM_QSTR(MP_QSTR_EXCLAIM), MP_ROM_PTR(&mod_rl_TD_EXCLAIM) },
	{ MP_ROM_QSTR(MP_QSTR_EXECUTE), MP_ROM_PTR(&mod_rl_TD_EXECUTE) },
	{ MP_ROM_QSTR(MP_QSTR_EXSEL), MP_ROM_PTR(&mod_rl_TD_EXSEL) },
	{ MP_ROM_QSTR(MP_QSTR_F1), MP_ROM_PTR(&mod_rl_TD_F1) },
	{ MP_ROM_QSTR(MP_QSTR_F10), MP_ROM_PTR(&mod_rl_TD_F10) },
	{ MP_ROM_QSTR(MP_QSTR_F11), MP_ROM_PTR(&mod_rl_TD_F11) },
	{ MP_ROM_QSTR(MP_QSTR_F12), MP_ROM_PTR(&mod_rl_TD_F12) },
	{ MP_ROM_QSTR(MP_QSTR_F13), MP_ROM_PTR(&mod_rl_TD_F13) },
	{ MP_ROM_QSTR(MP_QSTR_F14), MP_ROM_PTR(&mod_rl_TD_F14) },
	{ MP_ROM_QSTR(MP_QSTR_F15), MP_ROM_PTR(&mod_rl_TD_F15) },
	{ MP_ROM_QSTR(MP_QSTR_F16), MP_ROM_PTR(&mod_rl_TD_F16) },
	{ MP_ROM_QSTR(MP_QSTR_F17), MP_ROM_PTR(&mod_rl_TD_F17) },
	{ MP_ROM_QSTR(MP_QSTR_F18), MP_ROM_PTR(&mod_rl_TD_F18) },
	{ MP_ROM_QSTR(MP_QSTR_F19), MP_ROM_PTR(&mod_rl_TD_F19) },
	{ MP_ROM_QSTR(MP_QSTR_F2), MP_ROM_PTR(&mod_rl_TD_F2) },
	{ MP_ROM_QSTR(MP_QSTR_F20), MP_ROM_PTR(&mod_rl_TD_F20) },
	{ MP_ROM_QSTR(MP_QSTR_F21), MP_ROM_PTR(&mod_rl_TD_F21) },
	{ MP_ROM_QSTR(MP_QSTR_F22), MP_ROM_PTR(&mod_rl_TD_F22) },
	{ MP_ROM_QSTR(MP_QSTR_F23), MP_ROM_PTR(&mod_rl_TD_F23) },
	{ MP_ROM_QSTR(MP_QSTR_F24), MP_ROM_PTR(&mod_rl_TD_F24) },
	{ MP_ROM_QSTR(MP_QSTR_F3), MP_ROM_PTR(&mod_rl_TD_F3) },
	{ MP_ROM_QSTR(MP_QSTR_F4), MP_ROM_PTR(&mod_rl_TD_F4) },
	{ MP_ROM_QSTR(MP_QSTR_F5), MP_ROM_PTR(&mod_rl_TD_F5) },
	{ MP_ROM_QSTR(MP_QSTR_F6), MP_ROM_PTR(&mod_rl_TD_F6) },
	{ MP_ROM_QSTR(MP_QSTR_F7), MP_ROM_PTR(&mod_rl_TD_F7) },
	{ MP_ROM_QSTR(MP_QSTR_F8), MP_ROM_PTR(&mod_rl_TD_F8) },
	{ MP_ROM_QSTR(MP_QSTR_F9), MP_ROM_PTR(&mod_rl_TD_F9) },
	{ MP_ROM_QSTR(MP_QSTR_FIND), MP_ROM_PTR(&mod_rl_TD_FIND) },
	{ MP_ROM_QSTR(MP_QSTR_GREATER), MP_ROM_PTR(&mod_rl_TD_GREATER) },
	{ MP_ROM_QSTR(MP_QSTR_HASH), MP_ROM_PTR(&mod_rl_TD_HASH) },
	{ MP_ROM_QSTR(MP_QSTR_HELP), MP_ROM_PTR(&mod_rl_TD_HELP) },
	{ MP_ROM_QSTR(MP_QSTR_HOME), MP_ROM_PTR(&mod_rl_TD_HOME) },
	{ MP_ROM_QSTR(MP_QSTR_INSERT), MP_ROM_PTR(&mod_rl_TD_INSERT) },
	{ MP_ROM_QSTR(MP_QSTR_KBDILLUMDOWN), MP_ROM_PTR(&mod_rl_TD_KBDILLUMDOWN) },
	{ MP_ROM_QSTR(MP_QSTR_KBDILLUMTOGGLE), MP_ROM_PTR(&mod_rl_TD_KBDILLUMTOGGLE) },
	{ MP_ROM_QSTR(MP_QSTR_KBDILLUMUP), MP_ROM_PTR(&mod_rl_TD_KBDILLUMUP) },
	{ MP_ROM_QSTR(MP_QSTR_KP_0), MP_ROM_PTR(&mod_rl_TD_KP_0) },
	{ MP_ROM_QSTR(MP_QSTR_KP_00), MP_ROM_PTR(&mod_rl_TD_KP_00) },
	{ MP_ROM_QSTR(MP_QSTR_KP_000), MP_ROM_PTR(&mod_rl_TD_KP_000) },
	{ MP_ROM_QSTR(MP_QSTR_KP_1), MP_ROM_PTR(&mod_rl_TD_KP_1) },
	{ MP_ROM_QSTR(MP_QSTR_KP_2), MP_ROM_PTR(&mod_rl_TD_KP_2) },
	{ MP_ROM_QSTR(MP_QSTR_KP_3), MP_ROM_PTR(&mod_rl_TD_KP_3) },
	{ MP_ROM_QSTR(MP_QSTR_KP_4), MP_ROM_PTR(&mod_rl_TD_KP_4) },
	{ MP_ROM_QSTR(MP_QSTR_KP_5), MP_ROM_PTR(&mod_rl_TD_KP_5) },
	{ MP_ROM_QSTR(MP_QSTR_KP_6), MP_ROM_PTR(&mod_rl_TD_KP_6) },
	{ MP_ROM_QSTR(MP_QSTR_KP_7), MP_ROM_PTR(&mod_rl_TD_KP_7) },
	{ MP_ROM_QSTR(MP_QSTR_KP_8), MP_ROM_PTR(&mod_rl_TD_KP_8) },
	{ MP_ROM_QSTR(MP_QSTR_KP_9), MP_ROM_PTR(&mod_rl_TD_KP_9) },
	{ MP_ROM_QSTR(MP_QSTR_KP_A), MP_ROM_PTR(&mod_rl_TD_KP_A) },
	{ MP_ROM_QSTR(MP_QSTR_KP_AMPERSAND), MP_ROM_PTR(&mod_rl_TD_KP_AMPERSAND) },
	{ MP_ROM_QSTR(MP_QSTR_KP_AT), MP_ROM_PTR(&mod_rl_TD_KP_AT) },
	{ MP_ROM_QSTR(MP_QSTR_KP_B), MP_ROM_PTR(&mod_rl_TD_KP_B) },
	{ MP_ROM_QSTR(MP_QSTR_KP_BACKSPACE), MP_ROM_PTR(&mod_rl_TD_KP_BACKSPACE) },
	{ MP_ROM_QSTR(MP_QSTR_KP_BINARY), MP_ROM_PTR(&mod_rl_TD_KP_BINARY) },
	{ MP_ROM_QSTR(MP_QSTR_KP_C), MP_ROM_PTR(&mod_rl_TD_KP_C) },
	{ MP_ROM_QSTR(MP_QSTR_KP_CLEAR), MP_ROM_PTR(&mod_rl_TD_KP_CLEAR) },
	{ MP_ROM_QSTR(MP_QSTR_KP_CLEARENTRY), MP_ROM_PTR(&mod_rl_TD_KP_CLEARENTRY) },
	{ MP_ROM_QSTR(MP_QSTR_KP_COLON), MP_ROM_PTR(&mod_rl_TD_KP_COLON) },
	{ MP_ROM_QSTR(MP_QSTR_KP_COMMA), MP_ROM_PTR(&mod_rl_TD_KP_COMMA) },
	{ MP_ROM_QSTR(MP_QSTR_KP_D), MP_ROM_PTR(&mod_rl_TD_KP_D) },
	{ MP_ROM_QSTR(MP_QSTR_KP_DBLAMPERSAND), MP_ROM_PTR(&mod_rl_TD_KP_DBLAMPERSAND) },
	{ MP_ROM_QSTR(MP_QSTR_KP_DBLVERTICALBAR), MP_ROM_PTR(&mod_rl_TD_KP_DBLVERTICALBAR) },
	{ MP_ROM_QSTR(MP_QSTR_KP_DECIMAL), MP_ROM_PTR(&mod_rl_TD_KP_DECIMAL) },
	{ MP_ROM_QSTR(MP_QSTR_KP_DIVIDE), MP_ROM_PTR(&mod_rl_TD_KP_DIVIDE) },
	{ MP_ROM_QSTR(MP_QSTR_KP_E), MP_ROM_PTR(&mod_rl_TD_KP_E) },
	{ MP_ROM_QSTR(MP_QSTR_KP_ENTER), MP_ROM_PTR(&mod_rl_TD_KP_ENTER) },
	{ MP_ROM_QSTR(MP_QSTR_KP_EQUALS), MP_ROM_PTR(&mod_rl_TD_KP_EQUALS) },
	{ MP_ROM_QSTR(MP_QSTR_KP_EQUALSAS400), MP_ROM_PTR(&mod_rl_TD_KP_EQUALSAS400) },
	{ MP_ROM_QSTR(MP_QSTR_KP_EXCLAM), MP_ROM_PTR(&mod_rl_TD_KP_EXCLAM) },
	{ MP_ROM_QSTR(MP_QSTR_KP_F), MP_ROM_PTR(&mod_rl_TD_KP_F) },
	{ MP_ROM_QSTR(MP_QSTR_KP_GREATER), MP_ROM_PTR(&mod_rl_TD_KP_GREATER) },
	{ MP_ROM_QSTR(MP_QSTR_KP_HASH), MP_ROM_PTR(&mod_rl_TD_KP_HASH) },
	{ MP_ROM_QSTR(MP_QSTR_KP_HEXADECIMAL), MP_ROM_PTR(&mod_rl_TD_KP_HEXADECIMAL) },
	{ MP_ROM_QSTR(MP_QSTR_KP_LEFTBRACE), MP_ROM_PTR(&mod_rl_TD_KP_LEFTBRACE) },
	{ MP_ROM_QSTR(MP_QSTR_KP_LEFTPAREN), MP_ROM_PTR(&mod_rl_TD_KP_LEFTPAREN) },
	{ MP_ROM_QSTR(MP_QSTR_KP_LESS), MP_ROM_PTR(&mod_rl_TD_KP_LESS) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MEMADD), MP_ROM_PTR(&mod_rl_TD_KP_MEMADD) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MEMCLEAR), MP_ROM_PTR(&mod_rl_TD_KP_MEMCLEAR) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MEMDIVIDE), MP_ROM_PTR(&mod_rl_TD_KP_MEMDIVIDE) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MEMMULTIPLY), MP_ROM_PTR(&mod_rl_TD_KP_MEMMULTIPLY) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MEMRECALL), MP_ROM_PTR(&mod_rl_TD_KP_MEMRECALL) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MEMSTORE), MP_ROM_PTR(&mod_rl_TD_KP_MEMSTORE) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MEMSUBTRACT), MP_ROM_PTR(&mod_rl_TD_KP_MEMSUBTRACT) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MINUS), MP_ROM_PTR(&mod_rl_TD_KP_MINUS) },
	{ MP_ROM_QSTR(MP_QSTR_KP_MULTIPLY), MP_ROM_PTR(&mod_rl_TD_KP_MULTIPLY) },
	{ MP_ROM_QSTR(MP_QSTR_KP_OCTAL), MP_ROM_PTR(&mod_rl_TD_KP_OCTAL) },
	{ MP_ROM_QSTR(MP_QSTR_KP_PERCENT), MP_ROM_PTR(&mod_rl_TD_KP_PERCENT) },
	{ MP_ROM_QSTR(MP_QSTR_KP_PERIOD), MP_ROM_PTR(&mod_rl_TD_KP_PERIOD) },
	{ MP_ROM_QSTR(MP_QSTR_KP_PLUS), MP_ROM_PTR(&mod_rl_TD_KP_PLUS) },
	{ MP_ROM_QSTR(MP_QSTR_KP_PLUSMINUS), MP_ROM_PTR(&mod_rl_TD_KP_PLUSMINUS) },
	{ MP_ROM_QSTR(MP_QSTR_KP_POWER), MP_ROM_PTR(&mod_rl_TD_KP_POWER) },
	{ MP_ROM_QSTR(MP_QSTR_KP_RIGHTBRACE), MP_ROM_PTR(&mod_rl_TD_KP_RIGHTBRACE) },
	{ MP_ROM_QSTR(MP_QSTR_KP_RIGHTPAREN), MP_ROM_PTR(&mod_rl_TD_KP_RIGHTPAREN) },
	{ MP_ROM_QSTR(MP_QSTR_KP_SPACE), MP_ROM_PTR(&mod_rl_TD_KP_SPACE) },
	{ MP_ROM_QSTR(MP_QSTR_KP_TAB), MP_ROM_PTR(&mod_rl_TD_KP_TAB) },
	{ MP_ROM_QSTR(MP_QSTR_KP_VERTICALBAR), MP_ROM_PTR(&mod_rl_TD_KP_VERTICALBAR) },
	{ MP_ROM_QSTR(MP_QSTR_KP_XOR), MP_ROM_PTR(&mod_rl_TD_KP_XOR) },
	{ MP_ROM_QSTR(MP_QSTR_LALT), MP_ROM_PTR(&mod_rl_TD_LALT) },
	{ MP_ROM_QSTR(MP_QSTR_LCTRL), MP_ROM_PTR(&mod_rl_TD_LCTRL) },
	{ MP_ROM_QSTR(MP_QSTR_LEFT), MP_ROM_PTR(&mod_rl_TD_LEFT) },
	{ MP_ROM_QSTR(MP_QSTR_LEFTBRACKET), MP_ROM_PTR(&mod_rl_TD_LEFTBRACKET) },
	{ MP_ROM_QSTR(MP_QSTR_LEFTPAREN), MP_ROM_PTR(&mod_rl_TD_LEFTPAREN) },
	{ MP_ROM_QSTR(MP_QSTR_LESS), MP_ROM_PTR(&mod_rl_TD_LESS) },
	{ MP_ROM_QSTR(MP_QSTR_LGUI), MP_ROM_PTR(&mod_rl_TD_LGUI) },
	{ MP_ROM_QSTR(MP_QSTR_LSHIFT), MP_ROM_PTR(&mod_rl_TD_LSHIFT) },
	{ MP_ROM_QSTR(MP_QSTR_MAIL), MP_ROM_PTR(&mod_rl_TD_MAIL) },
	{ MP_ROM_QSTR(MP_QSTR_MEDIASELECT), MP_ROM_PTR(&mod_rl_TD_MEDIASELECT) },
	{ MP_ROM_QSTR(MP_QSTR_MENU), MP_ROM_PTR(&mod_rl_TD_MENU) },
	{ MP_ROM_QSTR(MP_QSTR_MINUS), MP_ROM_PTR(&mod_rl_TD_MINUS) },
	{ MP_ROM_QSTR(MP_QSTR_MODE), MP_ROM_PTR(&mod_rl_TD_MODE) },
	{ MP_ROM_QSTR(MP_QSTR_MUTE), MP_ROM_PTR(&mod_rl_TD_MUTE) },
	{ MP_ROM_QSTR(MP_QSTR_NUMLOCKCLEAR), MP_ROM_PTR(&mod_rl_TD_NUMLOCKCLEAR) },
	{ MP_ROM_QSTR(MP_QSTR_OPER), MP_ROM_PTR(&mod_rl_TD_OPER) },
	{ MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_PTR(&mod_rl_TD_OUT) },
	{ MP_ROM_QSTR(MP_QSTR_PAGEDOWN), MP_ROM_PTR(&mod_rl_TD_PAGEDOWN) },
	{ MP_ROM_QSTR(MP_QSTR_PAGEUP), MP_ROM_PTR(&mod_rl_TD_PAGEUP) },
	{ MP_ROM_QSTR(MP_QSTR_PASTE), MP_ROM_PTR(&mod_rl_TD_PASTE) },
	{ MP_ROM_QSTR(MP_QSTR_PAUSE), MP_ROM_PTR(&mod_rl_TD_PAUSE) },
	{ MP_ROM_QSTR(MP_QSTR_PERCENT), MP_ROM_PTR(&mod_rl_TD_PERCENT) },
	{ MP_ROM_QSTR(MP_QSTR_PERIOD), MP_ROM_PTR(&mod_rl_TD_PERIOD) },
	{ MP_ROM_QSTR(MP_QSTR_PLUS), MP_ROM_PTR(&mod_rl_TD_PLUS) },
	{ MP_ROM_QSTR(MP_QSTR_POWER), MP_ROM_PTR(&mod_rl_TD_POWER) },
	{ MP_ROM_QSTR(MP_QSTR_PRINTSCREEN), MP_ROM_PTR(&mod_rl_TD_PRINTSCREEN) },
	{ MP_ROM_QSTR(MP_QSTR_PRIOR), MP_ROM_PTR(&mod_rl_TD_PRIOR) },
	{ MP_ROM_QSTR(MP_QSTR_QUESTION), MP_ROM_PTR(&mod_rl_TD_QUESTION) },
	{ MP_ROM_QSTR(MP_QSTR_QUOTE), MP_ROM_PTR(&mod_rl_TD_QUOTE) },
	{ MP_ROM_QSTR(MP_QSTR_QUOTEDBL), MP_ROM_PTR(&mod_rl_TD_QUOTEDBL) },
	{ MP_ROM_QSTR(MP_QSTR_RALT), MP_ROM_PTR(&mod_rl_TD_RALT) },
	{ MP_ROM_QSTR(MP_QSTR_RCTRL), MP_ROM_PTR(&mod_rl_TD_RCTRL) },
	{ MP_ROM_QSTR(MP_QSTR_RETURN), MP_ROM_PTR(&mod_rl_TD_RETURN) },
	{ MP_ROM_QSTR(MP_QSTR_RETURN2), MP_ROM_PTR(&mod_rl_TD_RETURN2) },
	{ MP_ROM_QSTR(MP_QSTR_RGUI), MP_ROM_PTR(&mod_rl_TD_RGUI) },
	{ MP_ROM_QSTR(MP_QSTR_RIGHT), MP_ROM_PTR(&mod_rl_TD_RIGHT) },
	{ MP_ROM_QSTR(MP_QSTR_RIGHTBRACKET), MP_ROM_PTR(&mod_rl_TD_RIGHTBRACKET) },
	{ MP_ROM_QSTR(MP_QSTR_RIGHTPAREN), MP_ROM_PTR(&mod_rl_TD_RIGHTPAREN) },
	{ MP_ROM_QSTR(MP_QSTR_RSHIFT), MP_ROM_PTR(&mod_rl_TD_RSHIFT) },
	{ MP_ROM_QSTR(MP_QSTR_SCANCODE_MASK), MP_ROM_PTR(&mod_rl_TD_SCANCODE_MASK) },
	{ MP_ROM_QSTR(MP_QSTR_SCROLLLOCK), MP_ROM_PTR(&mod_rl_TD_SCROLLLOCK) },
	{ MP_ROM_QSTR(MP_QSTR_SELECT), MP_ROM_PTR(&mod_rl_TD_SELECT) },
	{ MP_ROM_QSTR(MP_QSTR_SEMICOLON), MP_ROM_PTR(&mod_rl_TD_SEMICOLON) },
	{ MP_ROM_QSTR(MP_QSTR_SEPARATOR), MP_ROM_PTR(&mod_rl_TD_SEPARATOR) },
	{ MP_ROM_QSTR(MP_QSTR_SLASH), MP_ROM_PTR(&mod_rl_TD_SLASH) },
	{ MP_ROM_QSTR(MP_QSTR_SLEEP), MP_ROM_PTR(&mod_rl_TD_SLEEP) },
	{ MP_ROM_QSTR(MP_QSTR_SPACE), MP_ROM_PTR(&mod_rl_TD_SPACE) },
	{ MP_ROM_QSTR(MP_QSTR_STOP), MP_ROM_PTR(&mod_rl_TD_STOP) },
	{ MP_ROM_QSTR(MP_QSTR_SYSREQ), MP_ROM_PTR(&mod_rl_TD_SYSREQ) },
	{ MP_ROM_QSTR(MP_QSTR_TAB), MP_ROM_PTR(&mod_rl_TD_TAB) },
	{ MP_ROM_QSTR(MP_QSTR_THOUSANDSSEPARATOR), MP_ROM_PTR(&mod_rl_TD_THOUSANDSSEPARATOR) },
	{ MP_ROM_QSTR(MP_QSTR_UNDERSCORE), MP_ROM_PTR(&mod_rl_TD_UNDERSCORE) },
	{ MP_ROM_QSTR(MP_QSTR_UNDO), MP_ROM_PTR(&mod_rl_TD_UNDO) },
	{ MP_ROM_QSTR(MP_QSTR_UNKNOWN), MP_ROM_PTR(&mod_rl_TD_UNKNOWN) },
	{ MP_ROM_QSTR(MP_QSTR_UP), MP_ROM_PTR(&mod_rl_TD_UP) },
	{ MP_ROM_QSTR(MP_QSTR_VOLUMEDOWN), MP_ROM_PTR(&mod_rl_TD_VOLUMEDOWN) },
	{ MP_ROM_QSTR(MP_QSTR_VOLUMEUP), MP_ROM_PTR(&mod_rl_TD_VOLUMEUP) },
	{ MP_ROM_QSTR(MP_QSTR_WWW), MP_ROM_PTR(&mod_rl_TD_WWW) },
	{ MP_ROM_QSTR(MP_QSTR_A), MP_ROM_PTR(&mod_rl_TD_A) },
	{ MP_ROM_QSTR(MP_QSTR_B), MP_ROM_PTR(&mod_rl_TD_B) },
	{ MP_ROM_QSTR(MP_QSTR_C), MP_ROM_PTR(&mod_rl_TD_C) },
	{ MP_ROM_QSTR(MP_QSTR_D), MP_ROM_PTR(&mod_rl_TD_D) },
	{ MP_ROM_QSTR(MP_QSTR_E), MP_ROM_PTR(&mod_rl_TD_E) },
	{ MP_ROM_QSTR(MP_QSTR_F), MP_ROM_PTR(&mod_rl_TD_F) },
	{ MP_ROM_QSTR(MP_QSTR_G), MP_ROM_PTR(&mod_rl_TD_G) },
	{ MP_ROM_QSTR(MP_QSTR_H), MP_ROM_PTR(&mod_rl_TD_H) },
	{ MP_ROM_QSTR(MP_QSTR_I), MP_ROM_PTR(&mod_rl_TD_I) },
	{ MP_ROM_QSTR(MP_QSTR_J), MP_ROM_PTR(&mod_rl_TD_J) },
	{ MP_ROM_QSTR(MP_QSTR_K), MP_ROM_PTR(&mod_rl_TD_K) },
	{ MP_ROM_QSTR(MP_QSTR_L), MP_ROM_PTR(&mod_rl_TD_L) },
	{ MP_ROM_QSTR(MP_QSTR_M), MP_ROM_PTR(&mod_rl_TD_M) },
	{ MP_ROM_QSTR(MP_QSTR_N), MP_ROM_PTR(&mod_rl_TD_N) },
	{ MP_ROM_QSTR(MP_QSTR_O), MP_ROM_PTR(&mod_rl_TD_O) },
	{ MP_ROM_QSTR(MP_QSTR_P), MP_ROM_PTR(&mod_rl_TD_P) },
	{ MP_ROM_QSTR(MP_QSTR_Q), MP_ROM_PTR(&mod_rl_TD_Q) },
	{ MP_ROM_QSTR(MP_QSTR_R), MP_ROM_PTR(&mod_rl_TD_R) },
	{ MP_ROM_QSTR(MP_QSTR_S), MP_ROM_PTR(&mod_rl_TD_S) },
	{ MP_ROM_QSTR(MP_QSTR_T), MP_ROM_PTR(&mod_rl_TD_T) },
	{ MP_ROM_QSTR(MP_QSTR_U), MP_ROM_PTR(&mod_rl_TD_U) },
	{ MP_ROM_QSTR(MP_QSTR_V), MP_ROM_PTR(&mod_rl_TD_V) },
	{ MP_ROM_QSTR(MP_QSTR_W), MP_ROM_PTR(&mod_rl_TD_W) },
	{ MP_ROM_QSTR(MP_QSTR_X), MP_ROM_PTR(&mod_rl_TD_X) },
	{ MP_ROM_QSTR(MP_QSTR_Y), MP_ROM_PTR(&mod_rl_TD_Y) },
	{ MP_ROM_QSTR(MP_QSTR_Z), MP_ROM_PTR(&mod_rl_TD_Z) },

};

STATIC MP_DEFINE_CONST_DICT(mp_module_rl_globals, mp_module_rl_globals_table);

const mp_obj_module_t mp_module_rl = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&mp_module_rl_globals,
};

#endif //PYROGUE_MOD_RL
