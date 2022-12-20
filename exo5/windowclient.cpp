#include "windowclient.h"
#include "ui_windowclient.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <cstring>

extern WindowClient *w;

#include "protocole.h" // contient la cle et la structure d'un message

extern char nomClient[40];
int idQ; // identifiant de la file de message

void handlerSIGUSR1(int sig);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowClient::WindowClient(QWidget *parent):QMainWindow(parent),ui(new Ui::WindowClient)
{
  ui->setupUi(this);
  setWindowTitle(nomClient);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CLIENT %s %d) Recuperation de l'id de la file de messages\n",nomClient,getpid());
  // TO DO (etape 2)
  if((idQ = msgget(CLE,0))==-1)
  {
    perror("Erreur de msgget \n "); 
    exit(1);

  } //recupere l'id dans idQ


  // Envoi d'une requete d'identification
  // TO DO (etape 5)

  MESSAGE  msg1 ; 
  
  msg1.type=1 ;
  msg1.expediteur=getpid();

  strcpy(msg1.texte , "identification");
  printf(" envoie de la requete : %s \n ",msg1.texte);


  if(msgsnd(idQ,&msg1,sizeof(MESSAGE)-sizeof(long),0)==-1)
  {
    perror("erreur d'envoi");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }



  // Armement du signal SIGUSR1
  // TO DO (etape 4)


  struct sigaction B;
  B.sa_handler = handlerSIGUSR1;
  B.sa_flags = 0;

  sigaction(SIGUSR1,&B,NULL);


}

WindowClient::~WindowClient()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setRecu(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditRecu->clear();
    return;
  }
  ui->lineEditRecu->setText(Text);
}

void WindowClient::setAEnvoyer(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditEnvoyer->clear();
    return;
  }
  ui->lineEditEnvoyer->setText(Text);
}

const char* WindowClient::getAEnvoyer()
{
  if (ui->lineEditEnvoyer->text().size())
  { 
    strcpy(aEnvoyer,ui->lineEditEnvoyer->text().toStdString().c_str());
    return aEnvoyer;
  }
  return NULL;
}

const char* WindowClient::getRecu()
{
  if (ui->lineEditRecu->text().size())
  { 
    strcpy(recu,ui->lineEditRecu->text().toStdString().c_str());
    return recu;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonEnvoyer_clicked()
{
  fprintf(stderr,"Clic sur le bouton Envoyer\n");
  // TO DO (etapes 2, 3, 4)

  MESSAGE  msg1 ; 
  
  msg1.type=1 ;
  msg1.expediteur=getpid();

  strcpy(msg1.texte , getAEnvoyer());
  printf(" envoie de la requete : %s \n ",msg1.texte);


  if(msgsnd(idQ,&msg1,sizeof(MESSAGE)-sizeof(long),0)==-1)
  {
    perror("erreur d'envoi");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }

printf("le pid est = a %d ",getpid());
   printf("\n");


 

}

void WindowClient::on_pushButtonQuitter_clicked()
{
  fprintf(stderr,"Clic sur le bouton Quitter\n");
  exit(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO DO (etape 4)


void handlerSIGUSR1(int sig )
{
  MESSAGE msg2 ;
   if(msgrcv(idQ,&msg2,sizeof(MESSAGE)-sizeof(long),getpid(),0)==-1)
  {
    perror("erreur de reception du message ");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);

  }
  printf("dans le sigusr1 ");
   w->setRecu(msg2.texte);
}
