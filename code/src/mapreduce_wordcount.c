#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define THREADS 4
#define HASH_SIZE 200003
#define WORD_MAX 128

/* ---------------- Data Structures ---------------- */

typedef struct Node {
    char word[WORD_MAX];
    int count;
    struct Node *next;
} Node;

typedef struct {
    long start;
    long end;
    const char *filename;
    Node **table;
} MapTask;

typedef struct {
    char word[WORD_MAX];
    int count;
} Entry;

/* ---------------- Hash Utilities ---------------- */

unsigned int hash(const char *s) {
    unsigned int h = 0;
    while (*s)
        h = h * 31 + *s++;
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
    Node *node = (Node *)malloc(sizeof(Node));
    strcpy(node->word, word);
    node->count = 1;
    node->next = table[h];
    table[h] = node;
}

void insert_with_count(Node **table, const char *word, int count) {
    unsigned int h = hash(word);
    Node *cur = table[h];
    while (cur) {
        if (strcmp(cur->word, word) == 0) {
            cur->count += count;
            return;
        }
        cur = cur->next;
    }
    Node *node = (Node *)malloc(sizeof(Node));
    strcpy(node->word, word);
    node->count = count;
    node->next = table[h];
    table[h] = node;
}

/* ---------------- Map Worker ---------------- */

void *map_worker(void *arg) {
    MapTask *task = (MapTask *)arg;
    FILE *file = fopen(task->filename, "r");
    if (!file)
        return NULL;

    fseek(file, task->start, SEEK_SET);

    char word[WORD_MAX];
    int idx = 0;
    long pos = task->start;
    int c;

    /* Skip partial word at chunk boundary */
    if (task->start != 0) {
        while ((c = fgetc(file)) != EOF) {
            pos++;
            if (!isalpha(c) || pos >= task->end)
                break;
        }
    }

    while (pos < task->end && (c = fgetc(file)) != EOF) {
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

    fclose(file);
    return NULL;
}

/* ---------------- Sort Comparator ---------------- */

int cmp(const void *a, const void *b) {
    Entry *x = (Entry *)a;
    Entry *y = (Entry *)b;

    if (x->count != y->count)
        return y->count - x->count;   /* descending by count */

    return strcmp(x->word, y->word); /* ascending lexicographical */
}

/* ---------------- Main ---------------- */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input output\n", argv[0]);
        return 1;
    }

    /* Get file size */
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    pthread_t threads[THREADS];
    MapTask tasks[THREADS];

    /* Allocate per-thread hash tables on heap */
    Node ***tables = (Node ***)malloc(sizeof(Node **) * THREADS);
    for (int t = 0; t < THREADS; t++) {
        tables[t] = (Node **)calloc(HASH_SIZE, sizeof(Node *));
    }

    long chunk = size / THREADS;

    /* Launch map threads */
    for (int t = 0; t < THREADS; t++) {
        tasks[t].start = t * chunk;
        tasks[t].end = (t == THREADS - 1) ? size : (t + 1) * chunk;
        tasks[t].filename = argv[1];
        tasks[t].table = tables[t];
        pthread_create(&threads[t], NULL, map_worker, &tasks[t]);
    }

    for (int t = 0; t < THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    /* Reduce phase */
    Node **global = (Node **)calloc(HASH_SIZE, sizeof(Node *));
    for (int t = 0; t < THREADS; t++) {
        for (int i = 0; i < HASH_SIZE; i++) {
            Node *cur = tables[t][i];
            while (cur) {
                insert_with_count(global, cur->word, cur->count);
                cur = cur->next;
            }
        }
    }

    /* Collect results */
    Entry *arr = (Entry *)malloc(sizeof(Entry) * 1000000);
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

    /* Sort and output */
    qsort(arr, n, sizeof(Entry), cmp);

    long long sum = 0;

    FILE *out = fopen(argv[2], "w");
    for (int i = 0; i < n; i++) {
        fprintf(out, "%s %d\n", arr[i].word, arr[i].count);
        sum += arr[i].count;
    }
    fprintf(out, "Total words: %lld\n", sum);
    fclose(out);


    
    return 0;
}
