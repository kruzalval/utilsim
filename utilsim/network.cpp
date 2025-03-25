#include "network.hpp"

namespace utilsim
{
	Network::Network() : SIGMA(CSM(0, 0)), T(CSM(0, 0)), J(CV::zeros(0)){
		// Badge initialization
		ID = NID();
		ID.Owner = this;
	}

	Network::~Network() {
		for (auto J : Junctions) {
			delete J;
		}
		for (auto E : Elements) {
			delete E;
		}
		for (auto L : Archive) {
			delete L;
		}
	}

	Junction* Network::insertJunction() {
		// Producing
		Junction* jnt = new Junction(&ID);

		// Saving
		Junctions.push_back(jnt);
		return jnt;
	}

	void Network::print() {
		cout << "Network statistics:\n";
		for (auto e : Elements) {
			e->print();
		}
		for (auto j : Junctions) {
			j->print();
		}
	}

	void Network::compute() {
		// Identifying network state
		bool topologyChanged = false;
		for (auto jnt : Junctions) {
			topologyChanged = topologyChanged || (jnt->getState() == ModifiedState::TOPOLOGY);
		}
		for (auto elem : Elements) {
			topologyChanged = topologyChanged || (elem->getState() == ModifiedState::TOPOLOGY);
		}
		// Updating
		if (topologyChanged) {
			int v_idx = 0;
			for (auto jnt : Junctions) {			
				jnt->index(v_idx);			
			}		
			int i_idx = 0;
			for (auto elem : Elements) {
				elem->index(i_idx);
			}
			SIGMA = CSM(i_idx, v_idx);
			T = CSM(v_idx, i_idx);
			J = CV::zeros(i_idx);
			std::cout << "Calculation. VDOFS: " << v_idx << " IDOFS: " << i_idx << std::endl;
		}
		
		for (auto elem : Elements) {
			if (topologyChanged || (elem->getState() == ModifiedState::PARAMETRIC)) {
				elem->fill(SIGMA,J);
			}			
		}
		for (auto jnt : Junctions) {
			if (topologyChanged || (jnt->getState() == ModifiedState::PARAMETRIC)) {
				jnt->fill(T);
			}			
		}

		CSM L = T * SIGMA;
		CV R = T * J;

		CV RES = L.solve(R);

		//RES.print();

		//std::cout << "SIGMA" << std::endl;
		//SIGMA.print();
		//std::cout << "T" << std::endl;
		//T.print();
		//std::cout << "J" << std::endl;
		//J.print();

		// mapping junction - list of nodes
		// Store indices inside elements
	}


}