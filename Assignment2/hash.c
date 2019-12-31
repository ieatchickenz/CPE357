#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define TSIZE 10000

struct words{
    char *word;
    int freq;        
};

struct table{
    words **entries;    
}

table *resize_hash(table *hashtable);

size_t size;
size_t size = 0 + TSIZE;

table *create_table(){
    int i;
    table *hashtable = malloc(sizeof(table)*1);

    hashtable -> entries = malloc(sizeof(words*) * size);

    for(i=0; i < size; i++){
        hashtable -> entries[i] =  NULL;
    }
    
    unique = 0;

    return hashtable;
}

size_t hash(const char *pointWord){ /*returns an index for a string key*/
    int i;
    size_t place = 0;
    int strlength = strlen(pointWord); /*use a hash function with a prime*/
    for(i = 0; i < strlength; i++){   /*add each letter in word*/
        place = (place*37) + pointWord[i]; 
    }
    return place % size;    /*mod to get an index actually in table*/ 
}

words *makeWords(const char *pointWord){
    words *temp = malloc((sizeof(words*) * 1));
    temp -> word = malloc(strlen(pointWord) + 1);
        
    if((temp -> word) == NULL){
        temp = NULL;
    }
    else{
        strcpy(temp->word, pointWord);
        temp -> freq = 0;
    }
    return temp;     
} 

int addWord(const char *pointWord, table *hashtable, int frequency){
    words *p;
    size_t index = hash(pointWord);
    
    p = hashtable -> entries[index];

    if(p != NULL && ((strcmp(p->word, pointWord))==0)){  /*increment
                                                           freq for same
                                                           word*/
        if(p -> word == NULL){
            return 1;
            } 
        p -> freq += 1;
        return 1;
    } 
    else if(p != NULL && ((strcmp(p -> word, pointWord))!=0)){ /*collision*/
        while((hashtable -> entries[index]) != NULL){
            index += 1;              /*linear probing don't go over size*/
            if(index >= (size - 1)){
                index = 0;
            }
        }       
        p = hashtable -> entries[index];
    }    

    if (p == NULL){             /*make a new word freq key pair if none*/

        hashtable->entries[index] = makeWords(pointWord);
        if(hashtable->entries[index] == NULL){
            return 1;
        }

        if(frequency == 0){
            (hashtable -> entries[index] -> freq) += 1;
        }
        else{
             (hashtable -> entries[index] -> freq) = frequency;    
        }

        if ((unique/size) > 0.7){
            hashtable = resize_hash(hashtable);    
        }
    }
    return 0;
}



table *resize_hash(table *hashtable){
    int i;
    table *newHash;
    size_t oldSize = size;
    size += TSIZE;
    
    newHash = create_table(size);

    /*copy all pairs from old table into new table*/ 
    unique = 0;
    for(i=0; i < oldSize; i++){
        words *p = hashtable -> entries[i];
        addWord(p -> word, newHash, p -> freq);
        free(p);
    }

    /*free all memory held by original table*/
    free(hashtable -> entries);
    hashtable = newHash;
    free(newHash);
    return hashtable;
}

int get_unique(){
    return unique;    
}

void inc_unique(){
    unique += 1;    
}

void dec_unique(){
    unique -= 1;
}

words **hash_to_array(table *hashtable){
    words **list;
    int i;
    int s = 0;
    if(NULL == (list = malloc((unique + 1) * sizeof(words*)))){
        perror("malloc");
        exit(7);
    }
    
    for(i=0; i<size; i++){
        if (hashtable -> entries[i] != NULL){
            list[s] = (hashtable -> entries[i]);
            s++;
        } 
    }
    return list;
}

