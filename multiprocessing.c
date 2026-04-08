// Lara Foqaha 1220071
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAX_WORD_LEN 50 // maximum word length
#define MAX_WORDS 270000 // maximum unique words (the file has 253854)
#define NUM_PROCESSES 2 // number of processes to fork

typedef struct 
{
    char word[MAX_WORD_LEN];
    int frequency;
} Word;

int addWord(Word words[], int *size, const char *new_word);
void quickSort(Word words[], int low, int high);
int partition(Word words[], int low, int high);
void findTop10(Word words[], int size);
void mergeResults(Word *globalWords, int *globalSize, Word *localWords, int localSize);
void countWordsInFile(const char *filename, Word *localWords, int *localSize, int start, int end);


int main() 
{
    struct timeval start, end, start_read, end_read;
    gettimeofday(&start, NULL);  // starting timing

    FILE *f = fopen("text8.txt", "r");
    if (f == NULL) 
    {
        printf("Failed to open file.\n");
        return 0;
    }

    // shared memory for the final word array and word count
    Word *globalWords = mmap(NULL, MAX_WORDS * sizeof(Word), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *globalSize = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *globalSize = 0;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    int chunk_size = file_size / NUM_PROCESSES;
    fclose(f);

    gettimeofday(&start_read, NULL); // starting timing for reading the file

    pid_t pid;
    for (int i = 0; i < NUM_PROCESSES; i++) 
    {
        pid = fork();
        if (pid == 0) 
        {
            // each child process processes its chunk independently
            int start = i * chunk_size;
            int end = (i == NUM_PROCESSES - 1) ? file_size : (i + 1) * chunk_size;

            Word *localWords = (Word *)malloc(MAX_WORDS * sizeof(Word));
            if (localWords == NULL) 
            {
                printf("Failed to allocate memory in child process.\n");
                exit(1);
            }
            int localSize = 0;

            countWordsInFile("text8.txt", localWords, &localSize, start, end);

            // merging results to the global array
            mergeResults(globalWords, globalSize, localWords, localSize);

            free(localWords);
            exit(0);
        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) 
        wait(NULL);

    gettimeofday(&end_read, NULL); // End timing for reading the file
    double elapsed_read = (end_read.tv_sec - start_read.tv_sec) +(end_read.tv_usec - start_read.tv_usec) / 1000000.0;
    printf("Time taken to read from file: %.6f seconds\n", elapsed_read);

    findTop10(globalWords, *globalSize);

    gettimeofday(&end, NULL);  // End overall timing
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Execution time: %.6f seconds\n", elapsed_time);

    // Cleanup
    munmap(globalWords, MAX_WORDS * sizeof(Word));
    munmap(globalSize, sizeof(int));

    return 0;
}

void countWordsInFile(const char *filename, Word *localWords, int *localSize, int start, int end) 
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) 
    {
        printf("Failed to open file in child process.\n");
        return;
    }

    fseek(f, start, SEEK_SET);

    // adjusting start to skip partial word
    if (start != 0) 
    {
        char c;
        while ((c = fgetc(f)) != EOF && c != ' ' && c != '\n') {
        }
    }

    char temp[MAX_WORD_LEN];
    while (ftell(f) < end && fscanf(f, "%49s", temp) == 1) 
        if (strlen(temp) > 0) 
            addWord(localWords, localSize, temp);

    fclose(f);
}

int addWord(Word words[], int *size, const char *new_word) 
{
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL); // Start timing

    for (int i = 0; i < *size; i++) 
        if (strcmp(words[i].word, new_word) == 0) 
        {
            words[i].frequency++;
            gettimeofday(&end_time, NULL); // End timing
            double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
            return 1;
        }

    if (*size < MAX_WORDS) 
    {
        strcpy(words[*size].word, new_word);
        words[*size].frequency = 1;
        (*size)++;
    }

    gettimeofday(&end_time, NULL); // End timing
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    return 1;
}


void mergeResults(Word *globalWords, int *globalSize, Word *localWords, int localSize) 
{
    for (int i = 0; i < localSize; i++) 
    {
        int found = 0;
        for (int j = 0; j < *globalSize; j++) 
        {
            if (strcmp(globalWords[j].word, localWords[i].word) == 0) 
            {
                globalWords[j].frequency += localWords[i].frequency;
                found = 1;
                break;
            }
        }
        if (!found && *globalSize < MAX_WORDS) 
        {
            strcpy(globalWords[*globalSize].word, localWords[i].word);
            globalWords[*globalSize].frequency = localWords[i].frequency;
            (*globalSize)++;
        }
    }
}

void findTop10(Word words[], int size) 
{
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    quickSort(words, 0, size - 1);

    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Time taken to sort: %.6f seconds\n", elapsed_time);

    printf("Top 10 most frequent words:\n");

    struct timeval start_time2, end_time2;
    gettimeofday(&start_time2, NULL);

    for (int i = 0; i < 10 && i < size; i++) 
        printf("%d. %s - %d times\n", i + 1, words[i].word, words[i].frequency);
    
    gettimeofday(&end_time2, NULL);
    double elapsed_time2 = (end_time2.tv_sec - start_time2.tv_sec) + (end_time2.tv_usec - start_time2.tv_usec) / 1000000.0;
    printf("Time taken to print words: %.6f seconds\n", elapsed_time2);
}

void quickSort(Word words[], int low, int high) 
{
    if (low < high) 
    {
        int pivotIndex = partition(words, low, high);
        quickSort(words, low, pivotIndex - 1);
        quickSort(words, pivotIndex + 1, high);
    }
}

int partition(Word words[], int low, int high) 
{
    Word pivot = words[high];
    int i = low - 1;

    for (int j = low; j < high; j++) 
        if (words[j].frequency > pivot.frequency) 
        {
            i++;
            Word temp = words[i];
            words[i] = words[j];
            words[j] = temp;
        }
    
    Word temp = words[i + 1];
    words[i + 1] = words[high];
    words[high] = temp;

    return i + 1;
}
