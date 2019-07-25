#include <stdio.h>
#include <stdlib.h>

#include "hashmap.h"

void init_hashmap(struct socket_fd_hashmap **ssfh)
{
    *ssfh = (struct socket_fd_hashmap *)malloc(sizeof(struct socket_fd_hashmap));

    struct socket_fd_hashmap *sfh = *ssfh;
    sfh->numbers = 0;
    sfh->hashmap_size = HASHMAP_INIT_SIZE;
    sfh->nodes = (struct hash_node **)malloc(sizeof(struct hash_node *) * sfh->hashmap_size);

    for (int i = 0; i < sfh->hashmap_size; i++)
    {
        sfh->nodes[i] = NULL;
    }
}

int delete_hashnode(struct hash_node **node, key_t key)
{
    if (*node == NULL)
        return 0;
    if ((*node)->key == key)
    {
        struct hash_node *p = (*node)->next;
        free(*node);
        *node = p;
    }
    else
    {
        struct hash_node *p = *node;

        //查找键值是否存在。
        while (p)
        {
            if (p->next == NULL || p->next->key == key)
                break;
            p = p->next;
        }

        if (p->next == NULL)
            return 0;

        struct hash_node *next = p->next->next;
        free(p->next);
        p->next = next;
    }
    return 1;
}

int insert_hashnode(struct hash_node **node, key_t key, value_t value)
{
    //当前链表为空。
    if (*node == NULL)
    {
        *node = (struct hash_node *)malloc(sizeof(struct hash_node));
        if (*node == NULL)
        {
            return 0;
        }
        (*node)->key = key;
        (*node)->value = value;
        (*node)->next = NULL;
    }
    else
    {
        struct hash_node *p = *node;
        //查找键值是否存在。
        do
        {
            if (p->key == key)
                return 0;
            if (p->next == NULL)
                break;
            p = p->next;
        } while (1);

        p->next = (struct hash_node *)malloc(sizeof(struct hash_node));
        if (p->next == NULL)
        {
            return 0;
        }
        p->next->key = key;
        p->next->value = value;
        p->next->next = NULL;
    }
    return 1;
}

int find_hashmap(struct socket_fd_hashmap *sfh, key_t key)
{
    int hashvalue = key % sfh->hashmap_size;
    int ret = 0;
    struct hash_node *p = sfh->nodes[hashvalue];
    while (p)
    {
        if (p->key == key)
            return 1;
        p = p->next;
    }
    return ret;
}

value_t getvalue_hashmap(struct socket_fd_hashmap *sfh, key_t key)
{
    int hashvalue = key % sfh->hashmap_size;
    value_t ret = -1;
    struct hash_node *p = sfh->nodes[hashvalue];
    while (p)
    {
        if (p->key == key)
            return p->value;
        p = p->next;
    }
    return ret;
}

int update_hashmap(struct socket_fd_hashmap *sfh, key_t key, value_t value)
{
    int hashvalue = key % sfh->hashmap_size;
    struct hash_node *p = sfh->nodes[hashvalue];
    int ret = 0;
    while (p)
    {
        if (p->key == key)
        {
            p->value = value;
            return 1;
        }
        p = p->next;
    }
    return ret;
}

int insert_hashmap(struct socket_fd_hashmap *sfh, key_t key, value_t value)
{
    int hashvalue = key % sfh->hashmap_size;
    int ret = insert_hashnode(&sfh->nodes[hashvalue], key, value);
    if (ret == 1)
        sfh->numbers++;
    return ret;
}

int delete_hashmap(struct socket_fd_hashmap *sfh, key_t key)
{
    int hashvalue = key % sfh->hashmap_size;
    int ret = delete_hashnode(&sfh->nodes[hashvalue], key);
    if (ret == 1)
        sfh->numbers--;
    return ret;
}

void clear_hashmap(struct socket_fd_hashmap *sfh)
{
    if (sfh == NULL)
    {
        return;
    }
    if (sfh->nodes != NULL)
    {
        //
        //
        // free(sfh->nodes);
        // sfh->nodes = NULL;
    }
    sfh->numbers = 0;
}

void print_hashmap(struct socket_fd_hashmap *sfh)
{
    if (sfh == NULL)
        return;
    for (int i = 0; i < sfh->hashmap_size; i++)
    {
        struct hash_node *p = sfh->nodes[i];
        while (p)
        {
            printf("%d %d\n", p->key, p->value);
            p = p->next;
        }
    }
}

// void socket_fd_clear(struct socket_fd_map *ctx)
// {
//     if (ctx->entries != NULL)
//     {
//         int i;
//         for (i = 0; i < ctx->nentries; ++i)
//         {
//             if (ctx->entries[i] != NULL)
//             {
//                 free(ctx->entries[i]);
//                 ctx->entries[i] = NULL;
//             }
//         }
//         free(ctx->entries);
//         ctx->entries = NULL;
//     }
//     ctx->nentries = 0;
// }