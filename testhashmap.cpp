#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "include/hashmap.h"

using namespace std;
int main()
{
    srand(time(0));

    struct socket_fd_hashmap *sfh = NULL;
    init_hashmap(&sfh);

    unordered_map<int, int> map;

    //先随机生成100000个数，加入哈希表中
    int insert = 1000000;

    for (int i = 0; i < insert; i++)
    {
        //ubuntu18.04这个rand返回不是0-65535啊
        int key = rand() % 100000;
        //cout << key << endl;
        int value = rand() % 100000;
        if (map.find(key) == map.end())
            map[key] = value;
        if (find_hashmap(sfh, key) == 0)
            insert_hashmap(sfh, key, value);
    }

    int findcount = 0;
    for (int i = 0; i < 100000; i++)
    {
        if (map.find(i) != map.end())
            findcount++;
        if (find_hashmap(sfh, i) == 1)
            findcount--;
    }
    if (findcount == 0)
        printf("find success\n");
    else
    {
        printf("find error\n");
    }

    //再先随机生成10000个数，从哈希表中删除
    insert = insert / 10;

    for (int i = 0; i < insert; i++)
    {
        //ubuntu18.04这个rand返回不是0-65535啊
        int key = rand() % 100000;
        //cout << key << endl;
        int value = rand() % 100000;
        if (map.find(key) != map.end())
            map.erase(key);
        if (find_hashmap(sfh, key) == 1)
            delete_hashmap(sfh, key);
    }

    findcount = 0;
    for (int i = 0; i < 100000; i++)
    {
        if (map.find(i) != map.end())
            findcount++;
        if (find_hashmap(sfh, i) == 1)
            findcount--;
    }
    if (findcount == 0)
        printf("find success\n");
    else
    {
        printf("find error\n");
    }

    //再随机选一些数，对其进行修改

    insert = insert / 2;

    for (int i = 0; i < insert; i++)
    {
        //ubuntu18.04这个rand返回不是0-65535啊
        int key = rand() % 100000;
        //cout << key << endl;
        int value = rand() % 100000;
        if (map.find(key) != map.end())
            map[key] = value;
        if (find_hashmap(sfh, key) == 1)
            update_hashmap(sfh, key, value);
    }

    for (int i = 0; i < 100000; i++)
    {
        if (map.find(i) != map.end())
        {
            if (map[i] != getvalue_hashmap(sfh, i))
            {
                printf("value error\n");
            }
        }
    }

    //  for (auto it = map.begin(); it != map.end(); it++)
    //     cout << it->first << endl;

    return 0;
    insert_hashmap(sfh, 2, 3);
    insert_hashmap(sfh, 2, 3);
    insert_hashmap(sfh, 3, 2);

    int key_to_find = 3;
    int ret = find_hashmap(sfh, key_to_find);
    if (ret == 0)
        printf("not found key:%d\n", key_to_find);
    else
    {
        printf("key:%d->value:%d\n", key_to_find, getvalue_hashmap(sfh, key_to_find));
    }

    delete_hashmap(sfh, 3);

    print_hashmap(sfh);

    clear_hashmap(sfh);

    return 0;
}