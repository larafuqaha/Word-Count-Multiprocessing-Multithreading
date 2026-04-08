// Lara Foqaha 1220071
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_WORD_LEN 50
#define MAX_WORDS 270000
#define NUM_THREADS 2

typedef struct 
{
    char word[MAX_WORD_LEN];
    int frequency;
} Word;

typedef struct 
{
    int thread_id;
    long start;
    long end;
    Word *globalWords;
    int *globalSize;
    pthread_mutex_t *mutex;
    char filename[256];
} ThreadData;

int addWord(Word words[], int *size, const char *new_word);
void quickSort(Word words[], int low, int high);
int partition(Word words[], int low, int high);
void findTop10(Word words[], int size);
void *countWordsInFile(void *arg);

int main() 
{
    struct timeval start, end, start_read, end_read;
    gettimeofday(&start, NULL);

    FILE *f = fopen("text8.txt", "r");
    if (f == NULL) 
    {
        printf("Failed to open file.\n");
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fclose(f);

    Word *globalWords = (Word *)malloc(MAX_WORDS * sizeof(Word));
    int globalSize = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    gettimeofday(&start_read, NULL);

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    long chunk_size = file_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) 
    {
        thread_data[i].thread_id = i;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == NUM_THREADS - 1) ? file_size : (i + 1) * chunk_size;
        thread_data[i].globalWords = globalWords;
        thread_data[i].globalSize = &globalSize;
        thread_data[i].mutex = &mutex;
        strcpy(thread_data[i].filename, "text8.txt");

        pthread_create(&threads[i], NULL, countWordsInFile, (void *)&thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) 
        pthread_join(threads[i], NULL);

    gettimeofday(&end_read, NULL);
    double elapsed_read = (end_read.tv_sec - start_read.tv_sec) +(end_read.tv_usec - start_read.tv_usec) / 1000000.0;
    printf("Time taken to read from file: %.6f seconds\n", elapsed_read);

    findTop10(globalWords, globalSize);

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Execution time: %.6f seconds\n", elapsed_time);

    free(globalWords);
    pthread_mutex_destroy(&mutex);

    return 0;
}

void *countWordsInFile(void *arg) 
{
    ThreadData *data = (ThreadData *)arg;
    Word *localWords = (Word *)malloc(MAX_WORDS * sizeof(Word));
    int localSize = 0;

    FILE *f = fopen(data->filename, "r");
    if (f == NULL) 
    {
        printf("Failed to open file in thread %d.\n", data->thread_id);
        pthread_exit(NULL);
    }

    fseek(f, data->start, SEEK_SET);

    // Adjust to skip partial words
    if (data->start != 0) 
    {
        char c;
        while ((c = fgetc(f)) != EOF && c != ' ' && c != '\n') {
        }
    }

    char temp[MAX_WORD_LEN];
    long current_pos = ftell(f);
    while (current_pos < data->end && fscanf(f, "%49s", temp) == 1) 
    {
        addWord(localWords, &localSize, temp);
        current_pos = ftell(f);
    }

    fclose(f);

    pthread_mutex_lock(data->mutex);
    for (int i = 0; i < localSize; i++) 
        addWord(data->globalWords, data->globalSize, localWords[i].word);
    
    pthread_mutex_unlock(data->mutex);

    free(localWords);
    pthread_exit(NULL);
}

int addWord(Word words[], int *size, const char *new_word) 
{
    for (int i = 0; i < *size; i++) 
        if (strcmp(words[i].word, new_word) == 0) 
        {
            words[i].frequency++;
            return 1;
        }


    if (*size < MAX_WORDS) 
    {
        strcpy(words[*size].word, new_word);
        words[*size].frequency = 1;
        (*size)++;
    }

    return 1;
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

void findTop10(Word words[], int size) 
{
    quickSort(words, 0, size - 1);

    printf("Top 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < size; i++) 
        printf("%d. %s - %d times\n", i + 1, words[i].word, words[i].frequency);
}
