# Simulateur SGF

## Introduction
Le Simulateur SGF est un gestionnaire de fichiers simulant une mémoire secondaire. Il permet de créer, manipuler et organiser des fichiers et des enregistrements à travers différentes opérations, y compris l'insertion, la suppression et la recherche.

## Fonctionnalités
- **Gestion des fichiers** : Créer, renommer, supprimer et afficher des fichiers.
- **Gestion des enregistrements** : Insérer, supprimer et rechercher des enregistrements par ID.
- **Modes de stockage** : Supporte les modes contigu et chaîné pour l'organisation des enregistrements.
- **Défragmentation et compactage** : Optimise l'utilisation de la mémoire.

## Structures de données
Le simulateur utilise plusieurs structures pour gérer les fichiers et les enregistrements :
1. `Adresse` : Pour stocker l'adresse d'un enregistrement dans un bloc.
2. `Index` : Pour indexer les enregistrements par ID.
3. `enregi` : Représente un enregistrement avec un ID et une valeur.
4. `bloc` : Contient un tableau d'enregistrements et des informations sur l'état du bloc.
5. `metadata` : Stocke les métadonnées des fichiers, y compris les informations de stockage et d'enregistrement.
6. `file` : Représente un fichier, incluant les métadonnées et les indices des blocs occupés.
7. `MS` : Structure principale représentant la mémoire secondaire avec des blocs et des fichiers.

## Installation et Exécution
1. **Compilation** : Compilez le code avec un compilateur C++ (comme g++). 
3. **Exécution** : Lancez le simulateur à partir de la ligne de commande.
 3. **Utilisation** :
- Suivez les instructions du menu pour effectuer les opérations désirées en entrant le numéro associé.

## Instructions d'utilisation
1. **Initialiser la mémoire secondaire** : Préparez l’environnement de mémoire.
2. **Créer un fichier** : Entrez un nom de fichier et spécifiez le nombre maximal d’enregistrements.
3. **Charger un fichier** : Chargez un fichier dans la mémoire secondaire pour effectuer d'autres opérations.
4. **Afficher l'état de la mémoire** : Obtenez un aperçu de chaque bloc dans la mémoire secondaire.
5. **Afficher les métadonnées** : Voir les informations détaillées sur les fichiers.
6. **Rechercher un enregistrement** : Trouvez un enregistrement en utilisant son ID.
7. **Insérer un enregistrement** : Ajoutez un nouvel enregistrement dans le fichier spécifié.
8. **Supprimer un enregistrement** : Supprimez un enregistrement par ID. 
- Accessoirement, choisir une suppression logique ou physique.
9. **Défragmenter un fichier** : Réorganisez les blocs pour optimiser l'espace.
10. **Supprimer un fichier** : Effacez un fichier de la mémoire secondaire.
11. **Renommer un fichier** : Changez le nom d'un fichier existant.
12. **Compactage de la mémoire** : Libérez de l'espace en combinant les blocs.
13. **Vider la mémoire** : Réinitialisez toute la mémoire secondaire.
14. **Quitter** : Fermez le simulateur.
## Tests
Les tests doivent être effectués pour assurer que toutes les fonctionnalités fonctionnent correctement et que toutes les cas d'erreur sont gérés. Des captures d’écran peuvent être prises pendant l’utilisation du simulateur pour illustrer son fonctionnement.
## Auteurs
- [messekher younes,Djebbar Seddik Adel,BALAMANE Mohamed Hani,Boubekeur Racim,Hamdaoui Yacine,Sendid Mohamed el hadi,Alwazir mohamed]

