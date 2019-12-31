#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "read_word.h"
#include "hash.h"



int compare_word_freq(void *left, void *right);
table *read_to_table(FILE *file);
FILE *try_open_file(char *arg);
int print_table_args(char *arg, int num);
int print_stdin(FILE *file, int num);

typedef int (*compare)(const void *, const void *);

int main(int argc, char **argv){
    int n;      /*number of times to print*/

    if(argc == 1){
        n = 10;
        print_stdin(stdin, n);
    }

    if(argv[1][0] == '-' && argv[1][1] == 'n' && (strlen(argv[1]) == 2)){
        int i;
        if (isalpha(argv[2])){
            perror("usage: fw [-n num] [ file1 [file2 ...] ]");
            exit(4);    
        }
        else{
            n = strtol(argv[2], NULL, 10);
        }
    
        for(i=3;i < argc;i++){
            print_table_args(argv[i], n);
        }
        if (argc == 3){
            print_stdin(stdin, n);
        }
    }
        
    else{
        int j;
        n = 10;
        for(j=1;j<argc;j++){
            print_table_args(argv[j], n);
        }
    }
    return 0;
}

table *read_to_table(FILE* file){
    table *wordTable = create_table();

    while(!feof(file)){
        char *word = read_long_word(file);
        if (addWord(word, wordTable, 0) == 0){
            inc_unique();
        free(word);
        }
    }
    dec_unique();
    return wordTable;
}


FILE *try_open_file(char *arg){
    FILE *file;
    file = fopen(arg, "r");
    if (file == NULL){
        perror("cannot open file: ");
    }
    return file;
}


int compare_word_freq(void *left, void *right){
    return ((*(const words **)right)->freq) - ((*(const words **)left)->freq);        
}

int print_table_args(char *arg, int num){
    table *WordHash;
    words **line;
    FILE *file;
    int j;
    int k;
    int u;
    compare Comp;
    file = try_open_file(arg);
    if(file != NULL){
        WordHash = read_to_table(file);
        u = get_unique();
        line = hash_to_array(WordHash);
        Comp = (compare)compare_word_freq;
        qsort(line, u, sizeof(words*), Comp);
        if(num > u){
            num = u;
        }
        printf("The top %d words (out of %d) in %s are: \n",
                num, u, arg);
        for(j=0;j<num;j++){
            printf("\t");
            printf("%d", line[j] -> freq);
            printf(" ");
            printf("%s\n", line[j] -> word);    
            }
        free(WordHash);
        for(k = 0;k<u;k++){
            free(line[k]->word);    
        }
        free(line);
        fclose(file);
    }
    return 0;
}

int print_stdin(FILE *file, int num){
    /*print out a table from stdin*/
    table *WordHash;
    words **line;
    int j;
    int k;
    compare Comp;
    int u = get_unique();
    WordHash = read_to_table(stdin);
    line = hash_to_array(WordHash);
    Comp = (compare)compare_word_freq;
    qsort(line, unique, sizeof(words*), Comp);
    printf("The top %d words (out of %d) in %s are: \n",
            num, u, "stdin");
    for(j=0;j<num;j++){
       printf("\t");
       printf("%d", line[j] -> freq);
       printf(" ");
       printf("%s\n", line[j] -> word);    
    }
    free(WordHash);
    for(k=0;k<u;k++){
        free(line[k]->word); 
    }
    free(line); 
    return 0;       
}

