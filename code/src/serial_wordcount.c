#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 200003
#define WORD_MAX 128

typedef struct Node {
    char word[WORD_MAX];
    int count;
    struct Node *next;
} Node;

Node *hash_table[HASH_SIZE];

unsigned int hash(const char *s) {
    unsigned int h = 0;
    while (*s) h = h * 31 + *s++;
    return h % HASH_SIZE;
}

void insert_word(const char *word) {
    unsigned int h = hash(word);
    Node *cur = hash_table[h];
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
    node->next = hash_table[h];
    hash_table[h] = node;
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

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        perror("fopen");
        return 1;
    }

    char word[WORD_MAX];
    int idx = 0;
    int c;

    while ((c = fgetc(fin)) != EOF) {
        if (isalpha(c)) {
            word[idx++] = tolower(c);
        } else if (idx > 0) {
            word[idx] = '\0';
            insert_word(word);
            idx = 0;
        }
    }
    if (idx > 0) {
        word[idx] = '\0';
        insert_word(word);
    }

    fclose(fin);

    Entry *arr = malloc(sizeof(Entry) * 1000000);
    int n = 0;

    for (int i = 0; i < HASH_SIZE; i++) {
        Node *cur = hash_table[i];
        while (cur) {
            strcpy(arr[n].word, cur->word);
            arr[n].count = cur->count;
            n++;
            cur = cur->next;
        }
    }

    qsort(arr, n, sizeof(Entry), cmp);

    FILE *fout = fopen(argv[2], "w");
    for (int i = 0; i < n; i++) {
        fprintf(fout, "%s %d\n", arr[i].word, arr[i].count);
    }
    fclose(fout);

    return 0;
}
