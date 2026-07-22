#ifndef DOA_HPP
#define DOA_HPP

#include <vector>
#include <complex>
#include <utility>

using namespace std;

// ============== FFT FUNCTIONS ==============
void fft(vector<complex<double>>& x);

// ============== EIGENDECOMPOSITION ==============
void eigendecomposition(const vector<vector<complex<double>>>& A,
                       vector<double>& eigenvalues,
                       vector<vector<complex<double>>>& eigenvectors);

// ============== DOA ESTIMATOR CLASS ==============
class DOAEstimator {
private:
    int fs;
    int nfft;
    int hop;
    int n_mics;
    vector<vector<double>> mic_positions;
    vector<double> array_center;
    
    // STFT computation
    vector<vector<vector<complex<double>>>> stft(
        const vector<vector<double>>& signals);
    
    // Covariance matrix computation
    vector<vector<complex<double>>> computeCovariance(
        const vector<vector<vector<complex<double>>>>& stft_data,
        int freq_idx);
    
public:
    DOAEstimator(int sample_rate = 44100, int fft_size = 512);
    
    void setMicPositions(const vector<vector<double>>& positions);
    void setArrayCenter(const vector<double>& center);
    
    // Main DOA estimation function
    pair<double, vector<double>> estimateDOA(
        const vector<vector<double>>& signals,
        const vector<double>& source_pos);
};

#endif // DOA_HPP