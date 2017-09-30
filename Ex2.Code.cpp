#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

/// Функция ReducerMaxTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element =  %d has index =  %d\n\n", maximum->get_reference(), maximum->get_index_reference());
}


/// Функция ReducerMinTest() определяет минимальный элемент массива
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimum element =  %d has index =  %d\n\n", minimum->get_reference(), minimum->get_index_reference());
}

/// Функция ParallelSort() сортирует массив в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void ParallelSort(int *begin, int *end)
{
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

/// CompareForAndCilk_For() выводит на консоль время работы стандартного цикла for, 
/// в котором заполняется случайными значениями и время работы параллельного цикла cilk_for 
void *CompareForAndCilk_For(size_t sz)
{
	duration<double> duration, duration1; /// Перемнные для измерения времени работы цикла for,cilk_for
	cilk::reducer<cilk::op_vector<int>>red_vec; /// Вектор в цикле cilk_for
	vector <int> vector;/// Вектор в цикле for
	/// Время работы цикла for
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (long i = 0; i < sz; ++i)
	{
		vector.push_back(rand() % 25000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	/// Время работы цикла cilk_for
	high_resolution_clock::time_point t3 = high_resolution_clock::now();
	cilk_for(long i = 0; i != sz; ++i)
	{
		red_vec->push_back(rand() % 25000 + 1);
	}
	high_resolution_clock::time_point t4 = high_resolution_clock::now();
	duration1 = (t4 - t3);
	cout << "Duration is: " << duration.count() << " seconds - for loop" << endl;
	cout << "Duration is: " << duration1.count() << " seconds - cilk_for loop" << endl;
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	size_t mass_size; //Размер массива
	cout << "Enter the size of the array: ";
	cin >> mass_size;

	duration<double> duration;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size];
	mass_begin = mass;
	mass_end = mass_begin + sz;
	for (long i = 0; i < sz; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}

	ReducerMaxTest(mass, mass_size); /// Поиск максимального элемента массива
	ReducerMinTest(mass, mass_size); /// Поиск минимального элемента массива

	// Измерение времени функции ParallelSort
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end); /// Сортировка массива
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration = (t2 - t1);

	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size); //Минмальный элемент после сортировки

	cout << "Duration is: " << duration.count() << " seconds - sorted function" << endl;
	CompareForAndCilk_For(mass_size);

	delete[]mass;
	return 0;
}
