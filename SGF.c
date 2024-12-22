#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define FB 3 // FB = facture de blockage 
#define maxEnregistrements 100 // maxEnregistrements = maxNombre des enregistrements dans le fichier
#define maxBlocs 30 // maxBlocs = maxNombre de bloc dans la MS  

 // déclaration des structures 


 // Enregistrement 
 typedef struct {
    int id;                   // Clée primaire
    char champs[50];          // Contenu des données de l'enregistrement
    bool supprime;
     } Enregistrement; 
 


 // Bloc
 typedef struct 
{
    Enregistrement T[FB];
    int nbrEnregistrements;
    int libre ; 
}bloc;

 //Mémoire Secondaire (MS)
 typedef struct {
    bloc B[maxBlocs];
    int nombreBlocs;
 }MS; 

 
  // métadonnées
  typedef struct { 
     char NomDuFichier[21];
     int tailleBlocs, tailleEnregistrements,addressDePremierBloc;
     char organisationGlobal[13], organisationinterne[13];
  }metadonnees;

   // Fonctions
   // fonction qui verfie si le bloc et plien 
     bool tableAlloation(int n ){ //n les numerode bloc 
      MS x ;
      if (x.B[n].nbrEnregistrements<FB){
          return true;  //le bloc n'est pas plien  
      } 
      else {
          return false; //le bloc est plien  
       }
      }
   
  
  //lire les caracteristique de metadonnées

   int lireCaracteristique(FILE *f, int nc)
{
    rewind(f);
    metadonnees caracteristique;
    fread(&caracteristique, sizeof(metadonnees), 1, f);
    switch(nc)
    {
    case 1:
        return caracteristique.NomDuFichier;
        break;
    case 2:
        return caracteristique.tailleBlocs;
        break;
    case 3:
        return caracteristique.tailleEnregistrements;
        break;
          case 4:
        return caracteristique.addressDePremierBloc;
        break;
          case 5:
        return caracteristique.organisationGlobal;
        break;
          case 6:
        return caracteristique.organisationinterne;
        break;
    default:
        printf("error ");
        break;
    }
}




 // Fonctions pour saisi les enregistrements dans un fichier 
 void saisiEnrg(FILE *F, int n) {
     Enregistrement E;
     int i;
     if (n>maxEnregistrements){ 
          printf("Erreur : stockage insuffisant\n");
          return;
     } else{ 
          while(i<n){ 
     printf("\nSaisie de l'enregistrement %d :\n",n);
     printf("ID : "); scanf("%d",&E.id);
     printf("Champs : "); scanf("%s",E.champs);
     E.supprime = false;
     fwrite(&E, sizeof(E), 1, F);
     i++;
     }
     } 
     
     
 }

 // Fonctions pour lire les enregistrements d'un fichier
 void lireEnrg(FILE *f)
{
    Enregistrement E;
    while(fread(&E, sizeof(E), 1, f))
    {
        printf("ID %d\n les autres champ: %s\n",
        E.id, E.champs);
    }
}







int main() {

     return 0;
}