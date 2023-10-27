#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "ku_input.h"

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("usage : %s <integet>\n", argv[0]);
		return 1;
	}

	int value = atoi(argv[1]);
	printf("child process num : %d\n", value);

	int layer_num = N-3+1;

	//printing last result matrix
	int resultMatrix[layer_num*layer_num];

	
	//ready for pipe
	int pipefd[2];
	//this pipe is for return
	int pipere[2];
	

	//ready for fork
	pid_t pid[value];
	int child_status;

	int filter[3][3] = {{-1,-1,-1},
			{-1,8,-1},
			{-1,-1,-1}};

	//make value child
	//for(int i=0;i<value;i++){

		if(pipe(pipefd)==-1){
                	perror("fd pipe");
        	        exit(EXIT_FAILURE);
	        }
		if(pipe(pipere)==-1){
                        perror("re pipe");
                        exit(EXIT_FAILURE);
                }

	//}

	for(int i=0;i<value;i++){

		//ready for pipe
	   //     int pipefd[2];
       		 //this pipe is for return
        //	int pipere[2];
		
	/*	if(pipe(pipefd)==-1){
                        perror("fd pipe");
                        exit(EXIT_FAILURE);
                }
                if(pipe(pipere)==-1){
                        perror("re pipe");
                        exit(EXIT_FAILURE);
                }
*/

		if((pid[i] = fork()) == 0){
			

			if(i <= layer_num*layer_num){
				//read pipe
                        close(pipefd[1]);
                        int buffer[9];
                        ssize_t byteRead;

                        byteRead = read(pipefd[0], &buffer, sizeof(buffer));
                        if(byteRead<0){
                                perror("pipe read error");
                                return 1;
                        }
                        for(int i=0;i<9;i++){
                                printf("childprocess %d read data %d\n", getpid(), buffer[i]);
                        }



                        printf("\n");


                        int intvalue[9];
                        for(int x=0;x<9;x++){
                                intvalue[x] = buffer[x];
                        }

                        int sum=0;
                        for(int x=0;x<3;x++){
                                for(int y=0;y<3;y++){
                                        sum += intvalue[(x*3)+y]*filter[x][y];
                                }
                        }
//write pipere

                        close(pipere[0]);
                        write(pipere[1], &sum, 4 );

                        close(pipefd[0]);
                        close(pipere[1]);
                        exit(0);

			//end of when i == layer_num*layer_num
			}else if(i>layer_num*layer_num){
				//printf("no data to read or write in child");
				exit(0);
			}
		}                           
		else{
			if(i <= layer_num*layer_num){
				//int is 4byte we have to send 9bit so 3 is cool
                        int intvalue[9];
                        //change ku_input's input value to char[] (first argv is 16)
                        //
                        int x=i/layer_num;
                        int y=i%layer_num;
                        int num=0;
                        for(int o=0;o<3;o++){
                                for(int q=0;q<3;q++){
                                        intvalue[num] = input[x][y];
                                        num++;
                                        y++;
                                }
                                x++;
                                y = y-3;
                        }
                        


                        close(pipefd[0]);
                        write(pipefd[1], &intvalue ,sizeof(intvalue));
                        

                        //read pipere
                        int data;
                        ssize_t byteRead;

                        close(pipere[1]);

                        byteRead = read(pipere[0], &data, 4);
                        if(byteRead < 0){
                                perror("pipere read error");
                                return 1;
                        }
                        printf("parent process read data %d\n", data);
                        printf("\n");


                        resultMatrix[i] = data;

                        close(pipefd[1]);
                        close(pipere[0]);	
			}else if(i > layer_num*layer_num){
				//printf("no data to read or write in parent");
			}
			
		}
	}	

	for(int j=0;j<value;j++){
		//write pipe
		wait(NULL);
	}
	
	for(int i=0;i<layer_num;i++){
		for(int j=0;j<layer_num;j++){
			printf("%d ",resultMatrix[(layer_num*i)+j]);

		}
		printf("\n");
	}

	return 0;
}
