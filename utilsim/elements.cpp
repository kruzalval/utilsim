#include "elements.hpp"
using namespace std::complex_literals;
using namespace std;

namespace utilsim {

	// SOURCE ============================================================
	
	// Settings DB
	SettingsData Source::SD = Source::getData();

	SettingsData Source::getData() {
		SettingsData sd = SettingsData("SOURCE");
		vector<const char*> convals = { "YN","Y","D" };

		float defaultFloat = 10000;
		size_t size[] = { 1,1 };
		sd.insertSetting<float>("Voltage", size, false, &defaultFloat);
		sd.insertSetting<float>("RatedVoltage", size, false, &defaultFloat);
		sd.insertSetting<float>("ISC", size, false, &defaultFloat);
		defaultFloat = 10;
		sd.insertSetting<float>("XR", size, false, &defaultFloat);
		sd.insertEnumSetting("Connection", size, true, convals, "Y");
		size[1] = 2;
		float ZN[] = {1e6, 1e6};
		sd.insertSetting<float>("ZN", size, false, ZN);
		return sd;
	}

	// Construction
	Source::Source(NID* parent) : Element(parent, {"P"}, & SD) {
		// Configure ports
		updateTopology();
	}

	void Source::updateTopology() {
		int conCode = 0;		
		SDR->getValue<int>(SET, "Connection", &conCode);
		if (SDR->decodeEnum("Connection", conCode) == "YN") {
			configurePort("P", { "A", "B", "C", "N" });
		}
		else {
			configurePort("P", { "A", "B", "C" });
		}
	}

	void Source::updateModel() {
		// Presentation update
		CDM B = CDM::ones(3, 3);
		CDM rho = CDM::eye(3) * CPX(0.1, 0.1);
		CDM alp = (B / rho) * (CPX(1, 0) / ((CDM::ones(1, 3) / rho) * CDM::ones(3, 1))(0, 0));
		CV V = CV(3);
		V << exp(0.0), exp(2.0 * 3.1415 * 2i / 3.0), exp(2.0 * 3.1415 * 1i / 3.0);
		// A B C
		S = rho | (alp - CDM::eye(3));
		J = (rho | (CDM::eye(3) - alp)) * V;
	}

	// LINE ==============================================================

	SettingsData Line::SD = Line::getData();
	SettingsData Line::LSD = Line::getLibData();

	SettingsData Line::getData() {
		SettingsData sd = SettingsData("LINE");
		float defaultFloat = 10000;
		size_t size[] = { 1,1 };
		sd.insertSetting<float>("Y", size, false, &defaultFloat);
		vector<const char*> vals = { "A","B","C" };
		sd.insertEnumSetting("Z", size, false, vals, "A");
		return sd;
	}

	SettingsData Line::getLibData() {
		SettingsData lsd = SettingsData("LINE");
		float defaultFloat = 10000;
		size_t size[] = { 1,1 };
		lsd.insertSetting<float>("Y", size, false, &defaultFloat);
		vector<const char*> vals = { "A","B","C" };
		lsd.insertEnumSetting("Z", size, false, vals, "A");
		return lsd;
	}

	Line::Line(NID* parent) : Element(parent, {"P","N"}, &SD) {
		// Configure ports
		updateTopology();
	}

	void Line::updateTopology() {
		// TBD - ports configure depending on library setting
		configurePort("P", { "A", "B", "C" });
		configurePort("N", { "A", "B", "C" });
	}

	void Line::updateModel() {
		CDM W = CDM(6, 6);
		CPX w = CPX(2.0 * 3.1415 * 50.0, 0);
		W << 0.0, 0.0, 0.0, 0.4 + w * 1e-3 * 1i, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.4 + w * 1e-3 * 1i, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.4 + w * 1e-3 * 1i,
			w * 1e-7 * 1i, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, w * 1e-7 * 1i, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, w * 1e-7 * 1i, 0.0, 0.0, 0.0;

		CDM X = expm(W * CPX(-2.5, 0.0));

		CDM X1 = X({ 0,1,2 }, { 0,1,2 });
		CDM X2 = X({ 0,1,2 }, { 3,4,5 });
		CDM X3 = X({ 3,4,5 }, { 0,1,2 });
		CDM X4 = X({ 3,4,5 }, { 3,4,5 });

		CDM x1 = ((X2 | X1) * CPX(-1, 0));
		CDM x2 = CDM::eye(3) / X2;
		CDM x3 = X3 * CPX(-1, 0) + X4 / X2 * X1;
		CDM x4 = X4 / X2 * CPX(-1, 0);

		S = CDM(6, 6);
		S << x1, x2, x3, x4;
		J = CV::zeros(6);
	}

	// LOAD ==============================================================

	SettingsData Load::SD = getData();

	SettingsData Load::getData() {
		SettingsData sd = SettingsData("LOAD");
		float defaultFloat = 10000;
		size_t size[] = { 1,1 };
		sd.insertSetting<float>("Y", size, false, &defaultFloat);
		vector<const char*> vals = { "A","B","C" };
		sd.insertEnumSetting("Z", size, false, vals, "A");
		return sd;
	}

	Load::Load(NID* parent) : Element(parent, {"P"}, &SD) {
		// Configure ports
		updateTopology();
	}

	void Load::updateTopology() {
		// TBD
		configurePort("P", { "A", "B", "C" });
	}

	void Load::updateModel() {
		S = CDM(3, 3);
		CPX Z = CPX(10.0, 10.0);
		S << 2.0 / Z, -1.0 / Z, -1.0 / Z,
			-1.0 / Z, 2.0 / Z, -1.0 / Z,
			-1.0 / Z, -1.0 / Z, 2.0 / Z;
		J = CV::zeros(3);
	}
}