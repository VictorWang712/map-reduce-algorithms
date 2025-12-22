#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define THREADS 4
#define HASH_SIZE 200003
#define WORD_MAX 128

typedef struct Node {
    char word[WORD_MAX];
    int count;
    struct Node *next;
} Node;

typedef struct {
    long start;
    long end;
    FILE *file;
    Node **table;
} MapTask;

unsigned int hash(const char *s) {
    unsigned int h = 0;
    while (*s) h = h * 31 + *s++;
    return h % HASH_SIZE;
}

void insert(Node **table, const char *word) {
    unsigned int h = hash(word);
    Node *cur = table[h];
    while (cur) {
        if (strcmp(cur->word, word) == 0) {
            cur->count++;
            return;
        }
        cur = cur->next;
    }
    Node *node = malloc(sizeof(Node));
    strcpy(node->word, word);
    node->count = 1;
    node->next = table[h];
    table[h] = node;
}

void *map_worker(void *arg) {
    MapTask *task = (MapTask *)arg;
    fseek(task->file, task->start, SEEK_SET);

    char word[WORD_MAX];
    int idx = 0;
    long pos = task->start;
    int c;

    while (pos < task->end && (c = fgetc(task->file)) != EOF) {
        pos++;
        if (isalpha(c)) {
            word[idx++] = tolower(c);
        } else if (idx > 0) {
            word[idx] = '\0';
            insert(task->table, word);
            idx = 0;
        }
    }
    if (idx > 0) {
        word[idx] = '\0';
        insert(task->table, word);
    }
    return NULL;
}

typedef struct {
    char word[WORD_MAX];
    int count;
} Entry;

int cmp(const void *a, const void *b) {
    Entry *x = (Entry *)a;
    Entry *y = (Entry *)b;
    if (x->count != y->count)
        return y->count - x->count;
    return strcmp(x->word, y->word);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input output\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    pthread_t threads[THREADS];
    MapTask tasks[THREADS];
    Node *tables[THREADS][HASH_SIZE];

    for (int t = 0; t < THREADS; t++)
        for (int i = 0; i < HASH_SIZE; i++)
            tables[t][i] = NULL;

    long chunk = size / THREADS;

    for (int t = 0; t < THREADS; t++) {
        tasks[t].start = t * chunk;
        tasks[t].end = (t == THREADS - 1) ? size : (t + 1) * chunk;
        tasks[t].file = file;
        tasks[t].table = tables[t];
        pthread_create(&threads[t], NULL, map_worker, &tasks[t]);
    }

    for (int t = 0; t < THREADS; t++)
        pthread_join(threads[t], NULL);

    Node *global[HASH_SIZE] = {0};

    for (int t = 0; t < THREADS; t++) {
        for (int i = 0; i < HASH_SIZE; i++) {
            Node *cur = tables[t][i];
            while (cur) {
                insert(global, cur->word);
                cur = cur->next;
            }
        }
    }

    Entry *arr = malloc(sizeof(Entry) * 1000000);
    int n = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Node *cur = global[i];
        while (cur) {
            strcpy(arr[n].word, cur->word);
            arr[n].count = cur->count;
            n++;
            cur = cur->next;
        }
    }

    qsort(arr, n, sizeof(Entry), cmp);

    FILE *out = fopen(argv[2], "w");
    for (int i = 0; i < n; i++)
        fprintf(out, "%s %d\n", arr[i].word, arr[i].count);

    fclose(out);
    fclose(file);
    return 0;
}
