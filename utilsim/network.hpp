#ifndef NETWORK_HPP
#define NETWORK_HPP
#include "elements.hpp"

// Network model database

namespace utilsim
{

	class Network;

	// Network identification badge
	class NID {
	private:
		Network* Owner;		
		friend class Network;
	};

	// Network objects database
	class Network {
	public:		
		Network();
		~Network();
		template<class T> T* insertElement() {
			// Constructing
			Element* elem = new T(&ID);
			// Saving
			Elements.push_back(elem);
			return (T*)elem;
		}
		template<class T> Record* appendLibrary() {
			// Constructing	
			void* data = T::LSD.getInstance();
			Record* R = new Record(data, &T::LSD);
			// Saving
			Archive.push_back(R);
			return R;
		}
		Junction* insertJunction();
		void compute();
		void print();
	private:
		NID ID;
		vector<Element*> Elements;
		vector<Junction*> Junctions;
		vector<Record*> Archive;
		CSM SIGMA;
		CSM T;
		CV J;
	};
}

#endif