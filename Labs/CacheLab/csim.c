#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct {
    int valid_bit,
        tag,
        time_stamp;
} _cache_line_;

typedef struct {
    int s, E, b;
    _cache_line_ **line; // 修改此处的结构体名称
} _cache_; // 修改此处的结构体名称

bool verbose_flag = false; // 初始化 verbose_flag

char *file_path = NULL; // 初始化 file_path
int set_bits = 0, E_lines = 0, block_bits = 0; // 初始化 set_bits, E_lines, block_bits
int hit_count = 0, miss_count = 0, eviction_count = 0; // 初始化 hit_count, miss_count, eviction_count

_cache_ mycache;

void init_cache(void) {
    int set_num = 1 << set_bits;
    mycache.s = set_bits;
    mycache.E = E_lines;
    mycache.b = block_bits;

    mycache.line = (_cache_line_ **)malloc(sizeof(_cache_line_ *) * set_num);
    for (int i = 0; i < set_num; i++) {
        mycache.line[i] = (_cache_line_ *)malloc(sizeof(_cache_line_) * E_lines);
        for (int j = 0; j < E_lines; j++) {
            mycache.line[i][j].valid_bit = 0;
            mycache.line[i][j].tag = -1;
            mycache.line[i][j].time_stamp = 0;
        }
    }
    return;
}

unsigned int get_line_index(unsigned int access_tag, unsigned int access_set_index) {
    for (int i = 0; i < E_lines; i++) {
        if (mycache.line[access_set_index][i].valid_bit != 0 &&
            mycache.line[access_set_index][i].tag == access_tag)
            return i;
    }
    return -1;
}

int is_full(unsigned int access_set_index) {
    for (int i = 0; i < E_lines; i++) {
        if (mycache.line[access_set_index][i].valid_bit == 0)
            return i;
    }
    return -1;
}

int LRU(unsigned int access_set_index) {
    int line_index = -1, max_time_stamp = -1;
    for (int i = 0; i < E_lines; i++) {
        if (mycache.line[access_set_index][i].time_stamp > max_time_stamp) {
            max_time_stamp = mycache.line[access_set_index][i].time_stamp;
            line_index = i;
        }
    }
    return line_index;
}

void update(int line_index, unsigned int access_tag, unsigned int access_set_index) {
    mycache.line[access_set_index][line_index].valid_bit = 1;
    mycache.line[access_set_index][line_index].tag = access_tag;
    for (int i = 0; i < E_lines; i++) {
        if (mycache.line[access_set_index][i].valid_bit == 1)
            mycache.line[access_set_index][i].time_stamp++;
    }
    mycache.line[access_set_index][line_index].time_stamp = 0;
    return;
}

void access_cache(unsigned int access_tag, unsigned int access_set_index) {
    int line_index = get_line_index(access_tag, access_set_index);
    if (line_index == -1) {
        miss_count++;
        if (verbose_flag)
            printf("miss\n");

        int empty_line = 0;
        empty_line = is_full(access_set_index);
        if (empty_line == -1) {
            eviction_count++;
            if (verbose_flag)
                printf("eviction\n");
            empty_line = LRU(access_set_index);
        }
        update(empty_line, access_tag, access_set_index);
    }

    else {
        hit_count++;
        if (verbose_flag)
            printf("hit\n");
        update(line_index, access_tag, access_set_index);
    }
    return;
}

void simulate(void) {
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL)
        exit(-1);

    char op;
    unsigned int address, Bytes;

    while (fscanf(fp, " %c %x,%d", &op, &address, &Bytes) == 3) { 
		// 加一个空格，确保读取操作正确
        unsigned int access_tag = address >> (set_bits + block_bits);
		unsigned int access_set_index = (address >> block_bits) & ((1 << set_bits) - 1);

        switch (op) {
            case 'M':
                access_cache(access_tag, access_set_index);
                access_cache(access_tag, access_set_index);
                break;
            case 'S':
            case 'L':
                access_cache(access_tag, access_set_index);
                break;
        }
    }
    fclose(fp);
    return;
}

int main(int argc, char *argv[]) {
    int option;
    while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (option) {
            case 'h':
                printf("Usage: ./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n"
                       "\t-h: Optional usage flag that prints the usage info.\n"
                       "\t-v: Optional verbose flag that displays trace info.\n"
                       "\t-s <s>: Number of set index bits.\n"
                       "\t-E <E>: Associativity (number of lines per set).\n"
                       "\t-b <b>: Number of block bits.\n"
                       "\t-t <tracefile>: Name of the valgrind trace to replay.\n");
                break;
            case 'v':
                verbose_flag = true;
                break;
            case 's':
                set_bits = atoi(optarg);
                break;
            case 'E':
                E_lines = atoi(optarg);
                break;
            case 'b':
                block_bits = atoi(optarg);
                break;
            case 't':
                file_path = optarg;
                break;
            default:
                printf("Wring argumrnt!\n");
                break;
        }
    }

    init_cache();

    simulate();

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

