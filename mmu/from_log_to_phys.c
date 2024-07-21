#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


struct Addr 
{
  uint64_t paddr;
  uint64_t value;
};

uint64_t createMask(const uint8_t from, const uint8_t to, const uint64_t value)
{
    uint64_t result = 0;
    for (int i = from, j = 0; i <= to; i++, j++)
        if ((value & ((uint64_t)1 << i)) != 0)
            result |= (uint64_t)1 << j;

    return result;
}

int main(void) 
{
    FILE * pfile;
    uint64_t raddr = 0;
    int m, q;
    pfile = fopen("dataset_44327_15.txt", "r");
	m = getchar();
	q = getchar();
	raddr = getchar();
	struct Addr *addr = (struct Addr *)malloc(m * sizeof(struct Addr));
	uint64_t *query = (uint64_t *)malloc(q * sizeof(uint64_t));

    raddr &= 0xFFFFFFFFFFFFF000;
    for (int i = 0; i < q; i++)
    {
        uint8_t from = 39, to = 47;
        uint64_t index = createMask(from, to, query[i]);
        index = index * 8 + raddr;
        if (addr[0].paddr == index)
        {
            if ((addr[0].value & 1) != 0)
            {
                uint64_t value = addr[0].value & 0xFFFFFFFFFFFFF000;
                from -= 9;
                to -= 9;
                uint64_t d = createMask(from, to, query[i]);
                d = d * 8 + value;
                uint64_t result = 0;

                for (int j = 1; j < m; j++)
                {
                    if (addr[j].paddr == d)
                    {
                        if ((addr[j].value & 1) != 0)
                        {
                            value = addr[j].value & 0xFFFFFFFFFFFFF000;
                            from -= 9;
                            to -= 9;
                            if (from >= 12)
                            {
                                d = createMask(from, to, query[i]);
                                d = d * 8 + value;
                            }
                            else
                            {
                                result = createMask(0, 11, query[i]);
                                result += value;
                                break;
                            }
                        }
                        else 
                        {
                            printf("fault\n");
                            break;
                        }
                    }
                    
                }
                if (result)
                    printf("%ld\n", result);
                else printf("fault\n");
            }
            else printf("fault\n");
        }
        else printf("fault\n");
    }
    free(addr);
    free(query);
    return EXIT_SUCCESS;
}

