#ifndef HASH
#define HASH
typedef struct words words;

typedef struct table table;

void inc_unique();
void dec_unique();
int get_unique();
int unique;
table *create_table();
words **hash_to_array(table *hashtable);
int addWord(const char *pointWord, table *hashtable, int frequency);

#endif
