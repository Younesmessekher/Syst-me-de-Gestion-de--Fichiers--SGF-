#include <stdio.h>
//le project final
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h> // Pour la coloration de texte sous Windows
#include <stdbool.h>

#define N 200      // Nombre de blocs
#define BlocSize 128 // Facteur de blocage
#define MAX_FILES 20 // Nombre maximum de fichiers
#define FB 10 // Nombre maximum d'enregistrements par bloc Facteur deBlocage
#define ERGMAX 100//nbr maximal de enregistremend dans un fichier

// Structure pour stocker l'adresse d'un enregistrement
typedef struct {
    int num_bloc;    // Numero du bloc
    int deplacement; // Position dans le bloc
} Adresse;

// Structure pour une entree d'index
typedef struct {
    int id;         // ID de l'enregistrement
    Adresse adr;    // Adresse de l'enregistrement
} Index;

// Structure d'enregistrement
typedef struct {
    int id;
    int value;
} enregi;

// Structure de bloc
typedef struct {
    enregi tab[FB];     // Tableau d'enregistrements (MAX=16)
    int state;          // 1 si occupe, 0 si libre
    char Fname[20];     // Nom du fichier
    int nbr_enregi;     // Nombre d'enregistrements dans ce bloc
} bloc;

// Structure metadata 
typedef struct {
    char filename[20];
    int nbr_bloc;//
    int nbr_enregi_maximal;// nembr de enregistrement maximal qui peut les optenir se file
    int nbr_enregi_actuelle;//nembr de enregistremen inserer dans les bolcs de se fichier
    int indice_du_premier_bloc;//indice de premier blocs du file dans le disque
    int mode_global;    // 1 pour contigu, 0 pour chaene
    int mode_local;     // 1 pour trie, 0 pour non trie
    Index index_tab[ERGMAX];   // ce tablo d'index est util pour le recher telleque il stoc le id de chaque enregistrement apre lincertion pour facilise la recherch

} metadata;

typedef struct {
    char filename[20];
    int file_size;
    int tab_bloc[10];   // Indices des blocs dans le disque qu'ils ont le nom de ce fils
    metadata Fmeta;
} file;

// Structure memoire secondaire
typedef struct {
    bloc disque[N];
    int tab_all[N];     // Table d'allocation
    file tab_file[MAX_FILES];//chaque nouvau fichier sera incerer dans ce tablaeu
    int nb_files;       // Nombre de fichiers actuels
} MS;
void initialiser_MS(MS *memoire) {
    // Initialiser tous les blocs du disque
    for (int i = 0; i < N; i++) {
        memoire->disque[i].state = 0;            // Marquer tous les blocs comme libres
        strcpy(memoire->disque[i].Fname, "");    // Aucun fichier n'est assigne aux blocs
        memoire->disque[i].nbr_enregi = 0;       // Aucun enregistrement dans les blocs
        memoire->tab_all[i] = 0;                 // Table d'allocation : 0 = libre
    }

    // Initialiser la table de fichiers
    memoire->nb_files = 0; // Aucun fichier present au depart
    for (int i = 0; i < MAX_FILES; i++) {
        strcpy(memoire->tab_file[i].filename, "");
        memoire->tab_file[i].file_size = 0;
        memset(memoire->tab_file[i].tab_bloc, -1, sizeof(memoire->tab_file[i].tab_bloc));
    }

    printf("Memoire secondaire initialisee avec succes :\n");
    printf("- %d blocs disponibles\n", N);
    printf("- Aucun fichier n'est present.\n");
}
void vider_MS(MS *memoire) {
    // Reinitialiser tous les blocs du disque
    for (int i = 0; i < N; i++) {
        memoire->disque[i].state = 0;            // Marquer tous les blocs comme libres
        strcpy(memoire->disque[i].Fname, "");    // Aucun fichier assigne
        memoire->disque[i].nbr_enregi = 0;       // Aucun enregistrement
        memoire->tab_all[i] = 0;                 // Table d'allocation : libre
        for (int j = 0; j < FB; j++) {
            memoire->disque[i].tab[j].id = -1;   // Reinitialiser les enregistrements
            memoire->disque[i].tab[j].value = 0;
        }
    }

    // Reinitialiser la table de fichiers
    memoire->nb_files = 0; // Aucun fichier
    for (int i = 0; i < MAX_FILES; i++) {
        strcpy(memoire->tab_file[i].filename, "");
        memoire->tab_file[i].file_size = 0;
        memset(memoire->tab_file[i].tab_bloc, -1, sizeof(memoire->tab_file[i].tab_bloc));

        // Reinitialiser les metadonnees
        strcpy(memoire->tab_file[i].Fmeta.filename, "");
        memoire->tab_file[i].Fmeta.nbr_bloc = 0;
        memoire->tab_file[i].Fmeta.nbr_enregi_maximal = 0;
        memoire->tab_file[i].Fmeta.nbr_enregi_actuelle = 0;
        memoire->tab_file[i].Fmeta.indice_du_premier_bloc = -1;
        memoire->tab_file[i].Fmeta.mode_global = 0;
        memoire->tab_file[i].Fmeta.mode_local = 0;
        memset(memoire->tab_file[i].Fmeta.index_tab, 0, sizeof(memoire->tab_file[i].Fmeta.index_tab));
    }

    printf("Memoire secondaire videe avec succes !\n");
}
void creer_fichier(MS *memoire) {
    // Verifier si on peut encore ajouter un fichier
    if (memoire->nb_files >= MAX_FILES) {
        printf("Erreur : Nombre maximum de fichiers (%d) atteint.\n", MAX_FILES);
        return;
    }

    file nouveau_fichier;
    metadata *meta = &nouveau_fichier.Fmeta;

    // Saisie et verification du nom de fichier
    do {
        printf("Entrez le nom du fichier (max 20 caracteres) : ");
        scanf("%s", nouveau_fichier.filename);

        // Verifier si le fichier existe deje dans tab_file
        bool existe = false;
        for (int i = 0; i < memoire->nb_files; i++) {
            if (strcmp(memoire->tab_file[i].filename, nouveau_fichier.filename) == 0) {
                printf("Erreur : Un fichier avec ce nom existe deje.\n");
                existe = true;
                break;
            }
        }

        if (!existe) break;
    } while (true);

    // Saisie et verification du nombre maximal d'enregistrements
    do {
        printf("Entrez le nombre maximal d'enregistrements pour ce fichier (max %d) : ", ERGMAX);
        scanf("%d", &meta->nbr_enregi_maximal);

        if (meta->nbr_enregi_maximal > 0 && meta->nbr_enregi_maximal <= ERGMAX) break;
        printf("Erreur : Le nombre maximal doit etre compris entre 1 et %d.\n", ERGMAX);
    } while (true);

    // Choix du mode global avec validation
    do {
        printf("Choisissez le mode global (1 - Contigué, 0 - Chaineé) : ");
        if (scanf("%d", &meta->mode_global) == 1 && (meta->mode_global == 0 || meta->mode_global == 1)) {
            break;
        }
        printf("Erreur : Veuillez entrer 1 pour Contigué ou 0 pour Chaineé.\n");
        while (getchar() != '\n'); // Vider le buffer pour eviter les erreurs
    } while (true);

    // Choix du mode local avec validation
    do {
        printf("Choisissez le mode local (1 - Trie, 0 - Non trie) : ");
        if (scanf("%d", &meta->mode_local) == 1 && (meta->mode_local == 0 || meta->mode_local == 1)) {
            break;
        }
        printf("Erreur : Veuillez entrer 1 pour Trie ou 0 pour Non trie.\n");
        while (getchar() != '\n'); // Vider le buffer pour eviter les erreurs
    } while (true);

    // Initialisation des metadonnees et autres champs
    strcpy(meta->filename, nouveau_fichier.filename);
    meta->nbr_bloc = 0;                        // Pas encore de blocs alloues
    meta->nbr_enregi_actuelle = 0;             // Aucun enregistrement au depart
    meta->indice_du_premier_bloc = -1;         // Aucun bloc encore associe
    memset(meta->index_tab, -1, sizeof(meta->index_tab)); // Reinitialisation de l'index

    nouveau_fichier.file_size = 0;             // Taille initiale du fichier
    memset(nouveau_fichier.tab_bloc, -1, sizeof(nouveau_fichier.tab_bloc)); // Aucun bloc attribue

    // Ajouter le fichier dans le tableau de fichiers et incrementer le compteur
    memoire->tab_file[memoire->nb_files] = nouveau_fichier;
    memoire->nb_files++;

    printf("Fichier '%s' cree avec succes !\n", nouveau_fichier.filename);
}

// Fonction pour changer la couleur de texte dans la console
void changer_couleur(int couleur) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, couleur);
}

// Fonction pour afficher l'etat de la memoire
void afficher_etat_MS(MS *memoire) {
    bloc BUFFER; // Tampon pour acceder au contenu des blocs

    printf("etat de la memoire secondaire :\n");
    printf("=====================================\n");

    for (int i = 0; i < N; i++) {
        // Charger le bloc actuel dans le tampon
        BUFFER = memoire->disque[i];

        // Verifier l'etat du bloc et afficher avec la couleur appropriee
        if (BUFFER.state == 0) { // Bloc libre
            changer_couleur(10); // Vert
            printf("Bloc %d : LIBRE\n", i);
        } else { // Bloc occupe
            changer_couleur(12); // Rouge
            printf("Bloc %d : OCCUPe - Fichier : %s | Nombre d'enregistrements : %d\n",
                   i, BUFFER.Fname, BUFFER.nbr_enregi);

            // Afficher le contenu du bloc (les enregistrements)
            printf("Contenu :\n");
            for (int j = 0; j < BUFFER.nbr_enregi; j++) {
                printf("\tEnregistrement %d : ID = %d, Value = %d\n",
                       j, BUFFER.tab[j].id, BUFFER.tab[j].value);
            }
        }
    }

    // Reinitialiser la couleur par defaut
    changer_couleur(7);
    printf("=====================================\n");
}
 void charger_fichier(MS *memoire) {
    char filename[20];
    int index_fichier = -1;

    // Saisie du nom du fichier e charger
    printf("Entrez le nom du fichier e charger : ");
    scanf("%s", filename);

    // Verifier si le fichier existe dans tab_file
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", filename);
        return;
    }

    file *fichier = &memoire->tab_file[index_fichier];
    metadata *meta = &fichier->Fmeta;

    // Calcul du nombre de blocs necessaires
    int nbr_bloc = meta->nbr_enregi_maximal / FB;
    if (meta->nbr_enregi_maximal % FB != 0) {
        nbr_bloc += 1;
    }
    printf("Nombre de blocs necessaires pour '%s' : %d\n", filename, nbr_bloc);

    // Verifier si suffisamment de blocs libres sont disponibles
    int blocs_disponibles = 0;
    for (int i = 0; i < N; i++) {
        if (memoire->tab_all[i] == 0) blocs_disponibles++;
    }

    if (blocs_disponibles < nbr_bloc) {
        printf("Erreur : Pas assez d'espace dans la memoire secondaire pour ce fichier.\n");
        return;
    }

    // Tampon pour modifier les blocs
    bloc BUFFER;

    // Allouer les blocs en fonction du mode global (1 = contigu, 0 = chaene)
    if (meta->mode_global == 1) { // Mode contigu
        int start = -1, count = 0;
        for (int i = 0; i < N; i++) {
            if (memoire->tab_all[i] == 0) {
                if (start == -1) start = i;
                count++;
                if (count == nbr_bloc) break;
            } else {
                start = -1;
                count = 0;
            }
        }

        if (count == nbr_bloc) {
            // Allocation contigue reussie
            for (int i = start; i < start + nbr_bloc; i++) {
                // Utiliser BUFFER pour modifier le bloc
                BUFFER = memoire->disque[i];
                BUFFER.state = 1; // Marquer le bloc comme occupe
                strcpy(BUFFER.Fname, filename); // Associer le nom du fichier au bloc

                // Mettre e jour la memoire secondaire
                memoire->disque[i] = BUFFER;
                memoire->tab_all[i] = 1; // Marquer dans la table d'allocation
                fichier->tab_bloc[meta->nbr_bloc] = i; // Enregistrer l'indice dans tab_bloc
                meta->nbr_bloc++; // Incrementer le compteur de blocs
                if (i==start) {fichier->Fmeta.indice_du_premier_bloc=i;
				}
            }
            printf("Fichier '%s' charge en mode contigu.\n", filename);
        } else {
            printf("Erreur : Pas assez d'espace contigu pour ce fichier.\n");
            return;
        }
    } else { // Mode chaene
        int blocs_alloues = 0;
        for (int i = 0; i < N && blocs_alloues < nbr_bloc; i++) {
            if (memoire->tab_all[i] == 0) {
                // Utiliser BUFFER pour modifier le bloc
                BUFFER = memoire->disque[i];
                BUFFER.state = 1; // Marquer le bloc comme occupe
                strcpy(BUFFER.Fname, filename); // Associer le nom du fichier au bloc

                // Mettre e jour la memoire secondaire
                memoire->disque[i] = BUFFER;
                memoire->tab_all[i] = 1; // Marquer dans la table d'allocation
                fichier->tab_bloc[meta->nbr_bloc] = i; // Enregistrer l'indice dans tab_bloc
                meta->nbr_bloc++; // Incrementer le compteur de blocs
				if (memoire->tab_all[i]==0 && blocs_alloues == 0 ) {fichier->Fmeta.indice_du_premier_bloc=i;
                }
				blocs_alloues++;
                
				
            }
        }
        printf("Fichier '%s' charge en mode chaene.\n", filename);
    }
}
void inserer_enregistrement(MS *memoire) {
    char filename[20];
    int id, value, index_fichier = -1;

    // Saisie du nom du fichier
    printf("Entrez le nom du fichier oe inserer l'enregistrement : ");
    scanf("%s", filename);

    // Verifier si le fichier existe
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", filename);
        return;
    }

    file *fichier = &memoire->tab_file[index_fichier];
    metadata *meta = &fichier->Fmeta;

    // Verifier si le fichier est charge
    if (meta->nbr_bloc == 0) {
        printf("Erreur : Le fichier '%s' n'est pas charge en memoire.\n", filename);
        return;
    }

    // Verifier si le fichier est complet
    if (meta->nbr_enregi_actuelle >= meta->nbr_enregi_maximal) {
        printf("Erreur : Le fichier '%s' est deje complet.\n", filename);
        return;
    }

    // Saisie de l'enregistrement
    printf("Entrez l'ID de l'enregistrement : ");
    scanf("%d", &id);

    // Verifier si l'ID est unique
    for (int i = 0; i < meta->nbr_enregi_actuelle; i++) {
        if (meta->index_tab[i].id == id) {
            printf("Erreur : Un enregistrement avec l'ID '%d' existe deje dans le fichier.\n", id);
            return;
        }
    }

    printf("Entrez la valeur de l'enregistrement : ");
    scanf("%d", &value);

    // Creation de l'enregistrement
    enregi new_enreg;
    new_enreg.id = id;
    new_enreg.value = value;

    if (meta->mode_local == 0) {
        // Cas non trie
        for (int b = 0; b < meta->nbr_bloc; b++) {
            bloc BUFFER = memoire->disque[fichier->tab_bloc[b]];
            if (BUFFER.nbr_enregi < FB) { // Trouver le premier bloc avec de l'espace
                BUFFER.tab[BUFFER.nbr_enregi] = new_enreg; // Ajouter e la fin
                BUFFER.nbr_enregi++; // Incrementer le nombre d'enregistrements
                memoire->disque[fichier->tab_bloc[b]] = BUFFER; // Sauvegarder le bloc modifie
                meta->nbr_enregi_actuelle++; // Incrementer le compteur global

                // Mise e jour du tableau d'index
                meta->index_tab[meta->nbr_enregi_actuelle - 1].id = id;
                meta->index_tab[meta->nbr_enregi_actuelle - 1].adr.num_bloc = fichier->tab_bloc[b];
                meta->index_tab[meta->nbr_enregi_actuelle - 1].adr.deplacement = BUFFER.nbr_enregi - 1;

                printf("Enregistrement insere avec succes dans le fichier '%s' (mode non trie).\n", filename);
                return;
            }
        }

        printf("Erreur : Aucun espace disponible dans les blocs pour inserer l'enregistrement.\n");
        return;
    } else {
        // Cas trie
        int pos_globale = 0;
        for (int i = 0; i < meta->nbr_enregi_actuelle; i++) {
            if (meta->index_tab[i].id > id) {
                break;
            }
            pos_globale++;
        }

        // Cas simple : L'ID est le plus grand, inserer directement e la fin
        if (pos_globale == meta->nbr_enregi_actuelle) {
            for (int b = 0; b< meta->nbr_bloc; b++) {
                bloc BUFFER = memoire->disque[fichier->tab_bloc[b]];
                if (BUFFER.nbr_enregi < FB) {
                    BUFFER.tab[BUFFER.nbr_enregi] = new_enreg; // Ajouter e la fin
                    BUFFER.nbr_enregi++; // Incrementer le nombre d'enregistrements
                    memoire->disque[fichier->tab_bloc[b]] = BUFFER; // Sauvegarder le bloc modifie

                    meta->nbr_enregi_actuelle++; // Incrementer le compteur global

                    // Mise e jour du tableau d'index
                    meta->index_tab[meta->nbr_enregi_actuelle - 1].id = id;
                    meta->index_tab[meta->nbr_enregi_actuelle - 1].adr.num_bloc = fichier->tab_bloc[b];
                    meta->index_tab[meta->nbr_enregi_actuelle - 1].adr.deplacement = BUFFER.nbr_enregi - 1;

                    printf("Enregistrement insere avec succes dans le fichier '%s' (ID le plus grand).\n", filename);
                    return;
                }
            }
        }

        // Cas general : Decalage global
        for (int b = meta->nbr_bloc - 1; b >= 0; b--) {
            bloc BUFFER = memoire->disque[fichier->tab_bloc[b]];

            // Decalage interne du bloc
            if (b == pos_globale / FB) {
                for (int j = BUFFER.nbr_enregi; j > pos_globale % FB; j--) {
                    BUFFER.tab[j] = BUFFER.tab[j - 1];
                }
                BUFFER.tab[pos_globale % FB] = new_enreg; // Inserer e la bonne position
                BUFFER.nbr_enregi++;
                memoire->disque[fichier->tab_bloc[b]] = BUFFER; // Sauvegarder le bloc modifie
                break;
            } else if (BUFFER.nbr_enregi == FB && b + 1 < meta->nbr_bloc) {
                // Si le bloc est plein, deplacer le dernier enregistrement au bloc suivant
                bloc next_BUFFER = memoire->disque[fichier->tab_bloc[b + 1]];
                for (int j = next_BUFFER.nbr_enregi; j > 0; j--) {
                    next_BUFFER.tab[j] = next_BUFFER.tab[j - 1];
                }
                next_BUFFER.tab[0] = BUFFER.tab[FB - 1]; // Dernier enregistrement du bloc actuel
                next_BUFFER.nbr_enregi++;
                BUFFER.nbr_enregi--;

                memoire->disque[fichier->tab_bloc[b + 1]] = next_BUFFER; // Sauvegarder le prochain bloc
                memoire->disque[fichier->tab_bloc[b]] = BUFFER; // Sauvegarder le bloc actuel
            }
        }

        // Mise e jour des metadonnees
        meta->nbr_enregi_actuelle++;

        // Mise e jour du tableau d'index
        for (int i = meta->nbr_enregi_actuelle - 1; i > pos_globale; i--) {
            meta->index_tab[i] = meta->index_tab[i - 1];
        }
        meta->index_tab[pos_globale].id = id;
        meta->index_tab[pos_globale].adr.num_bloc = fichier->tab_bloc[pos_globale / FB];
        meta->index_tab[pos_globale].adr.deplacement = pos_globale % FB;

        printf("Enregistrement insere avec succes dans le fichier '%s' (mode trie).\n", filename);
    }
}
void supprimer_fichier(MS *memoire) {
    char filename[20];
    int index_fichier = -1;

    // Saisie du nom du fichier e supprimer
    printf("Entrez le nom du fichier e supprimer : ");
    scanf("%s", filename);

    // Verifier si le fichier existe
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", filename);
        return;
    }

    file *fichier = &memoire->tab_file[index_fichier];
    metadata *meta = &fichier->Fmeta;

    // Liberer les blocs alloues au fichier
    if (meta->nbr_bloc > 0) {
        for (int i = 0; i < meta->nbr_bloc; i++) {
            int bloc_index = fichier->tab_bloc[i];
            if (bloc_index >= 0) {
                // Reinitialiser le bloc
                bloc BUFFER = memoire->disque[bloc_index];
                BUFFER.state = 0; // Marquer le bloc comme libre
                strcpy(BUFFER.Fname, ""); // Supprimer le nom du fichier associe
                BUFFER.nbr_enregi = 0; // Reinitialiser le compteur d'enregistrements

                // Reinitialiser les enregistrements du bloc
                for (int j = 0; j < FB; j++) {
                    BUFFER.tab[j].id = -1;
                    BUFFER.tab[j].value = 0;
                }

                memoire->disque[bloc_index] = BUFFER; // ecrire les modifications
                memoire->tab_all[bloc_index] = 0; // Marquer le bloc comme libre
            }
        }
    }

    // Supprimer le fichier dans tab_file
    for (int i = index_fichier; i < memoire->nb_files - 1; i++) {
        memoire->tab_file[i] = memoire->tab_file[i + 1];
    }
    memoire->nb_files--; // Decrementer le compteur de fichiers

    printf("Le fichier '%s' a ete supprime avec succes.\n", filename);
}
void renommer_fichier(MS *memoire) {
    char old_filename[20], new_filename[20];
    int index_fichier = -1;

    // Saisie de l'ancien nom du fichier
    printf("Entrez le nom actuel du fichier e renommer : ");
    scanf("%s", old_filename);

    // Verifier si le fichier existe
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, old_filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", old_filename);
        return;
    }

    // Verifier si le fichier est charge
    metadata *meta = &memoire->tab_file[index_fichier].Fmeta;
    if (meta->nbr_bloc == 0) {
        printf("Erreur : Le fichier '%s' n'est pas charge en memoire.\n", old_filename);
        return;
    }

    // Saisie du nouveau nom du fichier
    printf("Entrez le nouveau nom du fichier : ");
    scanf("%s", new_filename);

    // Verifier si le nouveau nom existe deje
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, new_filename) == 0) {
            printf("Erreur : Un fichier avec le nom '%s' existe deje.\n", new_filename);
            return;
        }
    }

    // Renommer le fichier dans tab_file
    strcpy(memoire->tab_file[index_fichier].filename, new_filename);

    // Mettre e jour Fname dans les blocs alloues e ce fichier
    for (int i = 0; i < meta->nbr_bloc; i++) {
        int bloc_index = memoire->tab_file[index_fichier].tab_bloc[i];

        // Charger le bloc dans le BUFFER
        bloc BUFFER = memoire->disque[bloc_index];

        // Modifier le champ Fname dans le BUFFER
        strcpy(BUFFER.Fname, new_filename);

        // ecrire le BUFFER dans le disque
        memoire->disque[bloc_index] = BUFFER;
    }

    printf("Le fichier '%s' a ete renomme en '%s' avec succes.\n", old_filename, new_filename);
}
void supprimer_enregistrement(MS *memoire) {
    char filename[20];
    int id, mode, index_fichier = -1;

    // Saisie du nom du fichier
    printf("Entrez le nom du fichier oe supprimer l'enregistrement : ");
    scanf("%s", filename);

    // Verifier si le fichier existe
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", filename);
        return;
    }

    file *fichier = &memoire->tab_file[index_fichier];
    metadata *meta = &fichier->Fmeta;

    // Verifier si le fichier est charge
    if (meta->nbr_bloc == 0) {
        printf("Erreur : Le fichier '%s' n'est pas charge en memoire.\n", filename);
        return;
    }

    // Saisie de l'ID de l'enregistrement e supprimer
    printf("Entrez l'ID de l'enregistrement e supprimer : ");
    scanf("%d", &id);

    // Verifier si l'ID existe dans le fichier
    int index_enregistrement = -1;
    for (int i = 0; i < meta->nbr_enregi_actuelle; i++) {
        if (meta->index_tab[i].id == id) {
            index_enregistrement = i;
            break;
        }
    }

    if (index_enregistrement == -1) {
        printf("Erreur : L'enregistrement avec l'ID '%d' n'existe pas dans le fichier.\n", id);
        return;
    }

    // Saisie du mode de suppression
    printf("Choisissez le mode de suppression (0 pour logique, 1 pour physique) : ");
    scanf("%d", &mode);

    if (mode == 0) {
        // Mode logique : Marquer l'enregistrement comme supprime
        int bloc_index = meta->index_tab[index_enregistrement].adr.num_bloc;
        int deplacement = meta->index_tab[index_enregistrement].adr.deplacement;

        bloc BUFFER = memoire->disque[bloc_index];
        BUFFER.tab[deplacement].id = -1; // Marquer l'enregistrement comme supprime
        memoire->disque[bloc_index] = BUFFER;

        // Supprimer l'enregistrement du tableau d'index
        for (int i = index_enregistrement; i < meta->nbr_enregi_actuelle - 1; i++) {
            meta->index_tab[i] = meta->index_tab[i + 1];
        }

        meta->nbr_enregi_actuelle--; // Decrementer le compteur d'enregistrements

        printf("Enregistrement avec l'ID '%d' supprime logiquement.\n", id);
    } else if (mode == 1) {
        // Mode physique : Decalage global pour combler l'espace
        for (int b = meta->index_tab[index_enregistrement].adr.num_bloc; b < meta->nbr_bloc; b++) {
            bloc BUFFER = memoire->disque[fichier->tab_bloc[b]];

            for (int i = meta->index_tab[index_enregistrement].adr.deplacement; i < BUFFER.nbr_enregi - 1; i++) {
                // Decaler les enregistrements dans le bloc
                BUFFER.tab[i] = BUFFER.tab[i + 1];

                // Mettre e jour les adresses dans tab_index
                for (int k = 0; k < meta->nbr_enregi_actuelle; k++) {
                    if (meta->index_tab[k].adr.num_bloc == fichier->tab_bloc[b] &&
                        meta->index_tab[k].adr.deplacement == i + 1) {
                        meta->index_tab[k].adr.deplacement = i; // Deplacer l'adresse
                    }
                }
            }

            if (b < meta->nbr_bloc - 1) {
                // Charger le premier enregistrement du bloc suivant dans l'espace libere
                bloc next_BUFFER = memoire->disque[fichier->tab_bloc[b + 1]];
                BUFFER.tab[BUFFER.nbr_enregi - 1] = next_BUFFER.tab[0];

                // Decaler les enregistrements dans le bloc suivant
                for (int j = 0; j < next_BUFFER.nbr_enregi - 1; j++) {
                    next_BUFFER.tab[j] = next_BUFFER.tab[j + 1];

                    // Mettre e jour les adresses dans tab_index
                    for (int k = 0; k < meta->nbr_enregi_actuelle; k++) {
                        if (meta->index_tab[k].adr.num_bloc == fichier->tab_bloc[b + 1] &&
                            meta->index_tab[k].adr.deplacement == j + 1) {
                            meta->index_tab[k].adr.deplacement = j; // Deplacer l'adresse
                        }
                    }
                }

                next_BUFFER.nbr_enregi--; // Reduire le nombre d'enregistrements dans le bloc suivant
                memoire->disque[fichier->tab_bloc[b + 1]] = next_BUFFER;
            }

            BUFFER.nbr_enregi--; // Reduire le nombre d'enregistrements dans le bloc actuel
            memoire->disque[fichier->tab_bloc[b]] = BUFFER;

            if (BUFFER.nbr_enregi == 0) {
                printf("Le bloc %d est desormais vide mais reste alloue au fichier.\n", fichier->tab_bloc[b]);
            }
        }

        // Supprimer l'enregistrement du tableau d'index
        for (int i = index_enregistrement; i < meta->nbr_enregi_actuelle - 1; i++) {
            meta->index_tab[i] = meta->index_tab[i + 1];
        }

        meta->nbr_enregi_actuelle--; // Decrementer le compteur d'enregistrements

        printf("Enregistrement avec l'ID '%d' supprime physiquement.\n", id);
    } else {
        printf("Erreur : Mode de suppression invalide.\n");
    }
}
void rechercher_enregistrement(MS *memoire) {
    char filename[20];
    int id, index_fichier = -1;

    // Saisie du nom du fichier
    printf("Entrez le nom du fichier oe rechercher l'enregistrement : ");
    scanf("%s", filename);

    // Verifier si le fichier existe
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", filename);
        return;
    }

    file *fichier = &memoire->tab_file[index_fichier];
    metadata *meta = &fichier->Fmeta;

    // Verifier si le fichier est charge
    if (meta->nbr_bloc == 0) {
        printf("Erreur : Le fichier '%s' n'est pas charge en memoire.\n", filename);
        return;
    }

    // Saisie de l'ID de l'enregistrement e rechercher
    printf("Entrez l'ID de l'enregistrement e rechercher : ");
    scanf("%d", &id);

    // Rechercher dans le tableau d'index
    for (int i = 0; i < meta->nbr_enregi_actuelle; i++) {
        if (meta->index_tab[i].id == id) {
            // Enregistrement trouve
            int num_bloc = meta->index_tab[i].adr.num_bloc;
            int deplacement = meta->index_tab[i].adr.deplacement;

            // Charger le bloc correspondant dans le BUFFER
            bloc BUFFER = memoire->disque[num_bloc];
            enregi enregistrement = BUFFER.tab[deplacement];

            // Afficher les informations de l'enregistrement
            printf("Enregistrement trouve :\n");
            printf("  - ID : %d\n", enregistrement.id);
            printf("  - Valeur : %d\n", enregistrement.value);
            printf("  - Bloc : %d\n", num_bloc);
            printf("  - Deplacement : %d\n", deplacement);
            return;
        }
    }

    // Si l'enregistrement n'est pas trouve
    printf("Erreur : Aucun enregistrement avec l'ID '%d' n'a ete trouve dans le fichier '%s'.\n", id, filename);
}
void compactage(MS *ms) {
    if (ms == NULL) {
        printf("Erreur: Memoire secondaire invalide\n");
        return;
    }

    // Tableau temporaire pour stocker une copie des blocs
    bloc temp_disque[N];
    memcpy(temp_disque, ms->disque, sizeof(bloc) * N);

    // Reinitialiser la table d'allocation
    for (int i = 0; i < N; i++) {
        ms->tab_all[i] = 0;
        // Reinitialiser les blocs du disque
        memset(&ms->disque[i], 0, sizeof(bloc));
    }

    // Index pour placer les nouveaux blocs
    int new_index = 0;

    // Pour chaque fichier dans le systeme
    for (int f = 0; f < ms->nb_files; f++) {
        file *current_file = &ms->tab_file[f];
        int premier_bloc_original = current_file->Fmeta.indice_du_premier_bloc;
        
        // Si le mode est contigu
        if (current_file->Fmeta.mode_global == 1) {
            // Trouver tous les blocs consecutifs du fichier
            int count_blocs = 0;
            for (int i = 0; i < N; i++) {
                if (strcmp(temp_disque[i].Fname, current_file->filename) == 0) {
                    // Copier le bloc dans sa nouvelle position
                    memcpy(&ms->disque[new_index], &temp_disque[i], sizeof(bloc));
                    ms->tab_all[new_index] = 1; // Marquer comme occupe
                    
                    // Mettre e jour le tableau des blocs du fichier
                    current_file->tab_bloc[count_blocs] = new_index;
                    count_blocs++;
                    new_index++;
                }
            }
            // Mettre e jour l'indice du premier bloc dans les metadonnees
            current_file->Fmeta.indice_du_premier_bloc = new_index - count_blocs;
        }
        // Si le mode est chaene
        else {
            int bloc_actuel = premier_bloc_original;
            while (bloc_actuel != -1 && strcmp(temp_disque[bloc_actuel].Fname, current_file->filename) == 0) {
                // Copier le bloc dans sa nouvelle position
                memcpy(&ms->disque[new_index], &temp_disque[bloc_actuel], sizeof(bloc));
                ms->tab_all[new_index] = 1; // Marquer comme occupe
                
                // Chercher le bloc suivant dans la chaene
                bool found_next = false;
                for (int i = 0; i < N && !found_next; i++) {
                    if (strcmp(temp_disque[i].Fname, current_file->filename) == 0 && i != bloc_actuel) {
                        bloc_actuel = i;
                        found_next = true;
                    }
                } 
                if (!found_next) bloc_actuel = -1;
                
                // Mettre e jour les metadonnees
                if (new_index == current_file->Fmeta.indice_du_premier_bloc) {
                    current_file->Fmeta.indice_du_premier_bloc = new_index;
                }
                new_index++;
            }
        }
    }

    printf("Compactage effectue avec succes!\n");
    printf("Nombre de blocs occupes apres compactage : %d\n", new_index);
}

void defragmentation(MS *memoire) {
    char filename[20];
    int index_fichier = -1;

    // Saisie du nom du fichier
    printf("Entrez le nom du fichier e defragmenter : ");
    scanf("%s", filename);

    // Verifier si le fichier existe
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", filename);
        return;
    }

    file *fichier = &memoire->tab_file[index_fichier];
    metadata *meta = &fichier->Fmeta;

    // Verifier si le fichier est charge
    if (meta->nbr_bloc == 0) {
        printf("Erreur : Le fichier '%s' n'est pas charge en memoire.\n", filename);
        return;
    }

    printf("Defragmentation du fichier '%s' en cours...\n", filename);

    int current_bloc = fichier->tab_bloc[0]; // Bloc oe ecrire les enregistrements
    int position_dans_bloc = 0;              // Position actuelle dans le bloc oe ecrire
    int nouvel_indice = 0;                   // Indice pour reconstruire `tab_index`

    for (int b = 0; b < meta->nbr_bloc; b++) {
        // Acces direct au bloc source
        bloc *bloc_source = &memoire->disque[fichier->tab_bloc[b]];

        for (int i = 0; i < bloc_source->nbr_enregi; i++) {
            if (bloc_source->tab[i].id != -1) { // Enregistrement valide
                // Acces direct au bloc de destination
                bloc *bloc_destination = &memoire->disque[current_bloc];

                // Copier l'enregistrement dans le bloc de destination
                bloc_destination->tab[position_dans_bloc] = bloc_source->tab[i];

                // Mettre e jour le tableau d'index
                meta->index_tab[nouvel_indice].id = bloc_source->tab[i].id;
                meta->index_tab[nouvel_indice].adr.num_bloc = current_bloc;
                meta->index_tab[nouvel_indice].adr.deplacement = position_dans_bloc;

                nouvel_indice++;
                position_dans_bloc++;

                // Si le bloc de destination est plein, passer au bloc suivant
                if (position_dans_bloc == FB) {
                    bloc_destination->nbr_enregi = FB; // Mettre e jour le nombre d'enregistrements
                    position_dans_bloc = 0;
                    current_bloc++;
                }
            }
        }

        // Reorganiser le bloc source (garder uniquement les enregistrements valides)
        int position_source = 0; // Nouvelle position pour les enregistrements valides
        for (int i = 0; i < bloc_source->nbr_enregi; i++) {
            if (bloc_source->tab[i].id != -1) {
                bloc_source->tab[position_source] = bloc_source->tab[i];
                position_source++;
            }
        }

        // Mettre e jour le compteur d'enregistrements du bloc source
        bloc_source->nbr_enregi = position_source;

        // Marquer le reste du bloc comme vide
        for (int i = position_source; i < FB; i++) {
            bloc_source->tab[i].id = -1;
            bloc_source->tab[i].value = 0;
        }
    }

    // Mettre e jour le dernier bloc de destination partiellement rempli
    if (position_dans_bloc > 0) {
        bloc *bloc_destination = &memoire->disque[current_bloc];
        bloc_destination->nbr_enregi = position_dans_bloc; // Mettre e jour le nombre d'enregistrements
    }

    // Mettre e jour le nombre total d'enregistrements
    meta->nbr_enregi_actuelle = nouvel_indice;

    printf("Defragmentation du fichier '%s' terminee.\n", filename);
}
void afficher_menu() {
    printf("======================================\n");
    printf("       Simulateur SGF - Menu\n");
    printf("======================================\n");
    printf("1. Initialiser la memoire secondaire\n");
    printf("2. Creér un fichier\n");
    printf("3. Afficher létat de la memoire secondaire\n");
    printf("4. Afficher les metadonnees des fichiers\n");
    printf("5. Rechercher un enregistrement par ID\n");
    printf("6. Inserer un nouvel enregistrement\n");
    printf("7. Supprimer un enregistrement\n");
    printf("8. Defragmenter un fichier\n");
    printf("9. Supprimer un fichier\n");
    printf("10. Renommer un fichier\n");
    printf("11. Compactage de la memoire secondaire\n");
    printf("12. Vider la memoire secondaire\n");
    printf("13. Quitter\n");
    printf("======================================\n");
    printf("Entrez votre choix : ");
}
void afficher_fichier(MS *memoire) {
    char filename[20];
    int index_fichier = -1;

    // Saisie du nom du fichier
    printf("Entrez le nom du fichier e afficher : ");
    scanf("%s", filename);

    // Verifier si le fichier existe
    for (int i = 0; i < memoire->nb_files; i++) {
        if (strcmp(memoire->tab_file[i].filename, filename) == 0) {
            index_fichier = i;
            break;
        }
    }

    if (index_fichier == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", filename);
        return;
    }

    file *fichier = &memoire->tab_file[index_fichier];
    metadata *meta = &fichier->Fmeta;

    // Verifier si le fichier est charge
    if (meta->nbr_bloc == 0) {
        printf("Erreur : Le fichier '%s' n'est pas charge en memoire.\n", filename);
        return;
    }

    printf("Contenu du fichier '%s' :\n", filename);
    printf("=========================================\n");
    printf("Bloc | Deplacement | ID    | Valeur\n");
    printf("=========================================\n");

    // Parcourir tous les blocs alloues au fichier
    for (int b = 0; b < meta->nbr_bloc; b++) {
        // Charger le bloc dans le tampon
        bloc BUFFER = memoire->disque[fichier->tab_bloc[b]];

        // Parcourir les enregistrements dans le bloc
        for (int i = 0; i < BUFFER.nbr_enregi; i++) {
            if (BUFFER.tab[i].id != -1) { // Afficher uniquement les enregistrements valides
                printf("%4d | %11d | %5d | %6d\n", 
                       fichier->tab_bloc[b], // Numero du bloc
                       i,                   // Deplacement dans le bloc
                       BUFFER.tab[i].id,    // ID de l'enregistrement
                       BUFFER.tab[i].value  // Valeur de l'enregistrement
                );
            }
        }
    }

    printf("=========================================\n");
    printf("Fin du fichier '%s'.\n", filename);
}


int main() {
    MS memoire; // Structure de memoire secondaire
    int choix;
    initialiser_MS(&memoire);

    do {
        // Affichage du menu
        printf("\n======================================\n");
        printf("       Simulateur SGF - Menu\n");
        printf("======================================\n");
        printf("1. Initialiser la memoire secondaire\n");
        printf("2. Creer un fichier\n");
        printf("3. Charger un fichier\n"); // Option ajoutee pour charger un fichier
        printf("4. Afficher leetat de la memoire secondaire\n");
        printf("5. Afficher les metadonnees des fichiers\n");
        printf("6. Rechercher un enregistrement par ID\n");
        printf("7. Inserer un nouvel enregistrement\n");
        printf("8. Supprimer un enregistrement\n");
        printf("9. Defragmenter un fichier\n");
        printf("10. Supprimer un fichier\n");
        printf("11. Renommer un fichier\n");
        printf("12. Compactage de la memoire secondaire\n");
        printf("13. Vider la memoire secondaire\n");
        printf("14. Quitter\n");
        printf("======================================\n");
        printf("Entrez votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                initialiser_MS(&memoire);
                break;
            case 2:
                creer_fichier(&memoire);
                break;
            case 3:
                charger_fichier(&memoire); // Appel e la fonction charger_fichier
                break;
            case 4:
                afficher_etat_MS(&memoire);
                break;
            case 5:
                afficher_fichier(&memoire);
                break;
            case 6:
                rechercher_enregistrement(&memoire);
                break;
            case 7:
                inserer_enregistrement(&memoire);
                break;
            case 8:
                supprimer_enregistrement(&memoire);
                break;
            case 9:
                defragmentation(&memoire);
                break;
            case 10:
                supprimer_fichier(&memoire);
                break;
            case 11:
                renommer_fichier(&memoire);
                break;
            case 12:
                compactage(&memoire);
                break;
            case 13:
                vider_MS(&memoire);
                break;
            case 14:
                printf("Merci d'avoir utilise le simulateur SGF. e bientet !\n");
                break;
            default:
                printf("Erreur : Choix invalide. Veuillez reessayer.\n");
                break;
        }
    } while (choix != 14);

    return 0;
}




