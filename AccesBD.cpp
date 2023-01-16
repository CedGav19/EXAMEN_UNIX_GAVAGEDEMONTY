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
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;
MYSQL_RES  *resultat;
MYSQL_ROW  Tuple;
MYSQL* connexion;
int qtedispo =0;
int qtedemandee=0;
int newqte ;
 char requete[150];


int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(ACCESBD %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération descripteur lecture du pipe
  int fdRpipe = atoi(argv[1]);

  // Connexion à la base de donnée
  // TO DO
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  MESSAGE m;
  MESSAGE reponse;
                                                                                                  /*************************if (mysql_query(connexion, "UPDATE UNIX_FINAL SET stock = 10") != 0)
                                                                                                        {
                                                                                                                fprintf (stderr, "Erreur de Mysqlquery\n");
                                                                                                   }********************$*/

  while(1)
  {
    // Lecture d'une requete sur le pipe
    // TO DO
     read(fdRpipe, &m, sizeof(MESSAGE));

    switch(m.requete)
    {
      case CONSULT :  // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD


                      // comme en php 
                      sprintf(requete,"select * from UNIX_FINAL where id = %d", m.data1);

                      if (mysql_query(connexion, requete) != 0)
                      {
                        fprintf (stderr, "Erreur de Mysql-query");
                      }

                      if((resultat = mysql_store_result(connexion)) == NULL)
                      {
                        fprintf (stderr, "Erreur de mysql store");
                      }
                      //
                      // Preparation de la reponse
                      if ((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = CONSULT;
                        reponse.data1 = atoi(Tuple[0]);
                        strcpy(reponse.data2, Tuple[1]);
                         strcpy(reponse.data3, Tuple[3]);
                        strcpy(reponse.data4, Tuple[4]);
                        reponse.data5 = atof(Tuple[2]);
                        // Envoi de la reponse au bon caddie grace a exped qui contenait le pid 
                        // caddie qui recevra et renverra a son client
                      }
                      else m.data1=-1;

                      // envoie de la requete ou data1 = id article et -1 si il y a pas
                      if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0) == -1)
                      {
                        perror("(AccesBD) Erreur de msgsnd");
                        msgctl(idQ,IPC_RMID,NULL);
                        exit(1);
                      }


                      // Envoi de la reponse au bon caddie
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // Acces BD , comme ne php l'annee passee 
                      sprintf(requete,"select * from UNIX_FINAL where id = %d", m.data1);
                      if (mysql_query(connexion, requete) != 0)
                      {
                        fprintf (stderr, "Erreur de Mysql-query");
                      }
                      if((resultat = mysql_store_result(connexion)) == NULL)
                      {
                        fprintf (stderr, "Erreur de mysql store");
                      }
                      if ((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                            
                        qtedispo = atoi(Tuple[3]);
                        qtedemandee = atoi(m.data2);//conversion pour pouvoir faire les calcus 
                        reponse.type = m.expediteur; 
                        reponse.expediteur = getpid();
                        reponse.requete = ACHAT;
                        reponse.data1 = atoi(Tuple[0]);
                        strcpy(reponse.data2, Tuple[1]);
                        strcpy(reponse.data4, Tuple[4]);

                        if (qtedemandee > qtedispo)
                        {
                         sprintf(reponse.data3,"0"); // condition donne par le prof , on renvoi 0 quand pas possible 
                        }
                        else
                        {
                          //maj
                          newqte = qtedispo - qtedemandee;

                          sprintf(reponse.data3,  m.data2);//qte
                          sprintf(requete,"UPDATE UNIX_FINAL SET stock = %d where id = %d", newqte, reponse.data1);
                          if (mysql_query(connexion, requete) != 0) //requete de mise a jour
                          {
                            fprintf (stderr, "Erreur de Mysql-query");
                          }

                        }


                        // Finalisation et envoi de la reponse
                        if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0) == -1)
                        {
                          perror("(AccesBD) Erreur de msgsnd");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }

                      }
                      // Finalisation et envoi de la reponse
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                       newqte= atoi(m.data2);
                      sprintf(requete,"UPDATE UNIX_FINAL SET stock = stock + %d where id = %d", newqte, m.data1);
                      fprintf(stderr,"%s  Envoye a la BDD\n",requete);
                      if (mysql_query(connexion, requete) != 0)
                      {
                        fprintf (stderr, "Erreur de Mysqlquery\n");
                      }

                      if(mysql_store_result(connexion) == NULL)
                      {
                        fprintf (stderr, "Erreur de mysqlstoreresult\n");
                      }
                      break;

    }
  }
}
