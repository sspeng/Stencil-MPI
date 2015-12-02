/**
 * Computaci�nn Paralela
 *
 *
 * Stencil - Parallel version
 */

// Muestra un display con la matrix (para matrices pequeñas)
// Compilar: make display
//define SHOW_DISPLAY

// Bibliotecas estándard
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
 
// Biblioteca de funciones para computacion paralela
#include <cputils.h>

// Constantes de los pesos del stencil
#define WEIGHT_CENTER   (0.5   * (1.0/1.0) )
#define WEIGHT_ADJACENT (0.295 * (1.0/4.0) )
#define WEIGHT_CORNER   (0.195 * (1.0/4.0) )
#define MAX_RESID 0.001

/** Variable para el cálculo de residuos.  */
m_type resid;

/** Constantes por la identifacacion de los mensajes col/row */
#define COL_ID 1
#define ROW_ID 2

/**
 * Lee la sub matrix contenido en el fichero filename en foncion del rank del
 * proceso.
 *
 * @param filename The name of the file.
 * @param matrix Returns el pointer de la matrix.
 * @param border 0/1 la matrix tiene una bordura.
 * @param rank indica el identificador del proceso.
 * @note You must free the matrix pointer.
 *
 */
void cp_read_sub_matrix(char * filename, m_type * matrix, int border, int rank);

/**
 * Genera la siguiente iteración
 * @param matrix_old El estado anterior.
 * @param matrix_new El nuevo estado.
 * @param rows El número de filas.
 * @param cols El número de columnas.
 * @param rank El identificador del proceso
 * @param size Numero de procesos
 */ 
void update(m_type * matrix_old, m_type * matrix_new, int rows, int cols, 
			int rank, int size);


/**
 * Suma los valores de matrix para verificar la solución
 * @param matrix La matrix
 * @param rows El número de filas.
 * @param cols El número de columnas.
 */
m_type check_sum(m_type * matrix, int rows, int cols);

/**
 * Función main
 */
int main(int argc, char *argv[]){

	//************************************************************
	// 0. Variables
	char * matrix_name; /**< Nombre de la matrix de entrada */
	int rows;           /**< Número de filas de la matriz */
	int cols;           /**< Número de columnas */
	m_type * matrix1;   /**< Matrix con los datos */
	m_type * matrix2;   /**< Copia de la matrix para calcular nuevos valores */
	double t_begin;     /**< Tiempo de inicio del calculo */
	double t_end;       /**< Tiempo de finalización */
	m_type sum;         /**< Suma de verificación */
	int iter = 0;		/**< Iteraciones realizadas hasta llegar a un punto estable */
	int rank; /**< Identificador del proceso */
	int size; /**< Tamano del grupo */
	int temporal = 0;
	int indice = 0;
	int i1,j1,k1;
	m_type * buf_col_x;
	// Macros para acceder a las matrices
#define m1(i,j) (matrix1[(i)*(cols+2)+(j)])
#define m2(i,j) (matrix2[(i)*(cols+2)+(j)])
	//************************************************************
	
	//************************************************************
	// 1. Leer los parametros de entrada.
	if(argc != 2){
		fprintf(stderr,"USO: %s <matriz de datos>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	matrix_name = argv[1];
	//************************************************************
	//************************************************************
	// 0. Inicialisacion del entorno MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	//************************************************************

	
	//************************************************************
	// 2. Cargar la matriz 	
	// Matrix de tamaño rows+2 x cols+2 (halo de tamaño 1)
	// (Los datos están en [1,rows+1][1,cols+1])
	cp_read_matrix_size(matrix_name, &rows, &cols);
	printf(" Matriz %s: %dx%d\n",matrix_name, rows, cols);
	int matrix_size = (rows + 2) 	* (cols + 2);
	matrix1 = malloc(sizeof(m_type) * (size_t) matrix_size);
	cp_read_matrix(matrix_name, matrix1, 1);
	//************************************************************

	buf_col_x = malloc(sizeof(m_type) * (size_t) (rows+2));	

		for (i1 = 1;i1 <= size;i1++){				
		indice = i1*(cols)/size;
			for (k1=temporal;k1<indice;k1++)
			{	

				for (j1 = 0; j1 < cols; j1++) {
					buf_col_x[j1] = m1(k1,j1);
				}

			 	MPI_Request request_col_x;
			 	//data_buf = calloc((size_t) (rows+2), sizeof(float));


				// Envia al proceso col_X
				MPI_Isend(buf_col_x, rows, MPI_FLOAT, i1, COL_ID,
			  		MPI_COMM_WORLD, &request_col_x);

				//2.3 Recibircepcion de los datos
				MPI_Recv(buf_col_x, rows, MPI_FLOAT, i1,
					COL_ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				MPI_Wait(&request_col_x, MPI_STATUS_IGNORE);
				
				temporal = indice;
			}		
			printf("hola mundo\n");
	}
	//************************************************************
	// 3. Copia de la matriz
	matrix2 = malloc(sizeof(m_type) * (size_t) matrix_size);
	//************************************************************

#ifdef SHOW_DISPLAY
	cp_display_create("Stencil", rows+2, cols+2);
	cp_display_draw_matrix(matrix1,CP_RGB(255,0,0),CP_RGB(0,255,0));
	cp_msleep(1000);
#endif

	//************************************************************
	// 4. Bucle principal
	t_begin = cp_Wtime(); 
	int i,j;
	
	// 4.1 Nos mantenemos en el bucle mientras el residuo calculado 
	//  sea mayor que el residuo objetivo
	do {
		
		resid = 0.0;

		// Iteración del algoritmo
		update(matrix1,matrix2,rows,cols, rank, size);

		// Recibir los datos de los otros procesos en una matricia
		// Actualizar la copia
			for (i=1; i<rows+1; i++) {
				for (j=1; j<cols+1; j++) {
					m1(i,j) = m2 (i,j);
				}
			}



#ifdef SHOW_DISPLAY
		cp_display_draw_matrix(matrix2,CP_RGB(255,0,0),CP_RGB(0,255,0));
		cp_msleep(50); // Para ver más despacio la evolucion del proceso
#endif

	iter++;

	} while (resid > MAX_RESID);
	//************************************************************
	
	
	//************************************************************
	// 5. Suma de verificación
	sum = check_sum(matrix1,rows,cols);
	t_end = cp_Wtime();
	//************************************************************
	
	
	//************************************************************
	// 6. Mostrar resultados
	printf(" Check sum: %f\n", sum);
	printf(" Iteraciones: %d\n", iter);
	printf(" Tiempo de ejecución: %f\n", t_end-t_begin);
	//************************************************************

#ifdef SHOW_DISPLAY	
	cp_msleep(250);
	cp_display_close();
#endif


	//************************************************************
	// 7. Liberar las matrices
	free(matrix1);
	free(matrix2);
	//************************************************************

#undef m1
#undef m2

	//************************************************************
	// 8. Finalizacion del entorno MPI
	MPI_Finalize();
	//************************************************************
	return EXIT_SUCCESS;
}

void update(m_type * matrix_old, m_type * matrix_new, int rows, int cols,
			 int rank, int size){

	//************************************************************
	// 1. Macros para acceder a las matrices
	#define m_old(i,j) (matrix_old[(i)*(cols+2)+(j)])
	#define m_new(i,j) (matrix_new[(i)*(cols+2)+(j)])
	//************************************************************

	//************************************************************
	int i, j;	
	
		// 2'. Envia de los bordes
		// 2.0 Inicialisacion de los buffers
		m_type * buf_col_b = malloc( sizeof(m_type) *(size_t) (rows+2));
		m_type * buf_col_e = malloc(sizeof(m_type) * (size_t) (rows+2));	
		for (i = 0; i < rows; i++) {
			buf_col_b[i] = m_old(i, 0);
			buf_col_e[i] = m_old(i, cols - 1);
		}
		MPI_Request request_col_b;
		MPI_Request request_col_e;
		// 2.1 Envia al proceso antecedente
		MPI_Isend(buf_col_b, rows, MPI_INT, ((rank - 1) % size), COL_ID,
				  MPI_COMM_WORLD, &request_col_b);
		//2.2 Envia al proceso despues 
		MPI_Isend(buf_col_e, rows, MPI_INT, ((rank + 1) % size),  COL_ID,
				  MPI_COMM_WORLD, &request_col_e);
		//2.3 Recepcion de los datos
		MPI_Recv(buf_col_b, rows, MPI_INT, ((rank - 1) % size),
				COL_ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(buf_col_e, rows, MPI_INT, ((rank + 1) % size),
				COL_ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				
		MPI_Wait(&request_col_b, MPI_STATUS_IGNORE);
		MPI_Wait(&request_col_e, MPI_STATUS_IGNORE);
		
	//************************************************************
	
	//************************************************************
	// 3. Bucle principal
	for(i=0; i<rows; i++){
		for(j=0; j<cols; j++){

			// 3.1 El nuevo valor es el anterior más la acción de los vecinos.	
			if (j == 0) {
				m_new (i,j) = ( m_old (i, j) * WEIGHT_CENTER) +
						 
						((m_old (i-1 % rows, j) +
							m_old (i+1 % rows, j) +
							buf_col_b[i] +
							m_old (i, j+1) ) * WEIGHT_ADJACENT) +
						 
						(buf_col_b[(i-1) % rows] +
							m_old (i-1 % rows, j+1) +
							buf_col_b[(i+1) % rows] +
							m_old (i+1 % rows, j+1) * WEIGHT_CORNER);
			
			} else {
				if ( j == (cols - 1)) {
						m_new (i,j) = ( m_old (i, j) * WEIGHT_CENTER) +
						 
						((m_old (i-1, j) +
							m_old (i+1, j) +
							m_old (i, j-1) +
							buf_col_e[i]) * WEIGHT_ADJACENT) +
						 
						((m_old (i-1, j-1) +
							buf_col_e[i-1] +
							m_old (i+1, j-1) +
							buf_col_e[i+1] ) * WEIGHT_CORNER);
				} else {
				
					m_new (i,j) = ( m_old (i, j) * WEIGHT_CENTER) +
						 
						((m_old (i-1, j) +
							m_old (i+1, j) +
							m_old (i, j-1) +
							m_old (i, j+1) ) * WEIGHT_ADJACENT) +
						 
						((m_old (i-1, j-1) +
							m_old (i-1, j+1) +
							m_old (i+1, j-1) +
							m_old (i+1, j+1) ) * WEIGHT_CORNER);
				
				}
			}
			
			// 3.2 Guardamos la mayor diferencia entre el valor antiguo y el actualizado
			m_type tmpResid = fabs (m_old(i, j) - m_new(i, j));
			if ( tmpResid > resid ) resid = tmpResid;
				
		}
		
	}
	//************************************************************

	#undef m_old
	#undef m_new
}


m_type check_sum(m_type * matrix, int rows, int cols){

	//************************************************************
	// 1. Macros para acceder a la matriz
	#define m(i,j) (matrix[(i)*(cols+2)+(j)])
	//************************************************************

	//************************************************************
	// 2. Bucle con la suma
	m_type sum = 0;
	int i,j;
	for(i=1; i<rows+1; i++)
		for(j=1; j<cols+1; j++)
			sum += m(i,j);
	//************************************************************

	return sum;

	#undef m
}




