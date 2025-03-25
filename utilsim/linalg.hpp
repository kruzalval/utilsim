#ifndef LINALG_HPP
#define LINALG_HPP

// Interface to external linear algebra library

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>

typedef std::complex<double> CPX;
using namespace Eigen;

namespace utilsim {
	// Complex double vector ------------------------------
	class CV {
	public:
		// Constructor
		CV(int n = 0);
		// Comma filling
		CV& operator<<(CPX val);
		CV& operator,(CPX val);
		// API
		CPX& operator[](int i);
		int numel();

		// Debug
		void print();

		// Factory
		friend class CDM;
		friend class CSM;
		static CV zeros(int numel);

	private:
		int iterator;
		Vector<CPX, Dynamic> V;
	};

	// Complex double dense matrix ---------------------------
	class CDM {
	public:
		// Constructor
		CDM(int nrows = 0, int ncols = 0);
		// Comma filling	
		CDM& operator<<(CPX val);
		CDM& operator,(CPX val);
		CDM& operator<<(CDM& submat);
		CDM& operator,(CDM& submat);
		friend CDM expm(CDM matrix);
		// API
		CPX& operator()(int i, int j);
		CDM operator()(std::vector<int> rows, std::vector<int> cols);
		
		CDM operator/(CDM den);
		CDM operator|(CDM num);
		CDM operator*(CDM mult);
		CDM operator*(CPX factor);
		CV operator*(CV mult);
		CDM operator+(CDM adder);
		CDM operator-(CDM deduct);
		int numel();
		int rows();
		int cols();
		// Debug
		void print();
		// Factory
		static CDM ones(int rows, int cols);
		static CDM zeros(int rows, int cols);
		static CDM eye(int dim);
	private:
		int iterator[2];
		Matrix<CPX, Eigen::Dynamic, Eigen::Dynamic> M;
	};

	CDM expm(CDM matrix);

	// Complex double sparse matrix --------------------------
	class CSM {
	public:
		// Constructor
		CSM(int nrows, int ncols);
		// Setter
		void setElements(std::vector<int>& rows, std::vector<int>& cols, CDM& mtx);
		CPX& operator()(int i, int j);
		CSM operator*(CSM mult);
		CV operator*(CV mult);
		CV solve(CV load);
		// Debug
		void print();
	private:
		SparseMatrix<CPX> M;
	};
}
#endif