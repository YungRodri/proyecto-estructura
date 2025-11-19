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






























//------------------------------------------------------------------------------------------------------------------------------------------------------


void ContarMesas(struct NodoMesa *mesas, int *contador){
    if (mesas){
        ContarMesas(mesas->izq, contador);
        (*contador)++;
        ContarMesas(mesas->der, contador);
    }

}

void ContarVotosEmitidos(struct NodoMesa *mesas, int *contadorVotos){
    if (mesas){
        ContarVotosEmitidos(mesas->izq, contadorVotos);
        (*contadorVotos)+= mesas->totalVotosEmitidos;
        ContarVotosEmitidos(mesas->der, contadorVotos);
    }

}

void ContarVotosNulos(struct NodoMesa *mesas, int *contadorNulos){
    if (mesas){
        ContarVotosNulos(mesas->izq, contadorNulos);
        (*contadorNulos)+= mesas->votosNulos;
        ContarVotosNulos(mesas->der, contadorNulos);
    }
}

void ContarVotosBlancos(struct NodoMesa *mesas, int *contadorBlancos){
    if (mesas){
        ContarVotosBlancos(mesas->izq, contadorBlancos);
        (*contadorBlancos)+= mesas->votosBlancos;
        ContarVotosBlancos(mesas->der, contadorBlancos);
    }
}

void ContarXcandidato(struct NodoMesa *mesas, int idCandidato, int *cantidadXcandidato){
    if (mesas){
        ContarXcandidato(mesas->izq, idCandidato, cantidadXcandidato);

        (*cantidadXcandidato) += mesas->votosCandidatos[idCandidato];
        
        ContarXcandidato(mesas->der, idCandidato, cantidadXcandidato);
    }
}


int ValidarSegundaVuelta(struct Resultado *resultados, struct Eleccion *eleccionActual){   //valida
    int i;
    for (i = 0; i < eleccionActual->nCands; i++){
        if(resultados->porcentajeCandidato[i] > 50.00){
            resultados->ganador = eleccionActual->cands[i];
            resultados->idxGanador = i;
            resultados->porcentajeGanador=resultados->porcentajeCandidato[i];
            return 0;
        } ;
    }

    return 1;
}



void mostrarResultados(struct Resultado *resultados, struct Eleccion *eleccion)
{
    if (eleccion == NULL) {
        printf("ERROR: No se encontró la elección asociada a este resultado.\n");
        return;
    }

    printf("ID de la eleccion = %d\n", eleccion->id);
    printf("Ronda = %s\n", eleccion->ronda == RONDA_PRIMERA ? "Primera vuelta" : "Segunda vuelta");

    printf("total de mesas = %d\n", resultados->totalMesas);
    printf("total de votantes registrados = %d\n", resultados->totalVotantesRegistrados);
    printf("total de votos = %d\n", resultados->totalVotantesVotaron);
    printf("total de votos nulos = %d\n", resultados->votosNulos);
    printf("total de votos blancos = %d\n", resultados->votosBlancos);
    printf("total de votos efectivos = %d\n\n", resultados->votosValidos);

    if (resultados->idxGanador != -1 && resultados->ganador != NULL)
    {
        printf("situacion de elecciones = Un candidato superó el 50%%, elecciones terminadas.\n\n");
        printf("GANADOR ELECCION\n");
        printf("ID DEL GANADOR = %d\n", resultados->ganador->id);
        printf("NOMBRE DEL GANADOR = %s\n", resultados->ganador->datos->nombre[0]);
        printf("PORCENTAJE DEL GANADOR = %.2f%%\n\n", resultados->porcentajeGanador);
    }
    else
    {
        printf("situacion de elecciones = Ningún candidato superó el 50%%.\n");
        printf("Se realizará una segunda vuelta.\n\n");
    }
}






void paraSegundaVuelta(struct Resultado *resultados, struct Eleccion *sistema, int *idX, int *idY){
    float por1 = -1.0f,por2 = -1.0f;
    int i;
    for (i = 0;i < sistema->nCands; i++){

        if(resultados->porcentajeCandidato[i] > por1){
            por2 = por1;
            *idY =*idX;
            
            
            
            por1 = resultados->porcentajeCandidato[i];
            *idX = i;
        }

        else if (resultados->porcentajeCandidato[i] > por2){
            por2 =  resultados->porcentajeCandidato[i];
            *idY = i;
        }
    }

}


struct Resultado* recopilarResultados(struct SistemaElectoral *sistema,struct Eleccion *eleccionActual){
    if (sistema->servel->totalVotantesRegistrados == 0) return NULL;

    int mesasTotales = 0, votosEmitidos = 0, TvotosBlancos = 0, TvotosNulos = 0, segunda_vuelta = 0, j;
    int i;

    int votosCandidato[MAX_CANDIDATOS] = {0};
    struct Resultado *final = malloc(sizeof(struct Resultado));
    final->ganador = NULL;
    final->idxGanador = -1;
    final->porcentajeGanador = 0.0f;
    final->sig = NULL;


    for (j = 0; j< MAX_CANDIDATOS; j++){
        final->porcentajeCandidato[j]=0.0f;
    }
    
    
    
    float participacion = 0.0f;

    ContarMesas(eleccionActual->arbolMesas, &mesasTotales);
    ContarVotosEmitidos(eleccionActual->arbolMesas, &votosEmitidos);
    ContarVotosNulos(eleccionActual->arbolMesas, &TvotosNulos);
    ContarVotosBlancos(eleccionActual->arbolMesas, &TvotosBlancos);


    eleccionActual->candidato_En_Resultado = final;
    final->totalMesas = mesasTotales;
    final->totalVotantesRegistrados = sistema->servel->totalVotantesRegistrados;
    final->totalVotantesVotaron = votosEmitidos;
    final->votosBlancos = TvotosBlancos;
    final->votosNulos = TvotosNulos;
    final->votosValidos = votosEmitidos - TvotosBlancos - TvotosNulos;

    
    if (final->totalVotantesRegistrados > 0){
        participacion =((float)votosEmitidos / (float)sistema->servel->totalVotantesRegistrados) * 100.0f;
        final->porcentajeParticipacion = participacion;
    }
    else{
        final->porcentajeParticipacion = 0.0;
    }


    
    for (i = 0; i < eleccionActual->nCands; i++) {

        ContarXcandidato(eleccionActual->arbolMesas, i, &votosCandidato[i]);

        if (votosEmitidos > 0) {
            final->porcentajeCandidato[i] =((float)votosCandidato[i] * 100.0f) / (float)final->votosValidos;
        }
        else {
            final->porcentajeCandidato[i] = 0.0f;
        }
    }

    segunda_vuelta = ValidarSegundaVuelta(final,eleccionActual);

    if (eleccionActual->ronda == RONDA_PRIMERA && segunda_vuelta == 1){
        printf("situacion de elecciones = No hay candidatos que superen el 50%% de votos, habrá una segunda vuelta.\n\n");
        
        int idX =-1, idY=-1, k;
        paraSegundaVuelta(final,eleccionActual,&idX, &idY);

        struct Eleccion *segunda = malloc(sizeof(struct Eleccion));
        memset(segunda, 0, sizeof(struct Eleccion)); //PARA INICIALIZAR TODO EN 0 o NULL;

        segunda->candidato_En_Resultado = NULL;
        segunda->sig = NULL; 
        segunda->nCands = 2;

        segunda->cands[0]=eleccionActual->cands[idX];
        segunda->cands[1]=eleccionActual->cands[idY];

        segunda->arbolMesas = eleccionActual->arbolMesas;
        segunda->id = eleccionActual->id + 1000;

        segunda->sig = sistema->servel->elecciones;
        sistema->servel->elecciones = segunda;

        printf("SEGUNDA VUELTA CREADA (ID = %d)\n", segunda->id);
        printf("Candidatos que pasan: %s y %s\n\n",segunda->cands[0]->datos->nombre[0],segunda->cands[1]->datos->nombre[0]);


        return final;

    }


    printf("HAY UN GANADOR, ELECCIONES TERMINADAS\n\n");
    return final;
}




void  agregarAtricel(struct Tricel * tricel, struct Resultado *resultadoNuevo){
    if (tricel->headResultados == NULL) {
        tricel->headResultados = resultadoNuevo;
        resultadoNuevo->sig = resultadoNuevo;
        return;
    }

    struct Resultado *aux = tricel->headResultados;

    do{
        aux=aux->sig;
    }while(aux->sig !=tricel->headResultados);

    aux->sig = resultadoNuevo;
    resultadoNuevo->sig = tricel->headResultados;
}






struct Eleccion * buscarEleccionPorResultado(struct Servel *servel, struct Resultado *r){
    struct Eleccion *aux = servel->elecciones;

    while (aux != NULL) {
        if (aux->candidato_En_Resultado == r) {
            return aux;
        }
        aux = aux->sig;
    }

    return NULL; 
}



void resultadoEleccionXid (struct Tricel *sistema, struct Servel *servel,int idBuscado){
    if (sistema->headResultados == NULL){
        printf("no hay resultados registrados en el sistema\n");
        return;
    }

    struct Resultado *cabeza = sistema->headResultados;
    struct Resultado *recorrido = sistema->headResultados;

    do {

        struct Eleccion *elec = buscarEleccionPorResultado(servel, recorrido);

        if (elec && elec->id == idBuscado){
            mostrarResultados(recorrido,elec);
            return;
        }
        recorrido = recorrido->sig;

    }while(recorrido!= cabeza);

    printf("No existen resultados referente a la id recibida (%d)\n",idBuscado);

}




void proclamarUnGanador (struct Tricel *tricel)
{
    if (tricel->headResultados == NULL){
        printf("NO HAY RESULTADOS REGISTRADOS\n\n");
        return;
    }

    struct Resultado *head = tricel->headResultados;
    struct Resultado *rec = head;

    do {
        rec = rec->sig;
    } while (rec->sig != head);

    printf("GANADOR ELECCION DE LAS ULTIMAS ELECCIONES REALIZADAS\n\n");
    printf("ID DEL GANADOR = %d\n", rec->ganador->id);
    printf("NOMBRE DEL GANADOR = %s\n", rec->ganador->datos->nombre[0]);
    printf("PORCENTAJE DEL GANADOR = %f\n\n", rec->porcentajeGanador);
}









void listarResultado (struct Tricel *tricel, struct Servel *servel){

    if (tricel->headResultados == NULL){
        printf("NO HAY RESULTADOS EN EL REGISTRO\n\n");
        return;
    }

    printf("LISTA DE RESULTADOS REGISTRADOS EN EL TRICEL\n\n");

    struct Resultado *rec = tricel->headResultados;
    
    do{

        struct Eleccion *elec = buscarEleccionPorResultado(servel, rec);

        mostrarResultados(rec,elec);
        rec=rec->sig;
    }while(rec!=tricel->headResultados);

    printf("\n\n");

    printf("resultados listados\n\n");


}


void generarVotosAleatorios(struct NodoMesa *m, int nCands) {   ///datos de votos aleatorios que use yo
    if (m == NULL) return;

    int total = 0;

    for(int i=0; i<nCands; i++){
        m->votosCandidatos[i] = rand() % 50;
        total += m->votosCandidatos[i];
    }

    m->votosBlancos = rand() % 5;
    m->votosNulos   = rand() % 3;

    m->totalVotosEmitidos = total + m->votosBlancos + m->votosNulos;

    generarVotosAleatorios(m->izq, nCands);
    generarVotosAleatorios(m->der, nCands);
}







void inicializarSistemaConDatos(struct SistemaElectoral *sistema)    ///estos fueron los datos de prueba mio, borralos
{
    // ----------------------
    // 1. Crear 3 personas
    // ----------------------
    struct Persona *p1 = malloc(sizeof(struct Persona));
    struct Persona *p2 = malloc(sizeof(struct Persona));
    struct Persona *p3 = malloc(sizeof(struct Persona));

    p1->nombre[0] = strdup("Juan Perez");
    p2->nombre[0] = strdup("Maria Gomez");
    p3->nombre[0] = strdup("Carlos Diaz");

    p1->edad = 45;
    p2->edad = 50;
    p3->edad = 39;

    p1->nacionalidad[0] = "chilena";
    p2->nacionalidad[0] = "chilena";
    p3->nacionalidad[0] = "chilena";

    // ----------------------
    // 2. Crear candidatos
    // ----------------------
    struct Candidato *c1 = malloc(sizeof(struct Candidato));
    struct Candidato *c2 = malloc(sizeof(struct Candidato));
    struct Candidato *c3 = malloc(sizeof(struct Candidato));

    c1->datos = p1;
    c2->datos = p2;
    c3->datos = p3;

    c1->id = 0;
    c2->id = 1;
    c3->id = 2;

    c1->esValido = 1;
    c2->esValido = 1;
    c3->esValido = 1;

    sistema->servel->candidatos[0] = c1;
    sistema->servel->candidatos[1] = c2;
    sistema->servel->candidatos[2] = c3;
    sistema->servel->totalCandidatos = 3;

    // ----------------------
    // 3. Crear elección
    // ----------------------
    struct Eleccion *elec = malloc(sizeof(struct Eleccion));

    elec->id = 100;
    elec->nCands = 3;
    elec->estado = ELEC_ABIERTA;
    elec->ronda = RONDA_PRIMERA;

    elec->cands[0] = c1;
    elec->cands[1] = c2;
    elec->cands[2] = c3;

    elec->sig = NULL;

    sistema->servel->elecciones = elec;

    // ----------------------
    // 4. Crear 2 mesas con votos
    // ----------------------
    struct NodoMesa *m1 = malloc(sizeof(struct NodoMesa));
    struct NodoMesa *m2 = malloc(sizeof(struct NodoMesa));

    m1->idMesa = 1;
    m2->idMesa = 2;

    m1->votosCandidatos[0] = 10;
    m1->votosCandidatos[1] = 5;
    m1->votosCandidatos[2] = 2;

    m2->votosCandidatos[0] = 7;
    m2->votosCandidatos[1] = 12;
    m2->votosCandidatos[2] = 4;

    m1->votosBlancos = 1; m2->votosBlancos = 1;
    m1->votosNulos = 0;   m2->votosNulos = 0;

    m1->totalVotosEmitidos = 18;
    m2->totalVotosEmitidos = 23;

    // ABB de mesas
    m1->izq = NULL; m1->der = m2;
    m2->izq = NULL; m2->der = NULL;

    elec->arbolMesas = m1;

    // ----------------------
    // 5. Registrar votantes
    // ----------------------
    sistema->servel->totalVotantesRegistrados = 300;
}











void MenuTricel( struct Tricel *tricel, struct SistemaElectoral *sistema)
{
    int indice = - 1;

    do
    {
        printf("\n-- TRICEL --\n");
        printf("1) Generar resultados desde Eleccion\n"); // -> (Esto acumula ABB de mesas)
        printf("2) Listar Resultados\n" );
        printf("3) Ver resultado por Id de ELECCION\n");
        printf("4) Proclamar Ganador\n");
        if (scanf("%d", &indice) != 1)
        {
            indice = - 1;
            continue;

        }
        switch (indice)
        {
            case 1:
            {
                int idB;
                printf("Ingrese id de la eleccion a buscar\n\n");
                scanf("%d",&idB);

                struct Eleccion *aux =sistema->servel->elecciones;

                while (aux != NULL && aux->id != idB){
                    aux = aux->sig;
                }

                if (aux == NULL){
                    printf("No existe una elección con ese ID.\n");
                    break;
                }

                generarVotosAleatorios(aux->arbolMesas, aux->nCands); ///esto lo puedes borrar, es para votos aleatorios

                struct Resultado *resultado = recopilarResultados(sistema, aux);

                agregarAtricel(tricel, resultado);


                printf("Generar Resultado, aun no \n");
                break;
            }
            case 2:
            {

                listarResultado(tricel, sistema->servel);
                printf("Listar Resultados, aun no  \n");
                break;

            }
            case 3:
            {
                int idB;
                printf("Ver resultado por id\n");
                printf("Ingrese el ID de la eleccion que desee buscar\n\n");
                scanf("%d",&idB);
                resultadoEleccionXid(tricel, sistema->servel, idB);
                break;

            }
            case 4:
            {
                proclamarUnGanador(tricel);
                printf("Ganador proclamado\n");
                break;

            }
            case 0:
            {
                printf("BREAK");
                break;

            }
        }


    }while (indice != 0);

}


//---------------------------------------------------------------------------------------------------

int main(void)
{
    srand(time(NULL));///lo borras si ya tienes los votos, si usas aleatorio entonces no
    struct SistemaElectoral *sistema = malloc(sizeof(struct SistemaElectoral));
    sistema->servel = malloc(sizeof(struct Servel));
    sistema->tricel = malloc(sizeof(struct Tricel));

    sistema->tricel->headResultados = NULL;
    sistema->tricel->totalResultados = 0;

    inicializarSistemaConDatos(sistema);

    MenuTricel(sistema->tricel, sistema);

    return 0;
}
