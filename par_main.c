#include "allinc.h"
#include <math.h>
#include <limits.h>
#include <omp.h>
using namespace std;
City map[CITIES];

//==============================================================================
// PROTOTYPES

bool inList(Individual a, int city);
Individual crossoverPm (Individual a, Individual b);
void mutateW(Individual &a);
void mutate(Individual &a);
void mutatePop(Population &p);
int fitness(Individual a);
void sort(Population &p);
void printInd(Individual a);
void printPop(Population p);
void evolve(Population &p);
void cargarCiudades();

int thread_count=1;

// =============================================================================
// EVOLUTION

bool inList(Individual a, int city)
{
	for (int n=0; n<CITIES; n++) if (a.route[n]==city) return true;
	return false;
}

Individual crossoverPm (Individual a, Individual b)
{
	int x = rand()%CITIES;
	if (x>CITIES/2) x=x-CITIES/4;
	int y = rand()%CITIES;
	if (y<CITIES/2) y=y+CITIES/4;
	if (x>y)
	{
		int temp = x;
		x=y;
		y=temp;
	}

	Individual c;
	for (int n=0; n<CITIES; n++) c.route[n]=-1;
	for (int n=x; n<=y; n++) c.route[n]=a.route[n];

		for (int i=0; i<CITIES; i++)
		{
			if (c.route[i]==-1)
			{
				for (int k=0; k<CITIES; k++) if (!inList (c, a.route[k])) c.route[i]=a.route[k];
			}
		}


	return c;
}

// cruce entre 2 lineas y retorna la linea cruzada
Individual crossoverInj (Individual a, Individual b)
{
	int q = rand()%CITIES/3;

	int x = rand()%CITIES;
	if (x>CITIES/2) x=x-CITIES/4;
	int y = rand()%CITIES;
	if (y<CITIES/2) y=y+CITIES/4;
	if (x>y)
	{
		int temp = x;
		x=y;
		y=temp;
	}

	Individual c;
	for (int n=0; n<CITIES; n++) c.route[n]=-1;

	Individual ab;
	for (int n=0; n<CITIES; n++) ab.route[n]=-1;
	for (int n=x; n<=y; n++) ab.route[n]=a.route[n];

	for (int n=0; n<=q; n++)
	{
		if (!inList(ab, b.route[n])) c.route[n]=b.route[n];
	}
	
	for (int h=q+1; h<(y-x); h++) c.route[h]=a.route[h];

		for (int i=0; i<CITIES; i++)
		{
			if (c.route[i]==-1)
			{
				for (int k=0; k<CITIES; k++) if (!inList (c, a.route[k])) c.route[i]=a.route[k];
			}
		}

	return c;
}

void mutateW(Individual &a)
{
	Individual candidate = a;
	for (int i=0; i<MUTATEW_POINTS; i++)
	{
		int k=rand()%(CITIES-1)+1;
		int temp=candidate.route[0];
		candidate.route[0]=candidate.route[k];
		candidate.route[k]=temp;
		if (fitness(candidate)<fitness(a)) a=candidate;
	}

}

void mutate(Individual &a)
{
	Individual candidate = a;
	for (int i=0; i<MUTATE_POINTS; i++)
	{
		int q = rand()%CITIES;
		int d = rand()%CITIES;
		int chance = rand()%2;
		if (chance==1)
		{
			int temp = candidate.route[q];
			candidate.route[q]=candidate.route[d];
			candidate.route[d]=temp;
		}
		if (fitness(candidate)<fitness(a)) a=candidate;
	}
}

void mutatePop(Population &p)
{
	double startTime1 = omp_get_wtime();
	#pragma omp parallel for num_threads(thread_count)
	for (int i=0; i<POP_SIZE; i++)
	{
		printf("i= %d %d\n",i, omp_get_thread_num());
		if (MUTATION_TYPE==0) mutate(p.pop[i]); else mutateW(p.pop[i]);
	}
	double endTime1 = omp_get_wtime();

	printf("TIME: %fs\n",(endTime1-startTime1));
}


// Calcula el costo total de llegar desde primera ciudad que tiene en sus 512 ciudades
// hasta la ultima + la vuelta desde la ultima hasta la primera
int fitness(Individual a)
{
	
	int  xd=0;
	int yd=0;
	int d=0;
	
	int h = 0;
	#pragma omp parallel num_threads(thread_count) private(h,xd,yd) shared(d)
	{
		#pragma omp for
		for (int i=0; i<CITIES-1; i++)
		{	
			xd = map[a.route[i]].x - map[a.route[i+1]].x;
			yd = map[a.route[i]].y - map[a.route[i+1]].y;
			h = h+(int)(sqrt((xd*xd) + (yd*yd)));
		}

		#pragma omp critical 
		{	
			
			d += h;
			
		}
		


	}
	
	xd=map[a.route[CITIES-1]].x - map[a.route[0]].x;
	yd=map[a.route[CITIES-1]].y - map[a.route[0]].y;
	d = d+(int)(sqrt((xd*xd) + (yd*yd)));
	
	return d;

}

// Ordena entre los population el que tiene menor fitness al mayor
void sort(Population &p)
{
	int c=0;
	Individual temp;
	while (c<SORT_LIMIT)
	{
		for (int i=0; i<POP_SIZE-1; i++)
		{
			if (fitness(p.pop[i]) > fitness(p.pop[i+1]))
			{
				temp=p.pop[i];
				p.pop[i]=p.pop[i+1];
				p.pop[i+1]=temp;
			}
		}
	c++;
	}
}

int getFreeIndex(Individual a)
{
	for (int i=0; i<CITIES; i++)
	{
		if (a.route[i]==-1) return i;
	}
	return -1;
}

void printInd(Individual a)
{

	for (int i=0; i<CITIES; i++) cout << a.route[i] << "->";
	int estanTodos = 0;

	for (int i = 0; i <CITIES; i++){
		estanTodos = 0;
		for (int j = 0; j< CITIES; j++){
			if (a.route[j] == i){
				estanTodos = 1;
			}
		} 
		if (estanTodos == 0 ){
			printf("NO ESTA %d\n", i);
			
		}
	}
	


}

void printPop(Population p)
{
	for (int i=0; i<POP_SIZE; i++)
	{
		printInd(p.pop[i]);
		cout << "x | F=" << fitness(p.pop[i]) << endl;
	}
}

void evolve(Population &p)
{
	// Muta cada pop es decir intercambia dos valores de cada linea
	// y verifica si el fitness del modificado es mejor 
	// y si es mejor reemplaza por esta nueva en la linea correspondiente
	mutatePop(p);
	//printPop(p);

	// en offsptring se guarda cruces de lineas
	Individual offspring[POP_SIZE/2];
	Individual winners[TOURN_N];
	Individual temp;
	int g=0;
	int c=0;
	
	{
		
		
		printf("thread number: %d and start g is %d\n",g, omp_get_thread_num());
		while (g<GENERATIONS)
		{

			if (SELECTION_TYPE==0)
			//FIXED RANK SELECTION
			{
				#pragma omp parallel for num_threads(thread_count)
				for (int i=0; i<POP_SIZE/2; i++)
				{
					int flip = rand()%2;
					if (flip==0) offspring[i]=crossoverInj (p.pop[i+1], p.pop[i]); else
							offspring[i]=crossoverPm (p.pop[i], p.pop[i+1]);
				}
			} else if (SELECTION_TYPE==1)
			//TOURNAMENT SELECTION
			{
				#	pragma omp parallel for num_threads(thread_count)
				for (int i=0; i<POP_SIZE/2; i++)
				{

					for (int j=0; j<=TOURN_N; j++)
					{
						winners[j]=p.pop[rand()%(POP_SIZE)];
					}
					//sort winners
					while (c<SORT_LIMIT)
					{
						for (int l=0; l<TOURN_N-1; l++)
						{
							if (fitness(winners[l]) > fitness(winners[l+1]))
								{
									temp=winners[l];
									winners[l]=winners[l+1];
									winners[l+1]=temp;
								}
						}
					c++;
					}

					int flip = rand()%2;
					if (flip==0) offspring[i]=crossoverInj(winners[0], winners[1]); else
							offspring[i]=crossoverPm (winners[1], winners[0]);



				}

			}
			sort(p);

				//#	pragma omp parallel for num_threads(thread_count)
				for (int i=0; i<POP_SIZE/2; i++)
				{
					for (int j=0; j<POP_SIZE/2; j++)
					{
						if (fitness(offspring[i]) < fitness(p.pop[j]))
						{
							p.pop[j]=offspring[i];

						}
					}
				}




			printInd(p.pop[0]);
			cout << " | GEN: " << g << "  | F=" << fitness(p.pop[0]) << endl << endl;

			g+= 1;
			sort(p);
		}
	}
	

}


int exist_in_map(int x,int y){
	for (int i=0; i<CITIES; i++){
		if (map[i].x==x && map[i].y==y){
			return 1;
		}
	}
	return 0;
}

// =============================================================================
// MAIN

int main(int argc, char **argv)
{
	srand(time(NULL));
	
	if (argc>1){
		thread_count = strtol(argv[1],NULL,10);
	}

	// City city;
	// city.x=0;
	// city.y=0;

	// for(int i=0; i<CITIES; i++){
	// 	city.name = i;
	// 	map[i]=city;
	// }
	// printf("CREATING MAP...\n");
	double startTime1 = omp_get_wtime();


	


	// for(int i=0; i<CITIES; i++){

	// 	map[i].x = rand()%1000;
	// 	map[i].y = rand()%1000;

		
	// }
	cargarCiudades();
	
	double endTime1 = omp_get_wtime();

	printf("TIME: %fs\n",(endTime1-startTime1));
	
	
	

	printf("MAP CREATED!!\n");

	Population population1;
	Individual indi[POP_SIZE];
	int catalog[50];
	int ins;
	for (int i=1; i<=CITIES; i++) catalog[i]=i;

	/*
	Tenemos 10 lineas de 512 ciudades cada linea y se inicializa con -1 cada espacio o ciudad de la linea
	[-1,-1,-1,-1,...., -1] (512 posciciones)
	....
	10
	....
	[-1,-1,-1,-1,...., -1] (512 posciciones)
	*/
	for (int k=0; k<POP_SIZE; k++)
	{
		for (int j=0; j<CITIES; j++) indi[k].route[j]=-1;

	}


	/*
	Cada Linea tenemos [-1,-1,-1, ... -1] 512 
	rellenar numeros randoms de 0 a 50  en cada linea hasta terminar las 10 (POP_SIZE) lineas
	*/
	for (int k=0; k<POP_SIZE; k++)
	{
		while(inList (indi[k], -1)) 
		{
			//printf("indi[%d].fitness: %d\n",k,indi[k].fitness);
			//cout << endl << "indi[k]: " << indi[k] << endl;
			ins = (rand()%(CITIES));
			//printf("ins: %d\n",ins);
			//cout << endl <<"ins: "<< ins << endl;
			for (int j=0; j<CITIES; j++) if (!inList (indi[k], ins)) indi[k].route[getFreeIndex(indi[k])]=ins;
		}

	}

	// copiamos los valores de indi en population.pop
	for (int r=0; r<POP_SIZE; r++) population1.pop[r]=indi[r];

	// ordena la lista
	// Ordena entre los population el que tiene menor fitness al mayor
	sort(population1);
	//cout << "Initial sorted population: " << endl;
	//printPop(population1);
	printf("AQUI");
	for(int xd=0; xd< POP_SIZE-1; xd++){
		printf("[");
		for(int xk=0; xk< CITIES; xk++){
			printf("%d,",population1.pop[xd].route[xk]);
			
		}
		printf("] fitness: %d\n\n", fitness(population1.pop[xd]));
	}
	printf("AQUI");
	printf("\n\nStarting evolution...\n\n");
	printf("N= %d with p = %d\n",CITIES,thread_count);
	double stime = omp_get_wtime();

	
	
	evolve(population1);
	
	
	
	
	double etime = omp_get_wtime();
	printf("Evolution ended\n");
	printf("N= %d with p = %d\n",CITIES,thread_count);
	printf("TIME: %fs\n",(etime-stime));
	return 0;
}

void cargarCiudades(){
	City c0;
	c0.name=0;
	c0.x=1;
	c0.y=100;

	City c1;
	c1.name=1;
	c1.x=565;
	c1.y=575;

	City c2;
	c2.name=2;
	c2.x=25;
	c2.y=185;

	City c3;
	c3.name=3;
	c3.x=345;
	c3.y=750;

	City c4;
	c4.name=4;
	c4.x=945;
	c4.y=685;

	City c5;
	c5.name=5;
	c5.x=845;
	c5.y=655;

	City c6;
	c6.name=6;
	c6.x=880;
	c6.y=660;

	City c7;
	c7.name=7;
	c7.x=25;
	c7.y=230;

	City c8;
	c8.name=8;
	c8.x=525;
	c8.y=1000;

	City c9;
	c9.name=9;
	c9.x=580;
	c9.y=1175;

	City c10;
	c10.name=10;
	c10.x=650;
	c10.y=1130;

	City c11;
	c11.name=11;
	c11.x=1605;
	c11.y=620;

	City c12;
	c12.name=12;
	c12.x=1220;
	c12.y=580;

	City c13;
	c13.name=13;
	c13.x=1465;
	c13.y=200;

	City c14;
	c14.name=14;
	c14.x=1530;
	c14.y=5;

	City c15;
	c15.name=15;
	c15.x=845;
	c15.y=680;

	City c16;
	c16.name=16;
	c16.x=725;
	c16.y=370;

	City c17;
	c17.name=17;
	c17.x=145;
	c17.y=665;

	City c18;
	c18.name=18;
	c18.x=415;
	c18.y=635;

	City c19;
	c19.name=19;
	c19.x=510;
	c19.y=875;

	City c20;
	c20.name=20;
	c20.x=560;
	c20.y=365;

	City c21;
	c21.name=21;
	c21.x=300;
	c21.y=465;

	City c22;
	c22.name=22;
	c22.x=520;
	c22.y=585;

	City c23;
	c23.name=23;
	c23.x=480;
	c23.y=415;

	City c24;
	c24.name=24;
	c24.x=835;
	c24.y=625;

	City c25;
	c25.name=25;
	c25.x=975;
	c25.y=580;

	City c26;
	c26.name=26;
	c26.x=1215;
	c26.y=245;

	City c27;
	c27.name=27;
	c27.x=1320;
	c27.y=315;

	City c28;
	c28.name=28;
	c28.x=1250;
	c28.y=400;

	City c29;
	c29.name=29;
	c29.x=660;
	c29.y=180;

	City c30;
	c30.name=30;
	c30.x=410;
	c30.y=250;

	City c31;
	c31.name=31;
	c31.x=420;
	c31.y=555;

	City c32;
	c32.name=32;
	c32.x=575;
	c32.y=665;

	City c33;
	c33.name=33;
	c33.x=1150;
	c33.y=1160;

	City c34;
	c34.name=34;
	c34.x=700;
	c34.y=580;

	City c35;
	c35.name=35;
	c35.x=685;
	c35.y=595;

	City c36;
	c36.name=36;
	c36.x=685;
	c36.y=610;

	City c37;
	c37.name=37;
	c37.x=770;
	c37.y=610;

	City c38;
	c38.name=38;
	c38.x=795;
	c38.y=645;

	City c39;
	c39.name=39;
	c39.x=720;
	c39.y=635;

	City c40;
	c40.name=40;
	c40.x=760;
	c40.y=650;

	City c41;
	c41.name=41;
	c41.x=475;
	c41.y=960;

	City c42;
	c42.name=42;
	c42.x=95;
	c42.y=260;

	City c43;
	c43.name=43;
	c43.x=875;
	c43.y=920;

	City c44;
	c44.name=44;
	c44.x=700;
	c44.y=500;

	City c45;
	c45.name=45;
	c45.x=555;
	c45.y=815;

	City c46;
	c46.name=46;
	c46.x=830;
	c46.y=485;

	City c47;
	c47.name=47;
	c47.x=1170;
	c47.y=65;

	City c48;
	c48.name=48;
	c48.x=830;
	c48.y=610;

	City c49;
	c49.name=49;
	c49.x=605;
	c49.y=625;

	City c50;
	c50.name=50;
	c50.x=595;
	c50.y=360;


	map[0]=c0;
	map[1]=c1;
	map[2]=c2;
	map[3]=c3;
	map[4]=c4;
	map[5]=c5;
	map[6]=c6;
	map[7]=c7;
	map[8]=c8;
	map[9]=c9;
	map[10]=c10;
	map[11]=c11;
	map[12]=c12;
	map[13]=c13;
	map[14]=c14;
	map[15]=c15;
	map[16]=c16;
	map[17]=c17;
	map[18]=c18;
	map[19]=c19;
	map[20]=c20;
	map[21]=c21;
	map[22]=c22;
	map[23]=c23;
	map[24]=c24;
	map[25]=c25;
	map[26]=c26;
	map[27]=c27;
	map[28]=c28;
	map[29]=c29;
	map[30]=c30;
	map[31]=c31;
	map[32]=c32;
	map[33]=c33;
	map[34]=c34;
	map[35]=c35;
	map[36]=c36;
	map[37]=c37;
	map[38]=c38;
	map[39]=c39;
	map[40]=c40;
	map[41]=c41;
	map[42]=c42;
	map[43]=c43;
	map[44]=c44;
	map[45]=c45;
	map[46]=c46;
	map[47]=c47;
	map[48]=c48;
	map[49]=c49;

	
	printf("mapa [");
	for (int i = 0; i < CITIES; i++){
		printf("%d,", map[i].name);
	}
	printf("]\n");

}