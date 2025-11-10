#Aqui hice los menus de cada parte , falta hacer las funciones principales
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
            case 1:
                printf("Aun no\n");
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
