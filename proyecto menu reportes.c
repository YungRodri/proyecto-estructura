#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> ///es para hacer funcionar los votos aleatorios
#define MAX_CANDIDATOS 20
#define IDX_SIN_VOTO   -1

/* Opcional: códigos de ronda/estado si los quieres como int */
#define RONDA_PRIMERA   1
#define RONDA_SEGUNDA   2
#define ELEC_ABIERTA    1
#define ELEC_CERRADA    2
#define ELEC_PROCLAMADA 3



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
    char *rut[12];
    char *nombre[50]; 
    char *nacionalidad[30];
    int  edad;
};

/* ====== Candidato (pool estático del Servel) ====== */
struct Candidato {
    struct Persona *datos;
    char *partido[40];
    char *tipo[20];     /* "Partido" o "Independiente" */
    int  firmasApoyo;
    int  esValido;     /* 1 si aprobado por Servel  0 Si no es aprobado*/
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
    char *comuna[40];
    char *direccion[100];

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
    struct Resultado * candidato_En_Resultado;
    struct Eleccion *sig;                     /* siguiente elección (historial) */
};

/* ====== Resultado (nodo de LISTA CIRCULAR en Tricel) ====== */
struct Resultado {
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
    struct Candidato *candidatos[MAX_CANDIDATOS]; /* pool estático */
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






struct Persona* crearPersonaEstatica(const char *nombre, int edad)
{
    struct Persona *p = malloc(sizeof(struct Persona));

    strcpy((char*)p->nombre, nombre);
    strcpy((char*)p->nacionalidad, "Chilena");
    strcpy((char*)p->rut, "11111111-1");
    p->edad = edad;

    return p;
}




struct Candidato* crearCandidatoEstatico(int id, const char *nombre)
{
    struct Candidato *c = malloc(sizeof(struct Candidato));

    c->datos = crearPersonaEstatica(nombre, 40);
    strcpy((char*)c->partido, "Test");
    strcpy((char*)c->tipo, "Partido");

    c->id = id;
    c->esValido = 1;
    c->firmasApoyo = 500;

    return c;
}



struct Eleccion* crearEleccionEstatica()
{
    struct Eleccion *e = malloc(sizeof(struct Eleccion));

    e->id = 999;
    e->ronda = RONDA_PRIMERA;
    e->estado = ELEC_CERRADA;
    e->nCands = 5;
    e->arbolMesas = NULL;
    e->candidato_En_Resultado = NULL;
    e->sig = NULL;

    e->cands[0] = crearCandidatoEstatico(10, "Ana Perez");
    e->cands[1] = crearCandidatoEstatico(20, "Luis Soto");
    e->cands[2] = crearCandidatoEstatico(30, "Maria Diaz");
    e->cands[3] = crearCandidatoEstatico(40, "Carlos Rey");
    e->cands[4] = crearCandidatoEstatico(50, "Juan Leon");

    return e;
}





struct Resultado* crearResultadoEstatico(struct Eleccion *elec)
{
    struct Resultado *r = malloc(sizeof(struct Resultado));

    r->totalMesas = 5;
    r->totalVotantesRegistrados = 1000;
    r->totalVotantesVotaron = 700;
    r->votosBlancos = 10;
    r->votosNulos = 15;
    r->votosValidos = 675;

    r->porcentajeParticipacion = 70.0;

    r->porcentajeCandidato[0] = 12.5;
    r->porcentajeCandidato[1] = 25.0;
    r->porcentajeCandidato[2] = 40.0;
    r->porcentajeCandidato[3] = 15.5;
    r->porcentajeCandidato[4] = 7.0;

    r->idxGanador = 2;
    r->ganador = elec->cands[2];
    r->porcentajeGanador = 40.0;

    r->sig = r;

    return r;
}




void rellenarSistemaEstatico(struct SistemaElectoral *sis)
{
    sis->servel = malloc(sizeof(struct Servel));
    sis->tricel = malloc(sizeof(struct Tricel));

    sis->servel->elecciones = NULL;
    sis->servel->totalCandidatos = 0;

    sis->tricel->headResultados = NULL;
    sis->tricel->totalResultados = 0;

    struct Eleccion *e = crearEleccionEstatica();
    sis->servel->elecciones = e;

    struct Resultado *r = crearResultadoEstatico(e);
    e->candidato_En_Resultado = r;

    sis->tricel->headResultados = r;
    sis->tricel->totalResultados = 1;

    printf("Sistema cargado con datos ESTÁTICOS.\n");
}


///--------------------------------------FUNCIONES PARA EL MENU DE REPORTES-------------------------------------------------------------------


void mostrarResultadosParaVotantes(struct Resultado *resultados, struct Eleccion *eleccion)
{


    printf("ID de la eleccion = %d\n\n", eleccion->id);

    printf("total de mesas = %d\n", resultados->totalMesas);
    printf("total de votantes registrados = %d\n", resultados->totalVotantesRegistrados);
    printf("total de votos = %d\n", resultados->totalVotantesVotaron);
    printf("total de votos nulos = %d\n", resultados->votosNulos);
    printf("total de votos blancos = %d\n", resultados->votosBlancos);
    printf("total de votos efectivos = %d\n\n", resultados->votosValidos);


}



struct Eleccion *buscarUltimaEleccion(struct Servel *servel){
    struct Eleccion *aux = servel->elecciones;

    /// si no hay elecciones, retornar NULL
    if (aux == NULL){
        return NULL;
    }

    /// recorrer la lista simple hasta llegar a la última elección
    while (aux->sig != NULL){   /// no se usa do while ya que podría estar vacía
        aux = aux->sig;
    }

    return aux; /// se retorna la última elección
}







void ordenarPorPorcentajeMayorAmenorExchange(struct Candidato **candidatos, float *porcentaje, int n)
{
    int i, j;
    float auxP;
    struct Candidato *auxC;

    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {

            if (porcentaje[i] < porcentaje[j]) {

                auxP = porcentaje[i];
                porcentaje[i] = porcentaje[j];
                porcentaje[j] = auxP;

                auxC = candidatos[i];
                candidatos[i] = candidatos[j];
                candidatos[j] = auxC;
            }
        }
    }
}



void ordenarPorPorcentajeMenorAmayorExchange(struct Candidato **candidatos, float *porcentaje, int n)
{
    int i, j; ///SE DECLARAN VARIABLES PARA RECORRER ARREGLO
    float auxP; /// VARIABLE AUXILIAR PARA INTERCAMBIAR LOS PORCENTAJES
    struct Candidato *auxC; /// AUXILIAR PARA EL INTERCAMBIO(NO SE USA DOBLE PUNTERO PORQUE NO GUARDA EL ARREGLO ENTERO)

    for (i = 0; i < n; i++) { /// CICLO 1
        for (j = i + 1; j < n; j++) { /// CICLO 2 QUE COMPARA I CON TODOS LOS ELEMENTOS SIGUIENTES

            if (porcentaje[i] > porcentaje[j]) { /// SI EL PORCENTAJE ACTUAL ES MEJOR QUE EL PORCENTAJE DE LA POSICION J SE INTERCAMBIAN

                auxP = porcentaje[i]; /// SE GUARDA EL PORCENTAJE DE LA POSICION I
                porcentaje[i] = porcentaje[j]; /// MUEVE EL PORCENTAJE J A LA POSICION I
                porcentaje[j] = auxP; ///  /// RESTAURA EL PUNTERO GUARDADO EN J (INTERCAMBIO DE PORCENTAJES)

                auxC = candidatos[i]; /// SE GUARDA EL PUNTERO DEL CANDIDATO EN LA POSICION I
                candidatos[i] = candidatos[j]; /// MUEVE EL CANDIDATO J A LA POSICION I
                candidatos[j] = auxC; /// RESTAURA EL PUNTERO GUARDADO EN J (INTERCAMBIO DE CANDIDATOS)
            }
        }
    }
}


///METODOS DE ORDENAMIENTO : EXCHANGE SORT PARA ORDENAR POR PORCENTAJES MAYOR A MENOR O VICEVERSA

/// SE LES PASA CON DOBLE PUNTERO (**) PORQUE CANDIDATOS ES UN ARREGLO CON PUNTEROS A CANDIDATOS, OSEA SE USA ESTO PARA PODER INTERCAMBIAR PUNTEROS 
///DURANTE EL SORT SIN COPIAR STRUCTS COMPLETOS

/// SI NO SE USARA DOBLE PUNTERO HABRIAN PROBLEMAS PARA INTERCAMBIAR CANDIDATOS (ORDENARLOS), YA QUE LOS TAMANOS SON DISTINOS, HAY RIESGO DE OVERWRITE
///Y SE INTENTARIAS COPIAR STRUCTS COMPLETOS

/// OTRO PORBLEMA ES QUE EL CODIGO NISIQUIERA COMPILARIA Y SE PERDERIA LA RELACION DE CANDIDATO Y PORCENTAJE


/// EXPLICACION INFORMAL : El ciclo va revisando cada candidato y lo compara con los que vienen después para ver quién tiene más porcentaje.
///Si encuentra uno más grande, simplemente los cambia de lugar para ir dejando a los más altos arriba.


void mostrarReportePorcentaje(struct Resultado *resultados, struct Eleccion *eleccion){

    printf("LISTA DE PORCENTAJES DE LA ULTIMA ELECCION\n");
    printf("-------------------------------------------\n\n");

    printf("PORCENTAJE DE PARTICIPACION: %.2f %%\n\n", resultados->porcentajeParticipacion);

    int n = eleccion->nCands;

    for(int i = 0; i < n; i++){
        printf("ID CANDIDATO: %d\n", eleccion->cands[i]->id);
        printf("NOMBRE: %s\n", eleccion->cands[i]->datos->nombre);
        printf("PORCENTAJE: %.2f %%\n\n", resultados->porcentajeCandidato[i]);
    }
}

///------------------------------------FIN FUNCIONES PARA REPORTES-------------------------------------------------------------------------

///------------------------------------MENU PARA REPORTES-----------------------------------------------------------------------------------


void MenuReportes(struct SistemaElectoral *sistema)
{
    int indice = -1;

    do
    {
        printf("\n-- REPORTES --\n");
        printf("1) Ordenar candidatos por porcentaje : MAYOR A MENOR(Ultima eleccion)\n");
        printf("2) Ordenar candidatos por porcentaje : MENOR A MAYOR(Ultima eleccion)\n");
        printf("3) Mostrar Resultados de tipo de votos (Ultima eleccion)\n");
        printf("0) Volver\n");

        if (scanf("%d", &indice) != 1)
        {
            indice = -1;
            continue;
        }

        /* obtener la última elección */
        struct Eleccion *aux = buscarUltimaEleccion(sistema->servel);

        if (aux == NULL)
        {
            printf("No hay elecciones registradas.\n");
            continue;
        }

        /* obtener resultado asociado */
        struct Resultado *res = aux->candidato_En_Resultado;

        if (res == NULL)
        {
            printf("No hay resultados para la última elección.\n");
            continue;
        }

        switch (indice)
        {
            case 1:
                ordenarPorPorcentajeMayorAmenorExchange(aux->cands, res->porcentajeCandidato, aux->nCands);
                printf("\nOrdenado: MAYOR a MENOR (ExchangeSort).\n");
                mostrarReportePorcentaje(res, aux);
                break;

            case 2:
                ordenarPorPorcentajeMenorAmayorExchange(aux->cands, res->porcentajeCandidato, aux->nCands);
                printf("\nOrdenado: MENOR a MAYOR .\n");
                mostrarReportePorcentaje(res, aux);
                break;

            case 3:
                printf("\n=== RESULTADOS DE VOTOS ===\n\n");
                mostrarResultadosParaVotantes(res, aux);
                break;

            case 0:
                printf("Volviendo...\n");
                break;

            default:
                printf("Opción inválida.\n");
        }

    } while (indice != 0);
}

///-------------------------------------------FIN DE MENU DE REPORTES-------------------------------------------------------------------------





int main()
{
    struct SistemaElectoral sistema;

    rellenarSistemaEstatico(&sistema);

    MenuReportes(&sistema);

    return 0;
}
