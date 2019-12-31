#include <stdio.h>
#define TABS 8

int main(void){
    int chars;       /*total number of characters in the current line*/
    int tabspace;    /*variable used to calculate how
                     many tabs are needed to the end of the tab stop*/
    int c;           /*a character c*/
    int pos;         /*position of cursor, resets to 0 every 8 spaces*/ 
    tabspace = 0;
    chars = 0;
    pos = 0;

    while((c = getchar()) != EOF){   /*This loop makes sure the end
                                     of the file isn't passed */
        if(pos == 8){      /*makes sure position does not
                             go over 8 spaces or under 0*/
            pos = 0;
        }

        else if(pos < 0){
            pos = 7;
        }        
        tabspace = TABS - pos;	/*calculates how many spaces are needed to
                                 reach the end of the tab stop*/
        if(c == '\t'){          /*checks for tab character then replaces
                                 it with the correct number of spaces*/
            while (tabspace > 0){
                putchar(' ');
                chars++;        /*increment chars for total, and decrement
                                 tabspace*/
                tabspace--;
            }
            pos = 0;
        }

        else if (c == '\n' || c == '\r'){    /*checks for newline and carriage
                                             return and if there is one resets
                                              pos*/
            pos = 0;
            chars = 0;
			putchar(c);
		}

		else if (c == '\b'){    /*checks for backspace and if there
                                  one, and there are no more characters
                                  in the file, ends the program*/
		    if (chars > 0){
			    chars--;
                if (pos < 0){
                    pos = 0;
                }
				else{
                    pos--;    
                }
				putchar(c);
			}
			else{
		        putchar(c);
				chars = 0;
				pos = 0;
			}
		}

        else{                   /*otherwise, the file is copied as is*/
            putchar(c);
            pos++;              /*position and total characters in line are
                                 incremented each time*/
            chars++;
        }
    }
    return 0;                   /*end when done*/
}
