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
	int flip;
	for (int i=0; i<POP_SIZE; i++)
	{
		if (MUTATION_TYPE==0) mutate(p.pop[i]); else mutateW(p.pop[i]);
	}
}

int fitness(Individual a)
{

	int xd=0;
	int yd=0;
	int d=0;
	for (int i=0; i<CITIES-1; i++)
	{
		xd = map[a.route[i]].x - map[a.route[i+1]].x;
		yd = map[a.route[i]].y - map[a.route[i+1]].y;
		 d = d+(int)(sqrt(xd*xd + yd*yd)+0.5);

	}
	xd=map[a.route[CITIES-1]].x - map[a.route[0]].x;
	yd=map[a.route[CITIES-1]].y - map[a.route[0]].y;
	d = d+(int)(sqrt(xd*xd + yd*yd)+0.5);

	return d;
}

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
	mutatePop(p);
	//printPop(p);

	Individual offspring[POP_SIZE/2];
	Individual winners[TOURN_N];
	Individual temp;
	int g=0;
	int c=0;
	while (g<GENERATIONS)
	{

		if (SELECTION_TYPE==0)
		//FIXED RANK SELECTION
		{
			#	pragma omp for
			for (int i=0; i<POP_SIZE/2; i++)
			{
				int flip = rand()%2;
				if (flip==0) offspring[i]=crossoverInj (p.pop[i+1], p.pop[i]); else
						offspring[i]=crossoverPm (p.pop[i], p.pop[i+1]);
			}
	        } else if (SELECTION_TYPE==1)
		//TOURNAMENT SELECTION
		{
			#	pragma omp for
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

			#	pragma omp for
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

	g++;
	sort(p);
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
	int thread_count=1;
	if (argc>1){
		thread_count = strtol(argv[1],NULL,10);
	}

	City city;
	city.x=0;
	city.y=0;

	for(int i=0; i<CITIES; i++){
		city.name = i;
		map[i]=city;
	}
printf("CREATING MAP...\n");
	for(int i=0; i<CITIES; i++){
		//do{
			map[i].x = rand();
			map[i].y = rand();
		//}while (exist_in_map(city.x,city.y));
	}

	printf("MAP CREATED!!\n");

	Population population1;
	Individual indi[POP_SIZE];
	int catalog[50];
	int ins;
	for (int i=1; i<=CITIES; i++) catalog[i]=i;

	for (int k=0; k<POP_SIZE; k++)
	{
		for (int j=0; j<CITIES; j++) indi[k].route[j]=-1;

	}


	for (int k=0; k<POP_SIZE; k++)
	{
		while(inList (indi[k], -1))
		{
			printf("indi[%d].fitness: %d\n",k,indi[k].fitness);
			//cout << endl << "indi[k]: " << indi[k] << endl;
			ins = (rand()%(CITIES+1));
			printf("ins: %d\n",ins);
			//cout << endl <<"ins: "<< ins << endl;
			for (int j=0; j<CITIES; j++) if (!inList (indi[k], ins)) indi[k].route[getFreeIndex(indi[k])]=ins;
		}

	}

	for (int r=0; r<POP_SIZE; r++) population1.pop[r]=indi[r];

	sort(population1);
	//cout << "Initial sorted population: " << endl;
	//printPop(population1);
	cout << endl << "Starting evolution..." << endl << endl;
	printf("N= %d with p = %d\n",CITIES,thread_count);
	double stime = omp_get_wtime();
	#	pragma omp parallel num_threads(thread_count)
	evolve(population1);
	double etime = omp_get_wtime();
	printf("Evolution ended\n");
	printf("N= %d with p = %d\n",CITIES,thread_count);
	printf("TIME: %fs\n",(etime-stime));
	return 0;
}
