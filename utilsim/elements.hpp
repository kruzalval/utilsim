#ifndef ELEMENTS_HPP
#define ELEMENTS_HPP

#include "topology.hpp"

// Power system elements models

namespace utilsim {
	class Source : public Element
	{
	private:
		// Settings DB
		static SettingsData SD;
		static SettingsData getData();
		// Calculation interface
		void updateModel();
		void updateTopology();
	public:
		Source(NID* parent);
	};

	class Line : public Element
	{
	private:
		// Settings DB
		static SettingsData SD;
		static SettingsData getData();		
		static SettingsData getLibData();
		// Calculation interface
		void updateModel();
		void updateTopology();
	public:
		static SettingsData LSD;
		Line(NID* parent);
	};

	class Load : public Element
	{
	private:
		// Settings DB
		static SettingsData SD;
		static SettingsData getData();
		// Calculation interface
		void updateModel();
		void updateTopology();
	public:
		Load(NID* parent);
	};
}
#endif