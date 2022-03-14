// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* struct for movie information */
struct movie
{
    char *title;
    int year;
    char *language;
    double rating;
    struct movie *next;
};

struct integers
{
    int lowest;
    int highest;
    int longest;
};

/* Parse the current line which is space delimited and create a
*  movie struct with the data in this line
*/
struct movie *createmovie(char *currLine)
{
    struct movie *currmovie = malloc(sizeof(struct movie));

    // For use with strtok_r
    char *saveptr;

    // The first token is the title
    char *token = strtok_r(currLine, ",", &saveptr);
    if(strcmp(token,"Title")==0)
    {
        return 0;
    }
    currmovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currmovie->title, token);

    // The next token is the year
    // convert to integer 
    token = strtok_r(NULL, ",", &saveptr);
    currmovie->year = atoi(token);

    // The next token is the language
    token = strtok_r(NULL, ",", &saveptr);
    currmovie->language = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currmovie->language, token);

    // The last token is the rating
    // converts to double
    token = strtok_r(NULL, "\n", &saveptr);
    currmovie->rating = strtod(token, &token);

    // Set the next node to NULL in the newly created movie entry
    currmovie->next = NULL;

    return currmovie;
}

/*
* Return a linked list of movies by parsing data from
* each line of the specified file.
*/
struct movie *processFile(char *filePath)
{
    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

    // add a counter to to track how many lines were read
    int counter = 0;

    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
        // Get a new movie node corresponding to the current line
        struct movie *newNode = createmovie(currLine);

        // Is this the first node in the linked list?
        if (counter == 0)
        {
            head = NULL;
            tail = NULL;
            counter++;
        }
        else if (head == NULL)
        {
            counter--;
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
            counter++;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
            counter++;
        }
    }

    // print message on how many movies were processed
    printf("\nProcessed file %s and parsed data for %d movies\n\n", filePath, counter);
    free(currLine);
    fclose(movieFile);
    return head;
}

//prints the movies by year
void printmoviesbyyear(struct movie* amovie, int year)
{
    //keep track on found movies for the curr year
    int counter = 0;

    //loop through amovie until null
    while(amovie != NULL)
    {
        //if movie is found then print
        if (amovie->year == year)
        {
            printf("%s\n", amovie->title);
            counter++;
        }

        //advance to next
        amovie = amovie->next;
    }

    //print if no movie was found
    if(counter == 0) printf("No movies in this year\n");
    printf("\n");
}

// finds the needed integers for comparison earliest date, latest date, and longest movie name
void findints(struct integers* record, struct movie* amovie)
{
    // set the highest, lowest, and longest in integer struct to the head of movie
    record->highest = amovie->year;
    record->lowest = amovie->year;
    record->longest = strlen(amovie->title);

    //advance movie
    amovie = amovie->next;

    //iterate through movie and update lowest, highest, and longest as needed
    while(amovie != NULL)
    {
        if(record->lowest > amovie->year) record->lowest = amovie->year;
        if(record->highest < amovie->year) record->highest = amovie->year;
        if(record->longest < strlen(amovie->title)) record->longest = strlen(amovie->title);

        amovie = amovie->next;
    }
}

void printratingcuryear(struct movie* amovie, int length, int years)
{
    //declare variables for use
    double ratings = 0.0;
    char nametitle[length];
    int curyear = 0;

    // loop through movie list to find movies of the current year that is passed and find the highest rating then store in the variables
    while(amovie != NULL)
    {
        if(amovie->year == years)
        {
            if(amovie->rating > ratings)
            {
                ratings = amovie->rating;
                strcpy(nametitle,amovie->title);
                curyear = amovie->year;
            }
        }
        amovie = amovie->next;
    }
    
    // print the highest rated movie of the current year
    if(curyear == years){
        printf("%d %.1lf %s\n",curyear,ratings,nametitle);
    }
}

//print highest rated movie by year
void printmoviebyrating(struct movie* amovie)
{
    //create a struct of type integer and pass it to findints
    struct integers record = {0,0,0};
    findints(&record, amovie);

    //iterate through the earliest release date in list to highest release date
    for(int i = record.lowest; i <= record.highest; i++)
    {
       printratingcuryear(amovie, record.longest, i);
    }
}

//prints movies by language
void printmoviebylanaguage(struct movie* amovie, char *langaugeinput)
{
    // kepp a counter of found movies for the language
    int counter = 0;

    // loop through while there are movies
    while (amovie != NULL)
    {
        // if the substring is found of the language then print the movie
        if(strstr(amovie->language, langaugeinput)!=NULL)
        {
            printf("%d %s\n",amovie->year,amovie->title);
            counter++;
        }

        //advance the pointer
        amovie = amovie->next;
    }

    //if no movies are found then display the message
    if(counter==0) printf("No movie was found for language in %s\n",langaugeinput);
}


/*
*   Process the file provided as an argument to the program to
*   create a linked list of movie structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o movies main.c
*/

int main(c )
{

    // requires the arguments of the exe and the input file
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./movies movie_info1.txt\n");
        return EXIT_FAILURE;
    }

    // process the struct in file
    struct movie *list = processFile(argv[1]);

    // declare a blank int to use for use input
    int x;

    // do while until user enters 4
    do {

        // display the options to the user and record input
        printf("1. Show movies released in the specified year\n");
        printf("2. Show highest rated movie for each year\n");
        printf("3. Show the title and year of release of all movies in a specific language\n");
        printf("4. Exit from the program\n\n");
        printf("Please chose 1 through 4: ");
        scanf("%d", &x);

        // based on what they entered present the data of the option they selected
        switch(x)
        {
            case 1:
                // grab the year and display the movies for the year
                printf("\nChoose a year: ");
                int inputyear;
                scanf("%d", &inputyear);
                printf("\n");
                printmoviesbyyear(list, inputyear);
                break;
            case 2:
                // list movies by highest rating per yeat
                printf("\n");
                printmoviebyrating(list);
                printf("\n");
                break;
            case 3:
                // list movies by language
                printf("\nChoose a language: ");
                char inputlanguage[20];
                scanf("%s", inputlanguage);
                printf("\n");
                printmoviebylanaguage(list, inputlanguage);
                printf("\n");
                break;
            case 4:
                // do nothing because its exist
                break;
            default:
                // display error if wrong option was entered
                printf("\nYou entered an incorrect choice. Try again.\n\n");
                break;
        }
        
    } while (x != 4);

    return EXIT_SUCCESS;
}