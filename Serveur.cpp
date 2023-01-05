#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include "protocole.h" // contient la cle et la structure d'un message

#include "FichierClient.h"
int idQ,idShm,idSem,idfilspub,idfilscaddie,idfilsaccesBD;
int fdPipe[2];
TAB_CONNEXIONS *tab;
int jmp;
sigjmp_buf contexte;

void afficheTab();

void handlerSIGINT(int sig);
void handlerSIGCHLD(int sig);
int main()
{
  // Armement des signaux
  // TO DO
  //Armement de sigint pour pouvoir couper le serveur et la file de msg en meme temps
   struct sigaction A;
  A.sa_handler = handlerSIGINT;
  A.sa_flags = 0;
  sigaction(SIGINT,&A,NULL);

 
  A.sa_handler = handlerSIGCHLD;
  A.sa_flags = 0;
  sigaction(SIGCHLD,&A,NULL);


  // Creation des ressources
  // Creation de la file de message
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0666)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }
  fprintf(stderr,"(SERVEUR %d) Creation de la mem partagee\n",getpid());
  // creation de la memoire partagée 

  if((idShm=shmget(CLE,52,IPC_CREAT | IPC_EXCL | 0666)) == -1) // 52 car taille de 52 carcatere 
  {
    perror("(SERVEUR) Erreur de memoire partagée ");
    exit(1);
  }

  // TO BE CONTINUED

  // Creation du pipe
  // TO DO
  if (pipe(fdPipe) == -1)
  {
    fprintf (stderr, "Erreur de pipe !");
    exit(1);
  }

  // Initialisation du tableau de connexions
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = 0;

  afficheTab();

  // Creation du processus Publicite (étape 2)
  // TO DO

  idfilspub=fork();
  if (idfilspub==0)
  {
    if(execl("./Publicite", "Publicite", NULL) == -1)
    {
      perror("erreur lors de la pub");
    }

  }
  
  // Creation du processus AccesBD (étape 4)

   if((idfilsaccesBD = fork()) == -1)
  {
    perror("Erreur de fork pour acces bd ");
    exit(1);
  }
  char tamponchar[15];
  if(idfilsaccesBD == 0)
  {
    sprintf(tamponchar, "%d", fdPipe[0]); // j'ai essaye itoa mais pas sous linux de ce que j'ai vu en ligne !
    if(execl("AccesBD", "AccesBD", tamponchar, NULL) == -1)
    {
      perror("Erreur execl de Acces Bd\n");
      exit(1);
    }
  }
  tab->pidAccesBD = idfilsaccesBD;
  tab->pidPublicite=idfilspub;

  MESSAGE m;
  MESSAGE reponse;

  while(1)
  {
    jmp = sigsetjmp(contexte, 1);

  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }
    else 
    switch(m.requete)
    {
      case CONNECT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);

                      for (int i=0 ; i<6 ; i++)
                      {
                        if(tab->connexions[i].pidFenetre == 0)
                          {
                            tab->connexions[i].pidFenetre = m.expediteur ;
                            i=6;
                          }
                      }
                      break;

      case DECONNECT : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);

                       for (int i=0 ; i<6 ; i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            tab->connexions[i].pidFenetre = 0 ;
                            
                            i=6;
                          }
                      }                     
                      break;
      case LOGIN :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);  

                      if (m.data1==1)
                        {
                          if (estPresent(m.data2)>0)
                          {
                            strcpy(m.data4,"«Client  déjà  existant!»");
                            m.data1=0;
                            }
                          else 
                          {
                            strcpy(m.data4,"«Nouveau client créé:bienvenue!»");

                              for (int i=0 ; i<6 ; i++)
                              {
                                if(tab->connexions[i].pidFenetre == m.expediteur)
                                  {
                                   strcpy(tab->connexions[i].nom,m.data2);
                                    i=6;
                                  }
                              } 
                              ajouteClient(m.data2 ,m.data3);
                              m.data1=1;
                            
                          }
                        }
                        else 
                        {
                          if (estPresent(m.data2)>0)
                          {
                              if (verifieMotDePasse(estPresent(m.data2), m.data3 )==1)
                              {
                                strcpy(m.data4,"«Re-bonjour cher client!»");
                                  for (int i=0 ; i<6 ; i++)
                                  {
                                    if(tab->connexions[i].pidFenetre == m.expediteur)
                                      {
                                       strcpy(tab->connexions[i].nom,m.data2);
                                        i=6;
                                      }
                                  } 
                                  m.data1=1;
                              }
                              else 
                              {
                                 strcpy(m.data4,"«Mot  de passe incorrect...»");
                                 m.data1=0;
                              }
                          }
                          else 
                          {
                            strcpy(m.data4,"«Client Inconnu ...»");
                            m.data1=0;
                          }

                        }
                        // on a verifié le login on envoi donc le msg vers le client 

                        m.type=m.expediteur;
                         if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1)
                          {
                            perror("erreur d'envoi");
                            msgctl(idQ,IPC_RMID,NULL);
                            exit(1);
                          }
                          kill(m.type,SIGUSR1);

                          for(int i=0;i<6 && m.data1==1;i++) // connexion etablie 
                          {
                            if(tab->connexions[i].pidFenetre == m.expediteur)
                            {
                               if((idfilscaddie = fork()) == 0)  
                            
                              {
                                //code executé par le fils (caddie)
                                sprintf (tamponchar, "%d", fdPipe[1]);
                                if(execlp("Caddie", "Caddie",tamponchar,  NULL) == -1)
                                {
                                  perror("Erreur d execution de Caddie\n");
                                  exit(1);
                                }
                              }
                              //code executé par le pere (serveur) -> preveitn le caddie avec une requete logina  quel client il est connecte 
                              tab->connexions[i].pidCaddie = idfilscaddie;

                              m.type = tab->connexions[i].pidCaddie;
                              if (msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                              {
                                  fprintf (stderr, "Erreur de msgsnd - 6");
                                  msgctl(idQ,IPC_RMID,NULL);
                                  exit(1);
                              }



                            }
                          }



                      
                      break; 

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      for (int i=0 ; i<6 ; i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                             strcpy(tab->connexions[i].nom,""); // suppression du client dans le tableau 

                            //suppression du caddie et de son processus 
                            reponse.type = tab->connexions[i].pidCaddie;
                            reponse.requete = LOGOUT;
                            reponse.expediteur=m.expediteur;
                            if (reponse.type!=0)
                            {
                              if (msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0) == -1)
                              {
                                  perror("Erreur de msgsnd ");
                                  msgctl(idQ,IPC_RMID,NULL);
                                  exit(1);
                              }

                                i=6;
                            }
                          }
                      } 

                       


                      
                      break;

      case UPDATE_PUB :  // TO DO
                        
                        for (int i = 0 ; i<6; i++)
                        {
                          if (tab->connexions[i].pidFenetre!=0)kill(tab->connexions[i].pidFenetre, SIGUSR2);
                        }
                        
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      
                       // on regarde si le pidfenetre est l'expediteur pour etre sur de pas ecrire n'importe ou 
                      for (int i = 0; i < 6; i++)
                        {
                          if (tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;
                            i = 6;
                          }
                        }
                        m.requete=CONSULT;
                        m.expediteur= 1 ; // plus besoin de laisser le pid du client car mltn chaque caddie connait son client depuis le login

                        fprintf(stderr,"(SERVEUR %d) Requete CONSULT envoyé a   %d\n",getpid(),m.type);
                        // on garde le meme expediteur et la meme requete de maniere a ce que le caddie puisse retrouver le pidclient
                        if (msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1) // type = pid du caddie donc lu par le rcv du caddie 
                        {
                          perror("(Serveur) Erreur de msgsnd - 3");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case PAYER : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
      case 100 : // pour tester ce queon nous renvoie 
                fprintf(stderr,"(SERVEUR %d) Requete 100 reçue de %d\n",getpid(),m.expediteur);
      break;
    }
    afficheTab();
  }
}

void afficheTab()
{
  fprintf(stderr,"Pid Serveur   : %d\n",tab->pidServeur);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid AccesBD   : %d\n",tab->pidAccesBD);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
  fprintf(stderr,"\n");
}


//definition de sigint 
void handlerSIGINT(int sig)
{
  kill(idfilspub,SIGINT);
  shmctl(idShm,IPC_RMID,NULL);
  msgctl(idQ,IPC_RMID,NULL);

  if (close(fdPipe[1]) == -1)
  {
    printf("Erreur de fermeture en de pipe d'ecriture\n");
  }

  if (close(fdPipe[0]) == -1)
  {
    printf("Erreur de fermeture en de pipe de lecture\n");
  }



   exit(1);
  
}

//gestion des fils 
void handlerSIGCHLD(int sig2)
{
  fprintf(stderr,"(SERVEUR %d) reception de sigchld\n",getpid());
  idfilscaddie = wait(NULL);

  for (int i = 0; i < 6; i++)
  {
    if (tab->connexions[i].pidCaddie == idfilscaddie)
    {
      tab->connexions[i].pidCaddie = 0;
      strcpy(tab->connexions[i].nom, "\0");
      i = 6;
    }
  }
   siglongjmp(contexte, 406);
}
