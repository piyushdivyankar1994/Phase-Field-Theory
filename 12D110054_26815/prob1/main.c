/************************************************************
gcc main.c -lgsl -lgslcblas -lfftw3 -lm
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <complex.h>

#include <gsl/gsl_math.h>
#include <fftw3.h>

int main(void) {
FILE *fpw,*gnu;

int i1,i2;
int half_nx, half_ny;
double kx, ky, delta_kx, delta_ky;
double kx2, ky2, k2, k4;
char name[50];
int INDEX;

double A = 1.0;
double alpha = 1.0;
double beta = 1.0;

double delta_x = 1.0;
double delta_y = 1.0;
double delta_t = 0.2;
int T,T_write;
int n_x, n_y;

n_x = n_y = 128;

/**Take input from file**/
fpw=fopen("input.dat","r");
fscanf(fpw,"%d",&n_x);
fscanf(fpw,"%le",&delta_x);
fscanf(fpw,"%d",&n_y);
fscanf(fpw,"%le",&delta_y);
fscanf(fpw,"%d",&T);//number of time steps
fscanf(fpw,"%le",&delta_t);
fscanf(fpw,"%d",&T_write);//after how many steps do we print the output in a file
fscanf(fpw,"%le",&alpha);
fscanf(fpw,"%le",&beta);
fscanf(fpw,"%le",&A);
fclose(fpw);

/**Allocating memory to the pointers for FFTW**/
fftw_complex *comp,*g;
fftw_plan planF, planB,planFg;

comp = fftw_malloc(n_x*n_y* sizeof(fftw_complex));
g = fftw_malloc(n_x*n_y* sizeof(fftw_complex));
planF = fftw_plan_dft_2d(n_x,n_y,comp,comp,FFTW_FORWARD,FFTW_ESTIMATE);
planFg = fftw_plan_dft_2d(n_x,n_y,g,g,FFTW_FORWARD,FFTW_ESTIMATE);
planB = fftw_plan_dft_2d(n_x,n_y,comp,comp,FFTW_BACKWARD,FFTW_ESTIMATE);


/**Making the initial input**/
for(i1=0; i1 < n_x; ++i1)
{
	for(i2=0; i2 < n_y; ++i2)
	{
		if( ((i1-n_x/2)*(i1-n_x/2) + (i2-n_y/2)*(i2-n_y/2)) < 225)
		{
			__real__ comp[i2+n_y*i1] = 0.0;
			__imag__ comp[i2+n_y*i1] = 0.0;
		}
		else
		{
			__real__ comp[i2+n_y*i1] = 0.1;
			__imag__ comp[i2+n_y*i1] = 0.0;
		}
	}
}

half_nx = (int) n_x/2;
half_ny = (int) n_y/2;

delta_kx = (2.0*M_PI)/(n_x*delta_x);
delta_ky = (2.0*M_PI)/(n_y*delta_y);

/**Making the gnuplot script file**/
gnu=fopen("plotAnimation.gp","w");
fprintf(gnu,"set cbrange[0:0.5]\n");
fprintf(gnu,"set xrange[0:%d]\n",n_x);
fprintf(gnu,"set yrange[0:%d]\n",n_y);

/**Starting time loop**/
for(INDEX=0; INDEX<T; ++INDEX){

	/** Let us take comp to the Fourier space **/
	for(i1=0; i1 < n_x; ++i1)
	{
		for(i2=0; i2 < n_y; ++i2)
		{
			g[i2+n_y*i1]=2*A*comp[i2+n_y*i1]*(1-comp[i2+n_y*i1])*(1-2*comp[i2+n_y*i1]);
		}
	}
	fftw_execute_dft(planF,comp,comp);
	fftw_execute_dft(planFg,g,g);

	/** Evolve composition **/

	for(i1=0; i1 < n_x; ++i1){
		if(i1 < half_nx) kx = i1*delta_kx;
		else kx = (i1-n_x)*delta_kx;
		kx2 = kx*kx;
	for(i2=0; i2 < n_y; ++i2){
		if(i2 < half_ny) ky = i2*delta_ky;
		else ky = (i2-n_y)*delta_ky;
		ky2 = ky*ky;
		k2 = kx2 + ky2;
		k4=k2*k2;
		comp[i2+n_y*i1] = (comp[i2+n_y*i1]-alpha*k2*delta_t*g[i2+n_y*i1])/(1+2*beta*k4*delta_t);
	}}

	/** Take composition back to real space **/

	fftw_execute_dft(planB,comp,comp);

	for(i1=0; i1<n_x; ++i1){
	for(i2=0; i2<n_y; ++i2){
	comp[i2+n_y*i1] = comp[i2+n_y*i1]/(n_x*n_y);
	}}

	/**Print the composition after every T_write time steps**/
	if(INDEX%T_write==0)
	{
	sprintf(name,"./output/c_%d.dat",INDEX);
	fpw=fopen(name,"w");
	for(i1=0; i1<n_x; ++i1)
	{
		for(i2=0; i2<n_y; ++i2)
		{
			fprintf(fpw,"%le ",__real__ comp[i2+n_y*i1]);
			__imag__ comp[i2+n_y*i1] = 0.0;
		}
		fprintf(fpw,"\n");
	}
	fclose(fpw);

	fprintf(gnu,"plot \"%s\" matrix with image\npause 0.2\n",name);
	}

}

fclose(gnu);
fftw_free(comp);
fftw_free(g);
fftw_destroy_plan(planFg);
fftw_destroy_plan(planF);
fftw_destroy_plan(planB);
}
