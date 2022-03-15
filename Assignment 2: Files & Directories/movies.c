#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#define _GNU_SOURCE
#define PREFIX "movies_"
#define SUFFIX ".csv"

/* struct for movie information */
struct movie
{
    char *title;
    char *year;
    char *language;
    double rating;
    
};

/* Parse the current line which is space delimited and create a
*  movie struct with the data in this line
*  Adapted from Assignment 1
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
    token = strtok_r(NULL, ",", &saveptr);
    currmovie->year = token;

    // The next token is the language
    token = strtok_r(NULL, ",", &saveptr);
    currmovie->language = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currmovie->language, token);

    // The last token is the rating
    // converts to double
    token = strtok_r(NULL, "\n", &saveptr);
    currmovie->rating = strtod(token, &token);

    return currmovie;
}

/*
* Return a linked list of movies by parsing data from
* each line of the specified file.
* Adapted from Assignment 1
*/
struct movie *processFile(char *filePath)
{
    
    // print a message about the file we are processing
    printf("Now processing the chosen file named %s\n", filePath);
    
    // create a new direct in format of your_onid.movie.random_number
    int randomss = rand() % 9999+1;
    char dirName[100];
    snprintf(dirName, sizeof(dirName), "%s%d", "gunert.movies.",randomss);
    mkdir(dirName, 0750);

    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

    // add a counter to to track how many lines were read
    int counter = 0;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
        // Get a new movie node corresponding to the current line
        struct movie *newNode = createmovie(currLine);

        // skip the first row
        if (counter == 0)
        {
            counter++;
        }
        else
        {
            // creates or appends data the title of the movies to the file YYYY.txt
            // Iron Man from 2008 will create a file called 2008.txt with Iron Man in one line
            // if another movies was released the same year than it would append to the file
            char newFile[200];
            snprintf(newFile, sizeof(newFile), "%s%s%s%s", dirName, "/", newNode->year, ".txt");
            int fd = open(newFile, O_RDWR | O_CREAT | O_APPEND, 0640);
            write(fd, newNode->title, strlen(newNode->title)+1);
            write(fd,"\n",strlen("\n"));
            close(fd);
            counter++;
        }
    }

    // let user know that the directory was created
    printf("Created directory with name %s\n\n", dirName);

    // free curline and cole movie file
    free(currLine);
    fclose(movieFile);
    return 0;
}

// This section below has been adapted from module 3 "Exploration: Directories" but varies to keep track of largest and smallest files
void movieProcessPredetermined(int largesmall)
{
    // Open the current directory
    DIR* currDir = opendir(".");
    struct dirent *aDir;
    struct stat dirStat; //stores the states of the file and directories
    
    // declare ints to 
    long int biggestFileSize;
    long int smallestFileSize;
    int i = 0;

    char entryNameLarge[256];
    char entryNameSmall[256];

    // Go through all the entries
    while((aDir = readdir(currDir)) != NULL){

        if((strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0) && (strcmp(SUFFIX, aDir->d_name+strlen(aDir->d_name)-4) == 0))
        {
            // Get meta-data for the current entry
            stat(aDir->d_name, &dirStat);  
            
            // save the largest file
            if((i == 0 || (dirStat.st_size - biggestFileSize) > 0) && largesmall == 1)
            {
                biggestFileSize = dirStat.st_size;
                memset(entryNameLarge, '\0', sizeof(entryNameLarge));
                strcpy(entryNameLarge, aDir->d_name);
                }

            // save the smallest file
            if((i == 0 || (dirStat.st_size - smallestFileSize) < 0) && largesmall == 2)
            {
                smallestFileSize = dirStat.st_size;
                memset(entryNameSmall, '\0', sizeof(entryNameSmall));
                strcpy(entryNameSmall, aDir->d_name);
                }

        i++;
        }
    }

    // Close the directory -------------- end modal code
    closedir(currDir);
 
    // process the struct in file based on if the user chose the largest or smallest file
    if(largesmall == 1)
    {
        struct movie *list = processFile(entryNameLarge);
    }
    else
    {
        struct movie *list = processFile(entryNameSmall); 
    }
    
}

int processChosenFile(char *fileName)
{
    int found = 0;
    // Open the current directory
    DIR* currDir = opendir(".");
    struct dirent *aDir;
    

    // loop through directory to find the file
    while((aDir = readdir(currDir)) != NULL)
    {
        // if found increment 1
        if(strcmp(aDir->d_name, fileName) == 0)
        {
            found++;
        }
    }

    closedir(currDir);

    // id a file was found then process it
    if(found!=0)
    {
        struct movie *list = processFile(fileName);
    }
    
    // return found
    return found;
}

/*
*   Process the file provided as an argument to the program to
*   create a linked list of movie structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o movies movies.c
*/

int main()
{

    // declare srand 
    srand(time(0));

    // create variables to store user inputs
    int mainmenu;
    int submenu;
    char title[256];
    int track;

    do 
    {
        // display options for the user to choose
        printf("1. Select file to process\n");
        printf("2. Exit the program\n\n");
        printf("Enter a choice 1 or 2: ");
        scanf("%d", &mainmenu);

        switch(mainmenu)
        {
            // if the user chose 1 then present the menu
            case 1:
                incorrect: // a jump to if input file is incorrect
                printf("\nWhich file you want to process?\n");
                printf("Enter 1 to pick the largest file\n");
                printf("Enter 2 to pick the smallest file\n");
                printf("Enter 3 to specify the name of a file\n");
                printf("\nEnter a choice from 1 to 3: ");
                scanf("%d", &submenu);

                //switch statment based on user chose
                switch(submenu)
                {
                    // process the largerst file
                    case 1:
                        movieProcessPredetermined(1);
                        break;
                    // process the smallest file
                    case 2:
                        movieProcessPredetermined(2);
                        break;
                    // allow the user to choose the file and if not found jump back to incorrect:
                    case 3:
                        track = 0;
                        printf("Enter the complete file name: ");
                        scanf("%s", title);
                        track = processChosenFile(title);     
                        if(track==0){
                            printf("The file %s was not found. Try again\n", title);
                            goto incorrect;
                        }                
                        break;
                    // if if other number was entered
                    default:
                        printf("\nPlease choose a correct option for Sub Menu choices.\n\n");
                        break;
                }
                break;
            case 2:
                // do nothing
                break;
            default:
                // print an error message to choose a correct input
                printf("\nPlease choose a correct option for the Main Menu choices.\n\n");
                break;
        }

    } while (mainmenu != 2); // exit when 2 was chosen in the main menu

    return EXIT_SUCCESS;
}