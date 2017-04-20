#include "query.h"

void new_edge(void){
  prov_entry_t* elt = (prov_entry_t*)malloc(sizeof(union prov_elt));
  relation_identifier(elt).id = rand();
  relation_identifier(elt).boot_id = 0;
  insert_edge(elt);
}

int main(void){
  int i;
  merge_bundle();
  srand(time(NULL));
  printf("Starting test\n");
  for(i=0; i<100; i++)
    new_edge();
  display_bundle();
  printf("Before merge\n");
  merge_bundle();
  printf("After merge\n");
  display_bundle();
  for(i=0; i<1000; i++)
    new_edge();
  display_bundle();
  printf("Before merge\n");
  merge_bundle();
  printf("After merge\n");
  display_bundle();
}
