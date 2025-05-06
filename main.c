#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- CONSTANTES ---------- */
#define MAX_ETUDIANTS     60
#define NB_MATIERES       12
#define TAILLE_NOM        50
#define TAILLE_PRENOM     50
#define TAILLE_MATRICULE  15
#define TAILLE_NOM_NOEUD  50

/* ---------- STRUCTURES ---------- */

typedef struct Noeud {
    char nom[TAILLE_NOM_NOEUD];
    char parent1[TAILLE_NOM_NOEUD];
    char parent2[TAILLE_NOM_NOEUD];
    struct Noeud* gauche, * droit;
} Noeud;

typedef struct {
    char   matricule[TAILLE_MATRICULE];
    char   nom[TAILLE_NOM];
    char   prenom[TAILLE_PRENOM];
    float  notes[NB_MATIERES];
    float  moyenne;
} Etudiant;

/* ---------- DONNEES GLOBALES ---------- */

static const char matieres_def[NB_MATIERES][50] = {
    "Analyse 1", "Algorithme", "Windows", "Electrocinetique",
    "Algebre general", "Anglais", "Optique geometrique",
    "Unix", "Architecture ordi", "Programmation C",
    "Algorithmique", "Statistique"
};

/* ---------- PROTOTYPES ---------- */

// Utilitaires
static void flush_stdin(void);
static void readString(const char* prompt, char* dest, size_t n);

// Arbre généalogique
Noeud* creer_noeud(const char* nom, const char* p1, const char* p2);
void inserer_noeud(Noeud** racine);
void afficher_infixe(Noeud* r);
void afficher_prefixe(Noeud* r);
void afficher_postfixe(Noeud* r);
void parcours_largeur(Noeud* r);
Noeud* rechercher(Noeud* r, const char* nom);
int hauteur(Noeud* r);

// Gestion étudiants
void enregistrerEtudiant(Etudiant tab[], int* count);
void supprimerEtudiant(Etudiant tab[], int* count);
void trierEtudiants(Etudiant tab[], int count);
void modifierEtudiant(Etudiant tab[], int count);
void chargerDepuisFichier(Etudiant tab[], int* count);
void afficherEtudiants(const Etudiant tab[], int count);
/**
 * @param overwrite 1 pour réécrire le fichier ("w"), 0 pour ajouter ("a+")
 */
void sauvegarderDansFichier(const Etudiant tab[], int count, int overwrite);

// Menus
void menuArbre(Noeud** racine);
void menuEtudiants(Etudiant tab[], int* count);

/* ---------- FONCTIONS UTILES ---------- */

static void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void readString(const char* prompt, char* dest, size_t n) {
    printf("%s", prompt);
    if (!fgets(dest, (int)n, stdin)) {
        fprintf(stderr, "Erreur d'entrée\n");
        exit(EXIT_FAILURE);
    }
    dest[strcspn(dest, "\n")] = '\0';
}

/* ---------- MODULE ARBRE GÉNÉALOGIQUE ---------- */

Noeud* creer_noeud(const char* nom, const char* p1, const char* p2) {
    Noeud* n = malloc(sizeof * n);
    if (!n) { perror("malloc"); exit(EXIT_FAILURE); }
    strncpy(n->nom, nom, TAILLE_NOM_NOEUD);     n->nom[TAILLE_NOM_NOEUD - 1] = '\0';
    strncpy(n->parent1, p1, TAILLE_NOM_NOEUD);  n->parent1[TAILLE_NOM_NOEUD - 1] = '\0';
    strncpy(n->parent2, p2, TAILLE_NOM_NOEUD);  n->parent2[TAILLE_NOM_NOEUD - 1] = '\0';
    n->gauche = n->droit = NULL;
    return n;
}

void inserer_noeud(Noeud** racine) {
    char nom[TAILLE_NOM_NOEUD], p1[TAILLE_NOM_NOEUD], p2[TAILLE_NOM_NOEUD];
    readString("Nom du noeud    : ", nom, sizeof nom);
    readString("Parent 1        : ", p1, sizeof p1);
    readString("Parent 2        : ", p2, sizeof p2);
    if (*racine == NULL) {
        *racine = creer_noeud(nom, p1, p2);
    }
    else {
        printf("Impossible d'insérer sous un noeud existant.\n");
    }
}

void afficher_infixe(Noeud* r) {
    if (!r) return;
    afficher_infixe(r->gauche);
    printf("Nom: %s | Parents: %s, %s\n", r->nom, r->parent1, r->parent2);
    afficher_infixe(r->droit);
}

void afficher_prefixe(Noeud* r) {
    if (!r) return;
    printf("Nom: %s | Parents: %s, %s\n", r->nom, r->parent1, r->parent2);
    afficher_prefixe(r->gauche);
    afficher_prefixe(r->droit);
}

void afficher_postfixe(Noeud* r) {
    if (!r) return;
    afficher_postfixe(r->gauche);
    afficher_postfixe(r->droit);
    printf("Nom: %s | Parents: %s, %s\n", r->nom, r->parent1, r->parent2);
}

void parcours_largeur(Noeud* r) {
    if (!r) return;
    Noeud* queue[100]; int head = 0, tail = 0;
    queue[tail++] = r;
    while (head < tail) {
        Noeud* n = queue[head++];
        printf("Nom: %s | Parents: %s, %s\n", n->nom, n->parent1, n->parent2);
        if (n->gauche) queue[tail++] = n->gauche;
        if (n->droit)  queue[tail++] = n->droit;
    }
}

Noeud* rechercher(Noeud* r, const char* nom) {
    if (!r) return NULL;
    if (strcmp(r->nom, nom) == 0) return r;
    Noeud* g = rechercher(r->gauche, nom);
    return g ? g : rechercher(r->droit, nom);
}

int hauteur(Noeud* r) {
    if (!r) return 0;
    int hg = hauteur(r->gauche);
    int hd = hauteur(r->droit);
    return 1 + (hg > hd ? hg : hd);
}

/* ---------- MODULE ETUDIANTS ---------- */

void enregistrerEtudiant(Etudiant tab[], int* count) {
    if (*count >= MAX_ETUDIANTS) {
        printf("Max étudiants atteint !\n");
        return;
    }
    Etudiant* e = &tab[*count];
    readString("Nom      : ", e->nom, sizeof e->nom);
    readString("Prenom   : ", e->prenom, sizeof e->prenom);
    readString("Matricule: ", e->matricule, sizeof e->matricule);
    flush_stdin();

    float somme = 0.0f;
    for (int i = 0; i < NB_MATIERES; i++) {
        float note;
        while (1) {
            printf("%s : ", matieres_def[i]);
            if (scanf("%f", &note) == 1 && note >= 0.0f && note <= 20.0f) {
                e->notes[i] = note;
                somme += note;
                break;
            }
            fprintf(stderr, "Note invalide, entrez un nombre entre 0 et 20.\n");
            flush_stdin();
        }
    }
    flush_stdin();
    e->moyenne = somme / NB_MATIERES;
    (*count)++;
    printf("Etudiant enregistre avec ses notes !\n");
}

void supprimerEtudiant(Etudiant tab[], int* count) {
    char mat[TAILLE_MATRICULE];
    readString("Matricule a supprimer: ", mat, sizeof mat);
    for (int i = 0; i < *count; i++) {
        if (strcmp(tab[i].matricule, mat) == 0) {
            for (int j = i; j < *count - 1; j++)
                tab[j] = tab[j + 1];
            (*count)--;
            printf("Supprime !\n");
            return;
        }
    }
    printf("Non trouve !\n");
}

void trierEtudiants(Etudiant tab[], int count) {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(tab[j].nom, tab[j + 1].nom) > 0) {
                Etudiant tmp = tab[j];
                tab[j] = tab[j + 1];
                tab[j + 1] = tmp;
            }
    printf("Trie complet !\n");
}

void modifierEtudiant(Etudiant tab[], int count) {
    char mat[TAILLE_MATRICULE];
    readString("Matricule a modifier: ", mat, sizeof mat);

    int idx = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(tab[i].matricule, mat) == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        printf("Etudiant non trouve !\n");
        return;
    }

    Etudiant* e = &tab[idx];
    int choix;
    do {
        printf("\n--- Modifier Etudiant %s %s ---\n", e->nom, e->prenom);
        printf("1) Nom\n2) Prenom\n3) Notes\n4) Tout afficher\n5) Retour\nVotre choix: ");
        if (scanf("%d", &choix) != 1) { flush_stdin(); choix = 5; }
        flush_stdin();

        switch (choix) {
        case 1:
            readString("Nouveau nom    : ", e->nom, sizeof e->nom);
            printf("Nom mis a jour.\n");
            break;
        case 2:
            readString("Nouveau prenom : ", e->prenom, sizeof e->prenom);
            printf("Prenom mis a jour.\n");
            break;
        case 3: {
            int sub;
            do {
                printf("\n--- Modifier Notes ---\n0) Retour\n");
                for (int j = 0; j < NB_MATIERES; j++)
                    printf("%2d) %-25s : %.2f\n", j + 1, matieres_def[j], e->notes[j]);
                printf("Votre choix: ");
                if (scanf("%d", &sub) != 1) { flush_stdin(); sub = 0; }
                flush_stdin();
                if (sub >= 1 && sub <= NB_MATIERES) {
                    float note;
                    do {
                        printf("Nouvelle note pour %s : ", matieres_def[sub - 1]);
                        if (scanf("%f", &note) != 1) { flush_stdin(); continue; }
                        flush_stdin();
                    } while (note < 0.0f || note > 20.0f);
                    e->notes[sub - 1] = note;
                    printf("Note mise a jour.\n");
                }
            } while (sub != 0);
            {
                float somme = 0;
                for (int j = 0; j < NB_MATIERES; j++) somme += e->notes[j];
                e->moyenne = somme / NB_MATIERES;
            }
            break;
        }
        case 4:
            printf("\nEtudiant : %s %s (%s)\n", e->nom, e->prenom, e->matricule);
            for (int j = 0; j < NB_MATIERES; j++)
                printf("  %s: %.2f\n", matieres_def[j], e->notes[j]);
            printf("Moyenne: %.2f\n\n", e->moyenne);
            break;
        case 5:
            break;
        default:
            printf("Choix invalide.\n");
        }
    } while (choix != 5);
}

void chargerDepuisFichier(Etudiant tab[], int* count) {
    FILE* f = fopen("etudiants.txt", "r");
    if (!f) { *count = 0; return; }
    *count = 0;
    while (*count < MAX_ETUDIANTS) {
        Etudiant e;
        if (fscanf(f, "%s %s %s", e.matricule, e.nom, e.prenom) != 3) break;
        for (int j = 0; j < NB_MATIERES; j++)
            if (fscanf(f, "%f", &e.notes[j]) != 1) e.notes[j] = 0.0f;
        if (fscanf(f, "%f", &e.moyenne) != 1) e.moyenne = 0.0f;
        tab[(*count)++] = e;
    }
    fclose(f);
}

void afficherEtudiants(const Etudiant tab[], int count) {
    if (count == 0) {
        printf("Aucun etudiant.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("\n%d) %s %s | Matricule: %s\n",
            i + 1, tab[i].nom, tab[i].prenom, tab[i].matricule);
        for (int j = 0; j < NB_MATIERES; j++)
            printf("  %s: %.2f\n", matieres_def[j], tab[i].notes[j]);
        printf("Moyenne: %.2f\n", tab[i].moyenne);
    }
}

void sauvegarderDansFichier(const Etudiant tab[], int count, int overwrite) {
    FILE* f = fopen("etudiants.txt", overwrite ? "w" : "a+");
    if (!f) { perror("fopen"); return; }
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s %s %s ", tab[i].matricule, tab[i].nom, tab[i].prenom);
        for (int j = 0; j < NB_MATIERES; j++)
            fprintf(f, "%.2f ", tab[i].notes[j]);
        fprintf(f, "%.2f\n", tab[i].moyenne);
    }
    fclose(f);
}

void menuArbre(Noeud** racine) {
    int choix;
    do {
        printf("\n--- Arbre Genealogique ---\n"
            "1) Inserer\n2) Infixe\n3) Prefixe\n"
            "4) Postfixe\n5) Largeur\n6) Rechercher\n"
            "7) Hauteur\n8) Retour\nVotre choix: ");
        if (scanf("%d", &choix) != 1) break;
        flush_stdin();
        switch (choix) {
        case 1: inserer_noeud(racine); break;
        case 2: afficher_infixe(*racine); break;
        case 3: afficher_prefixe(*racine); break;
        case 4: afficher_postfixe(*racine); break;
        case 5: parcours_largeur(*racine); break;
        case 6: {
            char nom[TAILLE_NOM_NOEUD];
            readString("Nom a chercher: ", nom, sizeof nom);
            Noeud* r = rechercher(*racine, nom);
            printf(r ? "Trouve : %s\n" : "Non trouve\n", nom);
            break;
        }
        case 7: printf("Hauteur: %d\n", hauteur(*racine)); break;
        }
    } while (choix != 8);
}

void menuEtudiants(Etudiant tab[], int* count) {
    chargerDepuisFichier(tab, count);
    int choix;
    do {
        printf("\n--- Gestion Etudiants ---\n"
            "1) Enregistrer\n2) Supprimer\n3) Trier\n"
            "4) Modifier\n5) Sauvegarder\n6) Afficher\n7) Retour\n"
            "Votre choix: ");
        if (scanf("%d", &choix) != 1) break;
        flush_stdin();
        switch (choix) {
        case 1:
            enregistrerEtudiant(tab, count);
            sauvegarderDansFichier(tab, *count, 1);
            break;
        case 2:
            supprimerEtudiant(tab, count);
            sauvegarderDansFichier(tab, *count, 1);
            break;
        case 3:
            trierEtudiants(tab, *count);
            sauvegarderDansFichier(tab, *count, 1);
            break;
        case 4:
            modifierEtudiant(tab, *count);
            sauvegarderDansFichier(tab, *count, 1);
            break;
        case 5:
            sauvegarderDansFichier(tab, *count, 1);
            break;
        case 6:
            afficherEtudiants(tab, *count);
            break;
        }
    } while (choix != 7);
}

int main(void) {
    Noeud* racine = NULL;
    Etudiant tabEtu[MAX_ETUDIANTS];
    int      nbEtu = 0;
    int      choix;
    do {
        printf("\n=== Menu Principal ===\n"
            "1) Arbre genealogique\n2) Gestion etudiants\n3) Quitter\n"
            "Votre choix: ");
        if (scanf("%d", &choix) != 1) break;
        flush_stdin();
        switch (choix) {
        case 1: menuArbre(&racine);       break;
        case 2: menuEtudiants(tabEtu, &nbEtu); break;
        }
    } while (choix != 3);
    printf("Au revoir !\n");
    return 0;
}
