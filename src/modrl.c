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

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/parsenum.h"
#include "py/runtime.h"
#include "py/stream.h"

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
	char* result = fs_load_asset(path, &size);
	if(result == NULL) return mp_const_none;
	return mp_obj_new_bytes(result, size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_fs_load_asset_obj, mod_fs_load_asset);

STATIC mp_obj_t mod_fs_load_pref(mp_obj_t path_in) {
	size_t len;
	const char *path = mp_obj_str_get_data(path_in, &len);
	uint32_t size;
	char* result = fs_load_pref(path, &size);
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

mp_obj_t mod_rl_array_get(mp_obj_t self_in, mp_obj_t i_in, mp_obj_t j_in) {
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

mp_obj_t mod_rl_array_free(mp_obj_t self_in) {
    mp_check_self(mp_obj_is_type(self_in, &mp_type_rl_array));
    mp_obj_rl_array_t *self = MP_OBJ_TO_PTR(self_in);
		fprintf(stderr, "free %p\n", self);
		rl_array_free(self->array);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rl_array_free_obj, mod_rl_array_free);

STATIC const mp_rom_map_elem_t mod_rl_array_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&mod_rl_array_free_obj) },
    { MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&mod_rl_array_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&mod_rl_array_set_obj) },
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
    //.print = list_print,
    .make_new = mod_rl_array_make_new,
    //.unary_op = list_unary_op,
    //.binary_op = list_binary_op,
    //.subscr = list_subscr,
    //.getiter = list_getiter,
    .locals_dict = (mp_obj_dict_t*)&mod_rl_array_locals_dict,
};

STATIC const mp_rom_map_elem_t mp_module_rl_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rl) },
		/************** rogue_random *********************/
    { MP_ROM_QSTR(MP_QSTR_random_next), MP_ROM_PTR(&mod_rl_random_next_obj) },
    { MP_ROM_QSTR(MP_QSTR_random_int), MP_ROM_PTR(&mod_rl_random_int_obj) },
    { MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mod_rl_random_obj) },
    { MP_ROM_QSTR(MP_QSTR_roll), MP_ROM_PTR(&mod_rl_roll_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_seed), MP_ROM_PTR(&mod_rl_get_seed_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_seed), MP_ROM_PTR(&mod_rl_set_seed_obj) },
		/************** rogue_filesystem *********************/
    { MP_ROM_QSTR(MP_QSTR_open_resources), MP_ROM_PTR(&mod_fs_open_resources_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_app_name), MP_ROM_PTR(&mod_fs_set_app_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_asset), MP_ROM_PTR(&mod_fs_load_asset_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_pref), MP_ROM_PTR(&mod_fs_load_pref_obj) },
    { MP_ROM_QSTR(MP_QSTR_save_pref), MP_ROM_PTR(&mod_fs_save_pref_obj) },
		/************* rogue_array ********************/
		{ MP_ROM_QSTR(MP_QSTR_Array), MP_ROM_PTR(&mp_type_rl_array) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rl_globals, mp_module_rl_globals_table);

const mp_obj_module_t mp_module_rl = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_rl_globals,
};

#endif //PYROGUE_MOD_RL
