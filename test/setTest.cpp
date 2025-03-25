#include "network.hpp"

using namespace utilsim;

void setTest(){
	Network N = Network();
	Element* S1 = N.insertElement<Source>();
	Element* L1 = N.insertElement<Line>();

	N.appendLibrary<Line>();
	//N.appendLibrary<Source>();
	//S1->print();
	//L1->set("Y", 22.0);

	//S1->set("X", 2.0);

	//float val;
	//S1->get("RatedVoltage", val);
	//cout << "RatedVoltage value: " << val;

}