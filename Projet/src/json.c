#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"


int format_string_to_json(char *code, char *content, char *output)
{
  sprintf(output, "{\n\t\"code\" : \"%s\",\n\t\"valeurs\" : [ \"%s\" ]\n}\n", code, content);
  return 0;
}

int format_value_to_json(char *code, char *content, char *output)
{
  sprintf(output, "{\n\t\"code\" : \"%s\",\n\t\"valeurs\" : [ %s ]\n}\n", code, content);
  return 0;
}

int format_num_to_json(char *code, float content, char *output)
{
  sprintf(output, "{\n\t\"code\" : \"%s\",\n\t\"valeurs\" : [ %.4f ]\n}\n", code, content);
  return 0;
}

int format_array_to_json(char *code, char **content , int count, char *output)
{
  sprintf(output, "{\n\t\"code\" : \"%s\",\n\t\"valeurs\" : [ ", code);
  int i=0;
  for(i=0;i<count;i++)
  {
    sprintf(output, "%s\"%s\"", output, content[i]);
    if(i != count-1)
    { sprintf(output, "%s, ", output); }
  }
  sprintf(output, "%s ]\n}\n", output);
  return 0;
}

int parse_json_string_to_array(char *data, char **array, int rows, int cols)
{
  char* cell;
  char c[strlen(data)+1];
  strcpy(c, data);
  // printf("%s\n%s\n", data, c);
  
  char *ptr = strtok(c, "\"");
  int i = 0;
  int j = 0;

  while( ptr != NULL || j < rows) 
  {
    // printf("%d %d> %s (%u)", i, j, ptr, strlen(ptr));
    if(i%2 == 0)
    {
      strcpy(array[j], ptr);
      j++;
    }
    // printf("\n");
    ptr = strtok(NULL, "\"");
    i++;
  }
  return 0;
}