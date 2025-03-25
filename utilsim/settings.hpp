#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#include <vector>
#include <iostream>

// Extendable element settings configurator

using namespace std;
namespace utilsim
{
	enum class SettingStatus {Unknown, Modified, Topology};

	class SettingsData {
	private:
		const char* Kind;
		struct Meta {
			const type_info& Type;
			const char* Name;
			size_t Size[2];
			bool Topological;
			vector<const char*> EnumValues;
			size_t Position;
			size_t Bytes;
		};
		vector<Meta> Data;
		void* Default;
		size_t Bytes;
	public:
		SettingsData(const char* kind);

		// General-purpose
		template <class T> void insertSetting(const char* name, size_t size[2], bool topo, void* default_value) {
			// Extending metadata
			vector<const char*> enumVals = vector<const char*>(0);
			size_t settingBytes = sizeof(T) * size[0] * size[1];
			Data.push_back({ typeid(T), name, {size[0],size[1]}, topo, enumVals, Bytes, settingBytes });
			// Building extended storage
			void* newData = malloc(Bytes + settingBytes);
			memcpy(newData, Default, Bytes);
			free(Default);
			memcpy(static_cast<char*>(newData) + Bytes, default_value, settingBytes);

			// Saving	
			Default = newData;
			Bytes += settingBytes;
		}

		// Enum-targeted
		void insertEnumSetting(const char* name, size_t size[2], bool topo, vector<const char*> enumVals, const char* default_value);

		// SetGet
		vector<const char*> getNames();

		const char* decodeEnum(const char* setName, int enumVal);

		template <class T> unsigned char setValue(void* data, const char* name, void* value) {
			// Consistency check
			int ns = getIndex(name);
			if (ns == -1) { return 1U << (int)SettingStatus::Unknown; }
			unsigned char status = Data[ns].Topological << (int)SettingStatus::Topology;

			// Trivial rewrite check
			void* setPtr = static_cast<char*>(data) + Data[ns].Position;
			if (!memcmp(setPtr, value, Data[ns].Bytes)) {
				return status;
			}
			status = status & (1U << (int)SettingStatus::Modified);

			// Finally assigning value
			memcpy(setPtr, value, Data[ns].Bytes);
			return status;
		}

		template <class T> unsigned char getValue(void* data, const char* name, void* value) {
			// Consistency check
			int ns = getIndex(name);
			if (ns == -1) { return 1U << (int)SettingStatus::Unknown; }
			unsigned char status = Data[ns].Topological << (int)SettingStatus::Topology;
			// Reading
			memcpy(value, static_cast<char*>(data) + Data[ns].Position, Data[ns].Bytes);
			return status;
		}

		// Factory
		void* getInstance();

		// Debug
		void print(void* s);
		template <class T> void printSetting(void* data, size_t rows, size_t cols) {
			T* castedData = (T*)data;
			cout << "[";
			for (int k = 0; k < rows; k++) {
				cout << "[";
				for (int m = 0; m < cols; m++) {
					// Float
					cout << *(castedData++) << " ";
				}
				cout << "]";
				if (k != rows-1) {
					cout << "], ";
				}
			}
			cout << "]" << endl;
		}
	private:
		size_t getIndex(const char* name);
	};
}
#endif