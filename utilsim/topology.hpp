#ifndef TOPOLOGY_HPP
#define TOPOLOGY_HPP

#include <vector>
#include <map>
#include <iostream>
#include "linalg.hpp"
#include "settings.hpp"
using namespace std;

// NEtwork topology objects

namespace utilsim
{
	// Forwards
	
	// Used to identify changes scope in elements and junctions since last calculation
	enum class ModifiedState {NONE, PARAMETRIC, TOPOLOGY};
	// Identifies calculation mode
	enum class Mode { INTERACTIVE, TRANSACTIONAL };
	struct Conductor;
	class Junction;
	struct Terminal;
	class Port;
	class Element;
	class Network;
	class NID;

	// Junction
	
	struct Conductor {
		// Junction's single connection point
		Junction* Parent;
		int DOF;
		vector<Terminal*> Plugs;
	};

	class Junction {
	public:
		Junction(NID* pid);
		bool isFromNetwork(NID* nptr);
		// Model assembly		
		void connectPort(Port* port);
		void connectTerminal(Terminal* term);
		Conductor* createConductor();
		Conductor* getConductor(Terminal* term);
		void reconnectTerminal(Terminal* term, Conductor* target);
		void disconnectTerminal(Terminal* term);
		// Matrix assembly
		ModifiedState getState();
		void index(int& pos);
		void fill(CSM& T);	
		//Debug
		void print();		
	private:
		Junction() = delete;
		NID* Parent;
		vector<Port*> ConnectedPorts;
		vector<Conductor> Conductors;
		ModifiedState State = ModifiedState::TOPOLOGY;
	};

	// Element

	struct Terminal {
		// Element's port single conductor
		Port* Parent;
		int DOF;
		Conductor* Socket;
		const char* ID;
	};

	class Port {
	public:
		const char* ID;
		Element* Parent;
		Junction* Connection;
		vector<Terminal> Terminals;
		Port(Element* parent, const char* id, vector<const char*> condNames);
		void setTerminals(vector<const char*> condNames);
	};

	class Element {
		/*	Generic network element
			I = S*V + J
			where:
			I - input currents to all ports
			V - port voltages
			S - effective conductivity
			J - source term
		*/
	public:
		Element(NID* pid, vector<const char*> pnames, SettingsData* sd);
		~Element();
		// Model assembly
		void connect(const char* portName, Junction* jnt);
		// Model settings
		template<class T> void setValue(const char* name, void* value) {
			// Performing operation
			unsigned char status = SDR->setValue(SET, name, value);
			// Rizing flags
			if (status & (1 << (int)SettingStatus::Modified)) {
				if (status & (1 << (int)SettingStatus::Topology)) {
					// Topological modification
					State = ModifiedState::TOPOLOGY;
					updateTopology();
				}
				else {
					// Minor change
					State = ModifiedState::PARAMETRIC;
				}
			}
		}
		template<class T> void getValue(const char* name, void* value) {
			SDR->getValue(SET, name, value);
		}		
		// Matrix assembly
		ModifiedState getState();
		void index(int& pos);
		void fill(CSM& sigma, CV& source);		
		// Debug
		void print();
	protected:
		// Settings
		SettingsData* SDR;
		void* SET;
		// Topology		
		NID* Parent;		
		void configurePort(const char* pid, vector<const char*> terms);
		virtual void updateModel() = 0;
		virtual void updateTopology() = 0;
		// Element representation
		CDM S;
		CV J;
		CDM& getS();
		CV& getJ();
	private:
		vector<Port> Ports;
		ModifiedState State = ModifiedState::TOPOLOGY;
	};


	//  Library record
	class Record {
	public:
		Record(void* data, SettingsData* sdr);
	private:
		void* Data;
		SettingsData* Meta;
	};
}

#endif

