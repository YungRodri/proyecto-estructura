
#include <stdio.h>    // printf, scanf, fprintf, etc.
#include <stdlib.h>   // malloc, calloc, realloc, free, EXIT_*
#include <string.h>
/* ====== Constantes útiles (sin enum) ====== */
#define MAX_CANDIDATOS 20
#define IDX_SIN_VOTO   -1

/* Opcional: códigos de ronda/estado si los quieres como int */
#define RONDA_PRIMERA   1
#define RONDA_SEGUNDA   2
#define ELEC_ABIERTA    1
#define ELEC_CERRADA    2
#define ELEC_PROCLAMADA 3

/* ====== Forward declarations (porque hay punteros cruzados) ====== */
struct Candidato;
struct DVotante;
struct NodoMesa;
struct Eleccion;
struct Resultado;
struct Servel;
struct Tricel;
struct SistemaElectoral;

/* ====== Persona ====== */
struct Persona {
    char rut[12];
    char nombre[50];
    char nacionalidad[30];
    int  edad;
};

/* ====== Candidato (pool estático del Servel) ====== */
struct Candidato {
    struct Persona datos;
    char partido[40];
    char tipo[20];     /* "Partido" o "Independiente" */
    int  firmasApoyo;
    int  esValido;     /* 1 si aprobado por Servel */
    int  id;           /* índice dentro del pool */
};

/* ====== Votante (lista DOBLEMENTE enlazada por mesa) ====== */
struct DVotante {
    struct Persona datos;
    int  habilitado;       /* 1 puede votar */
    int  haVotado;         /* 1 ya votó */
    int  idxCandVoto;      /* 0..nCands-1 dentro de la ELECCIÓN, o IDX_SIN_VOTO */
    struct DVotante *ant;
    struct DVotante *sig;
};

/* ====== Mesa (nodo del ABB de una elección) ====== */
struct NodoMesa {
    int  idMesa;
    char comuna[40];
    char direccion[100];

    int  votosCandidatos[MAX_CANDIDATOS]; /* usa 0..(nCands-1) de la elección */
    int  totalVotosEmitidos;
    int  votosBlancos;
    int  votosNulos;

    struct DVotante *headV;  /* lista doble: cabeza */
    struct DVotante *tailV;  /* lista doble: cola   */

    struct NodoMesa *izq;    /* ABB por idMesa */
    struct NodoMesa *der;
};

/* ====== Elección (nodo de LISTA SIMPLE en Servel) ====== */
struct Eleccion {
    int  id;
    int  ronda;    /* RONDA_PRIMERA / RONDA_SEGUNDA */
    int  estado;   /* ELEC_ABIERTA / ELEC_CERRADA / ELEC_PROCLAMADA */

    struct Candidato *cands[MAX_CANDIDATOS]; /* arreglo compacto de punteros */
    int   nCands;                             /* tamaño efectivo del arreglo */

    struct NodoMesa *arbolMesas;              /* ABB propio de esta elección */

    struct Eleccion *sig;                     /* siguiente elección (historial) */
};

/* ====== Resultado (nodo de LISTA CIRCULAR en Tricel) ====== */
struct Resultado {
    struct Eleccion  *eleccion;   /* referencia a la elección */
    struct Candidato *ganador;    /* puntero a uno de eleccion->cands[idxGanador] */

    int   totalMesas;
    int   totalVotantesRegistrados;
    int   totalVotantesVotaron;
    int   votosValidos;
    int   votosBlancos;
    int   votosNulos;
    float porcentajeParticipacion;               /* 0..100 */
    float porcentajeCandidato[MAX_CANDIDATOS];   /* solo 0..(nCands-1) */
    int   idxGanador;                            /* índice dentro del arreglo compacto */
    float porcentajeGanador;

    struct Resultado *sig;   /* anillo: lista circular simplemente enlazada */
};

/* ====== Servel: pool de candidatos + LISTA de elecciones ====== */
struct Servel {
    struct Candidato candidatos[MAX_CANDIDATOS]; /* pool estático */
    int   totalCandidatos;

    struct Eleccion *elecciones;                 /* cabeza de la lista simple */
    int   totalVotantesRegistrados;              /* opcional global */
};

/* ====== Tricel: LISTA CIRCULAR de resultados ====== */
struct Tricel {
    struct Resultado *headResultados;  /* NULL si vacío; si 1 nodo: head->sig == head */
    int   totalResultados;
};

/* ====== Sistema (punteros a módulos en heap) ====== */
struct SistemaElectoral {
    struct Servel *servel;
    struct Tricel *tricel;
};

int main()
{
    printf("hola");
    return 0;

}
