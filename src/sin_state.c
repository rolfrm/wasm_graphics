// This file is auto generated by icy-table.
#ifndef TABLE_COMPILER_INDEX
#define TABLE_COMPILER_INDEX
#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))
#include "icydb.h"
#include <stdlib.h>
#endif


sin_state * sin_state_create(const char * optional_name){
  static const char * const column_names[] = {(char *)"id", (char *)"dir", (char *)"freq", (char *)"phase"};
  static const char * const column_types[] = {"square_id", "vec2", "f32", "f32"};
  sin_state * instance = calloc(sizeof(sin_state), 1);
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  
  icy_table_init((icy_table * )instance, optional_name, 4, (unsigned int[]){sizeof(square_id), sizeof(vec2), sizeof(f32), sizeof(f32)}, (char *[]){(char *)"id", (char *)"dir", (char *)"freq", (char *)"phase"});
  
  return instance;
}

void sin_state_insert(sin_state * table, square_id * id, vec2 * dir, f32 * freq, f32 * phase, size_t count){
  void * array[] = {(void* )id, (void* )dir, (void* )freq, (void* )phase};
  icy_table_inserts((icy_table *) table, array, count);
}

void sin_state_set(sin_state * table, square_id id, vec2 dir, f32 freq, f32 phase){
  void * array[] = {(void* )&id, (void* )&dir, (void* )&freq, (void* )&phase};
  icy_table_inserts((icy_table *) table, array, 1);
}

void sin_state_lookup(sin_state * table, square_id * keys, size_t * out_indexes, size_t count){
  icy_table_finds((icy_table *) table, keys, out_indexes, count);
}

void sin_state_remove(sin_state * table, square_id * keys, size_t key_count){
  size_t indexes[key_count];
  size_t index = 0;
  size_t cnt = 0;
  while(0 < (cnt = icy_table_iter((icy_table *) table, keys, key_count, NULL, indexes, array_count(indexes), &index))){
    icy_table_remove_indexes((icy_table *) table, indexes, cnt);
    index = 0;
  }
}

void sin_state_clear(sin_state * table){
  icy_table_clear((icy_table *) table);
}

void sin_state_unset(sin_state * table, square_id key){
  sin_state_remove(table, &key, 1);
}

bool sin_state_try_get(sin_state * table, square_id * id, vec2 * dir, f32 * freq, f32 * phase){
  void * array[] = {(void* )id, (void* )dir, (void* )freq, (void* )phase};
  void * column_pointers[] = {(void *)table->id, (void *)table->dir, (void *)table->freq, (void *)table->phase};
  size_t __index = 0;
  icy_table_finds((icy_table *) table, array[0], &__index, 1);
  if(__index == 0) return false;
  unsigned int sizes[] = {sizeof(square_id), sizeof(vec2), sizeof(f32), sizeof(f32)};
  for(int i = 1; i < 4; i++){
    if(array[i] != NULL)
      memcpy(array[i], column_pointers[i] + __index * sizes[i], sizes[i]); 
  }
  return true;
}

void sin_state_print(sin_state * table){
  icy_table_print((icy_table *) table);
}

size_t sin_state_iter(sin_state * table, square_id * keys, size_t keycnt, square_id * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator){
  return icy_table_iter((icy_table *) table, keys, keycnt, optional_keys_out, indexes, cnt, iterator);

}