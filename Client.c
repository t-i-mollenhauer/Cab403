#define _GNU_SOURCE
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <stdbool.h>
#include <time.h> 

    #define MAXSIZE 1024
    #define MAXLEN 20
    #define MAXSIZE1 2048
    #define LOGIN 0
    #define MENU 1
    #define PLAY_GAME 2
    #define PLAY_HANGMAN 1
    #define SHOW_LEADERBOARD 2
    #define ENDGAME 3


int count_word_length(char * word) {
	int i = 0;
	char dash = '_';
	int counter = 0;
	while(word[i] !='\0') {
		if (word[i] == dash) {
			counter = counter + 1;
		}
		i++;
	}
	return counter;
}

int calculate_guesses(int wordlength) {
	int totalguess = wordlength + 10;
	if(totalguess >= 26) {
		return 26;
	}else {
		return totalguess;
	}
}

bool Game_over(char * word, int number_of_guesses)
    {
        int i = 0;
        char test = '_';
        bool gameover = true;

        /* Run till end of string */
        while(word[i] != '\0')
        {
            /* If occurrence of character is found */
            if(word[i] == test)
            {   
                gameover = false;
            }

            i++;
        }

        if (number_of_guesses == 0) {
        	gameover = true;
        }
        return gameover;
    }

    int main(int argc, char *argv[])
    {
        struct sockaddr_in server_info;
        struct hostent *he;
        int socket_fd,num, numbytes, current_status, menu_selection;
        char buffer[MAXSIZE];
        char username[MAXSIZE];
        char *word;
        char password[MAXSIZE];
        bool game_end;
        char guesses[30] = "";
        int wordcount;
        int number_of_guesses;
        char *username_password;
        int PORT;

        if (argc != 3) {
            fprintf(stderr, "Usage: hostname, port number\n");
            exit(1);
        }

        if ((he = gethostbyname(argv[1]))==NULL) {
            fprintf(stderr, "Cannot get host name\n");
            exit(1);
        }

        if (argv[2] == NULL) {
            fprintf(stderr, "Please provide port number\n");
        }

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
            fprintf(stderr, "Socket Failure!!\n");
            exit(1);
        }
        PORT = atoi(argv[2]);
        memset(&server_info, 0, sizeof(server_info));
        server_info.sin_family = AF_INET;
        server_info.sin_port = htons(PORT);
        server_info.sin_addr = *((struct in_addr *)he->h_addr);
        if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr))<0) {
            perror("connect");
            exit(1);
        }
        current_status = LOGIN;

     while(1) {

        if (current_status == LOGIN){
            printf("Welcome to hangman!\n");
            printf("Client: Please enter your username: ");
            fgets(username,MAXSIZE-1,stdin);
            strtok(username, "\n");
            printf("Client: Please enter you password: ");
            fgets(password,MAXLEN-1,stdin);

            username_password = malloc(strlen(username)+strlen(password));
     
            strcpy(buffer, username);
            strcat(buffer, password);

            buffer[strlen(buffer) -1] = '\0';
            int test = 0;

            if ((send(socket_fd,buffer, strlen(buffer),0))== -1) {
                    fprintf(stderr, "Failure Sending Message\n");
                    close(socket_fd);
                    exit(1);
            }
            else {
                    num = recv(socket_fd, buffer, sizeof(buffer),0);
                    if ( num <= 0 )
                    {
                            printf("Either Connection Closed or Error\n");
                            //Break from the While
                            break;
                    }

                    //buff[num] = '\0';
                    

                    //tests response from server
                    char *test = "Username or password is correct";
                    int rc = strncmp(test, buffer, MAXLEN);
                    if (rc == 0){
                        printf("Client:Message Received From Server -  %s\n",buffer);
                        current_status = MENU;
                        bzero(buffer, MAXSIZE-1);
                        
                    }
                    else{
                        printf("Username or password is incorrect\n");
                        break;
                    }
               }
           }  /// end of current status 0


           //Main Menu
        if (current_status == MENU){
            printf("\nPlease enter a selection\n");
            printf("<1> Play Hangman\n");
            printf("<2> Show Leaderboard\n");
            printf("<3> Quit\n");
            printf("\n");
            printf("Enter Selection: ");
            fgets(buffer,MAXSIZE-1,stdin);
            buffer[strlen(buffer) -1] = '\0';
            menu_selection = atoi(buffer);
            while((menu_selection != PLAY_HANGMAN) && (menu_selection != SHOW_LEADERBOARD) && (menu_selection != ENDGAME)) {
                printf("\n Invalid Selection, Try again: ");
                fgets(buffer,MAXSIZE-1,stdin);
                buffer[strlen(buffer) -1] = '\0';
                menu_selection = atoi(buffer); 
            }
            /// add error handling 


            if ((send(socket_fd,buffer, strlen(buffer),0))== -1) {
                    fprintf(stderr, "Failure Sending Message\n");
                    close(socket_fd);
                    exit(1);
            }
            else {  

                    if (menu_selection == ENDGAME) {
                        printf("\nExiting...\n");
                        close(socket_fd);
                        free(username_password);
                        free(word);
                        break; 
                    }

                    num = recv(socket_fd, buffer, sizeof(buffer),0);
                    if ( num <= 0 )
                    {
                        printf("Either Connection Closed or Error\n");
                        //Break from the While
                        break;
                    }

                    //buff[num] = '\0';
                    
                    
                    if (menu_selection == PLAY_HANGMAN){
                    word = malloc(strlen(buffer));
                    strcpy(word, buffer);
                    wordcount = count_word_length(word);
                    number_of_guesses = calculate_guesses(wordcount);
                    current_status = PLAY_GAME;
                    }
                       

                    if (menu_selection == SHOW_LEADERBOARD){
                         printf("\nPlayer     - %s\n", username);
                         printf("Number of games won     - ");
                        for (int i = 1; i < 3; i++){
                            printf("%c", buffer[i]);
                        }
                        printf("\n");
                        printf("Number of games Played     - ");
                        for (int i = 4; i < 6; i++){
                            printf("%c", buffer[i]);
                        }
                        printf("\n");
                    } 
               }
           }  /// end of current status 1
        // Start of current status 2
           if (current_status == PLAY_GAME){
            printf("=====================================================================\n");
            printf("\n");
            printf("\n");
            printf("\n");
            printf("Number of Guesses Remaining: %d\n", number_of_guesses);
            printf("\n");
            printf("Guessed Letters: %s\n", guesses);
            printf("\n");
            printf("Current word =   %s\n", word);
            printf("\n");
            printf("\n");
            printf("Please enter you guess ");
            fgets(buffer,MAXSIZE-1,stdin);
            buffer[strlen(buffer) -1] = '\0';
            strcat(guesses, buffer);
            /// add error handling 


            if ((send(socket_fd,buffer, strlen(buffer),0))== -1) {
                    fprintf(stderr, "Failure Sending Message\n");
                    close(socket_fd);
                    exit(1);
            }
            else {  

                    num = recv(socket_fd, buffer, sizeof(buffer),0);
                    if ( num <= 0 )
                    {
                            printf("Either Connection Closed or Error\n");
                            //Break from the While
                            break;
                    }

                    //buff[num] = '\0';
                    strcpy(word, buffer);
                    number_of_guesses = number_of_guesses - 1;
                    game_end = Game_over(word, number_of_guesses);

                    if (game_end == true){

                        printf("the game is over -  %s\n",buffer);
                        current_status = 1;
                		bzero(guesses, 30);
                        bzero(buffer, 1000);
                        
                        
                    }


               }
           }  /// end of current status 1
        }///// end of while loop
        close(socket_fd);
        exit(1);
    }//End of main