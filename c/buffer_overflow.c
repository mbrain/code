#include <stdio.h>
#include <string.h>

int main(void) {
    
    char buff[9];
    int pass = 0;

    printf("Enter password: ");    
    
    gets(buff);
    
    if(!strcmp(buff, "p@ssw0rd")) pass = 1;
    
    if(pass) printf ("Welcome root");

    return 0;
    
}
