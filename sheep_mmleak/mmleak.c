#define _GNU_SOURCE
#include <dlfcn.h>

#include <link.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
//方式1：宏定义
#if 0
void *nMalloc(size_t size, const char *filename, const char *funcname, int line)
{
    void *ptr = malloc(size);

    char buf[128] = {0};
    sprintf(buf, "./block/%p.mem", ptr);

    FILE *fp = fopen(buf, "w");
    if (!fp)
    {
        free(ptr);
        return NULL;
    }
    fprintf(fp, "[+][%s:%s:%d] %p: %ld malloc\n",filename, funcname, line, ptr, size);
    fflush(fp);
    fclose(fp);
    
    return ptr;
}

void nFree(void *ptr, const char *filename, const char *funcname, int line)
{
    char buf[128] = {0};
    sprintf(buf, "./block/%p.mem", ptr);

    if (unlink(buf) < 0)
    {
        printf("double free: %p\n", ptr);
        return;
    }
    
    return free(ptr);
}
#define malloc(size)    nMalloc(size, __FILE__, __func__, __LINE__);
#define free(ptr)       nFree(ptr,  __FILE__, __func__, __LINE__);


#else       //方式2：dlsym动态库

typedef void* (*malloc_t)(size_t size);
typedef void (*free_t)(void *ptr);

malloc_t malloc_f = NULL;
free_t free_f = NULL;

int enable_malloc = 1;
int enable_free = 1;


void *TranslateToSymbol(void *addr)
{
    Dl_info info;
    struct link_map *link;

    dladdr1(addr, &info, (void *)&link, RTLD_DL_LINKMAP);

    return (void *)(addr - link->l_addr);
}


void *malloc(size_t size)
{
    if (!malloc_f)
    {
        malloc_f = dlsym(RTLD_NEXT, "malloc");
    }

    void *ptr = NULL;

    if (enable_malloc)  
    {
        enable_malloc = 0;
        ptr = malloc_f(size);

        void *caller = __builtin_return_address(0);

        char buf[128] = {0};
        sprintf(buf, "./block/%p.mem", ptr);

        FILE *fp = fopen(buf, "w");
        if (!fp)
        {
            free(ptr);
            return NULL;
        }
        fprintf(fp, "[+][%p] %p: %ld malloc\n", TranslateToSymbol(caller), ptr, size);
        fflush(fp);
        fclose(fp);

        enable_malloc = 1;
    }else
    {
        ptr = malloc_f(size);
    }
    
    return ptr;
}

void free(void *ptr)
{
    if (!free_f)
    {
        free_f = dlsym(RTLD_NEXT, "free");
    }

    char buf[128] = {0};
    sprintf(buf, "./block/%p.mem", ptr);

    if (unlink(buf) < 0)
    {
        printf("double free: %p\n", ptr);
        return;
    }
    return free_f(ptr);
    
}

#endif

int main(int argc, char const *argv[])
{
    size_t size = 5;

	void *p1 = malloc(size);
	void *p2 = malloc(size * 2);
	void *p3 = malloc(size * 3);

	free(p1);
	free(p3);
    return 0;
}
