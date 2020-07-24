#include <iostream>
#include <omp.h>      // this library must be included to parallelize your code using OpenMP
#include <stdio.h>


// declaring the functions here allows the main function to call the other function while still being the first function that is written
int PrintHelloMultipleProcessors();
int UnderstandingVariables();
int UnderstandingBarriers();
int SingleMaster();
int SumCriticalInefficient();
int pseudosums();
int ForLoopsAndReduction();
int TwoCritical();


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////                                                                                                       //////
//////                                                                                                       //////
//////                                              Main function                                            //////
//////         Instructions: look at all the function written after the main function and uncomment          //////
//////         out any functions written in the main function that you want to see the behavior of.          //////
//////                                                                                                       //////
//////                                                                                                       //////
//////         To use OpenMP in Visual Studio: right click on project "parallelpractice1" ->                 //////
//////         "properties" -> "C/C++" -> "language" -> "OpenMP support" -> set to yes                       //////
//////                                                                                                       //////  
//////         If you get the error "two_phase name lookup is not suppported": go to project                 //////
//////         properties -> "C/C++" -> "command line" ->  Add /Zc:twoPhase- in "Additional Options"         //////
//////                                                                                                       //////
//////         The purpose of this document is to practice the basics of OpenMP and understand               //////
//////         some of the fundamental documentation                                                         //////
//////                                                                                                       //////
//////                                                                                                       //////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int main() {

	PrintHelloMultipleProcessors();

	UnderstandingVariables();

	//UnderstandingBarriers();

	//SingleMaster();

	//SumCriticalInefficient();

	//pseudosums();

	//ForLoopsAndReduction();

	//TwoCritical();

	return 0;

}




///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// Function to print hello world from different processors ///////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int PrintHelloMultipleProcessors()
{

	omp_set_num_threads(omp_get_num_procs());
	int num = omp_get_num_procs();
	printf("total number of cores this program has access to: %d \n", num);


	// parallel statements (printing) will happen num_threads times
	// how many times it printed is equal to the number of threads that were launched
#pragma omp parallel default(none)
	{
		printf("Hello, World, from thread # % d !\n", omp_get_thread_num());
	}

	printf("\n \n");

	return 0;

}





///////////////////////////////////////////////////////////////////////////////////////////////
///////////////// Function to understand various private and shared variables /////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
#define N 13
using namespace std;

// function for printing the variables
void printVar(int a[], int b, int c, int d, int m) {
	cout << "b = " << b << ", c = " << c << ", d = " << d << ", m = " << m << endl;
	cout << "a = ";
	for (int i = 0; i < N; i++)
		cout << a[i] << ", ";
	cout << endl;
}

// main function 
int UnderstandingVariables()
{
	int i;
	int a[N];
	int b = 0, c = 0, d = 0, m = 0;

	cout << "Before parallel for loop, here are the values stored in the variables" << endl;
	printVar(a, b, c, d, m);

	// private: another environment (like a function)
	// firstprivate: initialized with the value that it had before the parallel region
	// lastprivate: private version of whichever thread executes the final iteration  
	//              ↑ (for a for loop, this should always be the last iteration, NOT which iteration gets completed last)
	// shared: all threads within a team access the same storage area for shared variables
	// default(none) forces a programmer to explicitly specify the data-sharing attributes of all variables

	cout << "Now I'm going to jump into the parallel region" << endl;

#pragma omp parallel for default(none) private(i,b) firstprivate(c) lastprivate(d) shared(m,a) 
	for (i = 0; i < N; i++) {
		// b and d must be initialized
		b = 100;
		d = 10;
		printf("Thread %d (of %d total threads), iteration %d: b = %d, c = %d, d = %d, m = %d\n", omp_get_thread_num(), omp_get_num_threads(),
			i, b, c, d, m);
		a[i] = omp_get_thread_num();
		b = omp_get_thread_num();
		c = omp_get_thread_num();
		d = omp_get_thread_num();  //when printed below, d should be associated with the last iteration in the for loop (i = N - 1)
		m = omp_get_thread_num();  //when printed below, m should be associated with the iteration in the thread that was last to finish
	}
	cout << "After parallel for loop" << endl;
	printVar(a, b, c, d, m);

	return 0;
}









///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Function to understand barriers ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int UnderstandingBarriers() {

	// BONUS: note that if I run this line of code outside of the parallel section, there is only one thread, so it will be 1
	int numt_outside = omp_get_num_threads();

	printf("The number of threads is: %d \n", numt_outside);



	// BONUS 2: before I start the main function, if I use: #pragma omp threadprivate(tid) then if I define tid when I split once, 
	//          then if I use pragma to split into multiple threads again, tid will be saved with that specific thread for when
	//          I split again and I don't have to define tid again


	int numt;
	int tid;


	// Without the barrier, numt might not have been computed when some of the print statements get printed so some of the
	// threads would print the wrong number for numt
#pragma omp parallel shared(numt) private(tid)
	{
		tid = omp_get_thread_num();
		if (tid == 0) {
			numt = omp_get_num_threads();
		}


		printf("Hello world from thread %d of %d. \n", tid, numt);
	}


	printf("\n");


	// with the barrier, numt will be right for all of the prints because we've told the code to wait until all are finished,
	// so tid must have been computed when all of the prints happen
#pragma omp parallel shared(numt) private(tid)
	{
		tid = omp_get_thread_num();
		if (tid == 0) {
			numt = omp_get_num_threads();
		}

		// Barriers specify a point in the code where each thread must wait until all threads arrive (use inside a parallel region)

#pragma omp barrier

		printf("Hello world from thread %d of %d. \n", tid, numt);
	}


	printf("\n \n");

	return 0;

}










///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Function to understand "single" and "master" constructs //////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int SingleMaster() {


	int numt;
	int tid;

#pragma omp parallel shared(numt) private(tid)
	{

		tid = omp_get_thread_num();
#pragma omp single // this part will only be implemented by one thread
		{
			numt = omp_get_num_threads();
		} // each thread will wait here until all threads have finished this line (synchronized) then they will all continue

		printf("Hello world from thread %d of %d. \n", tid, numt);
	}


	// In the above code, there is a race condition because you want to be sure that numt has been calculated before you print...
	// but if there wasn't a race condition and whatever needed to be calculated by a single thread wasn't used by other threads,
	// use "#pragma omp single nowait", this makes it so that all other threads don't have to wait for that part to be computed.

	// Or if I wanted the master thread to do this (thread 0), I could use "#pragma omp master" 
	// and instead of the first one to get to this point, it would be the master thread completing this task.

	printf("\n \n");

	return 0;


}











///////////////////////////////////////////////////////////////////////////////////////////////
////////////// These global functions will be used in the next couple of functions ////////////
///////////////////////////////////////////////////////////////////////////////////////////////
#define ARR_SIZE 300000000
int a[ARR_SIZE];







///////////////////////////////////////////////////////////////////////////////////////////////
//////////////// Function to compute a sum using "critical" (HIGHLY INEFFICIENT) //////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int SumCriticalInefficient() {

	int i, tid, numt, sum = 0;
	double t1, t2;

	for (i = 0; i < ARR_SIZE; i++) {
		a[i] = 1;
	}


	//sum up the array sequentially and time how long it takes
	t1 = omp_get_wtime();
	for (i = 0; i < ARR_SIZE; i++) {
		sum += a[i];
	}
	t2 = omp_get_wtime();

	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);



	sum = 0;

	t1 = omp_get_wtime();
	//now do the sum in parallel
#pragma omp parallel default(shared) private(i, tid)
	{
		tid = omp_get_thread_num();
		numt = omp_get_num_threads();

		int from, to;

		// distribute the work
		from = (ARR_SIZE / numt) * tid;
		to = (ARR_SIZE / numt) * (tid + 1) - 1;
		if (tid == numt - 1) {
			to = ARR_SIZE - 1; // because total space may not be perfectly divisible by numt
		}
		printf("Hello from %d of %d, my range is from %d to %d \n", tid, numt, from, to);
		for (i = from; i <= to; i++) {
#pragma omp critical
			sum += a[i];                // any code in a critical section can only be done by one thread at a time. 
		}                               // Critical is VERY TIME INTENSIVE so you should only use it a few times in your code.

	}
	t2 = omp_get_wtime();

	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);


	printf("\n \n");

	return 0;

}









///////////////////////////////////////////////////////////////////////////////////////////////
//////////////// Function to compute a sum using pseudosums (more efficient) //////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int pseudosums() {

	int i, tid, numt, sum = 0;
	double t1, t2;

	for (i = 0; i < ARR_SIZE; i++) {
		a[i] = 1;
	}

	//sum up the array sequentially and time how long it takes
	t1 = omp_get_wtime();
	for (i = 0; i < ARR_SIZE; i++) {
		sum += a[i];
	}
	t2 = omp_get_wtime();

	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);





	sum = 0;

	// now do the sum in parallel
	t1 = omp_get_wtime();
#pragma omp parallel default(shared) private(i, tid)
	{
		tid = omp_get_thread_num();
		numt = omp_get_num_threads();

		int from, to, psum = 0;

		// distribute the work. Doing it this way takes advantage of the cashe, as opposed to distributing noncontiguous iterations to threads
		from = (ARR_SIZE / numt) * tid;
		to = (ARR_SIZE / numt) * (tid + 1) - 1;
		if (tid == numt - 1) {
			to = ARR_SIZE - 1; // because total space may not be perfectly divisible by numt
		}
		for (i = from; i <= to; i++) {
			psum += a[i];
		}

		// now we synchronize to sum up the partial sums
#pragma omp critical 
		sum += psum;

	}
	t2 = omp_get_wtime();

	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);

	printf("\n \n");

	return 0;

}
// Still not the best though... there is something even better in another function... see reduction in the next function









///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Function to use pragma for loop and sum using reduction //////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int ForLoopsAndReduction() {

	int i, sum = 0;
	double t1, t2;

	for (i = 0; i < ARR_SIZE; i++) {
		a[i] = 1;
	}

	//sum up the array and time how long it takes
	t1 = omp_get_wtime();
	for (i = 0; i < ARR_SIZE; i++) {
		sum += a[i];
	}
	t2 = omp_get_wtime();

	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);

	sum = 0;




	// sum up the array using the pragma for
	t1 = omp_get_wtime();
#pragma omp parallel default(shared) 
	{
		int psum = 0;

#pragma omp for // for loop is distributed throughout the threads (also ensures that variable i is made to be private for each thread)
		for (i = 0; i < ARR_SIZE; i++) {        // for nested for loops, look this up online
			psum += a[i];
		}

#pragma omp critical
		sum += psum;

	}
	t2 = omp_get_wtime();
	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);






	// or an even easier way to implement a for loop... 
	sum = 0;
	t1 = omp_get_wtime();
#pragma omp parallel default(shared) 
	{
#pragma omp for reduction(+: sum)       // could have the reduction(+: sum) next to "#pragma omp parallel default(shared) " instead
		for (i = 0; i < ARR_SIZE; i++) {
			sum += a[i];
		}
	}
	t2 = omp_get_wtime();
	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);




	// or an even easier way to code it up, doesn't need to be imbedded into a parallel region (FASTEST ONE)
	sum = 0;
	t1 = omp_get_wtime();

#pragma omp parallel for reduction(+: sum)      
	for (i = 0; i < ARR_SIZE; i++) {
		sum += a[i];
	}
	t2 = omp_get_wtime();
	printf("Sum of the array elements = %d. time = %g \n", sum, t2 - t1);


	printf("\n \n");

	return 0;

}

// REDUCTION INFO:  reduction  (operator : variable_name)
// possible reduction operators: +, -, *, &, |, ^, &&, ||, max, min (although max and min are not on Visual Studio 2019)
// inside the for loop, as above, use      variable_name (operator)= other_variable_name








///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// Function to show usefulness of two critical sections ////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int TwoCritical() {

	omp_set_num_threads(30);

	int i, sum = 0, prod = 1;
	double t1, t2;

	for (i = 0; i < ARR_SIZE; i++) {
		a[i] = 1;
	}

	int psum = 0, pprod = 1;

#pragma omp parallel default(shared)
	{
		psum = 0;
		pprod = 1;

#pragma omp parallel for
		for (i = 0; i < ARR_SIZE; i++) {
			psum += a[i];
			pprod *= a[i];
		}


		// since the sum and the product don't interfere with each other, can create two critical section that will not stop each other from
		// running and will allow your code to run faster than if you had a single critical section with both the sum and the product

		// intertwining won't happen a lot but uncomment out the for loops and you'll probably see them intertwine

#pragma omp critical (section1) 
		{
			int j;
			printf("In CS 1\n");
			//for (j = 1; j < ARR_SIZE; j++);
			sum += psum;
			printf("out CS 1\n");
		}

#pragma omp critical (section2)
		{
			int j;
			printf("In CS 2\n");
			//for (j = 1; j < ARR_SIZE; j++);
			prod *= pprod;
			printf("out CS 2\n");
		}


	}

	printf("\n \n");

	return 0;

}
