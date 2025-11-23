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

/// Funciones las cuales recorren un arbol binario de mesas usando la recursion, todas estas funciones tienen el mismo funcionamiento
/// el cual es siemopre recorrer el subarbol izquierdo, procesar ese nodo y luego se recorrerian los subarboles mas a la derecha
/// este tipo de recorrido se llama in order tiene como estructura izquierda->nodo->derecha

/// en todas estas funciones no se devuelven valores, el resultado se va acumulado en una variable con puntero la cual es recibida por la funcion    








int ValidarSegundaVuelta(struct Resultado *resultados, struct Eleccion *eleccionActual){   //valida si hay o no segunda vuelta
    int i;
    for (i = 0; i < eleccionActual->nCands; i++){
        if(resultados->porcentajeCandidato[i] > 50.00){     /// en caso de que haya un ganador se agregaran sus datos al struct del resultado de esa eleccion 
            resultados->ganador = eleccionActual->cands[i]; /// se guardan todos los datos del ganador;
            resultados->idxGanador = i; /// el id del ganador
            resultados->porcentajeGanador=resultados->porcentajeCandidato[i]; /// el porcentaje con el que gano
            return 0; /// retornara 0 ya que hay un ganador
        } ;
    }

    return 1; /// en caso de que no hayan candidatos que superen el 50% en votos se devolvera el valor 1 el cual significaria segunda vuelta
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











void paraSegundaVuelta(struct Resultado *resultados, struct Eleccion *sistema, int *idX, int *idY){   /// en caso de que haya segunda vuelta se usara esta funcion la cual recibe idX e idY como punteros
    float por1 = -1.0f,por2 = -1.0f; /// variables llamadas por1 y por2 los cuales guardan los 2 porcentajes de votos mas alto, se inician en -1.0 para que cualquier porcentaje real sea mayor
    int i;
    for (i = 0;i < sistema->nCands; i++){ /// se recorre todo el arreglo de los candidatos desde la posicion 0 hasta nCands

        if(resultados->porcentajeCandidato[i] > por1){ /// en caso de que el porcentaje de votos a un candidato sea mayor que 1...
            por2 = por1; /// el segundo lugar para a ser el primer lugar anterior
            *idY =*idX; /// ademas se actualizara la id del segundo lugar
            /// se hace esto para evitar que un candidato con menor porcentaje que el antiguo primer lugar quede para la 2da vuelta
            
            
            por1 = resultados->porcentajeCandidato[i]; /// se actualizara el porcentaje del primer lugar
            *idX = i; /// se actualizara el id del primer lugar
        }

        else if (resultados->porcentajeCandidato[i] > por2){ /// si el porcentaje no fue mayor al del primer lugar, ahora se compararia con el del segundo lugar
            por2 =  resultados->porcentajeCandidato[i]; /// se actualizan los porcentajes e ids del nuevo segundo lugar
            *idY = i;
        }
    }

}

/// un punto importante de esta funcion es que si o si se debe usar el else if y no doble if, porque un candidato no puede ser primer y segundo lugar al mismo tiempo

/// en este caso al usar el if y else if, un candidato que tenga un porcentaje mayor a la variable por1 solo entraria en la primera situacion y no entraria en el segundo
/// si no supera el primer if pero si entra en el else if asi que se podria convertir en 2do lugar

/// el riesgo de usar doble if es que si un candidato supera el primer lugar, automaticamente seria mayor que el segundo lugar y de ese modo entraria al segundo if

/// en resumen se usa else if para evitar que se devuelvan datos "duplicados", en este caso que se devuelvan 2 IDs iguales 

















/// solo se le pasa con puntero simple porque no se modifican campos dentro de las estructuras en si
struct Resultado* recopilarResultados(struct SistemaElectoral *sistema,struct Eleccion *eleccionActual){ /// funcion que recopula los datos de una eleccion
    if (sistema->servel->totalVotantesRegistrados == 0) return NULL; /// hay que aseguar que hayan votos en esa eleccion

    int mesasTotales = 0, votosEmitidos = 0, TvotosBlancos = 0, TvotosNulos = 0, segunda_vuelta = 0, j;
    int i;

    int votosCandidato[MAX_CANDIDATOS] = {0}; /// arreglo para contar votos por candidato, cada posicion representa un candidato y se empezara desde la posicion 0
    struct Resultado *final = malloc(sizeof(struct Resultado)); /// se crea una estructura del struct "Resultado" la cual se llenara de informacion recopilada a lo largo de la eleccion, esta al terminar sera retornada al tricel
    final->ganador = NULL; /// aca se marca que aun no hay ganador
    final->idxGanador = -1; /// se inicia en -1 porque hay candidatos que tienen como ID le 0
    final->porcentajeGanador = 0.0f;/// no se obtiene todavia el porcentaje mayor
    final->sig = NULL; /// al usar lista circular no se apuntara a nadie por ser la ultima eleccion realizada
    ///se inicializan los campos para un inicio seguro y valido 


    for (j = 0; j< MAX_CANDIDATOS; j++){
        final->porcentajeCandidato[j]=0.0f;
    }
    /// se inician los porcentajes de los candidatos en 0% para evitar datos basura 
    
    
    
    float participacion = 0.0f;

    ContarMesas(eleccionActual->arbolMesas, &mesasTotales);
    ContarVotosEmitidos(eleccionActual->arbolMesas, &votosEmitidos);
    ContarVotosNulos(eleccionActual->arbolMesas, &TvotosNulos);
    ContarVotosBlancos(eleccionActual->arbolMesas, &TvotosBlancos);
    /// se cuentan los diferentes datos 


    eleccionActual->candidato_En_Resultado = final;
    final->totalMesas = mesasTotales;
    final->totalVotantesRegistrados = sistema->servel->totalVotantesRegistrados;
    final->totalVotantesVotaron = votosEmitidos;
    final->votosBlancos = TvotosBlancos;
    final->votosNulos = TvotosNulos;
    final->votosValidos = votosEmitidos - TvotosBlancos - TvotosNulos;
    /// se completan los campos con los datos obtenidos


    /// se calcula el porcentaje de participacion de las elecciones actuales
    if (final->totalVotantesRegistrados > 0){
        participacion =((float)votosEmitidos / (float)sistema->servel->totalVotantesRegistrados) * 100.0f;
        final->porcentajeParticipacion = participacion;
    }
    else{
        final->porcentajeParticipacion = 0.0;
    }

    /// es importante validar que el total de votos sea mayor a 0 ya que no se puede hacer divison de algo por 0


    ///  aca se cuentan y de paso de calculan los porcentajes de votos de cada candidato de la eleccion
    for (i = 0; i < eleccionActual->nCands; i++) {

        ContarXcandidato(eleccionActual->arbolMesas, i, &votosCandidato[i]);

        if (votosEmitidos > 0) {
            final->porcentajeCandidato[i] =((float)votosCandidato[i] * 100.0f) / (float)final->votosValidos;
        }
        else {
            final->porcentajeCandidato[i] = 0.0f;
        }
    }

    segunda_vuelta = ValidarSegundaVuelta(final,eleccionActual); /// SE METEN TODOS LOS DATOS DE LA ELECCION PA CORROBORAR SI HAY SEGUNDA VUELTA O NO

    if (eleccionActual->ronda == RONDA_PRIMERA && segunda_vuelta == 1){ /// EN CASO DE QUE HAYA SEGUND VUELTA
        printf("situacion de elecciones = No hay candidatos que superen el 50%% de votos, habrá una segunda vuelta.\n\n");
        
        int idX =-1, idY=-1, k;
        paraSegundaVuelta(final,eleccionActual,&idX, &idY); /// ACA SE BUSCAN LOS 2 CANDIDATOS CON MAYOR PORCENTAJE DE VOTOS

        struct Eleccion *segunda = malloc(sizeof(struct Eleccion));  /// SE CREA UNA ESTRUCTURA PARA LA 2DA VUELTA DE LA ELECCION
        memset(segunda, 0, sizeof(struct Eleccion)); // SE UTILIZA MEMSET PARA INICIALIZAR TODO EN 0 o NULL 

        segunda->candidato_En_Resultado = NULL;
        segunda->sig = NULL; 
        segunda->nCands = 2;
        /// SE INICIALIZAN LOS DATOS PARA LA SEGUNDA ELECCION Y PARA nCands


        
        segunda->cands[0]=eleccionActual->cands[idX];
        segunda->cands[1]=eleccionActual->cands[idY];
        /// aca solo se copian solo los IDs de los 2 candidatos con mayores votos en la primera vuelta

        segunda->arbolMesas = eleccionActual->arbolMesas; /// se reutilizaran las mismas mesas para la segunda vuelta
        segunda->id = eleccionActual->id + 1000; /// se le suma 1000 al id para diferenciarla de la 2da vuelta (ejemplo: la primera vuelta es 1050, la segunda seria 2050)

        segunda->sig = sistema->servel->elecciones; /// esta linea se traduce a que la siguiente de la nueva eleccion ser la que antes era la primera en la lista
        sistema->servel->elecciones = segunda; ///se mueve la cabeza de la lista para que apunte a la eleccion nueva, osea la segunda vuelta quedaria registrada como la mas reciente
        /// lineas que ayudan a insertar la eleccionde la 2da vuelta en el sistema, si se llegaran a omitir se podrian crear ciclos infinitos, romper la lista o incluso perder los datos de elecciones anteriores
        
        /// en caso de omitir la primera linea se perderian las elecciones antiguas
        
        /// si no se usa la segunda linea se apunta bien al inicio pero la cabecera nunca cambiaria, por esto no se insertaria nada y provocaria una fuga de memoria
        ///en otras palabras esa eleccion quedaria desconectada y no se podria usar nunca
        
        printf("SEGUNDA VUELTA CREADA (ID = %d)\n", segunda->id);
        printf("Candidatos que pasan: %s y %s\n\n",segunda->cands[0]->datos->nombre[0],segunda->cands[1]->datos->nombre[0]);


        return final; /// SE RETORNARA EL RESULTADO DE LA ELECCION AUNQUE NO HAYA HABIDO GANADOR

    }


    printf("HAY UN GANADOR, ELECCIONES TERMINADAS\n\n");
    return final;
    /// EN CASO DE QUE HAYA GANADOR SE RETORNARA EL RESULTADO FINAL CON EL GANADOR YA PROCLAMADO (YA SE AGREGO DESDE LA FUNCION VALIDAR SEGUNDA VUELTA
}













/// solo se le pasa con puntero simple porque no se modifican campos dentro de las estructuras en si
void  agregarAtricel(struct Tricel * tricel, struct Resultado *resultadoNuevo){ /// agrega los resultados de una eleccion al tricel
    if (tricel->headResultados == NULL) { /// en caso de que no hayan datos en la lista circular de resultados
        tricel->headResultados = resultadoNuevo; /// el nuevo nodo es la nueva cabecera de la lista circular
        resultadoNuevo->sig = resultadoNuevo; /// al ser lista circular se debe apuntar a si mismo, si no se hace esto se perderia el concepto de lista circular
        return;
    }
    /// si no se hace esto, el aux se iniciara en null y nunca empezaria a recorrer la lista, el aux->sig seria igual a SEGFAULT y se caeria el programa
    /// no se insertaria nunca el primer nodo, en el ciclo while siempre se asumira que hay un ultimo nodo pero en verdad no existe ninguno

    struct Resultado *aux = tricel->headResultados;

    do{
        aux=aux->sig;
    }while(aux->sig !=tricel->headResultados);
    /// se usa do while porque en la lista circular se debe recorrer al menos un nodo antes de evaluar la condicion 
    /// si se usara solo while el ciclo no se ejecutaria en ciertos casos, como ejemplo cuando el aux ya esta ultimo en la lista (siempre hay que entrar al menos una vez a la lista circular)
    
    aux->sig = resultadoNuevo; ///aca se conecta el ultimo nodo al nuevo
   /// si no se hace esto, el nuevo nodo no entraria nunca en la lista
    resultadoNuevo->sig = tricel->headResultados; /// aca se cierra "el ciclo circular"
    ///si no se hace esto la lista circular se perderia y se convertiria en una lista normal, la cual probablemente este rota y probablemente genere inestabilidad en el programa

    
}












struct Eleccion * buscarEleccionPorResultado(struct Servel *servel, struct Resultado *r){
    struct Eleccion *aux = servel->elecciones;

    while (aux != NULL) { /// no se usa do while en lista circular ya que al entrar a la lista normal de una podria estar vacia y podria hacer que salga el error de SEGFAUL el ejecutar aux == NULL
        if (aux->candidato_En_Resultado == r) {
            return aux;
        }
        aux = aux->sig;
    }

    return NULL; /// si no se encuentra la eleccion se retornara null
}



void resultadoEleccionXid (struct Tricel *sistema, struct Servel *servel,int idBuscado){ /// sirve para buscar un resultado especifico en base a una id
    if (sistema->headResultados == NULL){ /// se hace esto para evitar el segfault mas adelante
        printf("no hay resultados registrados en el sistema\n");
        return;
    }

    struct Resultado *cabeza = sistema->headResultados; /// apunta al primer nodo
    struct Resultado *recorrido = sistema->headResultados; /// este sirve para irse moviendo por la lista

    do {/// se usa do while porque en una lista circular nunca habra NULL, si hay un null entremedio significa que la lista esta rota o la estructura dejo de ser circular

        struct Eleccion *elec = buscarEleccionPorResultado(servel, recorrido); ///se busca el resultado de la eleccion

        if (elec && elec->id == idBuscado){
            mostrarResultados(recorrido,elec);
            return;/// se retornada de la funcion si ya se encontro la eleccion buscada
        }
        recorrido = recorrido->sig;

    }while(recorrido!= cabeza);
    /// se usa do while porque en la lista circular se debe recorrer al menos un nodo antes de evaluar la condicion 
    /// si se usara solo while el ciclo no se ejecutaria en ciertos casos, como ejemplo cuando el aux ya esta ultimo en la lista (siempre hay que entrar al menos una vez a la lista circular)

    printf("No existen resultados referente a la id recibida (%d)\n",idBuscado); /// caso de que haya recorrido la lista y no se haya encontrado el id buscado

}









/// esta funcion se tiene que cambiar al definitivo

void proclamarUnGanador (struct Tricel *tricel) /// funcion que proclama al ultimo ganador de las elecciones
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

    if (rec->porcentajeGanador < 50.0f){ /// en caso de que no se haya podido proclamaar un ganador en las ultimas elecciones
        printf("NO HUBO GANADOR EN PRIMERA VUELTA.\n");
        printf("SE DEBE REALIZAR UNA SEGUNDA VUELTA.\n\n");
        return;
    }

    printf("GANADOR ELECCION DE LAS ULTIMAS ELECCIONES REALIZADAS\n\n");
    printf("ID DEL GANADOR = %d\n", rec->ganador->id);
    printf("NOMBRE DEL GANADOR = %s\n", rec->ganador->datos->nombre[0]);
    printf("PORCENTAJE DEL GANADOR = %f\n\n", rec->porcentajeGanador);
}















///FUNCION QUE MUESTRA TODAS LAS ELECCIONES REGISTRADAS 

void listarResultado (struct Tricel *tricel, struct Servel *servel){

    if (tricel->headResultados == NULL){
        printf("NO HAY RESULTADOS EN EL REGISTRO\n\n"); /// EN CASO DE QUE NO HAYAN ELECCIONES EN LA LISTA SALDRA ESTE MENSAJE Y SE SALDRA DE LA FUNCION
        return;
    }

    printf("LISTA DE RESULTADOS REGISTRADOS EN EL TRICEL\n\n");

    struct Resultado *rec = tricel->headResultados; /// se guarda la referencia a la cabecera de la lista circular para poder realizar el ciclo 
    
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
        m->votosCandidatos[i] = rand() % 50;  ///Rodri no pesques esta funcion ni la de inicializar
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