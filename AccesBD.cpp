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
 char requete[200];


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
                        reponse.data5 = atof(Tuple[5]);
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
                      // Acces BD

                      // Finalisation et envoi de la reponse
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Mise à jour du stock en BD
                      break;

    }
  }
}
