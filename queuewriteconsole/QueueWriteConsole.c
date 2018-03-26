#include <stdio.h>

#define DEVICE_NAME "/dev/qikchar"

int main(int argc, char** argv)
{
    int i;
    FILE *f;
    
    if(argc<2)
    {
        fprintf(stderr, "Not arguments\n");
        return 1;
    }

    f = fopen(DEVICE_NAME, "wt");
    if (f==NULL)
    {
        fprintf(stderr, "Error open device\n");
        return 1;
    }
    
    for(i=1; i<argc; i++)
    {
        fprintf(f, "%s\n", argv[i]);
        fflush(f);
    }
    
    fclose(f);
    
    return 0;
}