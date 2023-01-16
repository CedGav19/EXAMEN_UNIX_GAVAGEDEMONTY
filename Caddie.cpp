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
#include <mysql.h>
#include "protocole.h"


int idQ;

ARTICLE articles[10]; //pour la gestion du caddie 
int nbArticles = 0;


int fdWpipe;
int pidClient;
int stock ;

//MYSQL* connexion;

void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Armement des signaux
  // TO DO
  struct sigaction A;
  A.sa_handler = handlerSIGALRM;
  A.sa_flags = 0;
   sigaction(SIGALRM, &A, NULL) ;

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }
  // Connexion à la base de donnée
  /* connexion = mysql_init(NULL);
 if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }*/ // plus utilse car mtn c'est acces qui fais la connexcion 

  MESSAGE m;
  MESSAGE reponse;
  
  char requete[200];
  char newUser[20];
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;
  int tmpalrm ;

  // Récupération descripteur écriture du pipe
  fdWpipe = atoi(argv[1]);
  
  while(1)
  {
     alarm(60);
      //fprintf(stderr,"------+++--(CADDIE %d) lancement de alarm\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }
      tmpalrm = alarm(0);
      //fprintf(stderr,"------+++--(CADDIE %d) reset de alarm il restait %d  sec  \n",getpid(),tmpalrm);

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                      pidClient = m.expediteur;
                      break;

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      exit(1);
                      break;

      case CONSULT :  // TO DO
                    fprintf(stderr,"(CADDIE %d) Requete CONSULT recue de %d\n",getpid(),m.expediteur);
                    
                    if (m.expediteur == 1)
                    {
                        // On va pipe le message vers Access
                        m.expediteur = getpid();
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT envoyé a accesBD par pipe\n ",getpid());
                      write(fdWpipe, &m, sizeof(MESSAGE));

                    }
                    else 
                    {
                        if ( m.data1!=-1 &&(stock = atoi(m.data3)) > 0)
                        {
                          m.expediteur = getpid();
                          m.type = pidClient;
                          fprintf(stderr,"(CADDIE %d) Requete CONSULT avec article n %d  envoyé a %d\n",getpid(),m.data1,m.type);
                          if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
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
                      if (m.expediteur == 1)
                      {

                        m.expediteur = getpid();
                        write(fdWpipe, &m, sizeof(MESSAGE));

                      }
                      else
                      {

                        m.type = pidClient;
                        m.expediteur = getpid();
                        m.requete = ACHAT;



                          if (strcmp(m.data3,"0") != 0 )
                          {

                              articles[nbArticles].id = m.data1;
                              strcpy(articles[nbArticles].intitule, m.data2);
                              articles[nbArticles].prix = m.data5;
                              articles[nbArticles].stock = atoi(m.data3);
                              strcpy(articles[nbArticles].image, m.data4);
  
                              nbArticles ++;

                              if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                              {
                                perror("(Caddie) Erreur de msgsnd");
                                msgctl(idQ,IPC_RMID,NULL);
                                exit(1);
                              }

                          }
                          else  // L'achat ne s'est pas effectué
                          {
                              if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                              {
                                perror("(Caddie) Erreur de msgsnd");
                                msgctl(idQ,IPC_RMID,NULL);
                                exit(1);
                              }
                          }
                          kill(pidClient, SIGUSR1);
                        }

                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      for ( int i=0 ;i<nbArticles;i++)
                      {
                        reponse.type = pidClient;
                        reponse.expediteur = getpid();
                        reponse.requete = CADDIE;
                        reponse.data1 = articles[i].id;
                        strcpy(reponse.data2,articles[i].intitule);
                        reponse.data5 = articles[i].prix;
                        sprintf (reponse.data3, "%d", articles[i].stock);
                        strcpy(reponse.data4, articles[i].image);

                          if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0) == -1)
                          {
                            perror("(Caddie) Erreur de msgsnd");
                            msgctl(idQ,IPC_RMID,NULL);
                            exit(1);
                          }
                          kill(pidClient, SIGUSR1);

                      };
                     
                     
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d  , pour l'article numero %d qui est %s  et qui se trouve %d fois en stock\n",getpid(),m.expediteur,m.data1,articles[m.data1].intitule,articles[m.data1].stock);

                      reponse .expediteur = getpid();
                      reponse.requete = CANCEL;
                      sprintf (reponse.data2, "%d", articles[m.data1].stock); 
                      reponse.data1 = articles[m.data1].id;
                      write(fdWpipe, &reponse, sizeof(MESSAGE));


                      // Suppression de l'aricle du panier
                      for (m.data1; m.data1 < nbArticles; m.data1++) //m=msgrecu
                      {
                        articles[m.data1].id = articles[(m.data1 + 1)].id;
                        strcpy(articles[m.data1].intitule ,articles[(m.data1 + 1)].intitule);
                        articles[m.data1].prix = articles[(m.data1 + 1)].prix;
                        articles[m.data1].stock = articles[(m.data1 + 1)].stock;
                        strcpy(articles[m.data1].image ,articles[(m.data1 + 1)].image); // voir structure de articles 
                      }
                      nbArticles--;
                      
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                        for (int i = 0; i < nbArticles; i++)
                        {
                          reponse.expediteur = getpid();
                          reponse.data1 = articles[i].id;
                          reponse.requete = CANCEL;
                          sprintf (reponse.data2, "%d", articles[i].stock); 

                          write(fdWpipe, &reponse, sizeof(MESSAGE));
                        }
                        nbArticles = 0;
                      break;

      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier
                       nbArticles = 0; // on reecrira sur les artciles enft , j'avais pense a tt supprime mais bco bcp de probleme et j'avais plus trop de temps...
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());
  MESSAGE msg ;
  // Annulation du caddie et mise à jour de la BD
  for (int i = 0; i < nbArticles; i++)
  {
    msg.expediteur = getpid();
    msg.data1 = articles[i].id;
    msg.requete = CANCEL;
    sprintf (msg.data2, "%d", articles[i].stock); 

    write(fdWpipe, &msg, sizeof(MESSAGE));
  }

  // Envoi d'un Time Out au client (s'il existe toujours)

    msg.type = pidClient;
    msg.expediteur = getpid();
    msg.requete = TIME_OUT;
    if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
    {
    perror("(Caddie) Erreur de msgsnd");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
    }

    kill (pidClient, SIGUSR1); 
         
  exit(0);
}