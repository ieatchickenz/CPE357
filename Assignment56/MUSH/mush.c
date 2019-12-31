#include "mush.h"

int main(int argc, char *argv[]){
    int check;
    int count = 0;
    char line[LINESIZE];
    stage stages[NUMCMD];
    sigset_t mask;

    FILE *stream = stdin;

    if(argc > 1){
        if((stream = fopen(argv[1], "r")) == NULL){
            perror(argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    while(1) {
        if(get_line(line,LINESIZE,stream) == -1){
            continue;
        }
        if(!strcmp(line,"exit")){
            return 0;
        }
        
        count = get_stages(line,stages);
        if(count == -1)
            continue;
        if( parse_stages(stages,count) == -1){
            continue;
        }
        if(!strcmp(stages[0].argv[0], "cd")){
            if(stages[0].argcount > 2){
                fprintf(stderr, "cd: too many arguments.\n");
                continue;
            }
            if((check = chdir(stages[0].argv[1])) == -1){
                perror(stages[0].argv[1]);
               continue;
            }
        }
        sigemptyset(&mask);
        sigaddset(&mask,SIGINT);
        sigprocmask(SIG_BLOCK,&mask,NULL);
        launch_pipe(count,stages, mask);
        sigprocmask(SIG_UNBLOCK,&mask, NULL);
        memset(stages,0,count * sizeof(stage));
    }
    return 0;
}

void launch_pipe(int count,stage *stages, sigset_t mask){
    /*create pipes then fork children to have copies of pipe list*/
    int i, j, check,rfd, wfd, status;
    int max_pipes = (count-1);
    int fd[18];
    int ind = 0;
    pid_t cpids[10];
    memset(cpids, 0, 10);
    for(i=0;i<max_pipes;i++){
        ind = i*2;
        if((check = pipe(fd+ind)) == -1){
            perror("pipe");
            continue;
        }
    }


    /*from here we need to fork and execute*/
    for(j=0;j<count;j++){
        if((cpids[j] = fork())){ /* this is the parent */
            if(cpids[j] == -1){
                perror("fork");
                break;
            }

        } else { /* this is the child */
            int pipe_write = (j*2) + 1;
            int pipe_read = (j*2) - 2;
            char **args;
            int k, l;
    
            args = malloc(sizeof(char*) * 10);
            
            l=0;
            while(stages[j].argv[l][0] != '\0' &&  l < 10){
                args[l] = malloc(512);
                l++; 
            }
            
            args[l] = NULL;

            for(k=0;k<l;k++){
                strcpy(args[k], stages[j].argv[k]);
            }
    
            /*set up pipes*/
            if(j == 0){
                if(stages[j].in[0] != '\0'){
                    if((rfd = open(stages[j].in, O_RDONLY)) == -1){
                        perror("open fd");
                        printf("PID: %d", getpid());
                        break;
                    }
                    dup2(rfd, STDIN_FILENO);
                    close(rfd);
                }
                if(count > 1){
                    dup2(fd[pipe_write], STDOUT_FILENO);
                    close_fd(fd, max_pipes); /*close all file descriptors??*/
                }
            }
            if(j == count-1){
                if(stages[j].out[0] != '\0'){
                    if((wfd = open(stages[j].out,
                                    O_WRONLY | O_CREAT | O_TRUNC,
                                    0777)) == -1){
                        perror("open");
                        break;
                    }
                    dup2(wfd, STDOUT_FILENO);
                    close(wfd);
                }
                if(count > 1){
                    dup2(fd[pipe_read], STDIN_FILENO);
                    close_fd(fd, max_pipes);
                }
            }
            if(j != 0 && j != count-1){
                dup2(fd[pipe_read], STDIN_FILENO);
                dup2(fd[pipe_write], STDOUT_FILENO);
                /*close all?*/
                close_fd(fd, max_pipes);
            }
            /*unblock SIGINT*/
            fflush(stdout);
            sigprocmask(SIG_UNBLOCK,&mask,NULL);
            /*once the pipe has been set up then execute*/
            if(strcmp(stages[j].argv[0], "cd")){
                execvp(stages[j].argv[0],(char * const *)args);
                perror(stages[j].argv[0]);
                exit(3);
            }
            else{
                free_args(args, l);
                exit(EXIT_FAILURE);
            }

        }
    }
    /*close all file descriptors in parent*/
    close_fd(fd, max_pipes);

    /*wait for children to finish*/
    for(j=0;j<count;j++){
        /*flush stdout because buffered write*/
        waitpid(cpids[j], &status, 0);
       /*need to check status and see how wait exited*/ 
    }
    
}

void close_fd(int fd[], int num_pipes){
    int i;
    for(i=0;i<(num_pipes * 2);i++){
        close(fd[i]);
    }
}

void free_args(char **arguments, int length){
    int i;
    for(i=0;i<length;i++){
        free(*(arguments + i));
    }
    free(arguments);
}
