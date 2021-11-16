#ifndef __JSON_H__
#define __JSON_H__

int format_string_to_json(char *code, char *content, char *output);
int format_value_to_json(char *code, char *content, char *output);
int format_num_to_json(char *code, float content, char *output);
int format_array_to_json(char *code, char **content , int count, char *output);

#endif