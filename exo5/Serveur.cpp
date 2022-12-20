#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "protocole.h" // contient la cle et la structure d'un message

#include <cstring>

int idQ;
int pid1,pid2;

void handlerSIGINT(int sig);

int main()
{
  MESSAGE requete;
  pid_t destinataire;

  MESSAGE  msg1 , msg2 ; 
  char tempon[80]  ;
   

  // Armement du signal SIGINT
  // TO DO (etape 6)

  struct sigaction A;
  A.sa_handler = handlerSIGINT;
  A.sa_flags = 0;

  sigaction(SIGINT,&A,NULL);

  // Creation de la file de message
  fprintf(stderr,"(SERVEUR) Creation de la file de messages\n");
  // TO DO (etape 2)

  if((idQ = msgget(CLE,IPC_CREAT |IPC_EXCL |0600))==-1)
  {
    perror("Erreur de msgget (Serveur) \n "); 
    exit(1);

  }

  // Attente de connection de 2 clients
  fprintf(stderr,"(SERVEUR) Attente de connection d'un premier client...\n");
  // TO DO (etape 5)

  if(msgrcv(idQ,&msg2,sizeof(MESSAGE)-sizeof(long),1,0)==-1)
  {
    perror("erreur de reception du message ");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }
  pid1 = msg2.expediteur;

  fprintf(stderr,"(SERVEUR) Attente de connection d'un second client...\n");
  // TO DO (etape 5)

  if(msgrcv(idQ,&msg2,sizeof(MESSAGE)-sizeof(long),1,0)==-1)
  {
    perror("erreur de reception du message ");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }
  pid2 = msg2.expediteur;

  while(1) 
  {
    // TO DO (etapes 3, 4 et 5)
    strcpy(tempon,"(Serveur) " );
  
  	fprintf(stderr,"(SERVEUR) Attente d'une requete...\n");

    if(msgrcv(idQ,&msg2,sizeof(MESSAGE)-sizeof(long),1,0)==-1)
  {
    perror("erreur de reception du message ");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }
  strcat(tempon , msg2.texte);
  strcpy(msg1.texte, tempon );

  if (msg2.expediteur==pid1)
  {
    msg1.type=pid2;
  }
  else 
  {
    msg1.type=pid1;
  }
  
  if(msgsnd(idQ,&msg1,sizeof(MESSAGE)-sizeof(long),0)==-1)
  {
    perror("erreur d'envoi");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }

  kill(msg1.type,SIGUSR1);





    fprintf(stderr,"(SERVEUR) Requete recue de %d : --%s--\n",requete.expediteur,requete.texte);
    
    fprintf(stderr,"(SERVEUR) Envoi de la reponse a %d\n",destinataire);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO DO (etape 6)

void handlerSIGINT(int sig)
{

   msgctl(idQ,IPC_RMID,NULL);
  
}
