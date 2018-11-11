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
#include <sys/wait.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <semaphore.h>
#include <signal.h> 

#define BACKLOG 10
#define Lines 30
#define MaxLen 20



#define LINES 287
#define MAX_USERS 10



typedef struct {
    int         sock_fd;
    bool        connected;
} Client_Info;



static bool             server_running = true; 
static Client_Info      clients_infos[MAX_USERS];       
      
static int              client_fd_sockets_array[MAX_USERS];     
static pthread_mutex_t  nxtposmute;           
static pthread_mutex_t  nxtclientmute;    
static pthread_t        threads[MAX_USERS];     
static sem_t            clienthandler;         
static sem_t            sclient;                 
static int              nextq = 0;         
static int              nxtclient = 0;      

int get_client_from_queue()
{
    pthread_mutex_lock(&nxtclientmute);
    printf("nxtclient: %d\n", nxtclient);
    int client = client_fd_sockets_array[nxtclient];
    nxtclient++;
    nxtclient = nxtclient % MAX_USERS;
    pthread_mutex_unlock(&nxtclientmute);

    return client;
}

void add_client_to_queue(int client_fd)
{
    pthread_mutex_lock(&nxtposmute);
    printf("nextq: %d\n", nextq);
    client_fd_sockets_array[nextq] = client_fd;
    nextq++;
    nextq = nextq % MAX_USERS;
    pthread_mutex_unlock(&nxtposmute);
}

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



bool Game_over(char * word)
    {
        int i = 0;
        char test = '_';
        bool gameover = true;

        printf("%s\n", word);
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
        printf("%s\n", word);
        return gameover;
    }







char *generate_words(){
char c[1000];
    FILE *wordfile;
    int randline;
    int count = 0;
    char *word_return;
    
    //Random number generation
    srand(time(0));
    randline = rand()%(LINES + 1 - 0) + 0;

    //Opens file
    if ((wordfile = fopen("hangman_text.txt", "r")) == NULL)
    {
        printf("Error! opening file\n");
        exit(1);         
    }
    
    // Gets data from file and stops when random line number reached
    while(fscanf(wordfile,"%s",c) != EOF) { 
        fscanf(wordfile,"%[^\n]", c);
        if (count == randline)
            break;
        count = count + 1;

    }

    printf("Random Word Pair: %s\n", c);
    fclose(wordfile);

    word_return = malloc(1000);

    strcpy(word_return, c);
    return word_return;
    free(word_return);
}





char *replaceAll(char * message, char *buffer, char *current_word)
    {
        int i = 0;
        char oldchar = '_';
 

        printf("%s\n", message);
        /* Run till end of string */
        while(message[i] != '\0')
        {
            /* If occurrence of character is found */
            if(current_word[i] == *buffer)
            {
                message[i] = *buffer;
            }

            i++;
        }
        printf("%s\n", message);
        return message;
    }







    /// this code is used to get the string of guess words and convert them  to _ _ _ _ _ _ _ 
    char *append(const char* oldstring, const char c){

            int result;
            char *newstring;
            result = asprintf(&newstring, "%s%c", oldstring, c);
            if (result == -1) newstring = NULL;
           
            return newstring;
        }

    char *setupAppend(char *current_word)
    {
            char *str = "";
            char c = '_';
            char d = ' ';
            int run_test;
            
            
                for (int i =0; i < strlen(current_word); i = i+2) {
                    str = append(str, c);
                    str = append(str, d);
                
        
}
            printf("%s\n", str);
            return str;
    }

   void send_message(int client_fd, char *buffer){

        if ((send(client_fd,buffer, strlen(buffer),0))== -1) /// send message to client
            {
                fprintf(stderr, "Failure Sending Message\n");
                }
            
    }


bool recieve_message(int client_fd, char * buffer){
        printf("Entering current status 0\n");
                int num;
                if ((num = recv(client_fd, buffer, 1024,0))== -1) {
                        perror("recv");
                        exit(1);
                }
                else if (num == 0) {
                        //So I can now wait for another client
                        return true;
                }
                buffer[num] = '\0';
                printf("Server:Msg Received %s\n", buffer); //


        return false;

    }



bool test_user_name(char *buffer){

    printf("Function worked %s\n", buffer);
    /// Gets usernames and passwords from authentication.txt
    int i;
    bool valid_user = false;
    char * lines[Lines];
    char * linescon[Lines];
    FILE *file_handle = fopen ("Authentication.txt", "r");
    
    /// Adds usernames and passwords to array
    for (i =0; i < Lines; ++i) {
        lines[i] = malloc (128); /* allocate a memory slot of 128 chars */
        fscanf (file_handle, " %127s", lines[i]);
    }
    /// Combines array enteries together to make a combined username and password string
    for (i = 0; i < Lines; i = i +2){
        linescon[i] = malloc(strlen(lines[i])+strlen(lines[i+1]));
        strcpy(linescon[i], lines[i]);
        strcat(linescon[i], lines[i+1]);
    }

  for (i =0; i < Lines; i = i + 2){
    int rc = strncmp(linescon[i], buffer, MaxLen);
    //printf("%s\n",lines[i]);
    if (rc == 0){ printf("Username is valid\n");
        valid_user = true;}
    }
    if(valid_user == false) { printf("Username is invalid\n");}


    for (i =0; i < Lines; ++i)
    free (lines[i]); /* remember to deallocated the memory allocated */
    for (i =0; i < Lines; i = i + 2){  
        free (linescon[i]); /* remember to deallocated the memory allocated */ 
    } 

    return valid_user;
}





    void run_game(int client_fd){


  
        char commer = ',';





            int update_mode;
            bool game_end;
            int games_won;
            int games_played;
            int buffer_len = 10241;
            char *word;
            char buffer[buffer_len];
            char *buff;
            char * current_word;
            char * test_letter;
            char * new_word;
            
            bool valid_user = false;
            int current_status = 0;
            bool breakone = false;
            bool test = false;
            char * message;
            int number_of_guesses = 0;
            games_won = 100000;
            games_played = 000;


        while(1) {

                if (current_status == 0){
                    

                printf("Current status -  %d\n",current_status);
                breakone = recieve_message(client_fd, buffer); 
                if (breakone == true){
                    printf("Connection closed\n");
                    current_status = 0;
                    break;
                }   
                
                printf("Server:Msg Received %s\n", buffer); //

                valid_user = test_user_name(buffer);
                printf("%d", valid_user);



                if (valid_user == true){ 
                    //char *buffer = malloc(256);
                    strcpy(buffer, "Username or password is correct");
                    send_message(client_fd ,buffer);
                    current_status = 1;
                }
                else if (valid_user == false){ 
                    //char *buffer = malloc(256);
                    strcpy(buffer, "error");
                    send_message(client_fd ,buffer);
                    current_status = 0;
                }


                //printf("Server:Msg being sent: %s\nNumber of bytes sent: %d\n",buffer, strlen(buffer));
            }// end of mode 1 


            if (current_status == 1){
                bzero(buffer, buffer_len-1);
                
                printf("Current status -  %d\n",current_status);
                
                // recieve message
                breakone = recieve_message(client_fd, buffer); 
                if (breakone == true){
                    printf("Connection closed\n");
                    current_status = 0;
                    break;
                }   
                printf("Server:Msg Received %s\n", buffer); //
                //////////
                update_mode = atoi(buffer);
                if (update_mode == 1){

                    current_word = malloc(256);
                    test_letter = malloc(10); // fix this 
                    strcpy(test_letter, "_");
                    printf("%s\n", current_word );    
                    int length = 6;
                    //length = 6;//strlen(lines[6]);
                    new_word = generate_words();

                    int number = strlen(new_word) + strlen(new_word) -1;
                    int j = 0;
                    for (int i = 0; i < number; i++){

                        if(i%2 == 0){
                            current_word[i] = new_word[j];
                            j++;
                        }
                        else 
                        {
                            current_word[i] = '_';
                        }
                    }

                    message = setupAppend(current_word);
                    printf("test\n");
                    printf("test1 %s\n", message);

                    //char *buffer = malloc(256);





                    strcpy(buffer, ",");

                    message = replaceAll(message, buffer, current_word);

                    strcpy(buffer, message);

                    printf("test1 %s\n", buffer); //


                    int wordcount = count_word_length(current_word);
                    number_of_guesses = calculate_guesses(wordcount);
                    printf("\nNumber of Guesses: %d\n", number_of_guesses);

                    send_message(client_fd ,buffer);
                    current_status = 2;
                    //printf("Server:Msg being sent: %s\nNumber of bytes sent: %d\n",buffer, strlen(buffer));
                }

                if(update_mode == 2){
                    int games_send = games_played + games_won;
                    char * str;
                    str = malloc(16);
                    snprintf(str, 16, "%d", games_send);
                    
                    printf("%s\n", str);
                    strcpy(buffer, str);
                    send_message(client_fd ,buffer);
                    free(str);

                }

            }

 

            if (current_status == 2){ // playing the game logic
                printf("Current status -  %d\n",current_status);
                breakone = recieve_message(client_fd, buffer); 
                if (breakone == true){
                    printf("Connection closed\n");
                    current_status = 0;
                    break;
                }   
                printf("Server:Msg Received %s\n", buffer); //

                
                message = replaceAll(message, buffer, current_word);
                number_of_guesses = number_of_guesses - 1;
                printf("Number of Guesses Remaining: %d\n", number_of_guesses);
                strcpy(buffer, message);
                send_message(client_fd ,buffer);

                game_end = Game_over(buffer);

                    if (game_end == true || number_of_guesses == 0){
                        if(number_of_guesses > 0){
                            games_won = games_won + 1000;
                        }

                        games_played = games_played + 1;
                        printf("the game is over -  %s\n",buffer);
                        current_status = 1;
                        bzero(buffer, buffer_len-1);
                }
            } // end of current status 2







            
            } //End of Inner While...
        
        
    }


void* client_handler(void *client_Info)
{
    int client_fd = 0;

    while (server_running) {
        sem_wait(&sclient);

        if (!server_running) { 
            break;
        }

        client_fd = get_client_from_queue();
        run_game(client_fd);
    }
        close(client_fd);
        sem_post(&clienthandler);
        return NULL;
}


int main(int argc, char *argv[])
{   
    struct sockaddr_in server;
    struct sockaddr_in dest;
    int status,socket_fd, client_fd;
    socklen_t size;
    int yes =1;
    int PORT = 12345;

    if (argc == 2) {
        PORT = atoi(argv[1]);

    }

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    memset(&server, 0, sizeof(server));
    memset(&dest,0,sizeof(dest));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY; 
    if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    { //sizeof(struct sockaddr) 
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG))== -1){
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }

    if (sem_init(&clienthandler, 0, MAX_USERS) == -1) {
        
        exit(EXIT_FAILURE);
    }

    if (sem_init(&sclient, 0, 0) == -1) {
        
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_USERS; i++) {
        if (pthread_create(&threads[i], NULL, client_handler, (void *) &clients_infos[i]) != 0) {
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_init(&nxtposmute, NULL);    
    pthread_mutex_init(&nxtclientmute, NULL);

 while(1) { 

        size = sizeof(struct sockaddr_in);


        sem_wait(&clienthandler);// new code


        if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1 ) {
            perror("accept");
            exit(1);
        }
        printf("Server got connection from client\n");       

        add_client_to_queue(client_fd);
        sem_post(&sclient);
    } //Outer While


    
    close(socket_fd);
    return 0;
} //End of main