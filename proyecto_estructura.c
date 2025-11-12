
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
    struct Persona *datos;
    char *partido[40];
    char *tipo[20];     /* "Partido" o "Independiente" */
    int  firmasApoyo;
    int  esValido;     /* 1 si aprobado por Servel y 0 si no es aprobado*/
    int  id;           /* índice dentro del pool */
};

/* ====== Votante (lista DOBLEMENTE enlazada por mesa) ====== */
struct DVotante {
    struct Persona *datos;
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
// Aqui hice el menu el dia 10 de octubre




//valida que un candidato cumpla
int validarDatosCanditado(struct Candidato *candidato){
    int contadorValidador = 0;

    if (candidato->datos.edad >= 35) contadorValidador ++;

    if (strcmp(candidato->datos.nacionalidad,"chilena") == 0) contadorValidador++;

    if(strcmp(candidato->tipo,"independiente") == 0){
        if (candidato->firmasApoyo > 100000) contadorValidador ++;

    }


    if(strcmp(candidato->tipo,"independiente") == 0){

        if (contadorValidador == 3) return 1;

        else return 0;
    }



    if (contadorValidador == 2) return 1;

    return 0;


}



//
int ValidacionDeCandidatos (struct Servel *servel){
    int i, contadorDeValidos = 0;

    for (i = 0; i < servel->totalCandidatos; i ++){
        servel->candidatos[i].esValido=validarDatosCanditado(&servel->candidatos[i]);

        if (servel->candidatos[i].esValido == 1) contadorDeValidos++;
    }

    return contadorDeValidos;

}

// llena un arreglo con los candidatos que si cumplieron los requisitos
struct Candidato **rellenarCandidatosValidos(struct Servel *servel){

    int i,TotalValidados = 0, k = 0;
    struct Candidato **candidatos = NULL;
    TotalValidados = ValidacionDeCandidatos(servel);

    if (TotalValidados == 0){
        printf("No hay candidatos validos");
        return NULL;
    }

    candidatos = (struct Candidato **) malloc(TotalValidados * sizeof(struct Candidato *));

    for (i = 0; i < servel->totalCandidatos; i ++){

        if (servel->candidatos[i].esValido == 1){
            candidatos[k] = &servel->candidatos[i];
            k ++;
        }
    }


    printf("cantidad de candidatos validos = %d\n",TotalValidados);

    return candidatos;

}


void menuServel(void)
{
    int indice = -1;
    do
    {
        printf("\n-- SERVEL --\n");
        printf("1) Agregar candidato \n");
        printf("2) Listar candidatos \n");
        printf("3) Marcar validez de candidato\n");
        printf("4) Crear ELECCION (agregar al historial)\n");
        printf("5) Listar ELECCIONES\n");
        printf("6) Eliminar ELECCION\n");
        printf("0) Volver\n");
        printf("Opcion: ");

        if (scanf("%d", &indice) != 1) {
            printf("Entrada invalida.\n");
            indice = -1;
            continue;
        }

        switch (indice)
        {
            case 1: {
                //printf("Aun no\n");
                struct Candidato candidato;
                int rc;
                printf("Rut (sin puntos , sin guion): ");
                if (scanf("%11s", candidato.datos.rut) != 1)
                {
                    printf("Rut Invalido\n");
                    break;
                }
                printf("Nombre: ");
                if ((scanf("%49s", candidato.datos.nombre)) != 1)
                {
                    printf("Nombre Invalido\n");
                    break;
                }
                printf("Nacionalidad: ");
                if ((scanf("%29[^\n]"), candidato.datos.nacionalidad) != 1)
                {
                    printf("Nacionalidad Invalida\n");
                    break;

                }
                printf("Edad: ");
                if ((scanf("%d", &candidato.datos.edad)) != 1)
                {
                    printf("Edad Invalida\n");
                    break;

                }
            }
                break;
            case 2:
                printf("Aun no\n");
                break;
            case 3:
                printf("Aun no\n");
                break;
            case 4:
                printf("Aun no\n");
                break;
            case 5:
                printf("Aun no\n");
                break;
            case 6:
                printf("Aun no\n");
                break;
            case 0:
                printf("Volviendo al menu principal...\n");
                break;
            default:
                printf("Opcion invalida en Servel.\n");
                break;
        }
    } while (indice != 0);
}

int main(void)
{
    int Eleccion_Usuario = -1;
    do
    {
        printf("\nSistema Principal\n");
        printf("Seleccione\n");
        printf("0) Salir\n");
        printf("1) Servel\n");
        printf("2) Tricel\n");
        printf("3) Elecciones\n");
        printf("4) Votantes\n");
        printf("5) Reportes\n");
        printf("6) Mesas\n");
        printf("Opcion: ");

        if (scanf("%d", &Eleccion_Usuario) != 1) {
            printf("Entrada invalida.\n");
            Eleccion_Usuario = -1;
            continue;
        }

        switch (Eleccion_Usuario)
        {
            case 1:
                menuServel();
                break;
            case 2:
                printf("tricel\n");
                break;
            case 3:
                printf("elecciones\n");
                break;
            case 4:
                printf("votantes\n");
                break;
            case 5:
                printf("reportes\n");
                break;
            case 6:
                printf("mesas\n");
                break;
            case 0:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opcion invalida.\n");
                break;
        }

    } while (Eleccion_Usuario != 0);

    return 0;
}
