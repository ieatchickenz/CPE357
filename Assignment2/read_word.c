#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include"read_word.h"
#define BUFFERSIZE 40


char *read_long_word(FILE *file){  /*this function reads a line from a file  
                                     and returns a pointer to that line*/    
    size_t strlength = 0;          /*set the variables for memory size,       
                                     string length, a character, and         
                                     line pointer*/                           
    size_t memsize = 0;
    char *line = NULL;
    int ch = getc(file);
    int i;
                                                                             
    memsize += BUFFERSIZE;         /*allocate initial memory for line*/       
    line = malloc(memsize);                                              
    *(line) = '\0';                 /*don't know what's in here so put \0*/
    
    while((!isalpha(ch) || isspace(ch)) && ch != EOF && ch != '\0'){
        ch = getc(file);    
    }
      
    while( isalpha(ch) && !isspace(ch) && 
           (ch != EOF)){                           /*go through a line until    
                                                     a non char is found/EOF*/
        line[strlength] = ch;                              
        strlength++;
        ch = getc(file);                                                            
        if(strlength >=  memsize){      /*if the size of the string exceeds     
                                          the memory allocated then realloc*/   
            memsize += BUFFERSIZE;                                        
            line = realloc(line, memsize);                                    
        }                     
    }        

    for(i=0; i < strlength; i++){
        line[i] = tolower(line[i]);
    }

    line = (char *)realloc(line, (strlength+1)); /*allocate only the        
                                                   necessary amount and a 
                                                   null terminator*/      
    line[(strlength)] = '\0';                                       
    return line;                /*return pointer to string*/
}
