#include "windowgerant.h"
#include "ui_windowgerant.h"
#include <iostream>
using namespace std;
#include <mysql.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "protocole.h"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} arg;

struct sembuf action[1];
char Pub[51];
int idArticleSelectionne = -1;
MYSQL *connexion;
MYSQL_RES  *resultat;
MYSQL_ROW  Tuple;
char requete[200];
int idSem;
int idQ;

WindowGerant::WindowGerant(QWidget *parent) : QMainWindow(parent),ui(new Ui::WindowGerant)
{
    ui->setupUi(this);

    // Configuration de la table du stock (ne pas modifer)
    ui->tableWidgetStock->setColumnCount(4);
    ui->tableWidgetStock->setRowCount(0);
    QStringList labelsTableStock;
    labelsTableStock << "Id" << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetStock->setHorizontalHeaderLabels(labelsTableStock);
    ui->tableWidgetStock->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetStock->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetStock->horizontalHeader()->setVisible(true);
    ui->tableWidgetStock->horizontalHeader()->setDefaultSectionSize(120);
    ui->tableWidgetStock->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetStock->verticalHeader()->setVisible(false);
    ui->tableWidgetStock->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de la file de message
    // TO DO
    if ((idQ = msgget(CLE,0)) == -1)
    {
        perror("(GERANT) Erreur de msgget");
        exit(1);
    }
    // Récupération du sémaphore
    // TO DO
    if ((idSem = semget(CLE,0,0)) == -1)
    {
        perror("Erreur de semget");
        exit(1);
    }
    // Prise blocante du semaphore
    // TO DO
    action[0].sem_num = 0;
    action[0].sem_op = -1;
    action[0].sem_flg = 0;

    if (semop(idSem, action, 1) == -1)
    {
        printf ("Erreur de prise de semaphore\n");
        exit (1);
    }
    // Connexion à la base de donnée
    connexion = mysql_init(NULL);
    fprintf(stderr,"(GERANT %d) Connexion à la BD\n",getpid());
    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
    {
      fprintf(stderr,"(GERANT %d) Erreur de connexion à la base de données...\n",getpid());
      exit(1);  
    }

    // Recuperation des articles en BD
    // TO DO
     sprintf(requete,"select * from UNIX_FINAL");
    if (mysql_query(connexion, requete) != 0)
    {
        printf ("Erreur de Mysql-query");
    }

    if((resultat = mysql_store_result(connexion)) == NULL)
    {
        printf ("Erreur de mysql store");
    }

    
    while ((Tuple = mysql_fetch_row(resultat)) != NULL)
    {
         char Prix[20];
      strcpy(Prix,Tuple[2]);
      string tmp(Prix);
      size_t x = tmp.find(".");
      if (x != string::npos) tmp.replace(x,1,",");
      strcpy(Prix , tmp.data() );
     ajouteArticleTablePanier(atoi(Tuple[0]),Tuple[1], atof(Prix), atoi(Tuple[3]));
    }
}

WindowGerant::~WindowGerant()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du stock (ne pas modifier) //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::ajouteArticleTablePanier(int id,const char* article,float prix,int quantite)
{
    char Id[20],Prix[20],Quantite[20];

    sprintf(Id,"%d",id);
    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetStock->rowCount();
    nbLignes++;
    ui->tableWidgetStock->setRowCount(nbLignes);
    ui->tableWidgetStock->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Id);
    ui->tableWidgetStock->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetStock->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetStock->setItem(nbLignes-1,2,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetStock->setItem(nbLignes-1,3,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::videTableStock()
{
    ui->tableWidgetStock->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetStock->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_tableWidgetStock_cellClicked(int row, int column)
{
    //cerr << "ligne=" << row << " colonne=" << column << endl;
    ui->lineEditIntitule->setText(ui->tableWidgetStock->item(row,1)->text());
    ui->lineEditPrix->setText(ui->tableWidgetStock->item(row,2)->text());
    ui->lineEditStock->setText(ui->tableWidgetStock->item(row,3)->text());
    idArticleSelectionne = atoi(ui->tableWidgetStock->item(row,0)->text().toStdString().c_str());
    //cerr << "id = " << idArticleSelectionne << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float WindowGerant::getPrix()
{
    return atof(ui->lineEditPrix->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getStock()
{
    return atoi(ui->lineEditStock->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowGerant::getPublicite()
{
  strcpy(publicite,ui->lineEditPublicite->text().toStdString().c_str());
  return publicite;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::closeEvent(QCloseEvent *event)
{
  fprintf(stderr,"(GERANT %d) Clic sur croix de la fenetre\n",getpid());
  // TO DO
  // Deconnexion BD
  mysql_close(connexion);

  // Liberation du semaphore
  // TO DO
  action[0].sem_num = 0;
  action[0].sem_op = +1;

  if (semop(idSem, action, 1) == -1)
  {
    printf ("Erreur au niveau de rendre le semaphore\n");
    exit (1);
  }  

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonPublicite_clicked()
{
  fprintf(stderr,"(GERANT %d) Clic sur bouton Mettre a jour\n",getpid());
  // TO DO (étape 7)
  MESSAGE msg;
  int taille ;
  taille = strlen(getPublicite());
  if (taille < 51 && taille > 0)
  {
   strcpy(Pub, getPublicite());
   for (int i = 1; i < 51; i++)
   {
     if (Pub[i] == '\0')
     {
       for (i=i;i<51;i++)Pub[i] = ' ';
     }
    }
    Pub[51]='\0';
    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = NEW_PUB;
    strcpy (msg.data4, Pub);

    if (msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0) == -1)
    {
        perror("(GERANT) Erreur de msgsnd - 4");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
    }

  }
  else 
  {
    fprintf(stderr,"votre pub contient trop de cracatere !\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonModifier_clicked()
{
  fprintf(stderr,"(GERANT %d) Clic sur bouton Modifier\n",getpid());
  // TO DO
  cerr << "Prix  : --"  << getPrix() << "--" << endl;
  cerr << "Stock : --"  << getStock() << "--" << endl;

  char Prix[20];
  sprintf(Prix,"%f",getPrix());
  string tmp(Prix);
  size_t x = tmp.find(",");
  if (x != string::npos) tmp.replace(x,1,".");
  strcpy(Prix , tmp.data() );

  fprintf(stderr,"(GERANT %d) Modification en base de données pour id=%d\n",getpid(),idArticleSelectionne);

  sprintf(requete,"UPDATE UNIX_FINAL SET prix = %s where id = %d",Prix, idArticleSelectionne);

    if (mysql_query(connexion, requete) != 0)
    {
      printf ("Erreur de Mysql-query prix ");
    }
    sprintf(requete,"UPDATE UNIX_FINAL SET stock = %d where id = %d", getStock(), idArticleSelectionne);
    if (mysql_query(connexion, requete) != 0)
    {
         printf ("Erreur de Mysql-query stock");
    }


    WindowGerant::videTableStock();

  // Mise a jour table BD
  // TO DO
         sprintf(requete,"select * from UNIX_FINAL");
    if (mysql_query(connexion, requete) != 0)
    {
        printf ("Erreur de Mysql-query");
    }

    if((resultat = mysql_store_result(connexion)) == NULL)
    {
        printf ("Erreur de mysql store");
    }

    
    while ((Tuple = mysql_fetch_row(resultat)) != NULL)
    {
      strcpy(Prix,Tuple[2]);
      string tmp(Prix);
      size_t x = tmp.find(".");
      if (x != string::npos) tmp.replace(x,1,",");
      strcpy(Prix , tmp.data() );


     ajouteArticleTablePanier(atoi(Tuple[0]),Tuple[1], atof(Prix), atoi(Tuple[3]));
    }
}
