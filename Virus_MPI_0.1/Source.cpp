#include <iostream>
#include <time.h>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

//Foos Start

void generate_pople_and_infected();
void generate_pople_and_infected_for_all(); //ONLY FOR DEBUGGING
void fill_infection_matrix_and_compile_stats();
void cull(int i);
void infect(int i);
void move(int i);
void print_header();
void print_tick(int);

//Foos END

int input_people_total;
double input_infectiousness;
double input_recovery_chance;
int input_infection_duration;
float input_initial_infected_percentage;
int input_map_side;
int current_tick;
int process_id;
int process_total_ammount;
int people_per_process;

vector<vector<unsigned short>> vector_of_vectors_of_stats;


vector<unsigned short> vector_local_people; // 3 shorts per person
vector<unsigned short> vector_global_people;
vector<int> infection_matrix;



int main(int argc, char ** argv)
{
	current_tick = 100;
	input_recovery_chance = 0.5;
	input_infectiousness = 0.3;
	input_infection_duration = 25;

	input_initial_infected_percentage = 0.5;
	input_map_side = 10;
	input_people_total = 120;
	process_id = 2;
	process_total_ammount = 4;
	people_per_process = input_people_total / process_total_ammount;
	srand(time(NULL));


	vector_local_people.resize(people_per_process * 3);
	vector_global_people.resize(input_people_total * 3);
	infection_matrix.resize(input_map_side*input_map_side);
	//vector_of_stats.resize(4);
	vector_of_vectors_of_stats.reserve(500);

	generate_pople_and_infected_for_all();

	copy(vector_global_people.begin() + (people_per_process*process_id * 3), vector_global_people.begin() + (people_per_process*process_id * 3 + people_per_process * 3), vector_local_people.begin());
	fill_infection_matrix_and_compile_stats();

	print_header();
	print_tick(0);

	//cout << "PRINTING GLOBAL";

	//for (size_t i = 0; i < vector_global_people.size(); i++)
	//{
	//	if (i % 3 == 0)
	//	{
	//		cout << endl << " " << vector_global_people[i];
	//	}
	//	else
	//	{
	//		cout << " " << vector_global_people[i];
	//	}
	//}

	//cout << endl << endl << "PRINTING LOCAL" << endl;

	//for (size_t i = 0; i < vector_local_people.size(); i++)
	//{
	//	if (i % 3 == 0)
	//	{
	//		cout << endl << " " << vector_local_people[i];
	//	}
	//	else
	//	{
	//		cout << " " << vector_local_people[i];
	//	}
	//}

	//cout << endl << endl << "PRINTING INFECTION MATRIX" << endl;

	//for (size_t i = 0; i < infection_matrix.size(); i++)
	//{
	//	if (i % input_map_side == 0)
	//	{
	//		cout << endl << " " << infection_matrix[i];
	//	}
	//	else
	//	{
	//		cout << " " << infection_matrix[i];
	//	}
	//}

	//for (int i = 0; i < vector_local_people.size(); i += 3)
	//{
	//	cull(i);
	//	infect(i);
	//	move(i);
	//}

	//cout << endl << endl << "PRINTING LOCAL" << endl;

	//for (size_t i = 0; i < vector_local_people.size(); i++)
	//{
	//	if (i % 3 == 0)
	//	{
	//		cout << endl << " " << vector_local_people[i];
	//	}
	//	else
	//	{
	//		cout << " " << vector_local_people[i];
	//	}
	//}

	//fill_infection_matrix_and_compile_stats();

	//cout << endl << endl << "STATS" << endl;

	//for (size_t j = 0; j < vector_of_vectors_of_stats.size(); j++)
	//{
	//	for (size_t i = 0; i < vector_of_vectors_of_stats[j].size(); i++)
	//	{

	//		cout << " " << vector_of_vectors_of_stats[j][i];

	//	}
	//	cout << endl;
	//}

	cin.ignore();

}

void generate_pople_and_infected()
{
	for (int i = 0; i < vector_local_people.size(); i += 3)
	{
		//if (i / 3 < input_initial_infected_percentage * people_per_process) //the first people generated will be infected
		if (rand() / (float)RAND_MAX < input_initial_infected_percentage) //the first people generated will be infected
		{
			vector_local_people[i] = 20001;
		}
		else
		{
			vector_local_people[i] = 10000;
		}

		//if (i == 27)
		//{
		//	cout << input_initial_infected_percentage * people_per_process;
		//}


		//int new_x = rand() % input_map_side;
		//int new_y = rand() % input_map_side;
		//vector_local_people[i + 1] = new_x;
		//vector_local_people[i + 2] = new_y;

		vector_local_people[i + 1] = rand() % input_map_side;
		vector_local_people[i + 2] = rand() % input_map_side;

		//if (vector_local_people[i] / 10000 == 2)
		//{
		//	infection_matrix[input_map_side*new_y + new_x]++;
		//}

	}

	//cout << endl << "People generated!" << endl;
}

void generate_pople_and_infected_for_all()
{
	for (int i = 0; i < vector_global_people.size(); i += 3)
	{
		//if (i / 3 < input_initial_infected_percentage * input_people_total) //the first people generated will be infected
		if (rand() / (float)RAND_MAX < input_initial_infected_percentage) //the first people generated will be infected
		{
			vector_global_people[i] = 20001;
		}
		else
		{
			vector_global_people[i] = 10000;
		}

		//if (i == 117)
		//{
		//	cout << input_initial_infected_percentage * people_per_process;
		//}


		//int new_x = rand() % input_map_side;
		//int new_y = rand() % input_map_side;
		//vector_global_people[i + 1] = new_x;
		//vector_global_people[i + 2] = new_y;

		vector_global_people[i + 1] = rand() % input_map_side;
		vector_global_people[i + 2] = rand() % input_map_side;

		//if (vector_global_people[i] / 10000 == 2)
		//{
		//	infection_matrix[input_map_side*new_y + new_x]++;
		//}

	}

	//cout << endl << "People generated!" << endl;
}

void fill_infection_matrix_and_compile_stats()
{
	vector<unsigned short> vector_of_stats;
	vector_of_stats.resize(4);

	for (int i = 0; i < vector_global_people.size(); i += 3)
	{
		switch (vector_global_people[i] / 10000)
		{
		case 1:
			vector_of_stats[0]++; //succeptible			
			break; //optional
		case 2:
			vector_of_stats[1]++; //infected			
			infection_matrix[input_map_side * vector_global_people[i + 2] + vector_global_people[i + 1]]++;
			break; //optional
		case 3:
			vector_of_stats[2]++; //immune			
			break; //optional
		case 4:
			vector_of_stats[3]++; //dead			
			break; //optional
		}	

	}
	vector_of_vectors_of_stats.push_back(vector_of_stats);
}

void cull(int i)
{
	if (vector_local_people[i] / 10000 == 2)
	{
		if (current_tick - (vector_local_people[i] % 10000) >= input_infection_duration)
		{
			if ((rand() / (float)RAND_MAX) <= input_recovery_chance)
			{
				vector_local_people[i] = 30000; //becomes immune
			}
			else
			{
				vector_local_people[i] = 40000; //dies
			}
		}
	}

}

void infect(int i)
{
	if (vector_local_people[i] / 10000 == 1)
	{
		//comparing againts the cummulative infection chance
		if ((rand() / (float)RAND_MAX) <= (1 - pow(1 - input_infectiousness, infection_matrix[input_map_side * vector_global_people[i + 2] + vector_global_people[i + 1]])))
		{
			vector_local_people[i] = 20000 + current_tick; //marks the person as infected and adds the time of infection 
		}
	}
}

void move(int i)
{
	cout << " ";
	if ((vector_local_people[i] / 10000) != 4) //if he aint dead
	{
		int direction = rand() % 7; //rolling for one of the 8 (counting the zero) possible directions the person can go to
		if (direction >= 4) //this way we avoid having a position 4 in the roll, meaning that a person can't stay on their current square
		{
			direction++;
		}
		int new_x = (direction % 3) - 1; //-1 so it goes from -1 to 1
		int new_y = (direction / 3) - 1;

		vector_local_people[i + 1] = (vector_local_people[i + 1] + new_x + input_map_side) % input_map_side; //we add the side of the map again to avoid underflows when the new axis is -1
		vector_local_people[i + 2] = (vector_local_people[i + 2] + new_y + input_map_side) % input_map_side;

		//if (rand() % 2 == 1) //moving on X axis
		//{
		//	vector_local_people[i + 2] = vector_local_people[i + 1] + 1 % input_map_side; //forwards in axis
		//}
		//else
		//{
		//	vector_local_people[i + 2] = vector_local_people[i + 1] - 1 % input_map_side;	//backwards in axis
		//}

		//if (rand() % 2 == 1) //moving on Y axis
		//{
		//	vector_local_people[i + 2] = vector_local_people[i + 2] + 1 % input_map_side; //forwards in axis
		//}
		//else
		//{
		//	vector_local_people[i + 2] = vector_local_people[i + 2] - 1 % input_map_side; //backwards in axis
		//}


	}
}

void print_header()
{
	cout.precision(2);

	//To Console

	cout << left << setw(5) << "LEGEND:" << endl;
	cout << left << setw(7) << " " << "T = Tick" << endl;
	cout << left << setw(7) << " " << "uInf = Average Infected" << endl;
	cout << left << setw(7) << " " << "%Inf = Percentage Infected" << endl;
	cout << left << setw(7) << " " << "TInf = Total Infected" << endl;
	cout << left << setw(7) << " " << "uDead = Average Dead" << endl;
	cout << left << setw(7) << " " << "%Dead = Percentage Dead" << endl;
	cout << left << setw(7) << " " << "TDead = Total Dead" << endl;
	cout << left << setw(7) << " " << "uRec = Average Recovered" << endl;
	cout << left << setw(7) << " " << "%Rec = Percentage Recovered" << endl;
	cout << left << setw(7) << " " << "TRec = Total Recovered" << endl;
	cout << left << setw(7) << " " << "uSuc = Average Susceptible" << endl;
	cout << left << setw(7) << " " << "%Suc = Percentage Susceptible" << endl;
	cout << left << setw(7) << " " << "TSuc = Total Susceptible" << endl << endl;

	cout << left << setw(10) << "T";
	cout << left << setw(10) << "uInf";
	cout << left << setw(10) << "%Inf";
	cout << left << setw(10) << "TInf";
	cout << left << setw(10) << "uDead";
	cout << left << setw(10) << "%Dead";
	cout << left << setw(10) << "TDead";
	cout << left << setw(10) << "uRec";
	cout << left << setw(10) << "%Rec";
	cout << left << setw(10) << "TRec";
	cout << left << setw(10) << "uSuc";
	cout << left << setw(10) << "%Suc";
	cout << left << setw(10) << "TSuc" << endl;

	//To File
	/* Disabled for now

	*output_file << left << setw(5) << "LEGEND:" << endl;
	*output_file << left << setw(7) << " " << "T = Tick" << endl;
	*output_file << left << setw(7) << " " << "uInf = Average Infected" << endl;
	*output_file << left << setw(7) << " " << "%Inf = Percentage Infected" << endl;
	*output_file << left << setw(7) << " " << "TInf = Total Infected" << endl;
	*output_file << left << setw(7) << " " << "uDead = Average Dead" << endl;
	*output_file << left << setw(7) << " " << "%Dead = Percentage Dead" << endl;
	*output_file << left << setw(7) << " " << "TDead = Total Dead" << endl;
	*output_file << left << setw(7) << " " << "uRec = Average Recovered" << endl;
	*output_file << left << setw(7) << " " << "%Rec = Percentage Recovered" << endl;
	*output_file << left << setw(7) << " " << "TRec = Total Recovered" << endl;
	*output_file << left << setw(7) << " " << "uSuc = Average Susceptible" << endl;
	*output_file << left << setw(7) << " " << "%Suc = Percentage Susceptible" << endl;
	*output_file << left << setw(7) << " " << "TSuc = Total Susceptible" << endl << endl;

	*output_file << left << setw(10) << "T";
	*output_file << left << setw(10) << "uInf";
	*output_file << left << setw(10) << "%Inf";
	*output_file << left << setw(10) << "TInf";
	*output_file << left << setw(10) << "uDead";
	*output_file << left << setw(10) << "%Dead";
	*output_file << left << setw(10) << "TDead";
	*output_file << left << setw(10) << "uRec";
	*output_file << left << setw(10) << "%Rec";
	*output_file << left << setw(10) << "TRec";
	*output_file << left << setw(10) << "uSuc";
	*output_file << left << setw(10) << "%Suc";
	*output_file << left << setw(10) << "TSuc" << endl << endl;

	*/
}

void print_tick (int tick)
{
		cout.precision(2);

//To console

cout << left << setw(10) << fixed << tick << left << setw(10) << fixed << left << setw(10) << fixed << (double)vector_of_vectors_of_stats[tick][1] / (double)(tick + 1);
cout << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][1] * 100 / input_people_total << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][1];
cout << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] / (double)(tick + 1) << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] * 100 / input_people_total;
cout << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2] / (double)(tick + 1);
cout << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2] * 100 / input_people_total << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2];
cout << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] / (double)(tick + 1) << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] * 100 / input_people_total;
cout << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] << endl;

//To file
/*
*output_file << left << setw(10) << fixed << tick << left << setw(10) << fixed << left << setw(10) << fixed << (double)vector_of_vectors_of_stats[tick][1] / (double)(tick + 1);
*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][1] * 100 / input_people_total << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][1];
*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] / (double)(tick + 1) << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] * 100 / input_people_total;
*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2] / (double)(tick + 1);
*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2] * 100 / input_people_total << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2];
*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] / (double)(tick + 1) << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] * 100 / input_people_total;
*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] << endl;
*/
}
