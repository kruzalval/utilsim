#include "network.hpp"
using namespace utilsim;

//#include "loadSDN.cpp"
//#include <chrono>
//using namespace utilsim;

//#include <fstream>
//#include <nlohmann/json.hpp>
//using json = nlohmann::json;
void buildFromJSON(Network* N);
void setTest();
void tic();
double toc();

int main()
{
	Network N = Network();
	Element* S1 = N.insertElement<Source>();
	Element* L1 = N.insertElement<Line>();
	Element* LD1 = N.insertElement<Load>();
	Junction* J1 = N.insertJunction();
	Junction* J2 = N.insertJunction();

	S1->connect("P", J1);
	L1->connect("P", J1);
	L1->connect("N", J2);
	LD1->connect("P", J2);

	N.print();

	tic();
	N.compute();
	cout << "Calcultaion took: " << toc() << "ms";

	
	return 0;
}