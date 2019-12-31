#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#define MAXCODESIZE 255  /*this is the max possible length
                            of the encoded char*/
#define SYSTEMSIZE 10000

typedef struct node{    /*Node structure for storing tree/list nodes & data*/
  unsigned char letter;   /*the character that this node is created for*/
  int freq;               /*how many times this char appears in a file*/

  struct node *next;   /*store next for linked list-ness*/

  struct node *leftchild; /*lefat and right nodes for tree-ness */
  struct node *rightchild;

  char *huffcode;   /*determined huffcode*/
}letter_node;

typedef int (*compare)(const void *, const void *); /*typedef for qsort*/

letter_node freqCount[256];   /*Initialize an array for all possible chars
                                as well as a pointer for linked list
                                and an int for unique characters*/
letter_node *sorted;
int u;

int update_frequencies(int fd);
int compare_frequencies(void *left, void *right);
void add_node_list(letter_node newNode);
void add_super_node(letter_node super);
letter_node *remove_first();
letter_node *build_huff_tree();
void code_array(letter_node *root, char *huffcodes, int place);
void unsort_array();
void destroy_tree(letter_node *root);
void free_list(letter_node *list);
void write_file(int wfileDescrip, int rfileDescrip, char *huff);
char *generate_huffstring(int fileDesrip);

int main(int argc, char **argv){ 
    int rfd, wfd, i, unique;/*file descriptor and int for looping*/
    char *encoded_string;

    char *code = malloc(MAXCODESIZE);  /*these are initialized buffer and index
                                         for the code_array function*/
    int place = 0;

    if(argc < 2 || argc > 3){  /*check for right ammount of args*/
        fprintf(stderr, "usage: ./hencode infile [outfile]\n");
        exit(8);
    }

    if(argc >= 2){   /*set the read and write file descriptors*/
        rfd = open(argv[1], O_RDONLY);
        if(rfd < 0){
            fprintf(stderr, "The input file does not exist");
            exit(9);
        }
        if(argc == 3){
            wfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        }
        else{
            wfd = STDOUT_FILENO;
        }
    }
    
    unique = update_frequencies(rfd);
    if(unique == 0){
        write(wfd, &unique, 4);
        close(wfd);
        close(rfd);
        exit(7);
    }
    
    sorted = build_huff_tree();

    unsort_array();  /*unsort array to preserve huffcodes and location*/
    
    code_array(sorted, code, place);
    
    free(code); 

  
    /*encode the string*/
    encoded_string = generate_huffstring(rfd);

    /*Find a way to write the byte file here
     * re-read the input using lseek at the end of the byte file
     * return the file to either stdout or the given file*/  
    write_file(wfd, rfd, encoded_string);


    for(i =0; i < 256; i++){
        free(freqCount[i].huffcode);
        free(freqCount[i].next);
    }
    destroy_tree(sorted);
    free(sorted);
    free(encoded_string);
    close(rfd);  /*always close*/
    close(wfd);

    return 1;

}



int compare_frequencies(void *left, void *right){ /*comparison by frequencies*/
    return(((const letter_node *)left)->freq)
            -(((const letter_node *)right)->freq);
}

int compare_letter(void *left, void *right){ /*comparison by letter (char)*/
    return(((const letter_node *)left)->letter)
            -(((const letter_node *)right)->letter);
}


int update_frequencies(int fd){ 
                                        /*this function populates the array of 
                                        nodes based on the text read from the
                                        input file. It also creates a linked
                                        list based on the sorted array of
                                         frequencies. Oh, and it sorts the 
                                         array of 256 frequencies*/
    char c;
    unsigned int index;
    int i,j,r;
    compare Comp;
    
    char *buf;

    u = 0;
    buf = malloc(SYSTEMSIZE);

    for(i=0;i<256;i++){
        freqCount[i].letter = (char) i;
    }

    while((r=read(fd,buf,SYSTEMSIZE))!=0){
        for(j=0;j<r;j++){  
            c = buf[j];
            if(c<0){
                index = 256 + c;
            }
            else{
                index = ((unsigned int)c);
            }
            if(freqCount[index].freq == 0){
                u += 1;
            }
            freqCount[index].freq += 1;
        }
    }

    if(u==0){  /*edge case if there is nothing in the file*/
        return u;
    }

    Comp = (compare)compare_frequencies;
    qsort(freqCount, 256, sizeof(letter_node), Comp); /*sort by frequency*/

    i = 256 - u; /*where the first freq will not be 0*/
    
    sorted = NULL;

    while(i<256){
        add_node_list(freqCount[i]);
        i++;
    }
    return u;
}

void unsort_array(){ /*this sorts the array with huffcodes by letter, 
                        effectively unsorting it*/
    compare compLetter;
    compLetter = (compare)compare_letter;
    qsort(freqCount, 256, sizeof(letter_node), compLetter);
}

void add_node_list(letter_node newNode){   /*this function adds a node to 
                                             the linked list*/
    if(!sorted){
        sorted = malloc(sizeof(letter_node));
        *sorted = newNode; 
        sorted->next = NULL;
    }
    else{
        letter_node *prev; 
        letter_node *newList; 
        newList = sorted;
        prev = sorted;
        while(sorted != NULL && (sorted->freq <= newNode.freq)){
            prev = sorted;
            sorted = sorted -> next;
        }
        newNode.next = sorted;
        prev->next = malloc(sizeof(letter_node));
        *(prev -> next) = newNode;
        sorted = newList;
    }
}

void add_super_node(letter_node super){ 
                                        /*this function is meant to add a super
                                           node(a combined frequency)*/
     if(sorted == NULL){
         sorted = malloc(sizeof(letter_node));
         *sorted = super;
         sorted->next = NULL;
     }
     else{
        letter_node *prev; 
        letter_node *newList; 
        newList = sorted;
        prev = sorted;
        while(sorted != NULL && (sorted->freq < super.freq)){
            prev = sorted;
            sorted = sorted -> next;
        }
        if(prev == sorted){
            super.next = sorted;
            sorted = malloc(sizeof(letter_node));
            *sorted = super;
            
        }
        else{
            super.next = sorted;
            prev->next = malloc(sizeof(letter_node));
            *(prev -> next) = super;
            sorted = newList;
        }
     }
}

letter_node *remove_first(){    /*this function returns the first node of a 
                                   linked list and removes it*/
    letter_node *node_back;
    node_back = sorted;
    sorted = (sorted->next);
    return node_back;
}


letter_node *build_huff_tree(){ /*this function builds a huffman tree from
                                   the linked list*/
    letter_node *left, *right, top;
    while(sorted->next != NULL){
        left = remove_first();  /*left and right child*/
        right = remove_first();
        
        top.freq = left->freq + right->freq;   /*make a new node*/
        top.leftchild = left;
        top.rightchild = right;

        add_super_node(top);
    }
    
    return sorted;
}

void code_array(letter_node *root, char *huffcodes, int place){ /*step through
                                                           the tree recursively
                                                           and generate 
                                                           huffcode*/
    if(root->leftchild){
        *(huffcodes+place) = '0';
        code_array(root->leftchild, huffcodes, place+1);  
    }

    if(root->rightchild){
        *(huffcodes+place) = '1';
        code_array(root->rightchild, huffcodes, place+1);
    }

    if(!root->rightchild && !root->leftchild){
        int index = root->letter;
        if(index<0){
            index = 256+index;
        }
        huffcodes[place]='\0';
        freqCount[index].huffcode = malloc(strlen(huffcodes)+1);
        freqCount[index].huffcode =
                  strcpy(freqCount[index].huffcode, huffcodes);
    }
}

void destroy_tree(letter_node *root){/*step through a tree and free it's nodes
                                      */
    if(root->leftchild){
        destroy_tree(root->leftchild);
        free(root->leftchild);
    }

    if(root->rightchild){
        destroy_tree(root->rightchild);
        free(root->rightchild);
    }

    if(!root->rightchild && !root->leftchild){
        free(root->huffcode);
    }
}


void free_list(letter_node *list){/*free items in a linked list*/
    letter_node *next;
    while(list){
        next = list->next;
        free(list->next);
        free(list);
        list = next;
    }
}


void write_file(int wfileDescrip, int rfileDesrip, char *huff){
    uint8_t *code_buf;
    char onezero;
    int count,i,j,w1,w2,w3,buf_size;
    
    count = 4;

    write(wfileDescrip, &u, count);

    /*copy the letters and their frequencies to the written file*/

    j=0;
    for(i=0;i<256;i++){
        if(freqCount[i].freq != 0){
            w1 = write(wfileDescrip, &(freqCount[i].letter),1);
            w2 = write(wfileDescrip, &(freqCount[i].freq), 4);
            if(w1 == -1 || w2 == -1){
                perror("write");
                exit(5);
            }
        }
    }

    /*get the encoded string (*huff) and read/convert to binary*/
    buf_size = strlen(huff)/8;
    code_buf = calloc(buf_size, sizeof(uint8_t));
    
    i = 0;
    j = 0;
    while((onezero = huff[i]) != '\0'){
        if(onezero == '1'){
            code_buf[j] = code_buf[j] | 0x01;
        }
        i++;

        if((i%8) == 0){
            j++;
        }
        code_buf[j] = (code_buf[j] << 1) & 0xFF;
    }
    w3 = write(wfileDescrip, code_buf, buf_size);
        if(w3 == -1){
            perror("write(2)");
            exit(8);
        }

    /*make sure to free all buffers after done using them*/
    free(code_buf);
}

char *generate_huffstring(int fileDescrip){
    char *encoded, *buf;
    int r, index, size, final, i, j;
    char c;
        
    buf = malloc(SYSTEMSIZE);
    encoded = malloc(10);
    *encoded = '\0';
    size = 0;

    /*reset file read pointer to begining to generate hashcode*/
    lseek(fileDescrip, 0, SEEK_SET);

    while((r=read(fileDescrip, buf, SYSTEMSIZE)) != 0){
        for(j=0;j<r;j++){
            c = *(buf+j);
            if(c<0){
                index = 256 + c;
            }
            else{
                index = ((unsigned int)c);
            }
            size += strlen(freqCount[index].huffcode);
            encoded = realloc(encoded, size + 1);

            encoded = strcat(encoded, freqCount[index].huffcode);
        }
    }
    
    if((final = (strlen(encoded)%8))!= 0){ /*buffer final byte with '0's*/
        size = strlen(encoded)+(8-final);
        encoded = realloc(encoded, size+1);
        for(i=final;i<8;i++){
            *(encoded + ((size) - (8-i))) = '0';
        }
        encoded[size] = '\0';
    }
    free(buf);
    return encoded;
}


