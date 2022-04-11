#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include "zip.h"


#define EXIT_TRUE 0
#define EXIT_FALSE 1

typedef struct ArgsInformation
{
    bool pathFlag;
    bool caseSens;
    bool caseInsense;
    char* path[10];
    char* words[10];
    int pathSize;
    int wordsSize;

}DataArgs;

char* saveAddForRemove[100];
static int addCounter = 0; // number of extract zip folder for remove after iterate over them

void KMPSearch(int rowCounter, const char* fileName, char* pat, char* txt)
{
    int M = strlen(pat);
    int N = strlen(txt);
    static int idCounter = 0;
    int lps[M];

    computeLPSArray(pat, M, lps);

    int i = 0; // index for txt[]
    int j = 0; // index for pat[]
    while (i < N)
    {
        if (pat[j] == txt[i])
        {
            j++;
            i++;
        }

        if (j == M)
        {
            int sizeIndent = 130 - strlen(fileName);
            if(idCounter < 10)
                sizeIndent += 1;
            else if(idCounter >= 100 && idCounter < 1000)
                sizeIndent -= 1;


            printf("|id : %d| |FileName : [%s]| <====>%*c(Row: [%d], Column: [%d])%*c\n", idCounter++, fileName, sizeIndent, '|',rowCounter + 1, (i - j) + 1, 2, '|');

            char out[200];
            memset(out, '=', 200);
            out[199] = '\0';
            printf("%s\n", out);

            j = lps[j - 1];
        }

        else if (i < N && pat[j] != txt[i])
        {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
}

void computeLPSArray(char* pat, int M, int* lps)
{
    int len = 0;

    lps[0] = 0;

    int i = 1;
    while (i < M)
    {
        if (pat[i] == pat[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        else
        {
            if (len != 0)
            {
                len = lps[len - 1];

            }
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

const getFileExtension(const char* fileName)
{
    const char* findDot = strrchr(fileName, '.');

    if(!findDot || findDot == fileName)
        return "";
    return findDot + 1;

}

bool checkPossibleFiles(const char* fileName)
{
    if(strcmp(fileName, "cpp") == 0 || strcmp(fileName, "c") == 0 ||
       strcmp(fileName, "js") == 0 || strcmp(fileName, "html") == 0 ||
       strcmp(fileName, "py") == 0 || strcmp(fileName, "txt") == 0 ||
       strcmp(fileName, "h") == 0 || strcmp(fileName, "hpp") == 0 ||
       strcmp(fileName, "css") == 0 || strcmp(fileName, "ini") == 0 ||
       strcmp(fileName, "java") == 0)
    {

        return true;
    }
    return false;
}

void addPath(int index, char* pathData[], DataArgs* info, int* pathCounter)
{

    for(int counter = index;;counter++)
    {

        if(strcmp(pathData[counter], "-w") == 0)
        {
            info->pathSize = *pathCounter;
            *pathCounter += 1;

            break;
        }
        else
        {
            int sizeStringPath = strlen(pathData[counter]);
            info->path[*pathCounter] = (char*) malloc((sizeStringPath + 1) * sizeof(char));

            if(info->path[*pathCounter])
            {
                strcpy(info->path[*pathCounter], pathData[counter]);
                info->path[*pathCounter][sizeStringPath] = '\0';

                *pathCounter += 1;
            }
        }
    }

}

// Separate words for search
void addWords(int index, char* wordsData[], DataArgs* info, int argc)
{
    int wordsCounter = 0;

    for(int counter = index; counter < argc; counter++)
    {
        int sizeStringWords = strlen(wordsData[counter]);

        info->words[wordsCounter] = (char*) malloc((sizeStringWords + 1) * sizeof(char));

        if(info->words[wordsCounter])
        {
            strcpy(info->words[wordsCounter], wordsData[counter]);
            info->words[wordsCounter][sizeStringWords] = '\0';

            wordsCounter++;
        }
    }
    info->wordsSize = wordsCounter;
}

// Seperate Args for put in collection DataArgs
void argsOperation(int argsSize, char* argsData[], DataArgs* info, bool* recursionFlag, bool* zipFlag)
{

    for(int index = 1; index < argsSize; index++)
    {
        if(index == 1 && (strcmp(argsData[index], "ci")) == 0)
        {
            info->caseInsense = true;
            info->caseSens = false;
        }
        else if((strcmp(argsData[index], "-z")) == 0 && (index == 1 || index == 2 || index == 3))
        {
            *zipFlag = true;
        }
        else if((strcmp(argsData[index], "-r")) == 0 && (index == 1 || index == 2 || index == 3))
        {
            *recursionFlag = true;
        }
        else if((strcmp(argsData[index], "-p")) == 0 && (index == 1 || index == 2 || index == 3))
        {
            int pathCounter = 0;
            addPath(index + 1, argsData, info, &pathCounter);

            if(*zipFlag)
                index = pathCounter + 1;
            else
                index = pathCounter;

        }
        else if((strcmp(argsData[index], "-w")) == 0)
        {
            addWords(index + 1, argsData, info, argsSize);
        }
    }
}

// read file line by line
void readFile(const char* fileName, DataArgs* info)
{

    FILE* file = fopen(fileName, "r");
    char buffer[256];


    if(file)
    {
        for(int index = 0; index < info->wordsSize; index++)
        {

            rewind(file);
            fseek(file, 0, SEEK_SET);
            int counter = 0;

            while(fgets(buffer, 256, file) != NULL)
            {

                KMPSearch(counter, fileName, info->words[index], buffer);
                counter++;
            }

        }
        fclose(file);

        return;
    }
    else
    {
        perror("**File Could not be Open**");
        return;
    }
}

void concatAddress(char* fileAddress, char* path, char* diName)
{
    strcpy(fileAddress, path);
    strcat(fileAddress, "//");
    strcat(fileAddress, diName);
}

int on_extract_entry(const char *filename, void *arg)
{
    static int i = 0;
    int n = *(int *)arg;

    return 0;
}

void readDirFiles(DataArgs* info)
{
    for(int index = 0; index < info->pathSize; index++)
    {

        DIR* directObject;
        struct dirent* myDir;

        directObject = opendir(info->path[index]);

        if(directObject)
        {
            while((myDir = readdir(directObject)) != NULL)
            {
                if(myDir->d_type != DT_FIFO)
                {
                    const char* fileNameExtension = getFileExtension(myDir->d_name);

                    if(checkPossibleFiles(fileNameExtension))
                    {
                        char dirAddress[128];
                        concatAddress(dirAddress, info->path[index], myDir->d_name);
                        readFile(dirAddress, info);
                    }
                }
            }

            closedir(directObject);
        }
    }
}


void recursionSearchDir(const char *name, DataArgs* info, bool zipFlag)
{

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    char path[1024];

    while ((entry = readdir(dir)) != NULL)
    {
        const char* fileNameExtension = getFileExtension(entry->d_name);

        if(strcmp(fileNameExtension, "zip") == 0 && zipFlag)
        {
            char zipDir[1024];
            char foolderZipName[128];
            int arg = 2;

            snprintf(zipDir, sizeof(zipDir), "%s//%s", name, entry->d_name);

            char* dotString = strchr(entry->d_name, '.');
            int dotIndex = (int)(dotString - entry->d_name);
            char saver[dotIndex];

            memcpy(saver, &entry->d_name[0], dotIndex);
            snprintf(foolderZipName, sizeof(foolderZipName), "%s//%s", name, saver);

            zip_extract(zipDir, foolderZipName, on_extract_entry, &arg);

            saveAddForRemove[addCounter] = (char*) malloc(sizeof(char) * strlen(foolderZipName));
            strcpy(saveAddForRemove[addCounter], foolderZipName);
            saveAddForRemove[addCounter++][strlen(foolderZipName)] = '\0';

            recursionSearchDir(foolderZipName, info, zipFlag);

        }

        if (entry->d_type == DT_DIR)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(path, sizeof(path), "%s//%s", name, entry->d_name);

            recursionSearchDir(path, info, zipFlag);
        }
        else
        {
            char value [256];

            const char* fileNameExtension = getFileExtension(entry->d_name);

            if(checkPossibleFiles(fileNameExtension))
            {
                snprintf(path, sizeof(path), "%s//%s", name, entry->d_name);
                snprintf(value, sizeof(value), "%s", path);

                readFile(value, info);
            }

        }
    }

    closedir(dir);
}


int main(int argc, char* argv[])
{
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    if(argc >= 2)
    {
        DataArgs info;
        bool recursionFlag = false;
        bool zipFlag = false;

        argsOperation(argc, argv, &info, &recursionFlag, &zipFlag);

        if(recursionFlag)
        {
            if(info.pathSize > 1)
            {
                int counter;
                for(counter = 0; counter < info.pathSize; counter++)
                {
                    recursionSearchDir(info.path[counter], &info, zipFlag);
                }
            }

            recursionSearchDir(info.path[0], &info, zipFlag);

            int index;
            for(index = 0; index < addCounter; index++)
            {
                char test[256];

                snprintf(test, "%s%s", "rmdir /S ");
                strcat(test, saveAddForRemove[index]);

                int i = 0;
                for(int c = 0;i < strlen(test); i++)
                {
                    if(test[i] == '/' && c == 0)
                    {
                        c++;
                        continue;
                    }
                    else if(test[i] == '/')
                        test[i] = '\\';

                }

                system(test);
                memset(test, 0, 256);
            }

        }
        else
            readDirFiles(&info);


        return EXIT_TRUE;
    }
    else
    {
        perror("**Parameters are not True**");
        return EXIT_FALSE;
    }
}
