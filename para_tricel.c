
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
    int VotosXcandidato[MAX_CANDIDATOS];
    float porcentajeCandidato[MAX_CANDIDATOS];   /* solo 0..(nCands-1) */
    int segundaVuelta;                            /* 1 es que hay segunda vuelta y 0 es que no hay */
    int   idxGanador;                            /* índice dentro del arreglo compacto */
    float porcentajeGanador;

    struct Resultado *sig;   /* anillo: lista circular simplemente enlazada */
};





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

int ValidarSegundaVuelta (struct Resultado *resultados, struct Eleccion *eleccionActual){
    int i;
    for (i = 0; i < eleccionActual->nCands; i++){
        if(resultados->porcentajeCandidato[i] > 50.00){
            resultados->ganador = eleccionActual->cands[i];
            resultados->idxGanador = i;
            resultados->porcentajeGanador=resultados->porcentajeCandidato[i];
            return 1;
        } ;
    }

    return 0;
}




struct Resultado* recopilarResultados(struct SistemaElectoral *sistema,struct Eleccion *eleccionActual){
    if (sistema->servel->totalVotantesRegistrados == 0) return NULL;

    int mesasTotales = 0, votosEmitidos = 0, TvotosBlancos = 0, TvotosNulos = 0;
    int i;

    int votosCandidato[MAX_CANDIDATOS] = {0};
    struct Resultado *final = malloc(sizeof(struct Resultado));
    float participacion = 0.0f;


    ContarMesas(eleccionActual->arbolMesas, &mesasTotales);
    ContarVotosEmitidos(eleccionActual->arbolMesas, &votosEmitidos);
    ContarVotosNulos(eleccionActual->arbolMesas, &TvotosNulos);
    ContarVotosBlancos(eleccionActual->arbolMesas, &TvotosBlancos);


    final->eleccion = eleccionActual;
    final->totalMesas = mesasTotales;
    final->totalVotantesRegistrados = sistema->servel->totalVotantesRegistrados;
    final->totalVotantesVotaron = votosEmitidos;
    final->votosBlancos = TvotosBlancos;
    final->votosNulos = TvotosNulos;
    final->votosValidos = votosEmitidos - TvotosBlancos - TvotosNulos;

    
    participacion =((float)votosEmitidos / (float)sistema->servel->totalVotantesRegistrados) * 100.0f;

    final->porcentajeParticipacion = participacion;

    
    for (i = 0; i < eleccionActual->nCands; i++) {

        ContarXcandidato(eleccionActual->arbolMesas, i, &votosCandidato[i]);
        final->VotosXcandidato[i] = votosCandidato[i];

        if (votosEmitidos > 0) {
            final->porcentajeCandidato[i] =((float)votosCandidato[i] * 100.0f) / (float)final->votosValidos;
        }
        else {
            final->porcentajeCandidato[i] = 0.0f;
        }
    }

    final->segundaVuelta = ValidarSegundaVuelta(final,eleccionActual);


    return final;
}



void mostrarResultados (struct Resultado *resultados){
    int i;
    printf("total de mesas = %d\n",resultados->totalMesas);
    printf("total de votantes registrados = %d\n",resultados->totalVotantesRegistrados);
    printf("total de votos = %d\n",resultados->totalVotantesVotaron);
    printf("total de votos nulos = %d\n",resultados->votosNulos);
    printf("total de votos blancos = %d\n",resultados->votosBlancos);
    printf("total de votos efectivos = %d\n",resultados->votosValidos);
    printf("situacion de elecciones = ");
    if (resultados->segundaVuelta == 1){
        printf("Un candidato superó el 50%%, elecciones terminadas.\n\n");
    }
    if (resultados->segundaVuelta == 0) {
        printf("no hay candidatos que superen el 50%% de votos, habra una segunda vuelta\n");
    }
    
}

void  agregarAtricel(struct Tricel * tricel, struct Resultado *resultadoNuevo){
    if (tricel->headResultados == NULL) {
        tricel->headResultados = resultadoNuevo;
        resultadoNuevo->sig = resultadoNuevo;
        return;
    }
}


void resultadoEleccionXid (struct Tricel *sistema, int idBuscado){
    if (sistema->headResultados == NULL){
        printf("no hay resultados registrados en el sistema\n");
        return;
    }

    struct Resultado *cabeza = sistema->headResultados;
    struct Resultado *recorrido = sistema->headResultados;

    do {

        if (recorrido->eleccion->id == idBuscado){
            mostrarResultados(recorrido);
            return;
        }
        recorrido = recorrido->sig;

    }while(recorrido!= cabeza);

    printf("No existen resultados referente a la id recibida (%d)\n",idBuscado);

}

void proclamarUnGanador (struct)
