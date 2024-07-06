#include "headers.h"

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])

#define ALPHABET_SIZE (256)
 
#define CHAR_TO_INDEX(c) ((int)c)

struct TrieNode
{
	struct TrieNode *children[ALPHABET_SIZE];
	struct TrieNode *parent;
	int ss_id; // Is -1 if not valid
};

struct TrieNode *getNode(void)
{
	struct TrieNode *pNode = NULL;
	pNode = (struct TrieNode *)malloc(sizeof(struct TrieNode));
	if (pNode)
	{
		int i;
		pNode->ss_id = -1;
		for (i = 0; i < ALPHABET_SIZE; i++)
			pNode->children[i] = NULL;
		pNode->parent = NULL;
	}
	return pNode;
}

void insert(struct TrieNode *root, const char *key,int ss_id)
{
	int level;
	int length = strlen(key);
	int index;

	struct TrieNode *pCrawl = root;

	for (level = 0; level < length; level++)
	{
		index = CHAR_TO_INDEX(key[level]);
		if (!pCrawl->children[index])
		{
			pCrawl->children[index] = getNode();
			pCrawl->children[index]->parent = pCrawl;
		}
		pCrawl = pCrawl->children[index];
	}
	pCrawl->ss_id = ss_id;
}

int get_ss(struct TrieNode *root, const char *key)
{
	int level;
	int length = strlen(key);
	int index;
	struct TrieNode *pCrawl = root;

	for (level = 0; level < length; level++)
	{
		index = CHAR_TO_INDEX(key[level]);

		if (!pCrawl->children[index])
			return -1;

		pCrawl = pCrawl->children[index];
	}

	return (pCrawl->ss_id);
}

int delete_node(struct TrieNode *root, const char *key)
{
	int level;
	int length = strlen(key);
	int index;

	struct TrieNode *pCrawl = root;

	for (level = 0; level < length; level++)
	{
		index = CHAR_TO_INDEX(key[level]);
		if (!pCrawl->children[index])
			return -1;
		pCrawl = pCrawl->children[index];
	}

	if(pCrawl->ss_id == -1) return -1;
	pCrawl->ss_id = -1;

	struct TrieNode *tmp;

	for(level = length - 1 ; level >= 0 ; level --)
	{
		int check = 1;
		for(index = 0 ; index < ALPHABET_SIZE ; index++)
		{
			if(pCrawl->children[index]) 
			{
				check = 0;
				break;
			}
		}
		if(check)
		{
			tmp = pCrawl->parent;
			free(pCrawl);
			pCrawl = tmp;
			index = CHAR_TO_INDEX(key[level]);
			pCrawl->children[index] = NULL; // It's deleted just now
		}
		else
		{
			break;
		}
	}
	return 0;
}