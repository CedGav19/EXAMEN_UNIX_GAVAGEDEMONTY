#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <string>
using namespace std;

#include "protocole.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

extern WindowClient *w;

int idQ, idShm;
bool logged=0;
char* pShm;
ARTICLE articleEnCours;
float totalCaddie = 0.0;

void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);

#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de l'identifiant de la file de messages
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    // TO DO
    if((idQ = msgget(CLE,0))==-1)
  {
    perror("Erreur de msgget \n "); 
    exit(1);

  } 

    // Recuperation de l'identifiant de la mémoire partagée
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la mémoire partagée\n",getpid());
    // TO DO

    if ((idShm=shmget(CLE ,0,0))==-1)
    {
       perror("(PUBLICITE) Erreur de shmget");
      exit(1);
    }
    // Attachement à la mémoire partagée
    // TO DO
    pShm = (char*)shmat(idShm,NULL,0);
    // Armement des signaux
    // TO DO
     struct sigaction A;
    A.sa_handler = handlerSIGUSR1;
    A.sa_flags = 0;
    sigaction(SIGUSR1,&A,NULL);

    struct sigaction B;
    B.sa_handler = handlerSIGUSR2;
    B.sa_flags = 0;
    sigaction(SIGUSR2,&B,NULL);

    // Envoi d'une requete de connexion au serveur
    // TO DO


    MESSAGE  msg1 ; 
  
      msg1.type=1 ;
      msg1.expediteur=getpid();
      msg1.requete= CONNECT ;

      strcpy(msg1.data2 , "connexion");
      printf(" envoie de la requete : %s \n ",msg1.data2);


      if(msgsnd(idQ,&msg1,sizeof(MESSAGE)-sizeof(long),0)==-1)
      {
        perror("erreur d'envoi");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
      }

}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  totalCaddie = 0.0;
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  // TO DO (étape 1)
  MESSAGE  msg ; 

  if (logged==1)
  {
    msg.type = 1;
    msg.requete = CANCEL_ALL;
    msg.expediteur = getpid();
    if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
    {
        perror(" Erreur de msgsnd ");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
    }
  }
  msg.type=1 ;
  msg.expediteur=getpid();

  // envoi d'un logout si logged
  if (logged==1)
  {
    msg.requete= LOGOUT ;
      

    if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
    {
      perror("erreur d'envoi");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

  }
  // Envoi d'une requete de deconnexion au serveur
      msg.requete= DECONNECT ;

      if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
      {
        perror("erreur d'envoi");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
      }

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
    // Envoi d'une requete de login au serveur
      MESSAGE msg;
      msg.type=1 ;
      msg.expediteur=getpid();
      msg.requete= LOGIN ;
      msg.data1 = isNouveauClientChecked() ;
      strcpy(msg.data2,getNom() );
      strcpy(msg.data3,getMotDePasse());
      

      if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
      {
        perror("erreur d'envoi");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
      }

   
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
    // Envoi d'une requete CANCEL_ALL au serveur (au cas où le panier n'est pas vide)
    // TO DO
      MESSAGE  msg ; 
    
      msg.type = 1;
      msg.requete = CANCEL_ALL;
      msg.expediteur = getpid();

      if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
      {
          perror("(Client) Erreur de msgsnd - Suppression article");
          msgctl(idQ,IPC_RMID,NULL);
          exit(1);
      }
    // Envoi d'une requete de logout au serveur
    // TO DO

      msg.type=1 ;
      msg.expediteur=getpid();
      msg.requete= LOGOUT ;
      

      if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
      {
        perror("erreur d'envoi");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
      }


     
      logged=0;
      logoutOK();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
  MESSAGE  msg ;
  msg.type = 1;
  msg.requete = CONSULT;
  msg.expediteur = getpid();
  msg.data1 = (articleEnCours.id+1);


  if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
  {
      perror("(Client) Erreur de msgsnd");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
  MESSAGE  msg ;
  msg.type = 1;
  msg.requete = CONSULT;
  msg.expediteur = getpid();
  msg.data1 = (articleEnCours.id-1);


  if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
  {
      perror("(Client) Erreur de msgsnd");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
    // TO DO (étape 5)
    // Envoi d'une requete ACHAT au serveur
   MESSAGE  msg ;
  if (getQuantite() > 0)
  {
    msg.data1 = articleEnCours.id;      
    msg.requete = ACHAT;
    msg.type = 1;
    msg.expediteur = getpid();
    sprintf(msg.data2, "%d", getQuantite());

    if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
    {
        perror("(Client) Erreur de msgsnd (achat) ");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
    }
  }
  else
  {
      w->dialogueMessage("Achat", "Veuillez entrer une valeur plus garnde que 0 , car on veut acheter ici pas vendre ou ne rien faire ... ");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL au serveur
  MESSAGE msg;
    // Mise à jour du caddie
   if ((msg.data1=getIndiceArticleSelectionne() )== -1 )
    {
        w->dialogueMessage("Erreur :", "Aucun article séléctionné !");
    }
    else
    {
      msg.type = 1;
      msg.requete = CANCEL;
      msg.expediteur = getpid();

      if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
      {
          perror("(Client) Erreur de msgsnd - Suppression article");
          msgctl(idQ,IPC_RMID,NULL);
          exit(1);
      }


      // maj caddie
     
      w->videTablePanier();
      totalCaddie = 0.0;
      w->setTotal(0.0);
      msg.type = 1;
      msg.requete = CADDIE;
      msg.expediteur = getpid();

      if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
      {
      perror("(Client) Erreur de msgsnd");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
      }

    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
  // TO DO (étape 6)
  MESSAGE msg ;
  // Envoi d'une requete CANCEL_ALL au serveur
  msg.type = 1;
  msg.requete = CANCEL_ALL;
  msg.expediteur = getpid();

  if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
  {
      perror("(Client) Erreur de msgsnd - Suppression article");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
  }

    // Maj caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);
    msg.type = 1;
    msg.requete = CADDIE;
    msg.expediteur = getpid();

    if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
    {
    perror("(Client) Erreur de msgsnd");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
    // TO DO (étape 7)
    // Envoi d'une requete PAYER au serveur
  MESSAGE msg ;
    msg.type = 1;
    msg.requete = PAYER;
    msg.expediteur = getpid();
    msg.data1 = getIndiceArticleSelectionne();

    if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
    {
        perror("(Client) Erreur de msgsnd - Suppression article");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
    }
    char tmp[100];
    sprintf(tmp,"Merci pour votre paiement de %.2f ! Votre commande sera livrée tout prochainement.",totalCaddie);
    dialogueMessage("Payer...",tmp);

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR1(int sig)
{
    MESSAGE m;
    char affiche [50];
      float tmpqte=0;
    
    while (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),IPC_NOWAIT) != -1)  // !!! a modifier en temps voulu !!!
    {fprintf(stderr,"(CLIENT %d) Reception d'une requete  de %d\n",getpid(),m.expediteur  );
      switch(m.requete)
      {
        case LOGIN :
                    // affichage de login
                     fprintf(stderr,"(CLIENT %d) Requete login reçue de %d\n",getpid(),m.expediteur);
                     w->dialogueMessage("LOGIN",m.data4);
                    if (m.data1==1)
                    {
                      w->loginOK();
                      logged=1;
                      
                      
                      // dmd de reception du panier
                      m.type = 1;
                      m.requete = CONSULT;
                      m.expediteur = getpid();
                      m.data1 = 1; //id article dans ce cas , on dmd 1 car premiere reception 

                      if (msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                      {
                          perror("(Client) Erreur de msgsnd");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                      }
                    }
                    break;

        case CONSULT : // TO DO (étape 3)
                    //initialisation de l'article 
                    fprintf(stderr,"(CLIENT %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                    fprintf(stderr,"(CLIENT %d) Reception de l'article numero %d\n",getpid(),m.data1 );
                    articleEnCours.id = m.data1;
                    strcpy(articleEnCours.intitule, m.data2);
                    articleEnCours.prix = m.data5;
                    articleEnCours.stock = atoi(m.data3);
                    strcpy(articleEnCours.image, m.data4);
                    w->setArticle(articleEnCours.intitule, articleEnCours.prix, articleEnCours.stock, articleEnCours.image);

                    break;

        case ACHAT : // TO DO (étape 5)
                    if (strcmp(m.data3,"0") == 0)
                    {
                        w->dialogueMessage("Achat", "Stock insuffisant !");
                    }
                    else
                    {
                      sprintf(affiche,"%s unités de %s achetees",m.data3, m.data2);
                      w->dialogueMessage("Achat", affiche);

                      //envoie de caddie pour recevoir tte les infos du caddie 
                      m.type = 1;
                      m.requete = CADDIE;
                      m.expediteur = getpid();

                      if (msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                      {
                          perror("(Client) Erreur de msgsnd");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                      }
                          // On vide le panir pour qu'il puisse être mis à jour.
                      w->videTablePanier();
                      totalCaddie = 0; //tot a payer 
                    
                    }
                    break;

         case CADDIE : // TO DO (étape 5)
                    fprintf(stderr,"(CLIENT %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      w->ajouteArticleTablePanier(m.data2,m.data5,atoi(m.data3));
                     tmpqte =atof(m.data3);
                      totalCaddie = totalCaddie+  tmpqte*m.data5; 
                      w->setTotal(totalCaddie);
                    break;

         case TIME_OUT : // TO DO (étape 6)
                      fprintf(stderr,"(CLIENT %d) Requete TIME_OUT reçue de %d\n",getpid(),m.expediteur);
                        w->logoutOK();
                        w->dialogueErreur("TIME-OUT", "Déconecte pour cause d'inactivité");                
                    break;

         case BUSY : // TO DO (étape 7)
                     w->dialogueErreur("MAINTENANCE", "SERVEUR en maintenance veuillez réessayer ulterieurement ! "); 
                    break;

         default :
                    break;
      }
    }
}



void handlerSIGUSR2(int sig)
{
  w->setPublicite(pShm);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
