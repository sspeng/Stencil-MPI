/**
 * Computación Paralela
 * Funciones de ayuda
 *
 *
 */

#include "cputils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef CP_NO_MPI
#include <mpi.h>
#endif

// Aux buffer
char print_buffer[1024];


char * cp_array2string(int * array, int num){

	int i;
	int nchars;
	char * buffer = print_buffer;

	nchars = sprintf(buffer,"[");
	
	buffer += nchars;
	for(i=0; i<num-1; i++){
		nchars = sprintf(buffer,"%2d,",array[i]);
		buffer += nchars;
	}

	nchars = sprintf(buffer, "%d]",array[num-1]);

	return print_buffer;
}


char * cp_matrix2string(int * matrix, int rows, int cols){

	int i,j;
	int nchars;
	char * buffer = print_buffer;


	nchars = sprintf(buffer,"\n");
	buffer += nchars;

	for(i=0; i<rows; i++){

		nchars = sprintf(buffer,"[");
		buffer += nchars;

		for(j=0; j<cols-1; j++){
			nchars = sprintf(buffer,"%2d,",matrix[cols*i+j]);
			buffer += nchars;
		}

		nchars = sprintf(buffer,"%2d]\n",matrix[cols*i+j]);
		buffer += nchars;
	}

	return print_buffer;
}


double cp_Wtime(){
	struct timeval tv;
	gettimeofday(&tv, (void *) 0);
	return tv.tv_sec + 1.0e-6 * tv.tv_usec;
}


void cp_msleep(int msec){

	if(msec < 0) return;

	struct timespec ts = {0, 0};
	struct timespec ts2 = {0, 0};

	long tv_sec = msec / 1000;
	long tv_nsec = (msec % 1000) * 1000000;

	ts2.tv_sec =  tv_sec;
	ts2.tv_nsec = tv_nsec;


	while(ts2.tv_nsec > 0 || ts2.tv_sec > 0){
		ts = ts2;
		if (nanosleep(&ts,&ts2) >= 0){
			return;
		}

	}


}


// This is used to get a random seed
int cp_rand_initialized = 0;


int cp_rand(int num_min, int num_max){

	if(num_max < num_min) return 0;

	if(!cp_rand_initialized){
		int rank = 0;
#ifndef CP_NO_MPI
		int flag;
		MPI_Initialized(&flag);
		if(flag) MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif
		srand((unsigned) rank + (unsigned) time(NULL));
		cp_rand_initialized = 1;
	}

	return (num_min + rand() / (RAND_MAX / (num_max - num_min + 1) + 1));
}



/**
 * Macro to check the exit of fscanf
 */
#define cp_check_scanf(v) \
	if((v) != 1){ \
	 fprintf(stderr,"cp_read_matrix Error\n"); \
	 exit(EXIT_FAILURE); \
	}


void cp_read_matrix(char * filename, m_type * matrix, int border){

	// Open the file.
	FILE * file = fopen(filename,"r");

	if(file == NULL){
		fprintf(stderr,"cp_read_matrix File not found %s\n",filename);
		exit(EXIT_FAILURE);
	}

	int rows;
	int cols;

	// Type
	char type[256];
	cp_check_scanf(fscanf(file,"%s\n", type));

	// Read the number of rows and cols.
	cp_check_scanf(fscanf(file,"%d\n",&rows));
	cp_check_scanf(fscanf(file,"%d\n",&cols));

	// Macro to access the matrix.
	#define m(i,j) (matrix[(i)*(cols+2*border)+(j)])

	if(strcmp(type, "DATA") == 0){

		// Reads the context of the matrix.
		int i,j;
		for(i=0; i<rows; i++){
			for(j=0; j<cols; j++){

				double value;
				cp_check_scanf(fscanf(file,"%lf\n",&value));
				m(i+border,j+border) = (m_type) value;
			}
		}
		
	} else {

		// Read seed
		int seed;
		cp_check_scanf(fscanf(file,"%d\n",&seed));
		srand(seed);
		cp_rand_initialized = 1;

		// Put a random value
		int i,j;
		for(i=0; i<rows; i++){
			for(j=0; j<cols; j++){

				m_type value = (m_type) cp_rand(-10000,10000) / 10000.0;
				m(i+border,j+border) = value;
			}
		}

		cp_rand_initialized = 0;

	}

	// Close the file.
	fclose(file);
}




void cp_read_matrix_size(char * filename, int * rows, int * cols){

	// Open the file.
	FILE * file = fopen(filename,"r");

	if(file == NULL){
		fprintf(stderr,"cp_read_matrix File not found %s\n",filename);
		exit(EXIT_FAILURE);
	}
	

	// Type
	char type[256];
	cp_check_scanf(fscanf(file,"%s\n",type));

	// Read the number of rows and cols.
	cp_check_scanf(fscanf(file,"%d\n",rows));
	cp_check_scanf(fscanf(file,"%d\n",cols));

	fclose(file);

}



#ifdef CP_DISPLAY


// Includes for the X11 display
#include <X11/Xlib.h>
#include <X11/Xutil.h>


#include <string.h>
#include <math.h>

// Display max size
#define CP_DISPLAY_MAX_W 800
#define CP_DISPLAY_MAX_H 600

// Display variables
Display *display;
Window window;
GC gc;
int display_zoom;
int	display_cols;
int	display_rows;



void cp_display_create_zoom(char * name, int rows, int cols, int zoom){

	// Open connection with the server
	display = XOpenDisplay(NULL);
	if (display == NULL){
		fprintf(stderr, "Cannot open display\n");
		exit(1);
	}

	// Black color
	int blackColor = BlackPixel(display, DefaultScreen(display));

	// Display variables
	display_rows = rows;
	display_cols = cols;
	display_zoom = zoom;

	// Create the window
	int posxy = 30;
#ifndef CP_NO_MPI
	int rank = 0;
	int flag;
	MPI_Initialized(&flag);
	if(flag) MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	posxy = 30 * (rank+1);
	cp_msleep(50 * rank);
#endif


	window = XCreateSimpleWindow(display, DefaultRootWindow(display), posxy, posxy, cols * zoom, rows * zoom, 1, blackColor, blackColor);

	XSizeHints    my_hints = {0};
	my_hints.flags  = PPosition ;
	my_hints.x      = posxy;     
	my_hints.y      = posxy;
	XSetNormalHints(display, window, &my_hints);


	// Set the events
	XSelectInput(display, window, ExposureMask | KeyPressMask);

	// Change the window name
#ifndef CP_NO_MPI
	char auxstr[1024];
	sprintf(auxstr,"%s p %d",name,rank);
	name = auxstr;
#endif
	XStoreName(display, window, name);

	// Map (show) the window
	XMapWindow(display, window);

	// Graphic context
	gc = DefaultGC(display, 0);

	// Wait for the window to show up
	while(1) {
		XEvent event;
		XNextEvent(display, &event);
		if(event.type==Expose) break;
	}
}


void cp_display_create(char * name, int rows, int cols){

	int zoom_cols = cols;
	int zoom_rows = rows;

#ifndef CP_NO_MPI
	int size = 1;
	int flag;
	MPI_Initialized(&flag);
	if(flag) MPI_Comm_size(MPI_COMM_WORLD, &size);
	zoom_cols *= size;
	zoom_rows *= size;
#endif

	int zoom_w = (int) floor((m_type) CP_DISPLAY_MAX_W / (m_type) zoom_cols);
	int zoom_h = (int) floor((m_type) CP_DISPLAY_MAX_H / (m_type) zoom_rows);

	int zoom = min(zoom_w,zoom_h);

	if(zoom < 1){
		printf("Error: Display too big\n");
		exit(-1);
	} 

	// printf("Autozoom = %d\n",zoom);

	cp_display_create_zoom(name, rows, cols, zoom);
}



// Draw a pixel (with zoom)
void cp_display_draw_pixel(int x, int y, int color){

    XSetForeground(display, gc, color);
	int i, j;
	for(i=0; i<display_zoom; i++){
		for(j= 0; j<display_zoom; j++){
			XDrawPoint(display, window, gc, x*display_zoom+i, y*display_zoom+j);
		}
	}
}



void cp_display_draw_matrix(m_type * matrix, int color1, int color2){

	#define cp_display_matrix_elem(i,j) (matrix[(i)*(display_cols)+(j)])

	int i;
	for(i=0; i<display_rows; i++){
		int j;		
		for(j=0; j<display_cols; j++){

			int color;
			m_type value =  cp_display_matrix_elem(i,j);

			if(value < 0){
				value = min(1.0,max(0.0,-value));
				color = color1;
			} else {
				value = min(1.0,max(0.0,value));
				color = color2;
			}

			m_type dR = CP_GETR(color) * (value);
			m_type dG = CP_GETG(color) * (value);
			m_type dB = CP_GETB(color) * (value);

			int R = (int) dR;
			int G = (int) dG;
			int B = (int) dB;

			cp_display_draw_pixel(j,i, CP_RGB(R,G,B));
		}
	}

	XSync(display, 0);
}



void cp_display_draw_grid(int color){


	XSetForeground(display, gc, color);

	int i,j;
	for(i=0; i<display_zoom*display_rows; i+=display_zoom){
		for(j=0; j<display_zoom*display_cols; j++){
			XDrawPoint(display, window, gc, j, i);
		}		
	}

	for(i=0; i<display_zoom*display_rows; i++){
		for(j=0; j<display_zoom*display_cols; j+=display_zoom){
			XDrawPoint(display, window, gc, j, i);
		}		
	}

	XSync(display, 0);
}



void cp_display_close(){
	XCloseDisplay(display);
}



#endif










