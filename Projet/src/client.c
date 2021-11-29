/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <netinet/in.h>

#include "client.h"
#include "bmp.h"
#include "json.h"
/* 
 * Fonction d'envoi et de réception de messages
 * Il faut un argument : l'identifiant de la socket
 */

int recois_envoie_message(int socketfd) {
 
  char data[1024];
  char message[1024];
  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));
  char nb_s[3];
  int nb = 0;

  printf("Choix du mode : \n");
  printf(" 1. : Hostname \n");
  printf(" 2. : Message\n");
  printf(" 3. : Calcul\n");
  printf(" 4. : Couleurs\n");
  printf(" 5. : Balises\n");
  printf(" 6. : Graph\n");

  //Securité pour l'input de la commande
  while (nb == 0)
  {
    //scanf("%s", nb_s);
    fgets(nb_s, 3, stdin);
    nb = atol(nb_s);
    if(nb > 6 || nb < 0)
    { nb = 0; }
  }

  //SWITCH COMMANDE
  switch(nb)
  {
    case 1:
      //ENVOI DU HOSTNAME
      renvoie_nom(socketfd);
      break;
    case 2:
      //ENVOI DE MESSAGE
      //Demandez à l'utilisateur d'entrer un message
      
      printf("Votre message (max 1000 caracteres): ");
      scanf("%s", message);
      
      format_string_to_json("message", message, data);
      
      int write_status = write(socketfd, data, strlen(data));
      if ( write_status < 0 ) {
        perror("erreur ecriture");
        exit(EXIT_FAILURE);
      }
      break;
    case 3:
      //ENVOI DE CALCUL
      envoie_operateur_numeros(socketfd, message);
      break;
    case 4:
      //ENVOI DE COULEURS
      envoie_couleurs(socketfd, data);
      break;
    case 5:
      //ENVOI DE BALISES
      envoie_balises(socketfd, data);
      break;
    case 6:
      //ENVOI GRAPH
      envoi_graph(socketfd, data);
      break;
  }

  //LECTURE REPONSE

  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));


  // lire les données de la socket
  int read_status = read(socketfd, data, sizeof(data));
  if ( read_status < 0 ) {
    perror("erreur lecture");
    return -1;
  }

  printf("Message recu: %s\n", data);
 
  return 0;
}

void analyse(char *pathname, char *data) {
  //compte de couleurs
  couleur_compteur *cc = analyse_bmp_image(pathname);

  int nbrCouleurs = -1;
  int count;
  printf("\nNombre de couleurs a recuperer : (entre 0 et 30)\n");
  while (nbrCouleurs < 0 || nbrCouleurs > 30) {
    scanf("%d", &nbrCouleurs);
  }
  char temp_string[nbrCouleurs+1];

  if (cc->size < nbrCouleurs) {
    sprintf(temp_string, "%d,", cc->size);
  }
  strcat(data, temp_string);
  printf(">%s\n", data);
  //choisir 10 couleurs
  for (count = 1; count < nbrCouleurs+1 && cc->size - count >0; count++) {
    if(cc->compte_bit ==  BITS32) {
      sprintf(temp_string, "\"#%02x%02x%02x\", ", cc->cc.cc24[cc->size-count].c.rouge,cc->cc.cc32[cc->size-count].c.vert,cc->cc.cc32[cc->size-count].c.bleu);
    }
    if(cc->compte_bit ==  BITS24) {
      sprintf(temp_string, "\"#%02x%02x%02x\", ", cc->cc.cc32[cc->size-count].c.rouge,cc->cc.cc32[cc->size-count].c.vert,cc->cc.cc32[cc->size-count].c.bleu);
    }
    strcat(data, temp_string);
  }
  printf(">%s\n", data);

  //enlever le dernier virgule
  data[strlen(data)-2] = '\0';
}


int renvoie_nom(int socketfd)
{
  char message[1000];
  char nom[1024];
  gethostname(nom, 1023);
  // strcpy(message, "hostname: ");
  // strcat(message, nom);
  printf("Nom :%s\n", nom);
  format_string_to_json("hostname", nom, message);
  int write_status = write(socketfd, message, strlen(message));
  if ( write_status < 0 ) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }
}


int envoie_operateur_numeros(int socketfd, char *data)
{
  memset(data, 0, sizeof(data));
  char message[1000];
  char in[1024];
  printf("Calcul : ");
  fflush(stdin);
  fgets(in, 1024, stdin);
  in[strcspn(in, "\r\n")] = 0; //on enlève le retour à la ligne

  char *p = strtok(in, " ");
  int i = 0;
  char *array[3];

  while (p != NULL)
  {
      array[i++] = p;
      p = strtok(NULL, " ");
  }
  //On découpe l'input en différents caractères : operateur n1 n2

  format_array_to_json("calcul", array, 3, message);
  printf("Calcul :%s\n", message);
  int write_status = write(socketfd, message, strlen(message));
  if ( write_status < 0 ) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }
}

int envoie_couleurs(int socketfd, char *data)
{
  memset(data, 0, sizeof(data));
  char couleur[10];
  char output[1024] = {0};
  char nb_s[3];
  int nb = 0;

  //Securité pour l'input du nombre de couleurs
  while (nb == 0)
  {
    printf("Nombre de couleurs : ");
    scanf("%s", nb_s);
    nb = atol(nb_s);
    if(nb > 30)
    { nb = 0; }
  }

  //On input les couleurs et on les parse
  for (int i = 0; i < nb; i++)
  {
    printf("Couleur %d : #", i);
    scanf("%s", couleur);
    strcat(output, "\"#");
    strcat(output, couleur);
    strcat(output, "\"");
    if(i < nb-1)
      strcat(output, ", ");
  }

  format_value_to_json("couleurs", output, data);
  
  //On envoie
  int write_status = write(socketfd, data, strlen(data));
  if ( write_status < 0 ) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }

  return 0;
}

int envoie_balises(int socketfd, char *data)
{
  memset(data, 0, sizeof(data));
  char output[1024] = {0};
  char balise[100];
  char nb_s[3];
  int nb = 0;

  //Sécurité pour l'input des balises
  while (nb == 0)
  {
    printf("Nombre de balises : ");
    scanf("%s", nb_s);
    nb = atol(nb_s);
    if(nb > 30)
    { nb = 0; }
  }

  //Input et parse de la commande
  for (int i = 0; i < nb; i++)
  {
    printf("Balise %d : #", i);
    scanf("%s", balise);
    strcat(output, "\"#");
    strcat(output, balise);
    strcat(output, "\"");
    if(i < nb-1)
      strcat(output, ", ");
  }

  format_value_to_json("balise", output, data);
  //Envoi
  int write_status = write(socketfd, data, strlen(data));
  if ( write_status < 0 ) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }

  return 0;
}

int envoi_graph(int socketfd, char *pathname) {
  char data[1024];
  char output[1024];
  memset(data, 0, sizeof(data));

  printf("Chemin de l'image : ");
  scanf("%s", pathname);

  analyse(pathname, data);
  
  format_value_to_json("graph", data, output);

  int write_status = write(socketfd, output, strlen(output));
  if ( write_status < 0 ) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }

  return 0;
}


int main(int argc, char **argv) {
  int socketfd;
  int bind_status;

  struct sockaddr_in server_addr, client_addr;

  /*
   * Creation d'une socket
   */
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if ( socketfd < 0 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  //détails du serveur (adresse et port)
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  //demande de connection au serveur
  int connect_status = connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if ( connect_status < 0 ) {
    perror("connection serveur");
    exit(EXIT_FAILURE);
  }
  recois_envoie_message(socketfd);
  //envoie_couleurs(socketfd, argv[1]);

  close(socketfd);
}
