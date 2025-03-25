#include "topology.hpp"
#include "network.hpp"
#include "settings.hpp"

namespace utilsim
{
	// JUNCTION ==========================================================

	Junction::Junction(NID* pid) {
		Parent = pid;
		Conductors.reserve(4);
	}

	bool Junction::isFromNetwork(NID* nptr) {
		return nptr == Parent;
	}

	// Modification interface

	void Junction::connectPort(Port* prt) {
		// Adding to reference storage
		ConnectedPorts.push_back(prt);
		// Delayed processing avoided - expected immediate structure modification
		// Implementing default mapping on connection
		for (int k = Conductors.size(); k < prt->Terminals.size(); k++) {
			createConductor();
			cout << k << endl;
		}
		cout << this << "\n";
		// Registering in default order
		for (int k = 0; k < Conductors.size(); k++) {
			Conductors[k].Plugs.push_back(&prt->Terminals[k]);
			cout << "inter\n";
			prt->Terminals[k].Socket = &Conductors[k];
		}
	}
	
	Conductor* Junction::createConductor() {
		Conductors.push_back(Conductor());
		State = ModifiedState::TOPOLOGY;
		return &Conductors[Conductors.size() - 1];
	}

	Conductor* Junction::getConductor(Terminal* targetTerm) {
		// Searh
		for (auto& cond : Conductors) {
			for (auto& term : cond.Plugs) {
				if (term == targetTerm) {
					return &cond;
				}
			}
		}
		// Default
		return nullptr;
	}

	void Junction::connectTerminal(Terminal* term) {
		// Checking if terminal belongs to one of connected ports
		for (auto prt : ConnectedPorts) {
			if (term->Parent == prt) {
				// It does
				// Trying to find conductor not used by current port
				Conductor* cptr = nullptr;
				for (auto& cond : Conductors) {
					// Start value
					bool used = false;
					// Check
					for (auto& con : cond.Plugs) {
						if (con->Parent == prt) {
							// The Terminal already connected!
							if (con == term) {
								return;
							}
							// Conductor used
							used = true;
							break;
						}
					}
					// Free conductor found
					if (~used) {
						cptr = &cond;
						break;
					}
				}
				// If all conductors occupied
				if (cptr == nullptr) {
					cptr = createConductor();
				}
				// Finalization
				cptr->Plugs.push_back(term);
				term->Socket = cptr;
			}
		}
	}

	void Junction::disconnectTerminal(Terminal* term) {
		// Overengineered?
		// 
		// Disconnects single terminal and controlls resulting junction consistency
		for (auto it_cond = Conductors.begin(); it_cond < Conductors.end(); it_cond++) {			
			for (auto it_term = it_cond->Plugs.begin(); it_term < it_cond->Plugs.end(); it_term++) {
				if (*it_term == term) {
					// Removing
					it_cond->Plugs.erase(it_term);
					term->Socket = nullptr;
					// Controlling empty conductors
					if (it_cond->Plugs.empty()) {
						Conductors.erase(it_cond);
						State = ModifiedState::TOPOLOGY;
					}
					// Termination
					return;
				}
			}
			
		}
		
	}

	void Junction::reconnectTerminal(Terminal* term, Conductor* target) {
		// Disconnecting
		disconnectTerminal(term);
		
		// Reconnecting
		for (auto& cond : Conductors) {
			// Extending
			if (&cond == target) {
				cond.Plugs.push_back(term);
				term->Socket = target;
			}			
		}
	}

	// Calculation engine interface
	ModifiedState Junction::getState() {
		return State;
	}

	void Junction::index(int& idx) {
		// Filling voltage DOFs
		for (auto& cond : Conductors) {
			cond.DOF = idx++;
		}
	}

	void Junction::fill(CSM& T) {
		for (auto& cond : Conductors) {
			for (auto& term : cond.Plugs) {
				T(cond.DOF, term->DOF) = CPX(1, 0);
			}
		}
	}

	// Debug

	void Junction::print() {
		//for (auto cond = CondMap.begin(); cond != CondMap.end(); cond++) {
		//	std::cout << cond->first << "=>" << cond->second << std::endl;
		//}
	}



	// PORT ==============================================================

	Port::Port(Element* parent, const char* id, vector<const char*> conds) {
		Parent = parent;
		Connection = nullptr;
		ID = id;
		setTerminals(conds);
	}

	void Port::setTerminals(vector<const char*> conds) {

		// Renaming intersection
		for (int k = 0; k < Terminals.size() && k < conds.size(); k++) {
			Terminals[k].ID = conds[k];
		}

		// Adding missing terminals
		for (int k = Terminals.size(); k < conds.size(); k++) {
			// Common
			Terminals.push_back(Terminal());
			Terminals.back().ID = conds[k];
			Terminals.back().Parent = this;
			if (Connection != nullptr) {
				// New terminal requires connection!
				Connection->connectTerminal(&*Terminals.rbegin());
			}
		}

		// Removing excessive terminals
		for (int k = Terminals.size()-1;  k >= (int)conds.size(); k--) {
			Connection->disconnectTerminal(&Terminals[k]);
			Terminals.pop_back();
		}
	}

	
	
	// ELEMENT ===========================================================

	Element::Element(NID *parent, vector<const char*> pnames, SettingsData *sd) : SET(sd->getInstance()) {
		Parent = parent;
		SDR = sd;
		// Creating default ports (fixed for given element type)
		Ports = vector<Port>();
		Ports.reserve(pnames.size());
		for (int k = 0; k < pnames.size(); k++) {
			Ports.push_back(Port(this, pnames[k], {}));
		}
	}

	Element::~Element() {
		delete SET;
	}

	CDM& Element::getS() {
		return S;
	}

	CV& Element::getJ() {
		return J;
	}

	void Element::connect(const char* portName, Junction* jnt) {
		// Validation
		if (!jnt->isFromNetwork(Parent)) {
			// Attempt to connect between different networks
			return;
		}
		// Identifying port pointer
		for (auto& prt : Ports) {
			if (prt.ID == portName) {
				prt.Connection = jnt;
				jnt->connectPort(&prt);
				break;
			}
		}
	}

	ModifiedState Element::getState() {
		return State;
	}

	void Element::index(int& idx) {
		for (auto &prt : Ports) {
			for (auto term = prt.Terminals.begin(); term != prt.Terminals.end(); term++) {
				term->DOF = idx++;
			}
		}
	}

	void Element::fill(CSM& sigma, CV& source) {
		// Matrices update
		if (State != ModifiedState::NONE) {
			updateModel();
		}

		// Output writing
		// 
		// Consider persistent storage for indices
		vector<int> i_index;
		vector<int> v_index;
		i_index.reserve(S.cols());
		v_index.reserve(S.cols());
		for (auto prt : Ports) {
			for (auto term = prt.Terminals.begin(); term != prt.Terminals.end(); ++term) {
				i_index.push_back(term->DOF);
				v_index.push_back(term->Socket->DOF);
			}
		}

		// Sigma matrix update
		sigma.setElements(i_index, v_index, S);
		// Source term update
		for (int i = 0; i < J.numel(); i++) {
			source[i_index[i]] = J[i];
		}
		// Status
		State = ModifiedState::NONE;
	}

	void Element::configurePort(const char* pid, vector<const char*> terms) {
		for (auto& prt : Ports) {
			if (prt.ID == pid) {
				// The one

				// Checking if topology reset required
				if (prt.Terminals.size() != terms.size()) {
					State = ModifiedState::TOPOLOGY;
				}

				// Updating port
				prt.setTerminals(terms);

				return;
			}
		}
	}


	void Element::print() {
		//std::cout << Type << std::endl;
		SDR->print(SET);

		//std::cout << "[";

		//std::vector<int> i_index;
		//std::vector<int> v_index;
		//i_index.reserve(S.cols());
		//v_index.reserve(S.cols());
		/*
		for (auto prt : Ports) {
			for (auto term = prt.TerminalMap.begin(); term != prt.TerminalMap.end(); term++) {
				std::cout << term->second << ",";
			}
		}
		*/
		//for (auto ni : i_index) {
		//	std::cout << ni << ", ";
		//}
		//std::cout << "]" << std::endl;
	}	

	// RECORD ============================================================
	Record::Record(void* data, SettingsData* sdr) : Data(data), Meta(sdr) {

	}



}