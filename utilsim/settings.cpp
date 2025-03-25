#include "settings.hpp"
#include <iostream>
#include <algorithm>

namespace utilsim
{
	
	// Settings data =====================================================

	SettingsData::SettingsData(const char* kind) {
		Default = nullptr;
		Kind = kind;
		Data = vector<Meta>();
	}

	void SettingsData::insertEnumSetting(const char* name, size_t size[2], bool topo, vector<const char*> enumVals, const char* default_value) {
		// Converting char default to int
		int defval = 0;
		for (int k = 0; k < enumVals.size(); k++) {
			if (enumVals[k] == default_value) {
				defval = k;
				break;
			}
		}
		// Calling general-purpose
		insertSetting<int>(name, size, topo, &defval);

		// Filling the rest
		Data.rbegin()->EnumValues = enumVals;
	}

	void* SettingsData::getInstance() {
		void* res = malloc(Bytes);
		memcpy(res, Default, Bytes);		
		return res;
	}

	vector<const char*> SettingsData::getNames() {
		vector<const char*> names = vector<const char*>(0);
		names.reserve(Data.size());
		for (auto& s : Data) {
			names.push_back(s.Name);
		}
		return names;
	}

	const char* SettingsData::decodeEnum(const char* setName, int val) {
		for (auto& s : Data) {
			if (setName == s.Name) {
				return s.EnumValues[val];
			}
		}
		return nullptr;
	}

	void SettingsData::print(void* s) {
		cout << "--- " << Kind << " ---" << endl;
		//int idx_scalar, idx_array;
		//idx_scalar = idx_array = 0;
		for (auto& M : Data) {
			cout << M.Name << ": ";
			// Data
			void* set = static_cast<char*>(s) + M.Position;

			if (M.Type == typeid(float)) {
				printSetting<float>(set, M.Size[0], M.Size[1]);
			}
			else if (M.Type == typeid(int)) {
				printSetting<int>(set, M.Size[0], M.Size[1]);
			}
			else if (M.Type == typeid(bool)) {
				printSetting<bool>(set, M.Size[0], M.Size[1]);
			}
			else {
				cout << "Unexpected datatype\n";
				exit(0);
			}
		}
	}

	size_t SettingsData::getIndex(const char* sname) {
		// Returns -1 if not found
		size_t idx = 0;
		for (auto& M : Data) {			
			if (M.Name == sname) {
				return idx;
			}
			idx++;			
		}
		return -1;
	}

}