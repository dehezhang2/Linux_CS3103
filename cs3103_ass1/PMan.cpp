#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <vector>

int main(){
    char buffer[100], cmd[100]; char* args[100];
    int status=0; bool quit = 0; pid_t cpid=1;
    do{
        std::cout<<"PMan: > ";
        std::cin.getline(buffer,100);

        status = waitpid(-1,NULL,WNOHANG)>0?0:status;
        char* temp = strtok(buffer," ");
        if(!temp)continue;

        strcpy(cmd,temp);
        int argc=0;
        temp = strtok(NULL," ");
        while(temp){
            args[argc]=temp;argc++;
            temp = strtok(NULL," ");
        }

        if(strcmp(cmd,"bg")==0){

            if(status!=0){
                std::cerr<<"Error: there is a background process already.\n";
                continue;
            }
            if(!argc){
                std::cerr<<"Error: Name is null.\n";
                continue;
            }
            
            // need or not?
            char file[100]="./";
            strcat(file,args[0]);
            args[0]=file;

            args[argc++]=0;
            cpid = fork();
            if(cpid>0){
                // correct or not?
                std::cout<<"bg: Create new process: "<<cpid<<std::endl;
                usleep(1000);
                status = 1;
            } else if(cpid==0){
                int success = execvp(args[0],args);
                if(success==-1){
                     std::cout<<"Error: Invalid file name\n";
                }
            } else if(cpid<0){
                std::cout<<"fork () failed!\n";
                exit (EXIT_FAILURE);
            }

        } else if(strcmp(cmd,"bgkill")==0){
            if(!(status==1||status==2)){
                std::cout<<"Error: No background process to kill.\n";
                continue;
            }
            pid_t tar = atoi(args[0]);
            if(tar!=cpid) std::cout<<"The pid is not a valid process id of a background process.\n";
            else{
                int fail=kill(tar,SIGTERM);
                if(!fail){
                     std::cout<<tar<<" has been killed\n";
                     cpid = 1; status = 0;
                }
                else std::cout<<"Error: kill() is failed.\n";
            }

        } else if(strcmp(cmd,"bgstop")==0){
            if(status!=1){
                std::cout<<"Error: No running background process to stop.\n";
                continue;
            }
            pid_t tar = atoi(args[0]);
            if(tar!=cpid) std::cout<<"The pid is not a valid process id of a background process.\n";
            else{
                int fail = kill(tar,SIGSTOP);
                if(!fail){
                     std::cout<<tar<<" has been stopped\n";
                     status = 2;
                }
                else std::cout<<"Error: bgstop() is failed.\n";
            }
        } else if(strcmp(cmd,"bgstart")==0){
            if(status!=2){
                std::cout<<"Error: No stopped background process to start.\n";
                continue;
            }
            pid_t tar = atoi(args[0]);
            if(tar!=cpid) std::cout<<"The pid is not a valid process id of a background process.\n";
            else{
                int fail = kill(tar,SIGCONT);
                if(!fail){
                    std::cout<<tar<<" has been started\n";
                    status = 1;
                }
                else std::cout<<"Error: bgstart() is failed.\n";
            }
        } else if(strcmp(cmd,"exit")==0){
            if(status==1||status==2){
                int fail = kill(cpid,SIGTERM);
                if(fail) std::cout<<"Error: kill() is failed.\n";
                else std::cout<<cpid<<" has been killed\n";
            }
            quit=1;
        } else{
            std::cout<<"PMan: > "<<cmd<<": command not found\n";
        }
    } while(!quit&&cpid!=0);
    return 0;
}
// what if the bg process need user input?
// what if bgkill, bgstop, bgstart after the process is ended