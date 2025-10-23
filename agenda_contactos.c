#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ARCHIVO_CSV "contactos.csv"
#define MAX_CADENA 200
#define LINEA_MAX 600

//  Funciones auxiliares.
void quitar_nueva_linea(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n == 0) return;
    if (s[n - 1] == '\n' || s[n - 1] == '\r') s[n - 1] = '\0';
}

void leer_cadena(const char *mensaje, char *buffer, size_t tam) {
    printf("%s", mensaje);
    fflush(stdout);
    if (fgets(buffer, tam, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    quitar_nueva_linea(buffer);
}

void asegurar_encabezado(void) {
    FILE *f = fopen(ARCHIVO_CSV, "r");
    if (f == NULL) {
        f = fopen(ARCHIVO_CSV, "w");
        if (f) {
            fprintf(f, "\"nombre\",\"telefono\",\"email\"\n");
            fclose(f);
        } else {
            printf("Error al crear el archivo CSV.\n");
        }
    } else fclose(f);
}

void entrecomillar(char *dest, const char *src, size_t tam) {
    snprintf(dest, tam, "\"%s\"", src);
}
// Añadir contacto.
void anadir_contacto(void) {
    char nombre[MAX_CADENA], telefono[MAX_CADENA], email[MAX_CADENA];
    char qnombre[MAX_CADENA * 2], qtelefono[MAX_CADENA * 2], qemail[MAX_CADENA * 2];

    printf("\nAñadir contacto\n");
    leer_cadena("Nombre: ", nombre, sizeof(nombre));
    leer_cadena("Teléfono: ", telefono, sizeof(telefono));
    leer_cadena("Email: ", email, sizeof(email));

    if (strlen(nombre) == 0) {
        printf("Error: el nombre no puede estar vacío.\n");
        return;
    }

    entrecomillar(qnombre, nombre, sizeof(qnombre));
    entrecomillar(qtelefono, telefono, sizeof(qtelefono));
    entrecomillar(qemail, email, sizeof(qemail));

    FILE *f = fopen(ARCHIVO_CSV, "a");
    if (!f) {
        printf("Error al abrir el archivo CSV.\n");
        return;
    }
    fprintf(f, "%s,%s,%s\n", qnombre, qtelefono, qemail);
    fclose(f);
    printf("Contacto agregado correctamente.\n");
}

// Listar contactos.
void listar_contactos(void) {
    FILE *f = fopen(ARCHIVO_CSV, "r");
    if (!f) {
        printf("No hay contactos registrados.\n");
        return;
    }

    printf("\nLista de contactos\n");
    char linea[LINEA_MAX];
    int linea_num = 0;
    while (fgets(linea, sizeof(linea), f)) {
        if (linea_num == 0) { linea_num++; continue; } // Saltar encabezado.
        quitar_nueva_linea(linea);
        if (strlen(linea) == 0) continue;

        char nombre[MAX_CADENA], telefono[MAX_CADENA], email[MAX_CADENA];
        nombre[0] = telefono[0] = email[0] = '\0';

        sscanf(linea, "\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", nombre, telefono, email);
        printf("%d) %s - %s - %s\n", linea_num, nombre, telefono, email);
        linea_num++;
    }
    fclose(f);
    if (linea_num == 1) printf("No hay contactos.\n");
}

// Buscar contacto.
int contiene(const char *texto, const char *busqueda) {
    if (!texto || !busqueda) return 0;
    char t1[MAX_CADENA], t2[MAX_CADENA];
    strncpy(t1, texto, sizeof(t1)); t1[sizeof(t1)-1] = 0;
    strncpy(t2, busqueda, sizeof(t2)); t2[sizeof(t2)-1] = 0;
    for (char *p = t1; *p; ++p) *p = tolower((unsigned char)*p);
    for (char *p = t2; *p; ++p) *p = tolower((unsigned char)*p);
    return strstr(t1, t2) != NULL;
}

void buscar_contacto(void) {
    FILE *f = fopen(ARCHIVO_CSV, "r");
    if (!f) {
        printf("No hay contactos registrados.\n");
        return;
    }

    char busqueda[MAX_CADENA];
    leer_cadena("\nIngrese el nombre a buscar: ", busqueda, sizeof(busqueda));
    if (strlen(busqueda) == 0) {
        printf("Búsqueda vacía.\n");
        fclose(f);
        return;
    }

    char linea[LINEA_MAX];
    int linea_num = 0, encontrados = 0;
    printf("\nResultados de búsqueda\n");
    while (fgets(linea, sizeof(linea), f)) {
        if (linea_num == 0) { linea_num++; continue; }
        quitar_nueva_linea(linea);
        if (strlen(linea) == 0) continue;

        char nombre[MAX_CADENA], telefono[MAX_CADENA], email[MAX_CADENA];
        sscanf(linea, "\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", nombre, telefono, email);
        if (contiene(nombre, busqueda)) {
            printf("%d) %s - %s - %s\n", linea_num, nombre, telefono, email);
            encontrados++;
        }
        linea_num++;
    }
    fclose(f);
    if (!encontrados) printf("No se encontraron coincidencias.\n");
}

// Eliminar contacto.
void eliminar_contacto(void) {
    listar_contactos();
    int linea_eliminar;
    printf("\nIngrese el número del contacto a eliminar (0 para cancelar): ");
    if (scanf("%d", &linea_eliminar) != 1) {
        while (getchar() != '\n'); // Limpia el buffer.
        printf("Entrada inválida.\n");
        return;
    }
    while (getchar() != '\n'); // Limpiar. '\n'

    if (linea_eliminar <= 0) {
        printf("Operación cancelada.\n");
        return;
    }

    FILE *f = fopen(ARCHIVO_CSV, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!f || !temp) {
        printf("Error al abrir los archivos.\n");
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    char linea[LINEA_MAX];
    int linea_num = 0;
    while (fgets(linea, sizeof(linea), f)) {
        if (linea_num != linea_eliminar) fputs(linea, temp);
        linea_num++;
    }

    fclose(f);
    fclose(temp);
    remove(ARCHIVO_CSV);
    rename("temp.csv", ARCHIVO_CSV);
    printf("Contacto eliminado correctamente.\n");
}

// Editar contacto.
void editar_contacto(void) {
    listar_contactos();
    int linea_editar;
    printf("\nIngrese el número del contacto a editar (0 para cancelar): ");
    if (scanf("%d", &linea_editar) != 1) {
        while (getchar() != '\n');
        printf("Entrada inválida.\n");
        return;
    }
    while (getchar() != '\n');

    if (linea_editar <= 0) {
        printf("Operación cancelada.\n");
        return;
    }

    FILE *f = fopen(ARCHIVO_CSV, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!f || !temp) {
        printf("Error al abrir los archivos.\n");
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    char linea[LINEA_MAX];
    int linea_num = 0;
    while (fgets(linea, sizeof(linea), f)) {
        if (linea_num == linea_editar) {
            char nombre[MAX_CADENA], telefono[MAX_CADENA], email[MAX_CADENA];
            printf("\nEditar contacto\n");
            leer_cadena("Nuevo nombre: ", nombre, sizeof(nombre));
            leer_cadena("Nuevo teléfono: ", telefono, sizeof(telefono));
            leer_cadena("Nuevo email: ", email, sizeof(email));
            fprintf(temp, "\"%s\",\"%s\",\"%s\"\n", nombre, telefono, email);
        } else {
            fputs(linea, temp);
        }
        linea_num++;
    }

    fclose(f);
    fclose(temp);
    remove(ARCHIVO_CSV);
    rename("temp.csv", ARCHIVO_CSV);
    printf("Contacto editado correctamente.\n");
}

// Menú principal.

int main(void) {
    asegurar_encabezado();
    char opcion[10];

    while (1) {
        printf("\nSISTEMA DE CONTACTOS\n");
        printf("1) Añadir contacto.\n");
        printf("2) Listar contactos.\n");
        printf("3) Buscar contacto.\n");
        printf("4) Editar contacto.\n");
        printf("5) Eliminar contacto.\n");
        printf("6) Salir.\n");
        leer_cadena("Seleccione una opción: ", opcion, sizeof(opcion));

        if (strcmp(opcion, "1") == 0) anadir_contacto();
        else if (strcmp(opcion, "2") == 0) listar_contactos();
        else if (strcmp(opcion, "3") == 0) buscar_contacto();
        else if (strcmp(opcion, "4") == 0) editar_contacto();
        else if (strcmp(opcion, "5") == 0) eliminar_contacto();
        else if (strcmp(opcion, "6") == 0) {
            printf("Saliendo del sistema. ¡Hasta luego!\n");
            break;
        } else {
            printf("Opción no válida.\n");
        }
    }

    return 0;
}