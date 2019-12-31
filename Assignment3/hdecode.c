#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#define SYSTEMSIZE 4096



typedef struct node{    /*Node structure for storing tree/list nodes & data*/ 
    unsigned char letter;   /*the character that this node is created for*/
    int freq;               /*how many times this char appears in a file*/

    struct node *next;   /*store next for linked list-ness*/

    struct node *leftchild; /*lefat and right nodes for tree-ness */
    struct node *rightchild;
     
    char *huffcode;   /*determined huffcode*/
}letter_node;

typedef int (*compare)(const void *, const void *); /*typedef for qsort*/

             
int compare_frequencies(void *left, void *right);
letter_node *add_node_list(letter_node newNode, letter_node *head);
int get_bits(const uint8_t byte, int bit);
letter_node *add_super_node(letter_node super, letter_node *head);
letter_node *remove_first(letter_node *head);
letter_node *build_huff_tree(letter_node *head);
letter_node *add_letter(letter_node *head, letter_node *root, 
                        char *decoded, int bit, int fd);
void destroytree(letter_node *root);

int string_counter = 0;/*counts the total chars added*/

int main(int argc, char **argv){
    int rfd, wfd, i, j, l, read_check, read_check2, count, bit, wcheck;
    uint8_t index, *byte_array; 
    uint32_t unique, frequency;
    letter_node *tree, *root;
    char *dstring;
    compare Comp;


    letter_node nodeArray[256];  /*Initialize an array for all possible chars*/

    int total = 0;
   
    if(argc < 2){
        rfd = STDIN_FILENO;
        wfd = STDOUT_FILENO;
    }
    else if(argc > 3){
        fprintf(stderr, "Usage: ./hedecode [(infile|-) [outfile]]");
        exit(2);
    }
    else if(argc >= 2){
        if(argc == 2 && *(argv[1]) != '-'){
            rfd = open(argv[1], O_RDONLY);
            wfd = STDOUT_FILENO;
        }
        else{
            rfd = open(argv[1], O_RDONLY);
            wfd = open(argv[2], O_WRONLY | O_CREAT, 0644);/*gve permissions*/
        }
        if(*(argv[1]) == '-'){
            rfd = STDIN_FILENO;
        }
    }
    if(rfd == -1){
        fprintf(stderr, "Read: File not found\n");
        exit(4);
    }
    /*decoding part*/

    /*parse through the header and establish a new array of 256 letter nodes*/
    
    read_check = read(rfd, &(unique), 4);

    if(unique == 0){/*no characters/empty file*/
        exit(8);
    }
    if(read_check == -1){
        perror("read(2)");
        exit(4);
    }

    for(i=0;i<unique;i++){/*populate the nodes with letters/freqs from
                            header*/
        read_check = read(rfd, &(index), 1);
        nodeArray[i].letter = index;
        read_check2 = read(rfd, &(frequency), 4);
        nodeArray[i].freq = frequency;
        nodeArray[i].leftchild = NULL;
        nodeArray[i].rightchild = NULL;
        total += frequency;
        if(read_check == -1 || read_check2 == -1){
            perror("read(2)");
            exit(4);
        }
    }

    if(unique == 1){/*edge case for 1 char (no encoded string)*/
        dstring = malloc(total+1);
        for(i=0;i<total;i++){
            dstring[i] = index;
        }
        dstring[total] = '\0';
        wcheck = write(wfd, dstring, total);
        if(wcheck == -1){
            perror("write(2)");
            exit(8);
        }
        close(wfd);
        close(rfd);
        free(dstring);
    }

    else{

        /*sort the array of nodes*/
        Comp = (compare)compare_frequencies;
        qsort(nodeArray, unique, sizeof(letter_node), Comp);
    
        /*rebuild tree by sorting and creating nodes*/
        tree = NULL;
    
        for(j=0;j<unique;j++){
            tree = add_node_list(nodeArray[j], tree);  
        }

        tree = build_huff_tree(tree);

        byte_array = calloc(1, sizeof(uint8_t));

        root = tree;
        dstring = malloc(1);

        while(1){        
        /*1. using the bytes, traverse tree until hit leaf
        *2. then print letter from leaf to a final string*/
            count = read(rfd, byte_array, 1);
            
            l = 0;
            while(l<8){
                bit = get_bits(*byte_array, l);
                tree = add_letter(tree, root, dstring, bit, wfd);
                if(string_counter >= total){
                    break;
                }
                l++;
            }
            if(count == 0){
                break;
            }   
             
        }
        /*free everything when you are done*/   
        free(byte_array);
        destroytree(root);
        free(root); 
        close(rfd);
        close(wfd);
        free(dstring);
    }
    return 1;
}

int compare_frequencies(void *left, void *right){ /*comparison by frequencies*/
    return(((const letter_node *)left)->freq)    
                -(((const letter_node *)right)->freq);    
}
    
int get_bits(const uint8_t byte, int bit){/*get a certain bit from a byte*/
    uint8_t newbyte;
    uint8_t mask = 0x80 >> bit;
    newbyte = byte & mask;
    return (newbyte >> (7-bit)); 
}

letter_node *add_node_list(letter_node newNode, letter_node *head){
    if(!head){
        head = malloc(sizeof(letter_node));
        *head = newNode;
        head->next = NULL;
    }
    else{
        letter_node *prev;
        letter_node *newList;
        newList = head;
        prev = head;
        while(head != NULL && (head->freq <= newNode.freq)){
            prev = head;
            head = head -> next;
        }
        newNode.next = head;
        prev->next = malloc(sizeof(letter_node));
        *(prev->next) =  newNode;
        head = newList;
    }

    return head;
}

letter_node *add_super_node(letter_node super, letter_node *head){
    if(head == NULL){
        head = malloc(sizeof(letter_node));
        *head = super;
        head -> next = NULL;
    }
    else{
        letter_node *prev;
        letter_node *newList;
        newList = head;
        prev = head;
        while(head != NULL && (head->freq < super.freq)){
            prev = head;
            head = head -> next;
        }
        if(prev == head){
            super.next = head;
            head = malloc(sizeof(letter_node));
            *head = super;
        }
        else{
            super.next = head;
            prev->next = malloc(sizeof(letter_node));
            *(prev->next) = super;
            head = newList;
        }
    }
    return head;
}

letter_node *build_huff_tree(letter_node *head){ /*build up the tree
                                                   same as hencode*/
    letter_node *left, *right, top;
    while(head->next != NULL){
        left = head;
        head = head -> next;
        right = head;
        head = head -> next;

        top.freq = left->freq + right->freq;
        top.huffcode = NULL;
        top.leftchild = left;
        top.rightchild = right;

        head = add_super_node(top, head);
    }
    return head;
}

letter_node *add_letter(letter_node *head, letter_node *root, /*write to file*/
                       char *decoded, int bit, int fd){
    int wcheck;
    if(!head->rightchild && !head->leftchild){/*if leaf then add to file*/
        *decoded = head->letter;
        wcheck = write(fd, decoded, 1);
        if(wcheck == -1){
            perror("write(2): ");
            exit(4);
        }
        head = root;
        string_counter++;
    }
    if(bit == 1){ /*if 1 then go left*/
        head = head->rightchild;
    }
    else{ /*if 0 go right*/
        head = head->leftchild;
    }

    return head; /*new head*/
}

void destroytree(letter_node *root){/*recursively get rid of tree*/
    if(root->leftchild){
        destroytree(root->leftchild);
        free(root->leftchild);
    }

    if(root->rightchild){
        destroytree(root->rightchild);
        free(root->rightchild);
    }
}


