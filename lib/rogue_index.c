#include "rogue_index.h"

index_t* rl_index_new(int width, int height, int cell_size) {
	index_t* index = calloc(sizeof(index_t), 1);
	index->width = width / cell_size;
	index->height = height / cell_size;
	index->cell_size = cell_size;
	index->cells = calloc(sizeof(index_cell_t), index->width * index->height);
	return index;
}

void rl_index_free(index_t* index) {
	for(int i = 0; i < index->width * index->height; i++)
		free(index->cells[i].elements);
	free(index->cells);
	free(index->element_cell);
	free(index->element_index_in_cell);
}

int rl_index_add(index_t* index, int x, int y) {
	int cell_x = x / index->cell_size;
	int cell_y = y / index->cell_size;
	if(cell_x < 0) cell_x = 0;
	if(cell_x >= index->width) cell_x = index->width - 1;
	if(cell_y < 0) cell_y = 0;
	if(cell_y >= index->width) cell_y = index->width - 1;
	int cell_index = cell_y * index->width + cell_x;
	int object_id = index->num_elements;
	index->element_cell = realloc(index->element_cell, sizeof(int) * (index->num_elements + 1));
	index->element_cell[object_id] = cell_index;
	index_cell_t* cell = &index->cells[cell_index];
	cell->elements = realloc(cell->elements, sizeof(index_element_t) * (cell->num_elements + 1));
	index_element_t* element = &cell->elements[cell->num_elements];
	index->element_index_in_cell = realloc(index->element_index_in_cell, sizeof(int) * (index->num_elements + 1));
	index->element_index_in_cell[object_id] = cell->num_elements;
	element->object_id = object_id;
	element->x = x;
	element->y = y;
	cell->num_elements++;
	index->num_elements++;
	return object_id;
}

int rl_index_find(const index_t* index, const int x, const int y, const int distance, int** found) {
	int num_found = 0;
	for(int j = (y - distance) / index->cell_size; j <= (y + distance) / index->cell_size; j++) {
		for(int i = (x - distance) / index->cell_size; i <= (x + distance) / index->cell_size; i++) {
			if(i >= 0 && i < index->width && j >= 0 && j < index->height) {
				index_cell_t* cell = &index->cells[j * index->width + i];
				for(int element = 0; element < cell->num_elements; element++) {
					int dx = (x - cell->elements[element].x);
					int dy = (y - cell->elements[element].y);
					int object_distance = dx * dx + dy * dy;
					if(object_distance < distance * distance) {
						*found = realloc(*found, sizeof(int) * (num_found + 1));
						(*found)[num_found] = cell->elements[element].object_id;
						num_found++;
					}
				}
			}
		}
	}
	return num_found;
}

void rl_index_remove(index_t* index, int object_id) {
	if(object_id < 0 || object_id >= index->num_elements) return;
	index_cell_t* cell = &index->cells[index->element_cell[object_id]];
	int remove_index = index->element_index_in_cell[object_id];
	if(remove_index < cell->num_elements - 1) {
		int moved_object_id = cell->elements[cell->num_elements - 1].object_id;
		cell->elements[remove_index] = cell->elements[cell->num_elements - 1];
		index->element_index_in_cell[moved_object_id] = remove_index;
	}
	cell->num_elements--;
}

void rl_index_move(index_t* index, int object_id, int new_x, int new_y) {
	/*if(object_id < 0 || object_id >= index->num_element) return;
	index_cell_t* cell = &index->cells[index->element_cell[object_id]];
	
	int remove_index = index->element_index_in_cell[object_id];
	if(remove_index < cell->num_elements - 1) {
		int moved_object_id = cell->elements[cell->num_elements - 1].object_id;
		cell->elements[remove_index] = cell->elements[cell->num_elements - 1];
		index->element_index_in_cell[moved_object_id] = remove_index;
	}
	cell->num_elements--;*/
}


