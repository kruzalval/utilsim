#include "linalg.hpp"
#include <unsupported/Eigen/MatrixFunctions>

namespace utilsim {
	// Complex double vector
	CV::CV(int numel) {
		// Internal object initialization
		V = Vector<CPX, Eigen::Dynamic>(numel);
	}

	CV& CV::operator<<(CPX val) {
		iterator = 0;
		return *this, val;
	}

	CV& CV::operator,(CPX val) {
		V[iterator++] = val;
		return *this;
	}

	CPX& CV::operator[](int i) {
		return V(i);
	}

	void CV::print() {
		std::cout << V << std::endl << std::endl;
	}

	CV CV::zeros(int numel) {
		CV val = CV(numel);
		val.V.setZero();
		return val;
	}

	int CV::numel() {
		return (int)V.size();
	}

	// Complex double dense matrix
	CDM::CDM(int rows, int cols) {
		// Internal object initialization
		M = MatrixXcd(rows, cols);
	}

	void CDM::print() {
		std::cout << M << std::endl << std::endl;
	}

	int CDM::numel() {
		return (int)(M.rows() * M.cols());
	}

	int CDM::rows() {
		return (int)M.rows();
	}

	int CDM::cols() {
		return (int)M.cols();
	}

	CPX& CDM::operator()(int i, int j) {
		return M(i, j);
	}

	CDM CDM::operator()(std::vector<int> rows, std::vector<int> cols) {
		CDM RES = CDM((int)rows.size(), (int)cols.size());
		for (int k = 0; k < (int)rows.size(); k++) {
			for (int m = 0; m < (int)cols.size(); m++) {
				RES.M(k, m) = this->M(rows[k], cols[m]);
			}
		}
		return RES;
	}

	CDM& CDM::operator<<(CPX val) {
		iterator[0] = 0;
		iterator[1] = 0;
		return *this, val;
	}

	CDM& CDM::operator,(CPX val) {
		// Overflow handling
		if (iterator[0] >= M.rows() || iterator[1] >= M.cols()) {
			return *this;
		}
		// Assignment
		M(iterator[0], iterator[1]) = val;
		// Increment
		iterator[1]++;
		if (iterator[1] ==M.cols()) {
			iterator[1] = 0;
			iterator[0]++;
		}
		return *this;
	}

	CDM& CDM::operator<<(CDM& submat) {
		iterator[0] = 0;
		iterator[1] = 0;
		return *this, submat;
	}
	CDM& CDM::operator,(CDM& submat) {
		// Overflow handling
		if (iterator[0] >=M.rows() || iterator[1] >= M.cols()) {
			return *this;
		}
		// Assignment
		for (int k = 0; k < submat.M.rows(); k++) {
			for (int m = 0; m < submat.M.cols(); m++) {
				M(iterator[0] + k, iterator[1] + m) = submat.M(k,m);
			}
		}		
		// Increment
		iterator[1] += (int)submat.M.cols();
		if (iterator[1] == M.cols()) {
			iterator[1] = 0;
			iterator[0] += (int)submat.M.rows();
		}
		return *this;
	}
	
	CDM expm(CDM matrix) {
		CDM val = CDM(0, 0);
		val.M = matrix.M.exp();
		return val;
	}	

	CDM CDM::operator*(CDM matrix) {
		CDM val = CDM(0, 0);
		val.M = this->M * matrix.M;
		return val;
	}

	CDM CDM::operator*(CPX factor) {
		CDM val = CDM(0, 0);
		val.M = this->M * factor;
		return val;
	}

	CV CDM::operator*(CV mult) {
		CV val = CV(this->rows());
		val.V = this->M * mult.V;
		return val;
	}

	CDM CDM::operator/(CDM matrix) {
		// Right division
		CDM val = CDM(0, 0);
		val.M = this->M * matrix.M.inverse();
		return val;
	}

	CDM CDM::operator|(CDM matrix) {
		// Left division
		CDM val = CDM(0, 0);
		val.M = this->M.inverse() * matrix.M;
		return val;
	}

	CDM CDM::operator+(CDM adder) {
		// Left division
		CDM val = CDM(0, 0);
		val.M = this->M + adder.M;
		return val;
	}

	CDM CDM::operator-(CDM deduct) {
		// Left division
		CDM val = CDM(0, 0);
		val.M = this->M - deduct.M;
		return val;
	}

	CDM CDM::ones(int rows, int cols) {
		CDM mtx = CDM(rows, cols);
		mtx.M = MatrixXcd::Ones(rows, cols);
		return mtx;
	}

	CDM CDM::zeros(int rows, int cols) {
		CDM mtx = CDM(rows, cols);
		mtx.M = MatrixXcd::Zero(rows, cols);
		return mtx;
	}

	CDM CDM::eye(int dim) {
		CDM mtx = CDM(dim, dim);
		mtx.M = MatrixXcd::Identity(dim, dim);
		return mtx;
	}
		
	// Complex double sparse matrix

	CSM::CSM(int rows, int cols) {
		// Internal object initialization
		M = SparseMatrix<CPX>(rows, cols);
	}

	void CSM::setElements(std::vector<int>& rows, std::vector<int>& cols, CDM& mtx) {
		// Triplets preparation
		std::vector<Triplet<CPX>> triplets(mtx.numel());
		for (int i = 0; i < rows.size(); i++) {
			for (int j = 0; j < cols.size(); j++) {
				M.insert(rows[i], cols[j]) = mtx(i, j);				
			}
		}
	}

	CPX& CSM::operator()(int i, int j) {
		return M.insert(i, j);
	}

	CSM CSM::operator*(CSM mult) {
		CSM val = CSM(M.rows(), mult.M.cols());
		val.M = M * mult.M;
		return val;
	}

	CV CSM::operator*(CV mult) {
		CV val = CV(M.rows());
		val.V = M * mult.V;
		return val;
	}

	CV CSM::solve(CV load) {
		CV val = CV(M.rows());
		SparseLU<SparseMatrix<CPX>, COLAMDOrdering<int> >  solver;
		solver.analyzePattern(M);
		solver.factorize(M);
		val.V = solver.solve(load.V);
		return val;
	}

	void CSM::print() {
		std::cout << M << std::endl << std::endl;
		std::cout << "COMPRESSED: " << M.isCompressed() << std::endl;
	}

}
