#include <regex.h>                                                              
#include <locale.h>                                                             
#include <stdio.h>                                                              
#include <stdlib.h>                                                             
                                                                                
int main() {                                                                        
    regex_t    preg;                                                            
    char       *string;                                     
    char       *pattern = ".*(simple).*";                                       
    int        rc;                                                              
    size_t     nmatch = 2;                                                      
    regmatch_t pmatch[2];                                                       
                                                                                
    if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {                    
       printf("regcomp() failed, returning nonzero (%d)\n", rc);                
       exit(1);                                                                 
    }                                                                           
                                                                                
    if ((rc = regexec(&preg, string, nmatch, pmatch, 0)) != 0) {                
       printf("failed to ERE match '%s' with '%s',returning %d.\n",             
       string, pattern, rc);                                                    
    }                                                                           

    for (int i = 0; i < 2; ++i)
        for (int j = pmatch[0].rm_so; j < pmatch[0].rm_eo; ++j)
            printf("%c", string[j]);
    printf("\n");                                                            
    regfree(&preg);                                                             
}  