/*
 *                  Politecnico di Milano
 *
 *        Studente: Caravano Andrea
 *            A.A.: 2022/2023
 *
 * Ultima modifica: 18/07/2023
 *
 *     Descrizione: Algoritmi e Principi dell'Informatica: Prova Finale (2023)
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

/**
 * COSTANTI E VARIABILI DI UTILITÀ
 */
#define NIL_RB_KEY UINT_MAX

// costanti globali per tabella di Hash
#define HASH_DIM 2048
#define ALPHA 0.6180340 // (sqrt(5) - 1) / 2, costante aurea

u_int32_t *UINT32_INVALID;
u_int16_t UINT16_INVALID = USHRT_MAX;

/**
 * PARSER ALTERNATIVI A SCANF
 */

u_int16_t leggiU16() {
    u_int16_t carLetto;
    u_int16_t letto = getchar_unlocked() - '0';
    while (isdigit((carLetto = getchar_unlocked())))
        letto = 10 * letto + carLetto - '0'; // conversione basata su tabella ASCII
    return letto;
}

u_int32_t leggiU32() {
    u_int32_t carLetto;
    u_int32_t letto = getchar_unlocked() - '0';
    while (isdigit((carLetto = getchar_unlocked())))
        letto = 10 * letto + carLetto - '0'; // conversione basata su tabella ASCII
    return letto;
}

/**
 * ALBERI ROSSO-NERI/ROUTINE PER ALBERI e prototipi di supporto
 * Convenzioni secondo lezioni teoriche del corso
 */

enum RB_NODE_COLOR {
    red, black
};

// prototipi per uso Max-Heap in Alberi Rosso-Neri
typedef struct MAX_HEAP MAX_HEAP;

MAX_HEAP *CREA_MAX_HEAP();

void HEAP_CANCELLA_HEAP(MAX_HEAP *A);

struct RB_NODE {
    u_int32_t key;
    MAX_HEAP *autonomie;
    enum RB_NODE_COLOR color;
    struct RB_NODE *left;
    struct RB_NODE *right;
    struct RB_NODE *p;
};
typedef struct RB_NODE RB_NODE;

typedef struct HASH_RB_NODE HASH_RB_NODE;
typedef struct HASH_RB_TREE HASH_RB_TREE;

typedef struct LIST_RB_NODES_HEAD LIST_RB_NODES_HEAD;

struct TREE {
    RB_NODE *root;
    RB_NODE *NIL;
    HASH_RB_TREE *hash_table[HASH_DIM];
};
typedef struct TREE TREE;

// prototipi per uso Tabelle Hash di supporto agli Alberi Rosso-Neri
u_int32_t HASH_CALCOLA(u_int32_t k);

_Bool HASH_CONTIENE(TREE *T, u_int32_t k);

RB_NODE *HASH_CERCA(TREE *T, u_int32_t k);

void HASH_INSERISCI(TREE *T, RB_NODE *n);

void HASH_CANCELLA(TREE *T, u_int32_t k);

void HASH_SVUOTA(TREE *T);

RB_NODE *CREA_NODO_RB(u_int32_t key) {
    RB_NODE *n = (RB_NODE *) malloc(sizeof(RB_NODE));
    n->key = key;
    n->color = red;
    n->left = NULL;
    n->right = NULL;
    n->p = NULL;
    n->autonomie = CREA_MAX_HEAP();
    return n;
}

TREE *CREA_ALBERO() {
    TREE *T = malloc(sizeof(TREE));
    T->NIL = CREA_NODO_RB(NIL_RB_KEY);
    T->NIL->color = black;
    HEAP_CANCELLA_HEAP(T->NIL->autonomie);
    T->root = T->NIL;

    // creazione hash table
    for (u_int16_t i = 0; i < HASH_DIM; i++)
        T->hash_table[i] = NULL;
    return T;
}

RB_NODE *FAILOVER_RICERCA_BST(TREE *T, RB_NODE *n, u_int32_t key) {
    RB_NODE *cur = n;
    if (cur == T->NIL)
        return T->NIL;
    else if (cur->key == key) {
        return cur;
    } else {
        RB_NODE *ret = FAILOVER_RICERCA_BST(T, n->left, key);
        if (ret != T->NIL)
            return ret;
        ret = FAILOVER_RICERCA_BST(T, n->right, key);
        if (ret != T->NIL)
            return ret;
    }
    return T->NIL;
}

RB_NODE *MASCHERA_RICERCA_HASH_BST(TREE *T, u_int32_t key) {
    RB_NODE *ret = HASH_CERCA(T, key);
    if (ret != NULL)
        return ret;
    else
        return T->NIL;
}

void LEFT_ROTATE(TREE *T, RB_NODE *x) {
    RB_NODE *y = x->right;
    x->right = y->left;
    if (y->left != T->NIL)
        y->left->p = x;
    y->p = x->p;
    if (x->p == T->NIL)
        T->root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else x->p->right = y;
    y->left = x;
    x->p = y;
}

void RIGHT_ROTATE(TREE *T, RB_NODE *y) {
    RB_NODE *x = y->left;
    y->left = x->right;
    if (x->right != T->NIL)
        x->right->p = y;
    x->p = y->p;
    if (y->p == T->NIL)
        T->root = x;
    else if (y == y->p->left)
        y->p->left = x;
    else
        y->p->right = x;
    x->right = y;
    y->p = x;
}

void RB_RIPARA_INSERISCI(TREE *T, RB_NODE *z) {
    while (z->p->color == red) {
        if (z->p == z->p->p->left) {
            RB_NODE *y = z->p->p->right;

            if (y->color == red) {
                z->p->color = black;
                y->color = black;
                z->p->p->color = red;
                z = z->p->p;
            } else {
                if (z == z->p->right) {
                    z = z->p;
                    LEFT_ROTATE(T, z);
                }

                z->p->color = black;
                z->p->p->color = red;
                RIGHT_ROTATE(T, z->p->p);
            }
        } else {
            RB_NODE *y = z->p->p->left;

            if (y->color == red) {
                z->p->color = black;
                y->color = black;
                z->p->p->color = red;
                z = z->p->p;
            } else {
                if (z == z->p->left) {
                    z = z->p;
                    RIGHT_ROTATE(T, z);
                }

                z->p->color = black;
                z->p->p->color = red;
                LEFT_ROTATE(T, z->p->p);
            }
        }
    }

    T->root->color = black;
}

_Bool RB_INSERISCI(TREE *T, RB_NODE *z) {
    RB_NODE *y = T->NIL;
    RB_NODE *x = T->root;
    while (x != T->NIL) {
        y = x;
        if (z->key == x->key) {
            // non ammette inserimenti duplicati (cancella nodo duplicato)
            HEAP_CANCELLA_HEAP(z->autonomie);
            free(z);
            return 0;
        } else if (z->key < x->key)
            x = x->left;
        else x = x->right;
    }
    z->p = y;
    if (y == T->NIL)
        T->root = z;
    else if (z->key < y->key)
        y->left = z;
    else y->right = z;
    z->left = T->NIL;
    z->right = T->NIL;
    z->color = red;
    RB_RIPARA_INSERISCI(T, z);

    HASH_INSERISCI(T, z);
    return 1;
}

RB_NODE *BST_MIN(RB_NODE *n, RB_NODE *NIL) {
    RB_NODE *cur = n;
    RB_NODE *prev = n;
    while (cur != NIL) {
        prev = cur;
        cur = cur->left;
    }
    return prev;
}

RB_NODE *BST_SUCCESSORE(TREE *T, RB_NODE *x) {
    RB_NODE *y;
    if (x->right != T->NIL)
        return BST_MIN(x->right, T->NIL);
    y = x->p;
    while (y != T->NIL && y->right == x) {
        x = y;
        y = y->p;
    }
    return y;
}

void RB_RIPARA_CANCELLA(TREE *T, RB_NODE *x) {
    if (x == NULL)
        return;
    while (x != T->root && x->color == black) {
        if (x == x->p->left) {
            RB_NODE *w = x->p->right;
            if (w->color == red) {
                w->color = black;
                x->p->color = red;
                LEFT_ROTATE(T, x->p);
                w = x->p->right;
            }
            if (w->left->color == black && w->right->color == black) {
                w->color = red;
                x = x->p;
            } else {
                if (w->right->color == black) {
                    w->left->color = black;
                    w->color = red;
                    RIGHT_ROTATE(T, w);
                    w = x->p->right;
                }
                w->color = x->p->color;
                x->p->color = black;
                w->right->color = black;
                LEFT_ROTATE(T, x->p);
                x = T->root;
            }
        } else {
            RB_NODE *w = x->p->left;
            if (w->color == red) {
                w->color = black;
                x->p->color = red;
                RIGHT_ROTATE(T, x->p);
                w = x->p->left;
            }
            if (w->right->color == black && w->left->color == black) {
                w->color = red;
                x = x->p;
            } else {
                if (w->left->color == black) {
                    w->right->color = black;
                    w->color = red;
                    LEFT_ROTATE(T, w);
                    w = x->p->left;
                }
                w->color = x->p->color;
                x->p->color = black;
                w->left->color = black;
                RIGHT_ROTATE(T, x->p);
                x = T->root;
            }
        }
    }
    x->color = black;
}

void RB_TRANSPLANT(TREE *T, RB_NODE *u, RB_NODE *v) {
    if (u == T->NIL)
        return;
    else if (u->p == T->NIL) {
        T->root = v;
    } else if (u == u->p->left)
        u->p->left = v;
    else u->p->right = v;
    v->p = u->p;
}

_Bool RB_CANCELLA(TREE *T, u_int32_t key) {
    RB_NODE *z = MASCHERA_RICERCA_HASH_BST(T, key);

    if (z == T->NIL)
        return 0;

    RB_NODE *y = z;
    RB_NODE *x;
    enum RB_NODE_COLOR originale_y = y->color;
    if (z->left == T->NIL) {
        x = z->right;
        RB_TRANSPLANT(T, z, z->right);
    } else if (z->right == T->NIL) {
        x = z->left;
        RB_TRANSPLANT(T, z, z->left);
    } else {
        y = BST_MIN(z->right, T->NIL);
        originale_y = y->color;
        x = y->right;
        if (y->p == z)
            x->p = y;
        else {
            RB_TRANSPLANT(T, y, y->right);
            y->right = z->right;
            y->right->p = y;
        }
        RB_TRANSPLANT(T, z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }

    if (originale_y == black)
        RB_RIPARA_CANCELLA(T, x);

    HASH_CANCELLA(T, key);

    HEAP_CANCELLA_HEAP(z->autonomie);
    free(z);
    return 1;
}

void RB_VISITA_CANCELLA(TREE *T, RB_NODE *n) {
    if (n == T->NIL)
        return;
    else {
        RB_VISITA_CANCELLA(T, n->left);
        RB_VISITA_CANCELLA(T, n->right);
        HEAP_CANCELLA_HEAP(n->autonomie);
        free(n);
    }
}

void RB_CANCELLA_ALBERO(TREE *T) {
    RB_VISITA_CANCELLA(T, T->root);
    HASH_SVUOTA(T);
    free(T->NIL);
    free(T);
}

/**
 * MAX-HEAP (MUCCHI)
 */

#define HEAP_PARENT(i) ((i - 1) / 2)
#define HEAP_LEFT(i) (2 * i + 1)
#define HEAP_RIGHT(i) (2 * i + 2)
#define HEAP_DIM 512

struct MAX_HEAP {
    u_int16_t heapsize;
    u_int16_t capacity;
    u_int32_t heap[HEAP_DIM];
    u_int32_t NIL;
};

MAX_HEAP *CREA_MAX_HEAP() {
    MAX_HEAP *A = (MAX_HEAP *) malloc(sizeof(MAX_HEAP));
    A->heapsize = 0;
    A->capacity = HEAP_DIM;
    A->NIL = *UINT32_INVALID;
    return A;
}

void SCAMBIA_UINT32(u_int32_t *el1, u_int32_t *el2) {
    u_int32_t temp = *el1;
    *el1 = *el2;
    *el2 = temp;
}

void HEAP_MAX_HEAPIFY(MAX_HEAP *A, u_int16_t n) {
    u_int16_t l = HEAP_LEFT(n);
    u_int16_t r = HEAP_RIGHT(n);
    u_int16_t posmax;
    if (l < A->heapsize && A->heap[l] > A->heap[n])
        posmax = l;
    else posmax = n;
    if (r < A->heapsize && A->heap[r] > A->heap[posmax])
        posmax = r;
    if (posmax != n) {
        SCAMBIA_UINT32(&A->heap[n], &A->heap[posmax]);
        HEAP_MAX_HEAPIFY(A, posmax);
    }
}

u_int32_t HEAP_MAX(MAX_HEAP *A) {
    return A->heap[0];
}

u_int32_t *HEAP_CANCELLA_MAX(MAX_HEAP *A) {
    if (A->heapsize < 1)
        return UINT32_INVALID;
    u_int32_t *max = &A->heap[0];
    A->heap[0] = A->heap[A->heapsize - 1];
    A->heapsize--;
    HEAP_MAX_HEAPIFY(A, 1);
    return max;
}

u_int16_t HEAP_FAILOVER_RICERCA(MAX_HEAP *A, u_int16_t start, u_int32_t v) {
    for (u_int16_t i = start; i < A->heapsize; i++) {
        if (A->heap[i] == v)
            return i;
    }
    return UINT16_INVALID;
}

u_int16_t HEAP_RICERCA(MAX_HEAP *A, u_int16_t ind, u_int32_t v) {
    if (ind < A->heapsize) {
        if (A->heap[ind] == v)
            return ind;
        else if (A->heap[ind] > v) {
            if (HEAP_LEFT(ind) > A->capacity || HEAP_RIGHT(ind) > A->capacity) {
                // se anche solo uno dei due figli sfora la capacità del mucchio, procedo con ricerca lineare classica
                // entro un certo limite migliora la complessità alle costanti della ricerca
                return HEAP_FAILOVER_RICERCA(A, ind, v);
            } else {
                u_int16_t ret = HEAP_RICERCA(A, HEAP_LEFT(ind), v);
                if (ret != UINT16_INVALID)
                    return ret;
                ret = HEAP_RICERCA(A, HEAP_RIGHT(ind), v);
                if (ret != UINT16_INVALID)
                    return ret;
            }
        }
    }
    return UINT16_INVALID;
}

_Bool HEAP_CANCELLA_NODO(MAX_HEAP *A, u_int32_t key) {
    u_int16_t ind = HEAP_RICERCA(A, 0, key);
    if (ind == UINT16_INVALID)
        return 0;
    else {
        A->heap[ind] = A->heap[A->heapsize - 1];
        A->heapsize--;

        while (ind > 0 && A->heap[ind] > A->heap[(ind - 1) / 2]) {
            SCAMBIA_UINT32(&A->heap[ind], &A->heap[(ind - 1) / 2]);
            ind = (ind - 1) / 2;
        }

        HEAP_MAX_HEAPIFY(A, ind);

        return 1;
    }
}

// Per fini di visualizzazione esemplificativa
void STAMPA_MAX_HEAP(MAX_HEAP *A) {
    printf("MAX HEAP:\n");
    for (u_int16_t i = 0; i < A->heapsize; i++) {
        printf("E: %d, P: %d, L: %d, R: %d\n",
               A->heap[i], A->heap[HEAP_PARENT(i)], A->heap[HEAP_LEFT(i)], A->heap[HEAP_RIGHT(i)]);
    }
}

void HEAP_INSERISCI(MAX_HEAP *A, u_int32_t key) {
    A->heapsize++;
    A->heap[A->heapsize - 1] = key;
    u_int16_t i = A->heapsize - 1;

    // variazione personale: "figli" dei nodi inseriti sempre NIL
    if (HEAP_LEFT(i) < A->capacity)
        A->heap[HEAP_LEFT(i)] = A->NIL;
    if (HEAP_RIGHT(i) < A->capacity)
        A->heap[HEAP_RIGHT(i)] = A->NIL;

    while (i > 0 && A->heap[HEAP_PARENT(i)] < A->heap[i]) {
        SCAMBIA_UINT32(&A->heap[HEAP_PARENT(i)], &A->heap[i]);
        i = HEAP_PARENT(i);
    }
}

void HEAP_CANCELLA_HEAP(MAX_HEAP *A) {
    free(A);
}

/**
 * HASH TABLES (tabelle di hash)
 * N.B: la tabella hash "attutisce" la complessità della ricerca nelle collisioni attraverso i medesimi alberi rosso-neri utilizzati per le stazioni autostradali
 *      lo speed-up medio di questa soluzione è notevole
 */

struct HASH_RB_NODE {
    u_int32_t key;
    enum RB_NODE_COLOR color;
    struct HASH_RB_NODE *left;
    struct HASH_RB_NODE *right;
    struct HASH_RB_NODE *p;
    struct RB_NODE *real;
};

struct HASH_RB_TREE {
    HASH_RB_NODE *root;
    HASH_RB_NODE *NIL;
};

HASH_RB_NODE *HASH_CREA_NODO_RB(u_int32_t key) {
    HASH_RB_NODE *n = (HASH_RB_NODE *) malloc(sizeof(HASH_RB_NODE));
    n->key = key;
    n->color = red;
    n->left = NULL;
    n->right = NULL;
    n->p = NULL;
    n->real = NULL;
    return n;
}

HASH_RB_TREE *HASH_CREA_ALBERO() {
    HASH_RB_TREE *T = malloc(sizeof(HASH_RB_TREE));
    T->NIL = HASH_CREA_NODO_RB(NIL_RB_KEY);
    T->NIL->color = black;
    T->root = T->NIL;
    return T;
}

void HASH_RB_LEFT_ROTATE(HASH_RB_TREE *T, HASH_RB_NODE *x) {
    HASH_RB_NODE *y = x->right;
    x->right = y->left;
    if (y->left != T->NIL)
        y->left->p = x;
    y->p = x->p;
    if (x->p == T->NIL)
        T->root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else x->p->right = y;
    y->left = x;
    x->p = y;
}

void HASH_RB_RIGHT_ROTATE(HASH_RB_TREE *T, HASH_RB_NODE *y) {
    HASH_RB_NODE *x = y->left;
    y->left = x->right;
    if (x->right != T->NIL)
        x->right->p = y;
    x->p = y->p;
    if (y->p == T->NIL)
        T->root = x;
    else if (y == y->p->left)
        y->p->left = x;
    else
        y->p->right = x;
    x->right = y;
    y->p = x;
}

void HASH_RB_RIPARA_INSERISCI(HASH_RB_TREE *T, HASH_RB_NODE *z) {
    while (z->p->color == red) {
        if (z->p == z->p->p->left) {
            HASH_RB_NODE *y = z->p->p->right;

            if (y->color == red) {
                z->p->color = black;
                y->color = black;
                z->p->p->color = red;
                z = z->p->p;
            } else {
                if (z == z->p->right) {
                    z = z->p;
                    HASH_RB_LEFT_ROTATE(T, z);
                }

                z->p->color = black;
                z->p->p->color = red;
                HASH_RB_RIGHT_ROTATE(T, z->p->p);
            }
        } else {
            HASH_RB_NODE *y = z->p->p->left;

            if (y->color == red) {
                z->p->color = black;
                y->color = black;
                z->p->p->color = red;
                z = z->p->p;
            } else {
                if (z == z->p->left) {
                    z = z->p;
                    HASH_RB_RIGHT_ROTATE(T, z);
                }

                z->p->color = black;
                z->p->p->color = red;
                HASH_RB_LEFT_ROTATE(T, z->p->p);
            }
        }
    }

    T->root->color = black;
}

HASH_RB_NODE *HASH_RB_BST_RICERCA(HASH_RB_TREE *T, HASH_RB_NODE *n, u_int32_t key) {
    HASH_RB_NODE *cur = n;
    if (cur == T->NIL)
        return T->NIL;
    else if (cur->key == key) {
        return cur;
    } else {
        HASH_RB_NODE *ret = HASH_RB_BST_RICERCA(T, n->left, key);
        if (ret != T->NIL)
            return ret;
        ret = HASH_RB_BST_RICERCA(T, n->right, key);
        if (ret != T->NIL)
            return ret;
    }
    return T->NIL;
}

_Bool HASH_RB_INSERISCI(HASH_RB_TREE *T, HASH_RB_NODE *z) {
    HASH_RB_NODE *y = T->NIL;
    HASH_RB_NODE *x = T->root;
    while (x != T->NIL) {
        y = x;
        if (z->key == x->key) {
            // come stazioni autostradali, non ammette inserimenti duplicati (cancella nodo duplicato)
            free(z);
            return 0;
        } else if (z->key < x->key)
            x = x->left;
        else x = x->right;
    }
    z->p = y;
    if (y == T->NIL)
        T->root = z;
    else if (z->key < y->key)
        y->left = z;
    else y->right = z;
    z->left = T->NIL;
    z->right = T->NIL;
    z->color = red;
    HASH_RB_RIPARA_INSERISCI(T, z);

    return 1;
}

HASH_RB_NODE *HASH_RB_BST_MIN(HASH_RB_NODE *n, HASH_RB_NODE *NIL) {
    HASH_RB_NODE *cur = n;
    HASH_RB_NODE *prev = n;
    while (cur != NIL) {
        prev = cur;
        cur = cur->left;
    }
    return prev;
}

HASH_RB_NODE *HASH_RB_BST_SUCCESSORE(HASH_RB_TREE *T, HASH_RB_NODE *x) {
    HASH_RB_NODE *y;
    if (x->right != T->NIL)
        return HASH_RB_BST_MIN(x->right, T->NIL);
    y = x->p;
    while (y != T->NIL && y->right == x) {
        x = y;
        y = y->p;
    }
    return y;
}

void HASH_RB_RIPARA_CANCELLA(HASH_RB_TREE *T, HASH_RB_NODE *x) {
    if (x == NULL)
        return;
    while (x != T->root && x->color == black) {
        if (x == x->p->left) {
            HASH_RB_NODE *w = x->p->right;
            if (w->color == red) {
                w->color = black;
                x->p->color = red;
                HASH_RB_LEFT_ROTATE(T, x->p);
                w = x->p->right;
            }
            if (w->left->color == black && w->right->color == black) {
                w->color = red;
                x = x->p;
            } else {
                if (w->right->color == black) {
                    w->left->color = black;
                    w->color = red;
                    HASH_RB_RIGHT_ROTATE(T, w);
                    w = x->p->right;
                }
                w->color = x->p->color;
                x->p->color = black;
                w->right->color = black;
                HASH_RB_LEFT_ROTATE(T, x->p);
                x = T->root;
            }
        } else {
            HASH_RB_NODE *w = x->p->left;
            if (w->color == red) {
                w->color = black;
                x->p->color = red;
                HASH_RB_RIGHT_ROTATE(T, x->p);
                w = x->p->left;
            }
            if (w->right->color == black && w->left->color == black) {
                w->color = red;
                x = x->p;
            } else {
                if (w->left->color == black) {
                    w->right->color = black;
                    w->color = red;
                    HASH_RB_LEFT_ROTATE(T, w);
                    w = x->p->left;
                }
                w->color = x->p->color;
                x->p->color = black;
                w->left->color = black;
                HASH_RB_RIGHT_ROTATE(T, x->p);
                x = T->root;
            }
        }
    }
    x->color = black;
}

void HASH_RB_TRANSPLANT(HASH_RB_TREE *T, HASH_RB_NODE *u, HASH_RB_NODE *v) {
    if (u == T->NIL)
        return;
    else if (u->p == T->NIL) {
        T->root = v;
    } else if (u == u->p->left)
        u->p->left = v;
    else u->p->right = v;
    v->p = u->p;
}

_Bool HASH_RB_CANCELLA(HASH_RB_TREE *T, u_int32_t key) {
    HASH_RB_NODE *z = HASH_RB_BST_RICERCA(T, T->root, key);

    if (z == T->NIL)
        return 0;

    HASH_RB_NODE *y = z;
    HASH_RB_NODE *x;
    enum RB_NODE_COLOR originale_y = y->color;
    if (z->left == T->NIL) {
        x = z->right;
        HASH_RB_TRANSPLANT(T, z, z->right);
    } else if (z->right == T->NIL) {
        x = z->left;
        HASH_RB_TRANSPLANT(T, z, z->left);
    } else {
        y = HASH_RB_BST_MIN(z->right, T->NIL);
        originale_y = y->color;
        x = y->right;
        if (y->p == z)
            x->p = y;
        else {
            HASH_RB_TRANSPLANT(T, y, y->right);
            y->right = z->right;
            y->right->p = y;
        }
        HASH_RB_TRANSPLANT(T, z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }

    if (originale_y == black)
        HASH_RB_RIPARA_CANCELLA(T, x);

    free(z);
    return 1;
}

void HASH_RB_VISITA_CANCELLA(HASH_RB_TREE *T, HASH_RB_NODE *n) {
    if (n == T->NIL)
        return;
    else {
        HASH_RB_VISITA_CANCELLA(T, n->left);
        HASH_RB_VISITA_CANCELLA(T, n->right);
        free(n);
    }
}

void HASH_RB_CANCELLA_ALBERO(HASH_RB_TREE *T) {
    HASH_RB_VISITA_CANCELLA(T, T->root);
    free(T->NIL);
    free(T);
}

u_int32_t HASH_CALCOLA(u_int32_t k) {
    return (u_int32_t) (HASH_DIM * ((k * ALPHA) - (u_int32_t) (k * ALPHA)));
}

RB_NODE *HASH_CERCA(TREE *T, u_int32_t k) {
    u_int32_t calc = HASH_CALCOLA(k);
    if (T->hash_table[calc] != NULL)
        return HASH_RB_BST_RICERCA(T->hash_table[calc], T->hash_table[calc]->root, k)->real;
    return NULL;
}

_Bool HASH_CONTIENE(TREE *T, u_int32_t k) {
    return HASH_CERCA(T, k) != T->NIL;
}

void HASH_INSERISCI(TREE *T, RB_NODE *n) {
    u_int32_t calc = HASH_CALCOLA(n->key);
    if (T->hash_table[calc] == NULL) {
        T->hash_table[calc] = HASH_CREA_ALBERO();
    }
    HASH_RB_NODE *copia = HASH_CREA_NODO_RB(n->key);
    copia->real = n;
    HASH_RB_INSERISCI(T->hash_table[calc], copia);
}

void HASH_CANCELLA(TREE *T, u_int32_t k) {
    u_int32_t calc = HASH_CALCOLA(k);
    HASH_RB_CANCELLA(T->hash_table[calc], k);
}

void HASH_SVUOTA(TREE *T) {
    for (u_int16_t i = 0; i < HASH_DIM; i++) {
        if (T->hash_table[i] != NULL) {
            HASH_RB_CANCELLA_ALBERO(T->hash_table[i]);
        }
    }
}

/**
 * DEQUE (Coda a due fini, variante per nodi RB)
 */
typedef struct LIST_RB_NODES_ELEMENT LIST_RB_NODES_ELEMENT;
struct LIST_RB_NODES_ELEMENT {
    RB_NODE *n;
    LIST_RB_NODES_ELEMENT *next;
};

struct LIST_RB_NODES_HEAD {
    LIST_RB_NODES_ELEMENT *first;
    LIST_RB_NODES_ELEMENT *last;
    u_int8_t size;
};

void INSERISCI_LISTA_RB_NODES(LIST_RB_NODES_HEAD *head, RB_NODE *n) {
    LIST_RB_NODES_ELEMENT *el = (LIST_RB_NODES_ELEMENT *) malloc(sizeof(LIST_RB_NODES_ELEMENT));
    el->n = n;
    el->next = NULL;
    if (head->first != NULL) {
        head->last->next = el;
        head->last = el;
    } else {
        head->first = el;
        head->last = el;
    }
    head->size++;
}

LIST_RB_NODES_HEAD *CREA_LISTA_RB_NODES() {
    LIST_RB_NODES_HEAD *ret = (LIST_RB_NODES_HEAD *) malloc(sizeof(LIST_RB_NODES_HEAD));
    ret->first = NULL;
    ret->last = NULL;
    ret->size = 0;
    return ret;
}

LIST_RB_NODES_ELEMENT *CERCA_LISTA_RB_NODES(LIST_RB_NODES_HEAD *head, u_int32_t k) {
    if (head == NULL)
        return NULL;
    LIST_RB_NODES_ELEMENT *cur = head->first;
    while (cur != NULL) {
        if (cur->n->key == k)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

LIST_RB_NODES_ELEMENT *CERCA_PRED_LISTA_RB_NODES(LIST_RB_NODES_HEAD *head, u_int32_t k) {
    if (head == NULL)
        return NULL;
    LIST_RB_NODES_ELEMENT *cur = head->first;
    LIST_RB_NODES_ELEMENT *pred = NULL;
    while (cur != NULL) {
        if (cur->n->key == k)
            return pred;
        pred = cur;
        cur = cur->next;
    }
    return NULL;
}

void CANCELLA_ELEMENTO_LISTA_RB_NODES(LIST_RB_NODES_HEAD *head, u_int32_t k) {
    LIST_RB_NODES_ELEMENT *res = CERCA_LISTA_RB_NODES(head, k);
    if (res == NULL) return;
    LIST_RB_NODES_ELEMENT *pred = CERCA_PRED_LISTA_RB_NODES(head, k);
    if (head->first == res)
        head->first = res->next;
    else
        pred->next = res->next;
    if (head->last == res)
        head->last = pred;
    head->size--;
    free(res);
}

/**
 * ALGORITMICA DI PROGETTO
 */

RB_NODE *tornaStazione(TREE *T, u_int32_t dist) {
    return MASCHERA_RICERCA_HASH_BST(T, dist);
}

_Bool aggiungiAuto(TREE *T, RB_NODE *n, u_int32_t autonomia) {
    if (n != T->NIL) {
        HEAP_INSERISCI(n->autonomie, autonomia);
        return 1;
    } else return 0;
}

_Bool rottamaAuto(TREE *T, RB_NODE *n, u_int32_t autonomia) {
    if (n != T->NIL) {
        return HEAP_CANCELLA_NODO(n->autonomie, autonomia);
    } else return 0;
}

_Bool aggiungiStazione(TREE *T, RB_NODE *n) {
    if (RB_INSERISCI(T, n))
        return 1;
    else return 0;
}

_Bool demolisciStazione(TREE *T, u_int32_t key) {
    if (RB_CANCELLA(T, key))
        return 1;
    else return 0;
}

u_int32_t contaStazioni(TREE *T, RB_NODE *s1, RB_NODE *s2) {
    RB_NODE *cur = s1;
    u_int32_t conta = 1;
    while (cur != s2) {
        cur = BST_SUCCESSORE(T, cur);
        conta++;
    }
    return conta;
}

_Bool isRaggiungibile(RB_NODE *s1, RB_NODE *s2) {
    if (s1->key <= s2->key) {
        if (s1->autonomie->heapsize > 0 && (s1->key + HEAP_MAX(s1->autonomie) >= s2->key))
            return 1;
        else return 0;
    } else {
        if (s1->autonomie->heapsize > 0 &&
            ((HEAP_MAX(s1->autonomie) > s1->key) || (s1->key - HEAP_MAX(s1->autonomie) <= s2->key)))
            return 1;
        else return 0;
    }
}

void stampaPercorso(RB_NODE *sorgente, RB_NODE **prev, u_int32_t *prevInd, u_int32_t posTappa) {
    if (prev[posTappa] == sorgente) {
        printf("%d ", sorgente->key);
        return;
    }
    stampaPercorso(sorgente, prev, prevInd, prevInd[posTappa]);
    printf("%d ", prev[posTappa]->key);
}

// Variante ottimizzata di Dijkstra, non fa uso di un min-heap, essendo i nodi già ordinati in maniera naturale dall'autostrada
void pianificaPercorsoOppostoDijkstra(TREE *T, u_int32_t s1, u_int32_t s2) {
    RB_NODE *staz1 = MASCHERA_RICERCA_HASH_BST(T, s1);
    RB_NODE *staz2 = MASCHERA_RICERCA_HASH_BST(T, s2);
    u_int32_t conta = contaStazioni(T, staz2, staz1);
    RB_NODE **stazioni = (RB_NODE **) malloc(conta * sizeof(RB_NODE));
    u_int32_t *dist = (u_int32_t *) malloc(conta * sizeof(u_int32_t));
    RB_NODE **prev = (RB_NODE **) malloc(conta * sizeof(RB_NODE));
    u_int32_t *prevInd = (u_int32_t *) malloc(conta * sizeof(u_int32_t));

    RB_NODE *cur = staz2;
    stazioni[0] = staz1;
    dist[0] = 0;
    prev[0] = T->NIL;
    prevInd[0] = UINT_MAX;
    prev[conta - 1] = T->NIL;
    prevInd[conta - 1] = UINT_MAX;
    for (u_int32_t i = conta - 1; i >= 1; i--) {
        stazioni[i] = cur;
        dist[i] = UINT_MAX;
        cur = BST_SUCCESSORE(T, cur);
    }

    for (u_int32_t i = 0; i < conta; i++) {
        if (dist[i] == UINT_MAX)
            continue;
        RB_NODE *u = stazioni[i];

        u_int32_t j = i + 1;

        u_int32_t ndis = dist[i] + 1;
        while (j < conta && isRaggiungibile(u, stazioni[j])) {
            if (dist[j] >= ndis) {
                dist[j] = ndis;
                prev[j] = u;
                prevInd[j] = i;
            }

            j++;
        }
    }

    u_int32_t posTappa = conta - 1;
    RB_NODE *prec = prev[posTappa];
    if (prec == T->NIL)
        printf("nessun percorso\n");
    else {
        stampaPercorso(staz1, prev, prevInd, posTappa);
        printf("%d\n", s2);
    }
    free(stazioni);
    free(dist);
    free(prev);
    free(prevInd);
}

// La variante per il verso naturale fa già uso della visita in ampiezza, che ha complessità minore
void pianificaPercorsoDijkstra(TREE *T, u_int32_t s1, u_int32_t s2) {
    if (s1 < s2) {
        RB_NODE *staz1 = MASCHERA_RICERCA_HASH_BST(T, s1);
        RB_NODE *staz2 = MASCHERA_RICERCA_HASH_BST(T, s2);
        u_int32_t conta = contaStazioni(T, staz1, staz2);
        RB_NODE **stazioni = (RB_NODE **) malloc(conta * sizeof(RB_NODE));
        u_int32_t *dist = (u_int32_t *) malloc(conta * sizeof(u_int32_t));
        RB_NODE **prev = (RB_NODE **) malloc(conta * sizeof(RB_NODE));
        u_int32_t *prevInd = (u_int32_t *) malloc(conta * sizeof(u_int32_t));

        RB_NODE *cur = staz1;
        stazioni[0] = staz1;
        dist[0] = 0;
        prev[0] = T->NIL;
        prevInd[0] = UINT_MAX;
        prev[conta - 1] = T->NIL;
        prevInd[conta - 1] = UINT_MAX;
        for (u_int32_t i = 1; i < conta; i++) {
            cur = BST_SUCCESSORE(T, cur);
            stazioni[i] = cur;
            dist[i] = UINT_MAX;
        }

        for (u_int32_t i = 0; i < conta; i++) {
            if (dist[i] == UINT_MAX)
                continue;
            RB_NODE *u = stazioni[i];

            u_int32_t j = i + 1;

            u_int32_t ndis = dist[i] + 1;
            while (j < conta && isRaggiungibile(u, stazioni[j])) {
                if (dist[j] > ndis) {
                    dist[j] = ndis;
                    prev[j] = u;
                    prevInd[j] = i;
                }

                j++;
            }
        }

        u_int32_t posTappa = conta - 1;
        RB_NODE *prec = prev[posTappa];
        if (prec == T->NIL)
            printf("nessun percorso\n");
        else {
            stampaPercorso(staz1, prev, prevInd, posTappa);
            printf("%d\n", s2);
        }
        free(stazioni);
        free(dist);
        free(prev);
        free(prevInd);
    } else if (s1 > s2) {
        return pianificaPercorsoOppostoDijkstra(T, s1, s2);
    } else
        printf("%d\n", s1);
}

enum AMPIEZZA_COLOR {
    white, grey // nero non serve, l'ideale "grafo" è aciclico
};

// Utilizza la ricerca in ampiezza, per esplorare l'ideale grafo del percorso
// la complessità è di gran lunga attutita rispetto all'algoritmo di Dijkstra, a causa della coda di visita specializzata e non pre-determinata
// si noti che la soluzione proposta è valida da singola sorgente, su grafo orientato, aciclico e non pesato (DAG)
void pianificaPercorsoSemiAmpiezza(TREE *T, u_int32_t s1, u_int32_t s2) {
    if (s1 < s2) {
        RB_NODE *staz1 = MASCHERA_RICERCA_HASH_BST(T, s1);
        RB_NODE *staz2 = MASCHERA_RICERCA_HASH_BST(T, s2);
        u_int32_t conta = contaStazioni(T, staz1, staz2);
        RB_NODE **stazioni = (RB_NODE **) malloc(conta * sizeof(RB_NODE));
        enum AMPIEZZA_COLOR *colore = (enum AMPIEZZA_COLOR *) malloc(conta * sizeof(enum AMPIEZZA_COLOR));
        RB_NODE **prev = (RB_NODE **) malloc(conta * sizeof(RB_NODE));
        u_int32_t *prevInd = (u_int32_t *) malloc(conta * sizeof(u_int32_t));
        RB_NODE **visita = (RB_NODE **) malloc(conta * sizeof(RB_NODE));
        u_int32_t primo = 0, ultimo = 0;

        RB_NODE *cur = staz1;
        stazioni[0] = staz1;
        prev[0] = T->NIL;
        prevInd[0] = UINT_MAX;
        prev[conta - 1] = T->NIL;
        prevInd[conta - 1] = UINT_MAX;
        colore[0] = grey;
        for (u_int32_t i = 1; i < conta; i++) {
            cur = BST_SUCCESSORE(T, cur);
            stazioni[i] = cur;
            colore[i] = white;
        }

        visita[ultimo++] = staz1;

        u_int32_t i = 0;
        u_int32_t j = 1;
        u_int32_t ultimoVisitato = 0;
        while (primo < ultimo) {
            cur = visita[primo++];

            j = ultimoVisitato + 1;
            while (j < conta && isRaggiungibile(cur, stazioni[j])) {
                if (colore[j] == white) {
                    colore[j] = grey;
                    prev[j] = cur;
                    prevInd[j] = i;
                    visita[ultimo++] = stazioni[j];
                    ultimoVisitato = j;
                }
                j++;
            }

            i++;
        }
        if (colore[conta - 1] == white)
            printf("nessun percorso\n");
        else {
            stampaPercorso(staz1, prev, prevInd, conta - 1);
            printf("%d\n", s2);
        }
        free(stazioni);
        free(colore);
        free(prev);
        free(prevInd);
        free(visita);
    } else if (s1 > s2) {
        return pianificaPercorsoOppostoDijkstra(T, s1, s2);
    } else
        printf("%d\n", s1);
}

/*
 * DICHIARAZIONI GLOBALI PER TUTTE LE STRUTTURE
 */
void dichiarazioneGlobali() {
    UINT32_INVALID = malloc(sizeof(u_int32_t));
    *UINT32_INVALID = UINT_MAX;
}

void liberaDichiarazioniGlobali() {
    free(UINT32_INVALID);
}

#define MAXLENCOMANDI (18 + 1)

int main() {
    // return esempi();

    dichiarazioneGlobali();

    TREE *rbstazioni = CREA_ALBERO();

    while (!feof(stdin)) {
        char str[MAXLENCOMANDI];
        if (!scanf("%s", str))
            return 1;
        if (!feof(stdin)) {
            getchar_unlocked();
            if (strcmp(str, "aggiungi-auto") == 0) {
                // aggiungi-auto
                u_int32_t d = leggiU32();
                u_int32_t a = leggiU32();
                if (aggiungiAuto(rbstazioni, tornaStazione(rbstazioni, d), a))
                    printf("aggiunta\n");
                else printf("non aggiunta\n");
            } else if (strcmp(str, "rottama-auto") == 0) {
                // rottama-auto
                u_int32_t d = leggiU32();
                u_int32_t a = leggiU32();
                if (rottamaAuto(rbstazioni, tornaStazione(rbstazioni, d), a))
                    printf("rottamata\n");
                else printf("non rottamata\n");
            } else if (strcmp(str, "aggiungi-stazione") == 0) {
                // aggiungi-stazione
                u_int32_t d = leggiU32();
                _Bool inserisci = 0;
                RB_NODE *s = tornaStazione(rbstazioni, d);
                if (s != rbstazioni->NIL)
                    printf("non aggiunta\n");
                else {
                    s = CREA_NODO_RB(d);
                    aggiungiStazione(rbstazioni, s);
                    inserisci = 1;
                }
                u_int16_t numa = leggiU16();
                u_int32_t a;
                for (u_int16_t i = 0; i < numa; ++i) {
                    a = leggiU32();
                    if (inserisci)
                        aggiungiAuto(rbstazioni, s, a);
                }
                if (inserisci)
                    printf("aggiunta\n");
            } else if (strcmp(str, "demolisci-stazione") == 0) {
                // demolisci-stazione
                u_int32_t d = leggiU32();
                if (demolisciStazione(rbstazioni, d))
                    printf("demolita\n");
                else printf("non demolita\n");
            } else if (strcmp(str, "pianifica-percorso") == 0) {
                // pianifica-percorso
                // stampaVisualizzazioneAutostrada(rbstazioni);
                u_int32_t s1 = leggiU32();
                u_int32_t s2 = leggiU32();
                pianificaPercorsoSemiAmpiezza(rbstazioni, s1, s2);
            }
        }
    }

    RB_CANCELLA_ALBERO(rbstazioni);

    liberaDichiarazioniGlobali();
}