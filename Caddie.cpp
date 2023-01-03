#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <mysql.h>
#include <string>
#include <cstring>
#include <strings.h>

#include "protocole.h" // contient la cle et la structure d'un message

int idQ;

ARTICLE articles[10];
int nbArticles = 0;

int fdWpipe;
int pidClient;
int stock ;

MYSQL* connexion;

void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Armement des signaux
  // TO DO

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }
  // Connexion à la base de donnée
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  MESSAGE m;
  MESSAGE reponse;
  
  char requete[200];
  char newUser[20];
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;

  // Récupération descripteur écriture du pipe
  //fdWpipe = atoi(argv[1]);
  
  while(1)
  {
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                      break;

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      exit(1);
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      pidClient = m.expediteur;
                       
                        // acces a sql semblable a php de 1ere
                      sprintf(requete,"select * from UNIX_FINAL where id = %d", m.data1);
                      if (mysql_query(connexion, requete) != 0)
                      {
                        fprintf (stderr, "Erreur de Mysql-query");
                      }

                      if((resultat = mysql_store_result(connexion)) == NULL)
                      {
                        fprintf (stderr,"Erreur de mysql store");
                      }

                      if ((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        
                        reponse.requete=CONSULT ;
                        reponse.type = pidClient;
                        

                        reponse.expediteur = getpid();
                        
                        reponse.data1 = atoi(Tuple[0]);
                        strcpy(reponse.data2, Tuple[1]);
                        strcpy(reponse.data4, Tuple[4]);
                        strcpy(reponse.data3, Tuple[3]);
                        reponse.data5= atof(Tuple[2]);

                        /*char Prix[20];
                        strcpy(Prix, Tuple[2]);
                        String tmp4(Prix);
                        size_t x = tmp4.find(",");
                        if (x != String::npos) tmp4.replace(x,1,".");
                        
                        reponse.data5 = atof(tmp4.c_str());*/



                      fprintf(stderr,"(CADDIE %d) Requete CONSULT envoyé a %d\n",getpid(),reponse.type);
                        if ((stock = atoi(reponse.data3)) > 0);
                          {
                            if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0) == -1)
                            {
                              perror("(Caddie) Erreur de msgsnd");
                              msgctl(idQ,IPC_RMID,NULL);
                              exit(1);
                            }
                            kill(pidClient, SIGUSR1);
                          }
                      }
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // on transfert la requete à AccesBD
                      
                      // on attend la réponse venant de AccesBD
                        
                      // Envoi de la reponse au client

                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                      // on transmet la requete à AccesBD

                      // Suppression de l'aricle du panier
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                      // On envoie a AccesBD autant de requeres CANCEL qu'il y a d'articles dans le panier

                      // On vide le panier
                      break;

      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  // Annulation du caddie et mise à jour de la BD
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier

  // Envoi d'un Time Out au client (s'il existe toujours)
         
  exit(0);
}