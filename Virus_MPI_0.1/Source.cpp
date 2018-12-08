#include <iostream>
#include <time.h>
#include <vector>
#include <algorithm>

using namespace std;

//Foos Start

void generate_pople_and_infected();
void generate_pople_and_infected_for_all(); //ONLY FOR DEBUGGING
void fill_infection_matrix();
void cull(int i);
void infect(int i);
void move(int i);

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
	input_map_side = 4;
	input_people_total = 40;
	process_id = 0;
	process_total_ammount = 4;
	people_per_process = input_people_total / process_total_ammount;
	srand(time(NULL));


	vector_local_people.resize(people_per_process * 3);
	vector_global_people.resize(input_people_total * 3);
	infection_matrix.resize(input_map_side*input_map_side);

	generate_pople_and_infected_for_all();

	copy(vector_global_people.begin() + (people_per_process*process_id * 3), vector_global_people.begin() + (people_per_process*process_id * 3 + people_per_process * 3), vector_local_people.begin());
	fill_infection_matrix();

	cout << "PRINTING GLOBAL";

	for (size_t i = 0; i < vector_global_people.size(); i++)
	{
		if (i % 3 == 0)
		{
			cout << endl << " " << vector_global_people[i];
		}
		else
		{
			cout << " " << vector_global_people[i];
		}
	}

	cout << endl << endl << "PRINTING LOCAL" << endl;

	for (size_t i = 0; i < vector_local_people.size(); i++)
	{
		if (i % 3 == 0)
		{
			cout << endl << " " << vector_local_people[i];
		}
		else
		{
			cout << " " << vector_local_people[i];
		}
	}

	cout << endl << endl << "PRINTING INFECTION MATRIX" << endl;

	for (size_t i = 0; i < infection_matrix.size(); i++)
	{
		if (i % input_map_side == 0)
		{
			cout << endl << " " << infection_matrix[i];
		}
		else
		{
			cout << " " << infection_matrix[i];
		}
	}

	for (int i = 0; i < vector_local_people.size(); i += 3)
	{
		cull(i);
		infect(i);
		move(i);
	}

	cout << endl << endl << "PRINTING LOCAL" << endl;

	for (size_t i = 0; i < vector_local_people.size(); i++)
	{
		if (i % 3 == 0)
		{
			cout << endl << " " << vector_local_people[i];
		}
		else
		{
			cout << " " << vector_local_people[i];
		}
	}

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

void fill_infection_matrix()
{
	for (int i = 0; i < vector_global_people.size(); i += 3)
	{
		if (vector_global_people[i] / 10000 == 2)
		{
			infection_matrix[input_map_side * vector_global_people[i + 2] + vector_global_people[i + 1]]++;
		}
	}
}

void cull(int i)
{
	if (vector_local_people[i] / 10000 == 2)
	{
		if (current_tick - (vector_local_people[i] % 10000) >= input_infection_duration)
		{
			if ((rand() / (float)RAND_MAX) <= input_recovery_chance)
			{
				vector_local_people[i] += 10000; //becomes immune
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
		int direction = rand() % 7;
		if (direction >= 4)
		{
			direction++;
		}
		int new_x = (direction % 3) - 1;
		int new_y = (direction / 3) - 1;

		vector_local_people[i + 1] = (vector_local_people[i + 1] + new_x + input_map_side) % input_map_side;
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