#ifndef __ROGUE_INDEX_H__
#define __ROGUE_INDEX_H__

typedef struct {
	int object_id;
	int x;
	int y;
} index_element_t;

typedef struct {
	int num_elements;
	index_element_t* elements;
} index_cell_t;

/* spatial index */
typedef struct {
	int width, height, cell_size;
	int* element_cell;
	int* element_index_in_cell;
	int num_elements;
	index_cell_t* cells;
} index_t;

index_t* rl_index_new(int width, int height, int cell_size);
void rl_index_free(index_t* index);
int rl_index_add(index_t* index, int x, int y);
int rl_index_find(const index_t* index, const int x, const int y, const int distance, int** found);
void rl_index_remove(index_t* index, int object_id);
void rl_index_move(index_t* index, int object_id, int new_x, int new_y);

#endif
