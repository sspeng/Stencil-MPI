/**
 * Computación Paralela
 * Funciones de ayuda
 * 
 *
 */

#ifndef _CPUTILS_
#define _CPUTILS_



/* Macros for the matrix type */
#define CP_FLOAT 0
#define CP_DOUBLE 1

// You can change this macro to select floats or doubles
#ifndef CP_MATRIX_TYPE
#define CP_MATRIX_TYPE CP_FLOAT
// #define CP_MATRIX_TYPE CP_DOUBLE
#endif

#if MATRIX_TYPE == CP_FLOAT
typedef float m_type;
#elif  MATRIX_TYPE == CP_DOUBLE
typedef double m_type;
#else
#error "No Matrix Type Selected"
#endif







/* Min and Max */
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#define max(X,Y) ((X) > (Y) ? (X) : (Y))


/**
 * Transforms a integer array to string for printing.
 */
char * cp_array2string(int * array, int num);

/**
 * Transforms a integer integer to string for printing.
 */
char * cp_matrix2string(int * matrix, int rows, int cols);

/**
 * Time when there is no MPI.
 */
double cp_Wtime();

/**
 * Sleep function
 */
void cp_msleep(int msec);

/**
 * Get random number in an interval.
 */
int cp_rand(int num_min, int num_max);

/**
 * Reads a matrix from a plain text file.
 * @param filename The name of the file.
 * @param matrix Returns a pointer to the matrix.
 * @param border 0/1 The allocated matrix has a border.
 * @note You must free the matrix pointer.
 */
void cp_read_matrix(char * filename, m_type * matrix, int border);

/**
 * @param rows Returns the number of rows.
 * @param cols Returns the number of cols.
 */
void cp_read_matrix_size(char * filename, int * rows, int * cols);






/* Macros to transform RGB colors */
#define CP_RGB(r,g,b)  ((r << 16) | (g << 8) | (b << 0))
#define CP_GETR(color) ((color & 0x00FF0000) >> 16)
#define CP_GETG(color) ((color & 0x0000FF00) >>  8)
#define CP_GETB(color) ((color & 0x000000FF) >>  0)


/**
 * Creates a new X11 display - Auto zoom
 * @param name The window name
 * @param rows
 * @param cols
 */
void cp_display_create(char * name, int rows, int cols);

/**
 * Creates a new X11 display
 * @param name The window name
 * @param rows
 * @param cols
 * @param zoom (to see the pixels)
 */
void cp_display_create_zoom(char * name, int rows, int cols, int zoom);

/**
 * Draws a m_type matrix with values within [-1,1] using the colors
 * @param matrix Pointer to the matrix
 * @param color1 The color for negative numbers
 * @param color2 The color for positive numbers
 */
void cp_display_draw_matrix(m_type * matrix, int color1, int color2);

/**
 * Draw a grid
 * @param color The color of the grid lines
 */
void cp_display_draw_grid(int color);

/**
 * Close the display
 */
void cp_display_close();





#endif
