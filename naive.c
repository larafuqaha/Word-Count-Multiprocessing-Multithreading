// Lara Foqaha 1220071
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define MAX_WORD_LEN 50
#define MAX_WORDS 270000

typedef struct 
{
    char word[MAX_WORD_LEN];
    int frequency;
} Word;

int addWord(Word words[], int *size, const char *new_word);
void quickSort(Word words[], int low, int high);
int partition(Word words[], int low, int high);
void findTop10(Word words[], int size);

int main() 
{
    struct timeval start, end;
    gettimeofday(&start, NULL);  // start time

    FILE *f = fopen("text8.txt", "r");
    if (f == NULL) 
    {
        printf("Failed to open file.\n");
        return 0;
    }

    Word *words = malloc(MAX_WORDS * sizeof(Word));
    if (words == NULL) 
    {
        printf("Failed to allocate memory.\n");
        fclose(f);
        return 0;
    }

    char temp[MAX_WORD_LEN];
    int size = 0;

    while (fscanf(f, "%49s", temp) == 1)
        addWord(words, &size, temp);
    
    //printf("Total unique words: %d\n", size);

    findTop10(words, size);
    
    gettimeofday(&end, NULL);  // End time

    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Execution time: %.6f seconds\n", elapsed_time);

    free(words);
    fclose(f);
    printf("Program completed successfully.\n");

    return 0;
}

int addWord(Word words[], int *size, const char *new_word) 
{
    for (int i = 0; i < *size; i++) 
        if (strcmp(words[i].word, new_word) == 0) 
        {
            words[i].frequency++;
            return 1;
        }

    //printf("Adding new word: %s\n", new_word); 
    strcpy(words[*size].word, new_word);
    words[*size].frequency = 1;
    (*size)++;
    return 1;
}

void findTop10(Word words[], int size) 
{

    quickSort(words, 0, size - 1);
    printf("Top 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < size; i++) 
        printf("%d. %s - %d times\n", i + 1, words[i].word, words[i].frequency);
}

void quickSort(Word words[], int low, int high) 
{
    if (low < high) 
    {
        int pivotIndex = partition(words, low, high);

        // recursively sorting the subarrays
        quickSort(words, low, pivotIndex - 1);  // left array
        quickSort(words, pivotIndex + 1, high); // right array
    }
}

int partition(Word words[], int low, int high) 
{
    // last element as the pivot
    Word pivot = words[high];
    int i = low - 1;

    // rearranging the elements based on the pivot
    for (int j = low; j < high; j++) 
    {
        if (words[j].frequency > pivot.frequency) 
        { // Descending order
            i++;
            // swapping
            Word temp = words[i];
            words[i] = words[j];
            words[j] = temp;
        }
    }

    // placing the pivot in its correct position
    Word temp = words[i + 1];
    words[i + 1] = words[high];
    words[high] = temp;

    return i + 1; //the index of the pivot
}