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

#include "protocole.h" // contient la cle et la structure d'un message

int idQ, idShm;
char *pShm;
void handlerSIGUSR1(int sig);
int fd;

int main()
{
  MESSAGE msg ;
  char tmpchar ;
  // Armement des signaux
  // TO DO
    struct sigaction A;
  A.sa_handler = handlerSIGUSR1;
  A.sa_flags = 0;
  sigaction(SIGUSR1,&A,NULL);

  // Masquage des signaux
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask,SIGUSR1);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
 fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(PUBLICITE) Erreur de msgget");
    exit(1);
  }

  // Recuperation de l'identifiant de la mémoire partagée

  if ((idShm=shmget(CLE ,0,0))==-1)
  {
     perror("(PUBLICITE) Erreur de shmget");
    exit(1);
  }


  // Attachement à la mémoire partagée
  pShm = (char*)shmat(idShm,NULL,0); // 0pour lecture et ecriture , NULL car on choisit pas soi meme une adresse! 




  // Mise en place de la publicité en mémoire partagée
  char pub[51];
  strcpy(pub,"Bienvenue sur le site du Maraicher en ligne !");

  for (int i=0 ; i<=50 ; i++) pShm[i] = ' ';
  pShm[51] = '\0';
  int indDebut = 25 - strlen(pub)/2;
  for (int i=0 ; i<strlen(pub) ; i++) pShm[indDebut + i] = pub[i];

  while(1)
  {
    
    // Envoi d'une requete UPDATE_PUB au serveur
    msg.type=1 ;
    msg.expediteur=getpid();
    msg.requete=UPDATE_PUB;

  if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
      {
      //  perror("(PUB)erreur d'envoi sur la file de msg \n");
      // msgctl(idQ,IPC_RMID,NULL);
        exit(1);
      }
         
    sleep(1); 

    tmpchar =pShm[0];
    for (int i=0;i<50;i++)
    {
      pShm[i]=pShm[i+1];
    }
    pShm[50]=tmpchar;

    // Decallage vers la gauche
  }
}

void handlerSIGUSR1(int sig)
{
  fprintf(stderr,"(PUBLICITE %d) Nouvelle publicite !\n",getpid());
  MESSAGE m ;
   char pub[51];
  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),0,0) == -1)
  {
    perror("(PUB) Erreur de msgrcv - 4");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
   }

   strcpy(pub,m.data4);
  for (int i=0 ; i<=50 ; i++) pShm[i] = ' ';
  pShm[51] = '\0';
  int indDebut = 25 - strlen(pub)/2;
  for (int i=0 ; i<strlen(pub) ; i++) pShm[indDebut + i] = pub[i];
  
}
