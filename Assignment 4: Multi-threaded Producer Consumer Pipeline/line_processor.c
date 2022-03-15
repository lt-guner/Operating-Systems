#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>



// Size of the buffers
#define MAX_LINES 50
#define MAX_CHARS 1024

int char_count = 0;

int stop_flag = 0;

// Buffer 1, shared resource between Get Line Thread and Line Separator Thread
char buffer_1[MAX_LINES * MAX_CHARS];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the square-root thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;


// Buffer 2, shared resource between Line Separator Thread and Plus Sign Thread
char buffer_2[MAX_LINES * MAX_CHARS];
// Number of items in the buffer
int count_2 = 0;
// Index where the input thread will put the next item
int prod_idx_2 = 0;
// Index where the square-root thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;


// Buffer 3, shared resource between Line Separator Thread and Plus Sign Thread
char buffer_3[MAX_LINES * MAX_CHARS];
int num_chars = 0;
// Number of items in the buffer
int count_3 = 0;
// Index where the input thread will put the next item
int prod_idx_3 = 0;
// Index where the square-root thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;



/* ----------------------------------
 |                                   |
 |         Put Buffers               |
 |                                   | 
  ---------------------------------- */

/*
 Put an item in buff_1
*/
void put_buff_1(char* item){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_1);
    // Put the item in the buffer
    for(int i = 0; i < strlen(item); i++){
        buffer_1[prod_idx_1] = item[i];
        prod_idx_1++;
        char_count++;
    }
    // use count to tell how many items to pull
    count_1++;
    
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_1);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
}

/*
 Put an item in buff_2
*/
void put_buff_2(char* item){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_2);
    // Put the item in the buffer
    for(int i = 0; i < strlen(item); i++){
        buffer_2[prod_idx_2] = item[i];
        prod_idx_2++;
    }
    count_2++;
    // Signal to the consumer that the buffer is no longer empty
    //printf("%s\n\n", buffer_2);
    pthread_cond_signal(&full_2);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);
}

/*
 Put an item in buff_3
*/
void put_buff_3(char* item){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_3);
    // Put the item in the buffer
    for(int i = 0; i < strlen(item); i++){
        buffer_3[prod_idx_3] = item[i];
        prod_idx_3++;
        num_chars++;
    }
    // Signal to the consumer that the buffer is no longer empty
    count_3++;
    //printf("%s\n\n",buffer_3);
    pthread_cond_signal(&full_3);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
}


/* ----------------------------------
 |                                   |
 |         Get Buffers               |
 |                                   | 
  ---------------------------------- */

/*
 Get the next item from buffer 1
*/
char* get_buff_1()
{
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_1);
    
    while (count_1 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_1, &mutex_1);
  
    // get the start and end of the buffer_1
    char *start = &buffer_1[con_idx_1];
    char *end = &buffer_1[prod_idx_1];
    
    // allocate space for the pulled buffer and store in item
    char *item = (char *)calloc(1, end - start + 1);
    memcpy(item, start, end - start);

    // dec count_2 and increase con_idx to new starting point
    count_1--;
    con_idx_1 = prod_idx_1;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
  
    // Return the item
    return item;
}

/*
 Get the next item from buffer 2
*/
char* get_buff_2()
{
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_2);

    while (count_2 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_2, &mutex_2);
  
    // get the start and end of the buffer_2
    char *start = &buffer_2[con_idx_2];
    char *end = &buffer_2[prod_idx_2];
    
    // allocate space for the pulled buffer and store in item
    char *item = (char *)calloc(1, end - start + 1);
    memcpy(item, start, end - start);

    // dec count_2 and increase con_idx to new starting point
    count_2--;
    con_idx_2 = prod_idx_2;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);

    // Return the item
    return item;
}

/*
 Get the next item from buffer 3
*/
char* get_buff_3()
{
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_3);

    while (count_3 == 0)
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_3, &mutex_3);
  
    // we have a default string in case the string is not over 80 chars
    char *no = "No";
    
    // if the num_chars is greater than 80 then we can proceed
    if(num_chars >= 80){

        // get number of 80 character segments we can process
        int chars_segment = num_chars / 80;

        // get the start of the where to pull characters and the end where to pull characters from buffer 3
        // then store in item
        char *start = &buffer_3[con_idx_3];
        char *end = &buffer_3[con_idx_3 + (80* chars_segment)];
        char *item = (char *)calloc(1, end - start + 1);
        memcpy(item, start, end - start);
     
        // decrement count for the next buffer call in the while loop and increase and the con_ind_3 by 3 to move to the next starting point and decrease num_chars by 80 * segement
        count_3--;
        con_idx_3 += (80 * chars_segment);
        num_chars -= (80 * chars_segment);

        // unlock the mutex
        pthread_mutex_unlock(&mutex_3);

        //printf("%s",item);

        return item;

    }
    count_3--;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
  
    // Return the item
    return no;
}


/* ----------------------------------
 |                                   |
 |         Thread Functions          |
 |                                   | 
  ---------------------------------- */

/*
Function that the input thread will run.
Get input from the user.
Put the item in the buffer shared with the square_root thread.
*/
void *get_input(void *args)
{
    // create the necessary buffers and currline for input processing
    char *currline = NULL;
    size_t bufsize = MAX_CHARS;
    ssize_t nread;

    // use the for loop to come process each line until there is a stop
    // set stop_flag to 1 if STOP on a newline is found
    // if there is STDIN redirection is detected and stop_flag is encounters then exit
    for (int i = 0; i < MAX_LINES; i++)
    {
        getline(&currline, &bufsize, stdin);
        if(strcmp(currline,"STOP\n") !=0 && stop_flag == 0){
            put_buff_1(currline);
        }
        else{
            stop_flag = 1;
        }
        if (isatty(STDIN_FILENO) != 0 && stop_flag == 1){
            exit(0);
        }
    }

    return NULL;
}

/*
This function takes the string in buffer_1 and mutates it to replace that \n with space.
The new string is sent to put_buff_2 and stored buffer_2
NOTE: short function so no extra comments
*/
void *produce_separator(void *args)
{

    for (int i = 0; i < MAX_LINES; i++)
    {
        char *separatorline;
        separatorline = get_buff_1();
        for(int i = 0; i < strlen(separatorline); i++)
        {
            if (separatorline[i] == *"\n"){
                separatorline[i] = *" ";
            }
        }
        put_buff_2(separatorline);
    }
    return NULL;
}

/*
This function takes the string in buffer_2 and mutates it to replace that ++ with ^.
The new string is sent to put_buff_3 and stored buffer_3
*/
void *produce_carrot(void *args)
{
    for (int i = 0; i < MAX_LINES; i++)
    {
        // declare two string pointers and a buffer for processing the pulled buffer
        char *getseparatorline;
        char *carrotline;
        size_t bufsize = MAX_CHARS;

        // allocat space for carrotline and grabe the buffer in getseparatorline
        carrotline = calloc(bufsize, sizeof(char));
        getseparatorline = get_buff_2();

        // counters to keep track of the + and ^ manipulation positions in strings
        int sep_track = 0;
        int car_track = 0;

        // process data as long sep_track (indexing of getseparatorline)
        // if ++ is found in replace with ^ and increment the count for sep_track else just copy the character over
        // increment both counters at the end
        while(sep_track < strlen(getseparatorline)){
            if(getseparatorline[sep_track] == *"+" && getseparatorline[sep_track+1] == *"+")
            {
                carrotline[car_track] = *"^";
                sep_track++;
            }
            else{
                carrotline[car_track] = getseparatorline[sep_track];
            }

            sep_track++;
            car_track++;
        }

        // put into buffer
        put_buff_3(carrotline);
    }

    return NULL;
}

/*
 This writes the output using printf to the console or file 80 characters at a time
 It only does it if there is a full 80 characters
*/
void *write_output(void *args)
{

    // for loop to 
    for(int i = 0; i < MAX_LINES; i++){

        // get what is in the carrotbuffer to process and determine how many lines of 80 chars to process
        char *getcarrotline;
        getcarrotline = get_buff_3();
        int str_length = strlen(getcarrotline);
        int lines_count = str_length / 80;

        // start insdex is 0
        int start_index = 0;

        // while lines_count (# of 80 char lines) is greater than 0 procedd
        while(lines_count >= 1)
        {
            // get the starting and ending index pointers then allocate mem
            char *start = &getcarrotline[start_index];
            char *end = &getcarrotline[80+start_index];
            char *item = (char *)calloc(1, end - start + 1);

            // copy over the string data into item and print
            memcpy(item, start, end - start);
            printf("%s\n", item);

            // increment index by 80 and decrement line count
            start_index += 80;
            lines_count--;
            char_count -= 80;
            fflush(stdout);
            
        }
        fflush(stdout);

        // if there was no stdin file then exit here if stop_flag is one
        if(stop_flag == 1 && char_count < 80)
        {
            exit(0);
        }
    }

    return NULL;
}




int main()
{
    pthread_t input_t, separator_t, plus_sign_t, output_t;
    
    // Create the threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&separator_t, NULL, produce_separator, NULL);
    pthread_create(&plus_sign_t, NULL, produce_carrot, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);
   
    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(separator_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);
    return EXIT_SUCCESS;
}