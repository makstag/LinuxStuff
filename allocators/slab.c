#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdlib.h>


#define ALIGNMENT 4096
#define SLABELEMENTS 64
#define MAXORDER 10
#define MINORDER 0
#define VOID sizeof(void *)
#define ORDER(X) ALIGNMENT * (1 << X)
#define MASK 0x1ul


int order = 0;
bool hasInitialized = 0;
void *managedMemoryStart;
void *lastValidAddress;


struct memControlBlock 
{
	bool isAvailable;
	size_t size;
};

void allocInit(void) 
{
	lastValidAddress = sbrk(0);
	managedMemoryStart = lastValidAddress;
	hasInitialized = 1;
}

void *alloc(size_t size) 
{
	void *currentLocation;
	struct memControlBlock *currentLocationMcb;
	void *memoryLocation = NULL;
	
	if (!hasInitialized) 
		allocInit();

	size += sizeof *currentLocationMcb;
//	printf("Address managed memory start = %p\n", managedMemoryStart);
//	printf("Address last valid address = %p\n", lastValidAddress);
	currentLocation = managedMemoryStart;

	while (currentLocation != lastValidAddress) 
	{
		currentLocationMcb = (struct memControlBlock *)currentLocation;
//		printf("size allocation of %ld\n", currentLocationMcb->size);
//		printf("busy allocation %s\n", currentLocationMcb->isAvailable ? "true" : "false");
		if (currentLocationMcb->isAvailable && currentLocationMcb->size >= size) 
		{
			currentLocationMcb->isAvailable = false;
			memoryLocation = currentLocation;
			break;
		}

		currentLocation += currentLocationMcb->size;
	}

	if (!memoryLocation) 
	{
		sbrk(size);
//		printf("Address last valid address after sbrk = %p\n", lastValidAddress);
		memoryLocation = lastValidAddress;
		lastValidAddress += size;
		currentLocationMcb = (struct memControlBlock *)memoryLocation;
		currentLocationMcb->isAvailable = false;
		currentLocationMcb->size = size;
	}

	memoryLocation += sizeof *currentLocationMcb;
//	printf("Address memory location = %p\n", memoryLocation);
	return memoryLocation;
}

void freeAlloc(void *p) 
{
//	printf("Free memory location address = %p\n", p);
	struct memControlBlock *mcb;
	mcb = (struct memControlBlock *)(p - sizeof *mcb);
	mcb->isAvailable = true;
	return;
}

void *alignedAlloc(size_t alignment, size_t size)
{
	const size_t excessBytes = VOID < alignof(max_align_t) ? VOID : alignof(max_align_t);
	void *block = alloc((size + VOID) + (alignment - excessBytes));
	const uintptr_t blockRepr = (uintptr_t)block;
	const uintptr_t blockOffset = (alignment - (VOID + blockRepr)) & (alignment - 1);
	void *result = block + VOID + blockOffset;
	*(void **)(result - VOID) = block;
//	printf("Aligned address = %p\n", result);
	return result;
}

void freeAligned(void *p)
{
//	printf("Free aligned address = %p\n", p);
	if (p)
		freeAlloc(*((void **)p - 1));
	return;
}


/**
 * Эти две функции вы должны использовать для аллокации
 * и освобождения памяти в этом задании. Считайте, что
 * внутри они используют buddy аллокатор с размером
 * страницы равным 4096 байтам.
 **/

/**
 * Аллоцирует участок размером 4096 * 2^order байт,
 * выровненный на границу 4096 * 2^order байт. order
 * должен быть в интервале [0; 10] (обе границы
 * включительно), т. е. вы не можете аллоцировать больше
 * 4Mb за раз.
 **/
void *alloc_slab(int order)
{
	assert(order >= MINORDER && order <= MAXORDER);
    // return alignedAlloc(ORDER(order), ORDER(order));
	return aligned_alloc(ORDER(order), ORDER(order));
}

/**
 * Освобождает участок ранее аллоцированный с помощью
 * функции alloc_slab.
 **/
void free_slab(void *slab)
{
	// freeAligned(slab);
	free(slab);
	return;
}


/**
 * Эта структура представляет аллокатор, вы можете менять
 * ее как вам удобно. Приведенные в ней поля и комментарии
 * просто дают общую идею того, что вам может понадобится
 * сохранить в этой структуре.
 **/
typedef struct __slab 
{
	void *begin;
	uint64_t mask;
	struct __slab *prev;
	struct __slab *next;
} slab;

struct cache 
{
    /* список пустых SLAB-ов для поддержки cache_shrink */
    /* список частично занятых SLAB-ов */
    /* список заполненых SLAB-ов */

    size_t size; /* размер аллоцируемого объекта */
	slab *free;
	slab *partiallyOccupied;
	slab *busy;
};

int calcMemoryOrder(size_t size)
{
	double N = size * SLABELEMENTS;
	return (int)ceil(log2(N / (double)ALIGNMENT));
}

void createNewSlab(struct cache *cache)
{
	void *align = alloc_slab(order);
	slab *nb = (slab *)(align + cache->size * (SLABELEMENTS - 2));
	nb->begin = align;
	nb->prev = NULL;
	nb->next = NULL;
	nb->mask = 1;

	if (cache->free)
	{
		nb->prev = cache->free;
		cache->free->next = nb;
		cache->free = nb;
	}
	else cache->free = nb;

	return;
}
/**
 * Функция инициализации будет вызвана перед тем, как
 * использовать это кеширующий аллокатор для аллокации.
 * Параметры:
 *  - cache - структура, которую вы должны инициализировать
 *  - object_size - размер объектов, которые должен
 *    аллоцировать этот кеширующий аллокатор 
 **/
void cache_setup(struct cache *cache, size_t object_size)
{
	cache->size = object_size;
	cache->free = NULL;
	cache->partiallyOccupied = NULL;
	cache->busy = NULL;

	order = calcMemoryOrder(object_size);
	createNewSlab(cache);

	return;
}


/**
 * Функция освобождения будет вызвана когда работа с
 * аллокатором будет закончена. Она должна освободить
 * всю память занятую аллокатором. Проверяющая система
 * будет считать ошибкой, если не вся память будет
 * освбождена.
 **/
void cache_release(struct cache *cache)
{
	while(cache->free != NULL)
	{
		free_slab(cache->free->begin);
		cache->free = cache->free->prev;
	}

	while(cache->partiallyOccupied != NULL)
	{
		free_slab(cache->partiallyOccupied->begin);
		cache->partiallyOccupied = cache->partiallyOccupied->prev;
	}

	while(cache->busy != NULL)
	{
		free_slab(cache->busy->begin);
		cache->busy = cache->busy->prev;
	}

	return;
}


bool checkSlab(uint64_t mask, int position)
{
	return ((mask & (MASK << position)) == 0);
}

uint64_t setSlab(uint64_t mask, int position)
{
	return (mask | (MASK << position));
}

uint64_t unsetSlab(uint64_t mask, int position) 
{
	return (mask ^ (MASK << position));
}

/**
 * Функция аллокации памяти из кеширующего аллокатора.
 * Должна возвращать указатель на участок памяти размера
 * как минимум object_size байт (см cache_setup).
 * Гарантируется, что cache указывает на корректный
 * инициализированный аллокатор.
 **/
void *cache_alloc(struct cache *cache)
{
	void *pos = NULL;
	if (cache->partiallyOccupied) 
	{
		for (int i = 63; i > 1; i--)
			if (checkSlab(cache->partiallyOccupied->mask, i))
			{
				cache->partiallyOccupied->mask = setSlab(cache->partiallyOccupied->mask, i);
				pos = cache->partiallyOccupied->begin + ((63 - i) * cache->size);
				if (MASK == unsetSlab(cache->partiallyOccupied->mask, i) || UINT64_MAX == cache->partiallyOccupied->mask)
				{
					slab *prev = cache->partiallyOccupied->prev;
					cache->partiallyOccupied->prev = NULL;
					cache->partiallyOccupied->next = NULL;
					if (cache->busy) 
					{
						cache->partiallyOccupied->prev = cache->busy;
						cache->busy->next = cache->partiallyOccupied;
						cache->busy = cache->partiallyOccupied;
					}
					else cache->busy = cache->partiallyOccupied;
					if (prev)
						prev->next = NULL;
					cache->partiallyOccupied = prev;
				}
				break;
			}
	}
	else if (cache->free)
	{
		for (int i = 63; i > 1; i--)
			if (checkSlab(cache->free->mask, i))
			{
				cache->free->mask = setSlab(cache->free->mask, i);
				pos = cache->free->begin + ((63 - i) * cache->size);
				if (MASK == unsetSlab(cache->free->mask, i) || UINT64_MAX == cache->free->mask)
				{
					slab *prev = cache->free->prev;
					cache->free->prev = NULL;
					cache->free->next = NULL;
					if (cache->partiallyOccupied) 
					{
						cache->free->prev = cache->partiallyOccupied;
						cache->partiallyOccupied->next = cache->free;
						cache->partiallyOccupied = cache->free;
					}
					else cache->partiallyOccupied = cache->free;
					if (prev)
						prev->next = NULL;
					cache->free = prev;
				}
				break;
			}
	}
	else
	{
		createNewSlab(cache);
		for (int i = 63; i > 1; i--)
			if (checkSlab(cache->free->mask, i))
			{
				cache->free->mask = setSlab(cache->free->mask, i);
				pos = cache->free->begin + ((63 - i) * cache->size);
				if (MASK == unsetSlab(cache->free->mask, i) || UINT64_MAX == cache->free->mask)
				{
					slab *prev = cache->free->prev;
					cache->free->prev = NULL;
					cache->free->next = NULL;
					if (cache->partiallyOccupied) 
					{
						cache->free->prev = cache->partiallyOccupied;
						cache->partiallyOccupied->next = cache->free;
						cache->partiallyOccupied = cache->free;
					}
					else cache->partiallyOccupied = cache->free;
					if (prev)
						prev->next = NULL;
					cache->free = prev;
				}
				break;
			}
	}

	return pos;
}


void freeSlab(slab *s, slab *move, void *begin, void *ptr, size_t size)
{
	if (s)
	{
		if (s->begin == begin) 
		{
			int i = (ptr - s->begin) / size;
			s->mask = unsetSlab(s->mask, 63 - i);

			if (MASK == s->mask || UINT64_MAX == setSlab(s->mask, 63 - i))
			{
				slab *current = NULL;
				if (s->prev) 
				{
					s->prev->next = s->next;
					current = s->prev;
				}
				if (s->next) 
				{
					s->next->prev = s->prev;
					current - s->next;
				}

				s->prev = NULL;
				s->next = NULL;
				if (move) 
				{
					s->prev = move;
					move->next = s;
					move = s;
				}
				else move = s;
				s = current;
			}
		}
		else freeSlab(s->prev, move, begin, ptr, size);
	}

	return;
}
/**
 * Функция освобождения памяти назад в кеширующий аллокатор.
 * Гарантируется, что ptr - указатель ранее возвращенный из
 * cache_alloc.
 **/
void cache_free(struct cache *cache, void *ptr)
{
	if (!ptr) return;

	uintptr_t begin = (uintptr_t)ptr & ~(ORDER(order) - 1);
	// freeSlab(s, move, (void *)begin, ptr, cache->size)
	if (cache->partiallyOccupied->begin == (void *)begin) 
	{
		int i = (ptr - cache->partiallyOccupied->begin) / cache->size;
		cache->partiallyOccupied->mask = unsetSlab(cache->partiallyOccupied->mask, 63 - i);
		ptr = NULL;

		if (MASK == cache->partiallyOccupied->mask || UINT64_MAX == setSlab(cache->partiallyOccupied->mask, 63 - i))
		{
			slab *prev = cache->partiallyOccupied->prev;
			cache->partiallyOccupied->prev = NULL;
			cache->partiallyOccupied->next = NULL;
			if (cache->free) 
			{
				cache->partiallyOccupied->prev = cache->free;
				cache->free->next = cache->partiallyOccupied;
				cache->free = cache->partiallyOccupied;
			}
			else cache->free = cache->partiallyOccupied;
			if (prev)
				prev->next = NULL;
			cache->partiallyOccupied = prev;
		}
	}
		
	if (ptr) 
	{
		if (cache->busy->begin == (void *)begin) 
		{
			int i = (ptr - cache->busy->begin) / cache->size;
			cache->busy->mask = unsetSlab(cache->busy->mask, 63 - i);
			ptr = NULL;

			if (MASK == cache->busy->mask || UINT64_MAX == setSlab(cache->busy->mask, 63 - i))
			{
				slab *prev = cache->busy->prev;
				cache->busy->prev = NULL;
				cache->busy->next = NULL;
				if (cache->partiallyOccupied) 
				{
					cache->busy->prev = cache->partiallyOccupied;
					cache->partiallyOccupied->next = cache->busy;
					cache->partiallyOccupied = cache->busy;
				}
				else cache->partiallyOccupied = cache->busy;
				if (prev)
					prev->next = NULL;
				cache->busy = prev;
			}
		}
	}

	return;
}


/**
 * Функция должна освободить все SLAB, которые не содержат
 * занятых объектов. Если SLAB не использовался для аллокации
 * объектов (например, если вы выделяли с помощью alloc_slab
 * память для внутренних нужд вашего алгоритма), то освбождать
 * его не обязательно.
 **/
void cache_shrink(struct cache *cache)
{
	while (cache->free != NULL) 
	{
		free_slab(cache->free->begin);
		cache->free = cache->free->prev;
	}

	return;
}

int main(void)
{
	struct cache cache;
	cache_setup(&cache, 1024);
	printf("addr2 = %p\n", cache.free->begin);
	void *slab = cache_alloc(&cache);
	printf("addr = %p\n", slab);

	cache_free(&cache, slab);
	if (cache.partiallyOccupied) printf("partially occupied not nil\n");
	if (cache.free) printf("free not nil\n");
	if (slab) printf("slab addr = %p\n", slab);

    return EXIT_SUCCESS;
}
