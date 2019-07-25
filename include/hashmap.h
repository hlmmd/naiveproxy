
#ifndef INCLUDE_HASHMAP
#define INCLUDE_HASHMAP

#define MAP_MISSING -3 /* No such element */
#define MAP_FULL -2    /* Hashmap is full */
#define MAP_OMEM -1    /* Out of Memory */
#define MAP_OK 0       /* OK */

//1361, 2729, 5471, 10949, 21911,43853,87719
#define HASHMAP_INIT_SIZE 87719

typedef int key_t;
typedef int value_t;

struct hash_node
{
    key_t key;
    value_t value;
    struct hash_node *next;
};

struct socket_fd_hashmap
{
    struct hash_node **nodes;
    int numbers;
    int hashmap_size;
};

void init_hashmap(struct socket_fd_hashmap **ssfh);

int insert_hashmap(struct socket_fd_hashmap *sfh, key_t key, value_t value);

void clear_hashmap(struct socket_fd_hashmap *sfh);

int find_hashmap(struct socket_fd_hashmap *sfh, key_t key);

value_t getvalue_hashmap(struct socket_fd_hashmap *sfh, key_t key);

int update_hashmap(struct socket_fd_hashmap *sfh, key_t key, value_t value);

int delete_hashmap(struct socket_fd_hashmap *sfh, key_t key);

void print_hashmap(struct socket_fd_hashmap *sfh);

void destroy_hashmap(struct socket_fd_hashmap **ssfh);

#endif