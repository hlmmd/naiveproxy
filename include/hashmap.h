/*
 * Generic hashmap manipulation functions
 *
 * Originally by Elliot C Back - http://elliottback.com/wp/hashmap-implementation-in-c/
 *
 * Modified by Pete Warden to fix a serious performance problem, support strings as keys
 * and removed thread synchronization - http://petewarden.typepad.com
 */
#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#define MAP_MISSING -3 /* No such element */
#define MAP_FULL -2    /* Hashmap is full */
#define MAP_OMEM -1    /* Out of Memory */
#define MAP_OK 0       /* OK */

//1361, 2729, 5471, 10949, 21911,43853,87719
#define HASHMAP_INIT_SIZE 43853

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

#endif