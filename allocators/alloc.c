#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


static size_t EFFECTIVE_SIZE;
static size_t MAX_SIZE;
struct __metadata 
{
	void *prev;
	void *next;

	size_t offset;
	bool free;
};
#define METADATA sizeof(struct __metadata)

typedef struct 
{
	void *head;
	struct __metadata metadata;
} __alloc;
__alloc alloc;


void mysetup(void *buf, size_t size)
{
	EFFECTIVE_SIZE = size / 9;
	MAX_SIZE = 8 * size / 9;
	alloc.head = buf;
	alloc.metadata.offset = size;
	// alloc.metadata.free = true;
	alloc.metadata.prev = NULL;
	alloc.metadata.next = NULL;
}

void *recursionAllocate(size_t size, void *head)
{
	void *pos = NULL;
	if (head) 
	{
		size_t OFFSET = size + METADATA;
		struct __metadata *current = (struct __metadata *)(head - METADATA);
		if (current->free && size <= current->offset) 
		{
			int diff = current->offset - OFFSET;
			if (diff >= EFFECTIVE_SIZE)
			{
				struct __metadata *remain = (struct __metadata *)(head + size);
				remain->offset = diff; // TODO: will make chek offset for metadata and allocate memory
				remain->free = true;
				remain->prev = head;
				remain->next = current->next;
				current->next = head + OFFSET;
				current->offset = size;
			}
			pos = head;
			current->free = false;
		}
		else pos = recursionAllocate(size, current->prev);
	}

	return pos;
}

void *myalloc(size_t size)
{
	// if (size < EFFECTIVE_SIZE || size > MAX_SIZE) return NULL;

	void *pos;
	size_t OFFSET = size + METADATA;
	if (OFFSET <= alloc.metadata.offset)
	{
		struct __metadata *current = (struct __metadata *)alloc.head;
		current->offset = size;
		current->free = false;
		current->prev = alloc.metadata.prev;
		current->next = NULL;

		pos = alloc.head + METADATA;
		if (alloc.metadata.prev) 
		{
			struct __metadata *prev = (struct __metadata *)(alloc.metadata.prev - METADATA);
			prev->next = pos;
		}
		alloc.metadata.prev = pos;
		alloc.metadata.offset -= OFFSET;
		// if (!alloc.metadata.offset) alloc.metadata.free = false;
		alloc.head += OFFSET;
	}
	else pos = recursionAllocate(size, alloc.metadata.prev);

	return pos;
}

void recursionFree(void *p, void *head) 
{
	if (head) 
	{
		struct __metadata *current = (struct __metadata *)(head - METADATA);
		if (p == head) 
		{
			p = NULL;
			current->free = true;
			if (current->next) 
			{
				struct __metadata *next = (struct __metadata *)(current->next - METADATA);
				if (next->free)
				{
					current->offset = current->offset + next->offset + METADATA;
					if (next->next) 
					{
						current->next = next->next;
						struct __metadata *nn = (struct __metadata *)(next->next - METADATA);
						nn->prev = head;
					}
					else
					{
						current->next = NULL;
						alloc.metadata.prev = head;
					}
				}
			}
			if (current->prev)
			{
				struct __metadata *prev = (struct __metadata *)(current->prev - METADATA);
				if (prev->free) 
				{
					prev->offset = prev->offset + current->offset + METADATA;
					if (current->next) 
					{
						prev->next = current->next;
						struct __metadata *next = (struct __metadata *)(current->next - METADATA);
						next->prev = current->prev;
					}
					else
					{
						prev->next = NULL;
						alloc.metadata.prev = current->prev;
					}
				}
			}
		}
		else recursionFree(p, current->prev);
	}

	return;
}

void myfree(void *p)
{
	if (!p) return;
	if (alloc.metadata.prev) 
	{
		recursionFree(p, alloc.metadata.prev);
		struct __metadata *prev = (struct __metadata *)(alloc.metadata.prev - METADATA);
		if (prev->free) 
		{
			alloc.metadata.offset = alloc.metadata.offset + prev->offset + METADATA;
			if (prev->prev) 
			{
				struct __metadata *pp = (struct __metadata *)(prev->prev - METADATA);
				pp->next = NULL;
			}
			alloc.metadata.prev = prev->prev;
			alloc.head = alloc.head - prev->offset - METADATA;
		}
	}

	return;
}


#define MAX 524288
int main(void)
{
	void *buf = malloc(MAX);
	mysetup(buf, MAX);
    printf("MAX_SIZE = %ld\n", MAX_SIZE);
    printf("EFFECTIVE_SIZE = %ld\n", EFFECTIVE_SIZE);

	void *rp = myalloc(132);
	void *mp = myalloc(132);
	void *qp = myalloc(256);
	//printf("adress buf = %p\n", buf);
	//printf("adress alloc.head = %p\n", alloc.head);
	
	printf("allocate length = %ld\n", alloc.head - buf);
	myfree(mp);
	//printf("adress alloc.head = %p\n", alloc.head);
	printf("free length = %ld\n", alloc.head - buf);
	myfree(qp);
	void *tp = myalloc(512);
	printf("free length = %ld\n", alloc.head - buf);
	myfree(rp);
	myfree(tp);
	printf("free length = %ld\n", alloc.head - buf);
	
	free(buf);
    return EXIT_SUCCESS;
}
