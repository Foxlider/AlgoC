/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serveur.h"
#include "json.h"
#include "validateur.h"

void plot(char *data) {

  //Extraire le compteur et les couleurs RGB 
  FILE *p = popen("gnuplot -persist", "w");
  printf("Plot");
  int count = 0;
  int n;
  char *saveptr = NULL;
  char *str = data;
  fprintf(p, "set xrange [-15:15]\n");
  fprintf(p, "set yrange [-15:15]\n");
  fprintf(p, "set style fill transparent solid 0.9 noborder\n");
  fprintf(p, "set title 'Top 10 colors'\n");
  fprintf(p, "plot '-' with circles lc rgbcolor variable\n");
  while(1) {
    char *token = strtok_r(str, ",", &saveptr);
    if (token == NULL) {
      break;
    }
    str=NULL;
    if (count == 0) {
      n = atoi(token);
    }
    else {
      // Le numéro 36, parceque 360° (cercle) / 10 couleurs = 36
      fprintf(p, "0 0 10 %d %d 0x%s\n", (count-1)*36, count*36, token+1);
    }
    count++;
  }
  fprintf(p, "e\n");
  printf("Plot: FIN\n");
  pclose(p);
}

/* renvoyer un message (*data) au client (client_socket_fd)
 */
int renvoie_nom(int client_socket_fd, char *data)
{
    char c[1000];
    char nom[512];
    gethostname(nom, 512);
    sprintf(c, "{\n\t\"code\" : \"message\",\n\t\"valeurs\" : [ \"%s\" ]\n}\n\n", nom);
    return renvoie_message(client_socket_fd, c);
}

int recois_numeros_calcule(int client_socket_fd, char *data)
{
  float ans;
  char code[20];
  char mode[2];
  char s_num1[100];
  char s_num2[100];
  float num1;
  float num2;

  char * ptr = strtok(data, " ");
  strcpy(code, ptr);
  
  ptr = strtok(NULL, " ");
  strcpy(mode, ptr);
  
  ptr = strtok(NULL, " ");
  strcpy(s_num1, ptr);
  
  ptr = strtok(NULL, " ");
  strcpy(s_num2, ptr);

  printf("mode : %s\n", mode);
  printf("code : %s\n", code);
  printf("n1 : %s\n", s_num1);
  printf("n2 : %s\n", s_num2);
  
  num1 = atof(s_num1);
  num2 = atof(s_num2);

  if (strcmp(mode, "+") == 0)
  { ans = num1 + num2; }
  if (strcmp(mode, "-") == 0)
  { ans = num1 - num2; }
  if (strcmp(mode, "*") == 0)
  { ans = num1 * num2; }
  if (strcmp(mode, "/") == 0)
  { ans = num1 / num2; }

  printf("RES : %f\n", ans);
  char c[1000]; //size of the number
  // sprintf(c, "%g", ans);
  sprintf(c, "{\n\t\"code\" : \"calcule\",\n\t\"valeurs\" : [ \"%s\", \"%.4f\", \"%.4f\" ],\n\t\"answer\" : \"%.4f\"\n}\n\n", mode, num1, num2, ans);
  return renvoie_message(client_socket_fd, c);
}

int recois_couleurs(int client_socket_fd, char *data)
{
  char out[2048];
  FILE* fichier = NULL;

  //On lit la commande
  char * ptr = strtok(data, " ");
  
  //On lit le nombre de couleurs
  ptr = strtok(NULL, " ");

  //On lit les couleurs
  ptr = strtok(NULL, " ");
  strcat(out, "{\n\t\"code\" : \"couleurs\",\n\t\"valeurs\" : [");
  while( ptr != NULL ) 
  {
    strcat(out, " \"#");
    strcat(out, ptr);
    strcat(out, "\"");
    printf( " %s\n", ptr );
  
    ptr = strtok(NULL, " ");
    if (ptr != NULL )
      strcat(out, ",");
  }
  strcat(out, "]\n}\n");

  printf("%s\n", out);
  fichier = fopen("couleurs.txt", "w+");
  if(fichier != NULL)
  {
    fprintf(fichier, out);
    fclose(fichier);
  }
  else
  { printf("Impossible d'ouvrir le fichier"); }
  
  renvoie_message(client_socket_fd, out);
}

int recois_balises(int client_socket_fd, char *data)
{
  char out[2048];
  char i_s[10];
  FILE* fichier = NULL;
  int i;

  
  //On lit la commande
  char * ptr = strtok(data, " ");

  //On lit le nombre de couleurs
  ptr = strtok(NULL, " ");
  //strcpy(nb, ptr);

  //On lit les couleurs
  ptr = strtok(NULL, " ");
  i = 0;
  strcat(out, "{\n\t\"code\" : \"balises\",\n\t\"valeurs\" : [");
  while( ptr != NULL ) 
  {
    strcat(out, " \"#");
    strcat(out, ptr);
    strcat(out, "\"");
    printf( " %s\n", ptr );
  
    ptr = strtok(NULL, " ");
    if (ptr != NULL )
      strcat(out, ",");
  }
  strcat(out, "]\n}\n");
  
  //printf("%s\n", out);
  fichier = fopen("balise.txt", "w+");
  if(fichier != NULL)
  {
    fprintf(fichier, out);
    fclose(fichier);
  }
  else
  { printf("Impossible d'ouvrir le fichier"); }
    
  renvoie_message(client_socket_fd, "Enregistré");
}


int renvoie_message(int client_socket_fd, char *data) {
  int data_size = write (client_socket_fd, (void *) data, strlen(data));
      
  if (data_size < 0) {
    perror("erreur ecriture");
    return(EXIT_FAILURE);
  }
}

/* accepter la nouvelle connection d'un client et lire les données
 * envoyées par le client. En suite, le serveur envoie un message
 * en retour
 */
int recois_envoie_message(int socketfd) {
  struct sockaddr_in client_addr;
  char data[1024];

  int client_addr_len = sizeof(client_addr);
 
  // nouvelle connection de client
  int client_socket_fd = accept(socketfd, (struct sockaddr *) &client_addr, &client_addr_len);
  if (client_socket_fd < 0 ) {
    perror("accept");
    return(EXIT_FAILURE);
  }

  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));

  //lecture de données envoyées par un client
  int data_size = read (client_socket_fd, (void *) data, sizeof(data));
      
  if (data_size < 0) {
    perror("erreur lecture");
    return(EXIT_FAILURE);
  }
  
  /*
   * extraire le code des données envoyées par le client. 
   * Les données envoyées par le client peuvent commencer par le mot "message :" ou un autre mot.
   */
  printf ("Message recu: %s\n", data);
  char code[10];
  sscanf(data, "%s", code);

  //Si le message commence par le mot: 'message:' 
  if (strcmp(code, "message:") == 0) {
    char message[100];
    printf("Reponse (max 1000 caracteres): ");
    scanf("%s", message);    

    char c[1000];
    sprintf(c, "{\n\t\"code\" : \"message\",\n\t\"valeurs\" : [ \"%s\" ]\n}\n\n", message);

    renvoie_message(client_socket_fd, c);
  }
  if (strcmp(code, "hostname:") == 0) 
  { renvoie_nom(client_socket_fd, data); }

  if (strcmp(code, "calcul:") == 0)
  { recois_numeros_calcule(client_socket_fd, data); }

  if (strcmp(code, "couleur:") == 0)
  { recois_couleurs(client_socket_fd, data); }

  if (strcmp(code, "balise:") == 0)
  { recois_balises(client_socket_fd, data); }
  if (strcmp(code, "graph:") == 0)
  {
    plot(data);
  }
  else {
    //plot(data);
  }

  //fermer le socket 
  close(socketfd);
}


int main() {

  int socketfd;
  int bind_status;
  int client_addr_len;

  struct sockaddr_in server_addr, client_addr;

  /*
   * Creation d'une socket
   */
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if ( socketfd < 0 ) {
    perror("Unable to open a socket");
    return -1;
  }

  int option = 1;
  setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  //détails du serveur (adresse et port)
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Relier l'adresse à la socket
  bind_status = bind(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (bind_status < 0 ) {
    perror("bind");
    return(EXIT_FAILURE);
  }
 

	// char output[1024];
  // char c1[254] = {0};
  // char c2[1024] = {0};
  // int ret;
  // char *buff[4] = {"#test" , "#pouic" , "#pouet"};

  // printf("TEST VALUE TO JSON\n");
  // format_value_to_json("test", "2", output);
  // printf("%s\n", output);
  // ret = validate_json_base(output, c1, c2);
  // printf("code:'%s'\ncontent:'%s'\n", c1, c2);
  // if(ret != 0)
  //   return 1;
  
  // printf("TEST ARRAY TO JSON\n");
  // format_array_to_json("test", buff, 3, output);
  // printf("%s\n", output);
  // ret = validate_json_base(output, c1, c2);
  // printf("code:'%s'\ncontent:'%s'\n", c1, c2);
  // if(ret != 0)
  //   return 1;


  // printf("TEST STRING TO JSON\n");
  // format_string_to_json("test", "pouic", output);
  // printf("%s\n", output);
  // ret = validate_json_base(output, c1, c2);
  // printf("code:'%s'\ncontent:'%s'\n", c1, c2);
  // if(ret != 0)
  //   return 1;
  

  // printf("TEST NUM TO JSON\n");
  // format_num_to_json("test", 3, output);
  // printf("%s\n", output);
  // ret = validate_json_base(output, c1, c2);
  // printf("code:'%s'\ncontent:'%s'\n", c1, c2);
  // if(ret != 0)
  //   return 1;


  // Écouter les messages envoyés par le client
  listen(socketfd, 10);

  //Lire et répondre au client
  recois_envoie_message(socketfd);

  return 0;
}
