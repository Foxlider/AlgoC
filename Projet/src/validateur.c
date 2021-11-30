#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>


#include "json.h"

int validate_json_base(char * input, char *code, char *content)
{
    int j = 0;
    char c[1024];
    memset(content, 0, strlen(content));
    memset(code, 0, strlen(code));

    strcpy(c, input);
    char *p = strtok(c, "\n");
    char *array[4];

    while (p != NULL)
    {
        array[j++] = p;
        p = strtok(NULL, "\n");
    }

    // for (j = 0; j < 4; ++j) 
    // {
    //     printf("%d>'%s'\n", j, array[j]);
    //     fflush(stdout);
    // }

    int reti;
    int ok = 1;
    char msgbuf[100];
    for (j = 0; j < 4; ++j) 
    {
        regex_t regex;
        switch (j)
        {
            case 0:
                //regex pour la première accolade
                // Compile regular expression
                reti = regcomp(&regex, "^[:spaces:]*{[:spaces:]*", 0);
                reti = regexec(&regex, array[j], 0, NULL, 0);
                if (!reti) 
                {
                    ok = ok*1;
                    // printf("%d %d>Match\n", ok, j);
                }
                else if (reti == REG_NOMATCH) 
                {
                    ok = ok*0;
                    // printf("%d %d>Match\n", ok, j);
                    return 10;
                }
                else {
                    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
                    fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                    exit(1);
                }
                break;

            case 1:
            {
                //STRTOK car ni le regex ni sscanf ne fonctionnent
                int k = 0;
                char *p = strtok(array[j], "\"");
                char *line[4];

                while (p != NULL)
                {
                    line[k++] = p;
                    p = strtok(NULL, "\"");
                }

                // for (k = 0; k < 4; ++k) 
                // {
                //     printf("\t%d>'%s'\n", k, line[k]);
                //     fflush(stdout);
                // }

                //On vérifie que le json est bien formatté
                if(strcmp(line[1], "code")==0 && strcmp(line[2], " : ")==0)
                {
                    ok = ok*1;
                    strcpy(code, line[3]);
                    // printf("%d %d>Match => code='%s'\n", ok, j, code);
                }
                else
                {
                    puts(line[2]);
                    ok = ok*0;
                    // printf("%d %d>No Match\n", ok, j);
                    //return 11;
                }
                break;
            }
            case 2:
            {
                //SSCANF car il y a un espace entre le contenu et les crochets


                const char *ptr1 = strchr(array[j], '[');
                const char *ptr2 = strchr(array[j], ']');
                
                if(!ptr1 || !ptr2)
                    ok = ok*0;
                else
                {
                    int p1 = ptr1-array[j] + 2; //+- 2 pour enlever l'accolade et les espaces
                    int p2 = ptr2-array[j];
                    int len = p2-p1;

                    strncpy(content, (array[j] + p1), len-1); //On copie le string à partir de l'index de '[' jusqu'à ']'
                }
                break;
            }
            case 3:
                //regex pour la dernière accolade
                /* Compile regular expression */
                reti = regcomp(&regex, "^[:spaces:]*}[:spaces:]*", 0);
                reti = regexec(&regex, array[j], 0, NULL, 0);
                if (!reti) 
                {
                    ok = ok*1;
                    // printf("%d %d>Match\n", ok, j);
                }
                else if (reti == REG_NOMATCH) 
                {
                    ok = ok*0;
                    // printf("%d %d>No match\n", ok, j);
                    return 13;
                }
                else {
                    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
                    fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                    exit(1);
                }
                break;
            default:
                break;
        }
        regfree(&regex);
    }

    //OK est censé être un boolean donc 0 pour false, 1 pour true mais on veut retourner 0 si tout s'est bien passé
    return 1-ok;
}
