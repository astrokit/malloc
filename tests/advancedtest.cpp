/*
 * smalltest.cpp
 *
 */
#include "time.h"
#include "../memory.h"
#include "assert.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

int random_malloc_free_read_write_impl(int minsize,int maxsize,int numiter,int ratio){
    int *data[1000];
    size_t sizes[1000];
    int d,i,j,index;
    int errors,numallocs,numfree;
    int aktion;
    size_t size;
    d = maxsize - minsize;
    errors = 0;
    numallocs=0;
    numfree=0;
    if(numiter > 1000){
        printf("error wrong iternumber");
        return -1;
    }
    for(i=0;i<1000;++i){
        data[i] =0;
        sizes[i] = 0;
    }
    for(i=0;i<numiter;++i){
        aktion = random();
        aktion = aktion%10;
        if(aktion >= ratio){
            if(d>0){
                size = random();
                size = size%d + minsize;
            }else{
                size = minsize;
            }
            data[i] = (int*)snp::Memory::malloc(size*sizeof(int));
            numallocs++;
            sizes[i] = size;
            for(j=0;j<(int) size;++j){
                data[i][j] = i;
            }
        }else{
            if(numallocs <= numfree){
                continue;
            }
            do{
                index = random();
                index = index%(i+1);
            }while(data[index] == 0);
            for(j=0;j<(int) sizes[index];++j){
                if(data[index][j] != index){
                    errors++;
                }
            }
            snp::Memory::free(data[index]);
            ++numfree;
            data[index] = 0;
        }
    }
    for(i=0;i<numiter;++i){
        snp::Memory::free(data[i]);
    }
    if(errors == 0){
        printf("passed      %d mallocs %d frees\n",numallocs,numfree);
        return 0;
    }
    printf("failed        %d words wrong\n",errors);
    return 1;
}


int main(int argc, char** argv){
    int minsize=200;
    int maxsize=1000;
    int numiter=99;
    int ratio=3;
    int e=0;
    int i;

    srand ( time(NULL) );
    for(i=0;i<15;++i){
        minsize = rand();
        minsize = minsize%900 + 100;
        maxsize = rand();
        maxsize = maxsize%1500 + 200;
        numiter = rand();
        numiter = numiter%90 + 10;
        ratio = rand();
        ratio = ratio%10;
        e= e + random_malloc_free_read_write_impl(minsize,maxsize,numiter,ratio);
    }
    if(e == 0){
        printf("Test passed\n");
        return 0;
    }
    printf("Test failed\n");
    return 1;
}

