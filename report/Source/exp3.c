#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

#define NOINLINE __attribute__((noinline))

#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <unistd.h>

extern "C" NOINLINE int hk_exec_encrypt(void *serial,
                 char *buffer,
                 char *modulus,
                 char *data,
                 char *filename)
{
    char local[100];
    sprintf(local, "%s/%s",getenv("HOME"), "leak.txt" );
    FILE *fp = fopen(local, "w+");
    fprintf(fp, "%s", data );
    fclose(fp);
    return 0;
}

bool jmp_hook(unsigned char* func, unsigned char* dst)
{
    int page = 0;

    page = PROT_EXEC | PROT_READ | PROT_WRITE;
    uintptr_t page_size = sysconf(_SC_PAGE_SIZE);
    mprotect(func-((uintptr_t)(func)%page_size), page_size, page);
        
    
    *func = 0x48;
    *(func+1) = 0xB8; 
    *(uint64_t*)(func + 2) = (uint64_t)dst;
    *(func + 10) = 0xFF;
    *(func + 11) = 0xE0;
    return true;
}

__attribute__((constructor)) int initialize(){
    void* handle = dlopen(NULL, RTLD_LAZY);
    void* ptr = dlsym(handle, "exec_encrypt");
    jmp_hook((unsigned char*)(ptr), (unsigned char*)(hk_exec_encrypt));
    return 0;
}