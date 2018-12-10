#include <iostream>
#include <time.h>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <mpi.h>
#include <fstream>

using namespace std;

//Foos Start

void generate_people_and_infected();
void fill_infection_matrix_and_compile_stats();
void cull(int i);
void infect(int i);
void move(int i);
void print_header(ofstream*);
void print_lengend_and_top_of_table(ofstream*);
void print_tick(int, ofstream*);
char validate_parameters(int argc, char* argv[]);
ofstream output_file; //File to write the output in

//Foos END

int input_people_total;
double input_infectiousness;
double input_recovery_chance;
int input_infection_duration;
int input_initial_infected_percentage;
int input_map_side;
int current_tick = 1;
int people_per_process;

vector<vector<int>> vector_of_vectors_of_stats; // [0] susceptible, [1] infected, [2] immune, [3] dead


vector<unsigned short> vector_local_people; // 3 shorts per person
vector<unsigned short> vector_global_people;
vector<int> infection_matrix;

//MPI variables
int process_id;
int process_total_ammount;
char parameters_are_valid = '0';
double local_start, local_finish, local_elapsed, elapsed;

#define PRINTFILE //can be commented out to disable file output


int main(int argc, char* argv[])
{

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
	MPI_Comm_size(MPI_COMM_WORLD, &process_total_ammount);

	if (process_id == 0)
	{
		parameters_are_valid = validate_parameters(argc, argv); //only the main process validates the parameters and then broadcasts the approval/rejection
	}

	MPI_Bcast(&parameters_are_valid, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	if (parameters_are_valid == '1')
	{
		input_people_total = 1000000;
		input_infectiousness = 0.65;
		input_recovery_chance = 0.5;
		input_infection_duration = 20;
		input_initial_infected_percentage = 10;
		input_map_side = 500;

	}
	else
	{
		input_people_total = atoi(argv[1]);
		input_infectiousness = atof(argv[2]);
		input_recovery_chance = atof(argv[3]);
		input_infection_duration = atoi(argv[4]);
		input_initial_infected_percentage = atoi(argv[5]);
		input_map_side = atoi(argv[6]);
	}


	people_per_process = input_people_total / process_total_ammount;
	srand(process_id + time(NULL));


	vector_local_people.resize(people_per_process * 3);
	vector_global_people.resize(input_people_total * 3);
	infection_matrix.resize(input_map_side*input_map_side);

	vector_of_vectors_of_stats.reserve(500);

	MPI_Barrier(MPI_COMM_WORLD);
	local_start = MPI_Wtime();
	
	generate_people_and_infected();

	if (process_id == 0) //only the main process prints
	{
		output_file.open("output.txt");

		print_header(&output_file);
		print_lengend_and_top_of_table(&output_file);
	}

	do
	{

		MPI_Allgather(vector_local_people.data(), vector_local_people.size(), MPI_UNSIGNED_SHORT, vector_global_people.data(), vector_local_people.size(), MPI_UNSIGNED_SHORT, MPI_COMM_WORLD);

		fill_infection_matrix_and_compile_stats();

		for (int i = 0; i < vector_local_people.size(); i += 3)
		{
			cull(i);
			infect(i);
			move(i);
		}

		if (process_id == 0)
		{
			print_tick(current_tick - 1, &output_file); //only the main process prints, by doing it per tick we can see the progression of the algorithm without having to wait for it to complete
		}

		current_tick++;
	} while (vector_of_vectors_of_stats[current_tick - 2][1] != 0);

	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (process_id == 0)
	{
		//To Console

		cout.precision(5);

		cout << endl << "Process Completed!" << endl;
		cout << "Total time taken = " << elapsed << endl;
		cout << "Average time per tick = " << elapsed / (current_tick - 1) << endl;

		//To file

		output_file.precision(5);

		output_file << endl << "Process Completed!" << endl;
		output_file << "Total time taken = " << elapsed << endl;
		output_file << "Average time per tick = " << elapsed / (current_tick - 1) << endl;
		output_file.close();
	}

	if (process_id == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // to sync the before finalize

	MPI_Finalize();

}

void generate_people_and_infected()
{

	for (int i = 0; i < vector_local_people.size(); i += 3)
	{

		if (i / 3 < (input_initial_infected_percentage / 100.0) * people_per_process) //the first people generated will be infected, doesn't add a bias since they all have random coordinates
		{
			vector_local_people[i] = 20000; //tick of infection set to zero
		}
		else
		{
			vector_local_people[i] = 10000;
		}

		vector_local_people[i + 1] = rand() % input_map_side;
		vector_local_people[i + 2] = rand() % input_map_side;

	}
}


void fill_infection_matrix_and_compile_stats()
{
	vector<int> vector_of_stats;
	vector_of_stats.resize(4);
	infection_matrix.clear(); //wipe clean the infection matrix to avoid carrying over information from the previous tick
	infection_matrix.resize(input_map_side*input_map_side);

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
		//comparing againts the cummulative infection chance, by using this formula we can call rand() only once instead of once per infection "roll"
		if ((rand() / (float)RAND_MAX) <= (1 - pow(1 - input_infectiousness, infection_matrix[input_map_side * vector_global_people[i + 2] + vector_global_people[i + 1]])))
		{
			vector_local_people[i] = 20000 + current_tick; //marks the person as infected and adds the time of infection 
		}
	}
}

void move(int i)
{
	if ((vector_local_people[i] / 10000) != 4) //if he ins't dead
	{
		int direction = rand() % 7; //rolling for one of the 8 (starting from zero) possible directions the person can go to
		if (direction >= 4) //this way we avoid having a position 4 in the roll, meaning that a person can't stay on their current square
		{
			direction++;
		}
		int new_x = (direction % 3) - 1; //-1 so it goes from -1 to 1
		int new_y = (direction / 3) - 1;

		vector_local_people[i + 1] = (vector_local_people[i + 1] + new_x + input_map_side) % input_map_side; //we add the side of the map again to avoid underflows when the new axis is -1
		vector_local_people[i + 2] = (vector_local_people[i + 2] + new_y + input_map_side) % input_map_side;
			   		 
	}
}

void print_header(ofstream* output_file)
{
	cout.precision(2);

	cout << endl;
	cout << "	       =================================================================" << endl;
	cout << "               |                                                               |" << endl;
	cout << "               |                        Virus Simulator                        |" << endl;
	cout << "               |              By Luis Ramirez & Marcial Carrillo               |" << endl;
	cout << "               |                    CI-0117   -  12/09/2018                    |" << endl;
	cout << "               |                                                               |" << endl;
	cout << "	       =================================================================" << endl;
	cout << endl;

	cout << "DATA: " << endl << endl;
	cout << fixed << "-Number of People: " << input_people_total << endl;
	cout << fixed << "-Infectiousness: " << input_infectiousness << endl;
	cout << fixed << "-Chance to recover: " << input_recovery_chance << endl;
	cout << fixed << "-Infection duration: " << input_infection_duration << endl;
	cout << fixed << "-Initial infected: " << input_initial_infected_percentage << "%" << endl;
	cout << fixed << "-Map size: " << input_map_side << "x" << input_map_side << endl;

	//To file
#	ifdef PRINTFILE

	output_file->precision(2);

	*output_file << endl;
	*output_file << "	       =================================================================" << endl;
	*output_file << "               |                                                               |" << endl;
	*output_file << "               |                        Virus Simulator                        |" << endl;
	*output_file << "               |              By Luis Ramirez & Marcial Carrillo               |" << endl;
	*output_file << "               |                    CI-0117   -  12/09/2018                    |" << endl;
	*output_file << "               |                                                               |" << endl;
	*output_file << "	       =================================================================" << endl;
	*output_file << endl;

	*output_file << "DATA: " << endl << endl;
	*output_file << fixed << "-Number of People: " << input_people_total << endl;
	*output_file << fixed << "-Infectiousness: " << input_infectiousness << endl;
	*output_file << fixed << "-Chance to recover: " << input_recovery_chance << endl;
	*output_file << fixed << "-Infection duration: " << input_infection_duration << endl;
	*output_file << fixed << "-Initial infected: " << input_initial_infected_percentage << "%" << endl;
	*output_file << fixed << "-Map size: " << input_map_side << "x" << input_map_side << endl;

#  endif
}

void print_lengend_and_top_of_table(ofstream* output_file)
{
	cout.precision(2);

	//To Console

	cout << endl;
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

#	ifdef PRINTFILE

	output_file->precision(2);

	*output_file << endl;
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

#	endif

}

void print_tick(int tick, ofstream* output_file)
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
#	ifdef PRINTFILE

	output_file->precision(2);

	*output_file << left << setw(10) << fixed << tick << left << setw(10) << fixed << left << setw(10) << fixed << (double)vector_of_vectors_of_stats[tick][1] / (double)(tick + 1);
	*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][1] * 100 / input_people_total << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][1];
	*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] / (double)(tick + 1) << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] * 100 / input_people_total;
	*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][3] << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2] / (double)(tick + 1);
	*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2] * 100 / input_people_total << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][2];
	*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] / (double)(tick + 1) << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] * 100 / input_people_total;
	*output_file << left << setw(10) << fixed << vector_of_vectors_of_stats[tick][0] << endl;

#	endif
}

char validate_parameters(int argc, char* argv[])
{
	char validation_status = '0';
	int override;
	if (argc != 7)
	{
		cout << endl << "Incorrect ammount of paramenters entered (should be 6)" << endl;
		return '1';
	}
	else
	{
		if (10000000 < atoi(argv[1]) || atoi(argv[1]) < 10)
		{
			cout << endl << "The ammount of people should range from 10 to 10 million, you entered: " << atoi(argv[1]) << endl;
			validation_status = '1';
		}
		if (1.0 < atof(argv[2]) || atof(argv[2]) < 0.0)
		{
			cout << endl << "The infectiouness of the virus should range from 0.0 to 1.0, you entered: " << atof(argv[2]) << endl;
			validation_status = '1';
		}
		if (1.0 < atof(argv[3]) || atof(argv[3]) < 0.0)
		{
			cout << endl << "The chance of recovery should range from 0.0 to 1.0, you entered: " << atof(argv[3]) << endl;
			validation_status = '1';
		}
		if (50 < atoi(argv[4]) || atoi(argv[4]) < 5)
		{
			cout << endl << "The duration of the infection should range from 5 to 50, you entered: " << atoi(argv[4]) << endl;
			validation_status = '1';
		}
		if (10 < atoi(argv[5]) || atoi(argv[5]) < 0)
		{
			cout << endl << "The ammount of initial infected should range from 0 to 10, you entered: " << atoi(argv[5]) << endl;
			validation_status = '1';
		}
		if (100 != atoi(argv[6]) && 500 != atoi(argv[6]) && 1000 != atoi(argv[6]))
		{
			cout << endl << "The map can only be 100x100, 500x500 or 1000x1000, you entered: " << atoi(argv[6]) << "x" << atoi(argv[6]) << endl;
			validation_status = '1';
		}

		if (validation_status == '1')
		{
			cout << endl << "Would you like to run these parameters anyway [1] or would you like to run exp1 (500x500 map, 1m people) [any key]? " << endl;
			cin >> override;
			cin.ignore();
			if (override == 1)
			{
				return '0';
			}
		}

		return validation_status;
	}

}