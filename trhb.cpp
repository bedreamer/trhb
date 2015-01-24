// trhb.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <memory.h>
#include <time.h>
#include <stdlib.h>

#define NR  8
int link[NR][NR]={
//   A   B   C   D   E   F   G   H
	{0,  1,  1,  1, -1, -1, -1, -1}, // A
	{1,  0, -1,  1,  1,  1, -1,  1}, // B
	{1, -1,  0,  1,  1, -1,  1, -1}, // C
	{1,  1,  1,  0,  1, -1,  1, -1}, // D
	{-1, 1,  1,  1,  0,  1,  1,  1}, // E
	{-1, 1, -1, -1,  1,  0, -1,  1}, // F
	{-1,-1,  1,  1,  1, -1,  0,  1}, // G
	{-1, 1, -1, -1,  1,  1,  1,  0} // H
};
int ** p_link = NULL;
typedef enum {
	F_NORMAL,
	F_DELETE,
	F_PASSED,
	F_FOUND
}FLAG;

struct track_dog {
	int ttl;
	int nr;
	int *path;
	FLAG flag;
	struct track_dog *next;
};
int malloced = 0;
int dog_inpath(int dog, const struct track_dog *path)
{
	int i = 0, ret = 0;

	for (; i < path->nr; i ++ ) {
		if ( path->path[i] == dog ) {
			ret ++;
			break;
		}
	}

	return ret;
}

void dog_clone(struct track_dog *prestep, struct track_dog **dog, int me, int ttl, int nrnode)
{
	int i;
	char *p;
	if ( NULL == *dog ) {
		*dog = (struct track_dog*)malloc(sizeof(struct track_dog) + sizeof(int) * nrnode);
		malloced ++;
	}
	p = (char *)(*dog);
	(*dog)->path = (int *)(void*)(p + sizeof(struct track_dog));
	for ( i = 0; i < nrnode; i ++ ) {
		(*dog)->path[i] = -1;
	}
	(*dog)->next = NULL;
	(*dog)->nr = 0;
	(*dog)->ttl = ttl;
	(*dog)->flag = F_NORMAL;
	if ( prestep == NULL ) {
		(*dog)->path[0] = me;
		(*dog)->nr = 1;
	} else {
		//memcpy(*dog, prestep, sizeof(struct track_dog));
		memcpy((*dog)->path, prestep->path, nrnode * sizeof(int));
		(*dog)->nr = prestep->nr;
		(*dog)->next = NULL;
		(*dog)->path[ (*dog)->nr ++ ] = me;
		(*dog)->ttl = prestep->ttl - 1;
	}
}

int could_be_cloned(const struct track_dog *dog, int nrnode)
{
	if ( dog->nr > nrnode ) return 0;
	if ( dog->ttl <= 0 ) return 0;
	if ( dog->flag == F_DELETE ) return 0;
	if ( dog->flag == F_FOUND ) return 0;
	return 1;
}

int next_link_not_in_passed(const struct track_dog *dog, int nextlink)
{
	int i, ret = 1;

	for ( i = 0; i < dog->nr; i ++ ) {
		if ( dog->path[i] == nextlink ) {
			return 0;
		}
	}

	return 1;
}

void track_print(const struct track_dog *dog, int nr)
{
	int i;

	printf("%d:BEGIN>>", nr);
	for ( i = 0; i < dog->nr; i ++ ) {
		printf(" %c >>", dog->path[i] + 'A');
	}
	printf("END\n");
}

void track_path(int from, int to, int ttl, int nrnode)
{
	struct track_dog *head = NULL;
	struct track_dog *newdog = NULL;
	int path_nr = 0, unpassed = 1;
	int nodes = 0;
	dog_clone(NULL, &head, from, ttl, nrnode);
	if (head == NULL) {
		fprintf(stderr, "内存不足\n");
		return;
	}
	while ( unpassed )
	{
		do { // 生成节点
			struct track_dog *gen = head;
			while ( gen ) {
				gen->flag = F_PASSED;
				if ( ! could_be_cloned(gen, nrnode) ) {
					gen = gen->next;
					continue;
				}
				int linknode = gen->path[ gen->nr - 1 ];
				int nextlink, linknr = 0;
				struct track_dog *insert_point = head;
				for ( nextlink = 0; nextlink < nrnode; nextlink ++ ) {
					insert_point = head;
					if ( p_link[linknode][nextlink] <= 0 ) continue;
					if ( ! next_link_not_in_passed(gen, nextlink) ) continue;
					linknr ++;
					dog_clone(gen, &newdog, nextlink, gen->ttl - 1, nrnode);
					while ( insert_point ) {
						if ( newdog->nr == insert_point->nr 
							&& 0 == memcmp(newdog->path, 
							insert_point->path, 
							sizeof(int)*(newdog->nr))) {
							// 重复路径
							//free(newdog);
							break;
						}
						if ( insert_point->next == NULL ) {
							insert_point->next = newdog;
							newdog->next = NULL;
							newdog = NULL;
							break;
						}
						insert_point = insert_point->next;
					}

					if (linknr == 0) {
						// 没有更多的路径了
						gen->flag = F_DELETE;
					} else {
					}
				}
				gen = gen->next;
			}
		} while (0);

		do { // 标记节点
			struct track_dog *valid_dog = head;
			while ( valid_dog ) {
				if ( valid_dog->path[ valid_dog->nr - 1 ] == to ) {
					// 找到路径
					valid_dog->flag = F_FOUND;
					track_print(valid_dog, ++path_nr);
				} else if ( valid_dog->ttl <= 0 ) {
					valid_dog->flag = F_DELETE;
				} else if ( valid_dog->nr > nrnode ) {
					valid_dog->flag = F_DELETE;
				} else ;
				valid_dog = valid_dog->next;
			}
		}while (0);
		do {
			unpassed = 0;
			nodes = 0;
			struct track_dog *thiz = head;
			while ( thiz ) {
				nodes ++;
				if ( thiz->flag == F_NORMAL ) {
					unpassed ++;
					continue;
				}
				thiz = thiz->next;
			}
		} while (0);
#if 0
		do { // 删除无效节点
			struct track_dog *thiz = head;
			struct track_dog *pre = head;
			while ( thiz ) {
				if ( thiz->flag != F_DELETE ) {
					pre = thiz;
					thiz = thiz->next;
					continue;
				}
				if ( thiz == head ) {
					thiz = head->next;
					free(head);
					head = thiz;
					continue;
				}
				pre->next = thiz->next;
				free(thiz);
				thiz = pre->next;
			}
		} while (0);
#endif
	}
	if ( newdog ) {
		free(newdog);
	}
	do {
		struct track_dog *thiz = head;
		while ( head ) {
			thiz = head->next;
			free(head);
			head = thiz;
		}
	} while (0);
	printf("%d malloc: %d\n", nodes, malloced);
	malloced = 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int from, to, step, nrnode;
	int x, y;

	srand(time(NULL));

	do {
		printf("输入起点, 终点, 步数, 节点个数:");
		scanf("%d %d %d %d", &from, &to, &step, &nrnode);

		p_link = (int **)malloc(sizeof(int) * nrnode);
		for ( y = 0; y < nrnode; y ++ ) {
			p_link[y] = (int *)malloc(sizeof(int) * nrnode);
			printf("\t");
			for ( x  = 0; x < nrnode; x ++ ) {
				if ( x == y ) {
					p_link[y][x] = 0;
					printf("%3d ", p_link[y][x]);
					continue;
				}
				if ( x > y ) {
					if ( rand() % 1000 > 500 )
						p_link[y][x] = 1;
					else p_link[y][x] = -1;
				} else {
					p_link[y][x] = p_link[x][y];
				}
				printf("%3d ", p_link[y][x]);
			}
			printf("\n");
		}

		if ( nrnode < 1 || from < 0 || to < 0 || step <= 0 || 
			step >= nrnode || from >= nrnode || to >= nrnode ) break;

		track_path(from, to, step, nrnode);
	} while (1);
	return 0;
}

