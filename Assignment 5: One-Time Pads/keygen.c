#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int main (int argc, char **argv)
{
    // run the random number generator
    srand(time(0));

    // if the arguments are not passed correctly then print the error message
    if(argc != 2)
    {
        fprintf(stderr, "Please enter a valid input, such as ./keygen 256\n");
        return EXIT_FAILURE;
    }

    // if a valid interger is not passed then we let the user know
    if(atoi(argv[1]) <= 0)
    {
        fprintf(stderr, "Please enter a valid integer greater than or equal to 0\n");
        return EXIT_FAILURE;
    }

    // iterate through the until the number of characters processed is what what passed by the user
    for(int i; i <= atoi(argv[1]); i++)
    {
        // if i is what the user has passed then it will be the newline
        if(i == atoi(argv[1]))
        {
            fprintf(stdout, "\n");
        }

        // use rand to generate numbers between 65 and 91 for ascii characters of capital letter
        // 91 is [ so replace that with 32 for space
        else
        {
            int ascii = (rand()%27) + 65;
            if (ascii == 91){
                fprintf(stdout,"%c", 32);
            }
            else
            {
                fprintf(stdout,"%c", ascii);
            }
        }
    }
    
    return 0;
}