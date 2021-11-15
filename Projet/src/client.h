/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__

/*
 * port d'ordinateur pour envoyer et recevoir des messages
 */
#define PORT 8089

/* 
 * Fonction d'envoi et de réception de messages
 * Il faut un argument : l'identifiant de la socket
 */
int recois_envoie_message(int socketfd);
void analyse(char *pathname, char *data);
int renvoie_nom(int socketfd);
int envoie_operateur_numeros(int socketfd, char *data);
int envoie_couleurs(int socketfd, char *data);
int envoie_balises(int socketfd, char *data);
int envoi_graph(int socketfd, char *pathname);


#endif
