#include "doa.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>

using namespace std;

// ============== FFT IMPLEMENTATION ==============
void fft(vector<complex<double>>& x) {
    int n = x.size();
    if (n <= 1) return;
    
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            swap(x[i], x[j]);
        }
    }
    
    // FFT computation
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2 * M_PI / len;
        complex<double> wlen(cos(angle), sin(angle));
        for (int i = 0; i < n; i += len) {
            complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; j++) {
                complex<double> u = x[i + j];
                complex<double> v = x[i + j + len / 2] * w;
                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

// ============== EIGENDECOMPOSITION ==============
void eigendecomposition(const vector<vector<complex<double>>>& A,
                       vector<double>& eigenvalues,
                       vector<vector<complex<double>>>& eigenvectors) {
    int n = A.size();
    
    // Copy A to working matrix
    vector<vector<complex<double>>> B = A;
    
    // Initialize eigenvectors as identity
    eigenvectors.resize(n, vector<complex<double>>(n, 0.0));
    for (int i = 0; i < n; i++) {
        eigenvectors[i][i] = 1.0;
    }
    
    // Jacobi iteration
    const int max_iter = 100;
    for (int iter = 0; iter < max_iter; iter++) {
        // Find maximum off-diagonal element
        double max_offdiag = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                double mag = abs(B[i][j]);
                if (mag > max_offdiag) {
                    max_offdiag = mag;
                    p = i;
                    q = j;
                }
            }
        }
        
        if (max_offdiag < 1e-10) break;
        
        // Compute rotation angle
        double theta = 0.5 * atan2(2.0 * B[p][q].real(), 
                                   B[q][q].real() - B[p][p].real());
        
        double cos_theta = cos(theta);
        double sin_theta = sin(theta);
        
        // Apply rotation to B
        vector<vector<complex<double>>> newB = B;
        for (int k = 0; k < n; k++) {
            if (k != p && k != q) {
                newB[k][p] = cos_theta * B[k][p] - sin_theta * B[k][q];
                newB[p][k] = conj(newB[k][p]);
                newB[k][q] = sin_theta * B[k][p] + cos_theta * B[k][q];
                newB[q][k] = conj(newB[k][q]);
            }
        }
        newB[p][p] = cos_theta * cos_theta * B[p][p] + 
                     sin_theta * sin_theta * B[q][q] -
                     2.0 * sin_theta * cos_theta * B[p][q].real();
        newB[q][q] = sin_theta * sin_theta * B[p][p] + 
                     cos_theta * cos_theta * B[q][q] +
                     2.0 * sin_theta * cos_theta * B[p][q].real();
        newB[p][q] = 0.0;
        newB[q][p] = 0.0;
        B = newB;
        
        // Apply rotation to eigenvectors
        for (int i = 0; i < n; i++) {
            complex<double> v_p = eigenvectors[i][p];
            complex<double> v_q = eigenvectors[i][q];
            eigenvectors[i][p] = cos_theta * v_p - sin_theta * v_q;
            eigenvectors[i][q] = sin_theta * v_p + cos_theta * v_q;
        }
    }
    
    // Extract eigenvalues (diagonal of B)
    eigenvalues.resize(n);
    for (int i = 0; i < n; i++) {
        eigenvalues[i] = B[i][i].real();
    }
    
    // Sort eigenvalues in ascending order
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int i, int j) { return eigenvalues[i] < eigenvalues[j]; });
    
    vector<double> sorted_eigenvalues(n);
    vector<vector<complex<double>>> sorted_eigenvectors(n, vector<complex<double>>(n));
    for (int i = 0; i < n; i++) {
        sorted_eigenvalues[i] = eigenvalues[idx[i]];
        for (int j = 0; j < n; j++) {
            sorted_eigenvectors[j][i] = eigenvectors[j][idx[i]];
        }
    }
    
    eigenvalues = sorted_eigenvalues;
    eigenvectors = sorted_eigenvectors;
}

// ============== DOA ESTIMATOR IMPLEMENTATION ==============

DOAEstimator::DOAEstimator(int sample_rate, int fft_size) 
    : fs(sample_rate), nfft(fft_size) {
    hop = nfft / 2;
    array_center = {0.0, 0.0}; // Changed default center to origin
}

void DOAEstimator::setMicPositions(const vector<vector<double>>& positions) {
    mic_positions = positions;
    n_mics = positions[0].size();
}

void DOAEstimator::setArrayCenter(const vector<double>& center) {
    array_center = center;
}

vector<vector<vector<complex<double>>>> DOAEstimator::stft(
    const vector<vector<double>>& signals) {
    
    int n_frames = (signals[0].size() - nfft) / hop + 1;
    int n_freq_bins = nfft / 2 + 1;
    
    vector<vector<vector<complex<double>>>> stft_data(
        n_mics, vector<vector<complex<double>>>(
            n_frames, vector<complex<double>>(n_freq_bins)));
    
    // Hann window
    vector<double> window(nfft);
    for (int i = 0; i < nfft; i++) {
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (nfft - 1)));
    }
    
    for (int ch = 0; ch < n_mics; ch++) {
        for (int frame = 0; frame < n_frames; frame++) {
            int start = frame * hop;
            
            vector<complex<double>> fft_data(nfft);
            for (int i = 0; i < nfft; i++) {
                fft_data[i] = signals[ch][start + i] * window[i];
            }
            
            fft(fft_data);
            
            for (int freq = 0; freq < n_freq_bins; freq++) {
                stft_data[ch][frame][freq] = fft_data[freq];
            }
        }
    }
    
    return stft_data;
}

vector<vector<complex<double>>> DOAEstimator::computeCovariance(
    const vector<vector<vector<complex<double>>>>& stft_data,
    int freq_idx) {
    
    int n_frames = stft_data[0].size();
    
    vector<vector<complex<double>>> cov(
        n_mics, vector<complex<double>>(n_mics, 0.0));
    
    for (int frame = 0; frame < n_frames; frame++) {
        vector<complex<double>> freq_data(n_mics);
        for (int ch = 0; ch < n_mics; ch++) {
            freq_data[ch] = stft_data[ch][frame][freq_idx];
        }
        
        for (int i = 0; i < n_mics; i++) {
            for (int j = 0; j < n_mics; j++) {
                cov[i][j] += freq_data[i] * conj(freq_data[j]);
            }
        }
    }
    
    for (int i = 0; i < n_mics; i++) {
        for (int j = 0; j < n_mics; j++) {
            cov[i][j] /= n_frames;
        }
    }
    
    return cov;
}

// ============== FIXED DOA ESTIMATION ==============
pair<double, vector<double>> DOAEstimator::estimateDOA(
    const vector<vector<double>>& signals,
    const vector<double>& /* source_pos */)   // unused
{
    // ----- Validate inputs -------------------------------------------------
    if (signals.empty() || signals[0].empty()) {
        vector<double> empty_spectrum(360, 0.0);
        return {0.0, empty_spectrum};
    }

    int n_channels = signals.size();
    if (n_mics != n_channels) n_mics = n_channels;

    // ----- STFT ------------------------------------------------------------
    auto stft_data = stft(signals);

    // ----- Frequency selection (speech range) ------------------------------
    vector<int> freq_indices;
    for (int f = 20; f < 150; ++f) {
        double freq_hz = f * (double(fs) / nfft);
        if (freq_hz > 200.0 && freq_hz < 3000.0)
            freq_indices.push_back(f);
    }
    if (freq_indices.empty()) {
        vector<double> empty_spectrum(360, 0.0);
        return {0.0, empty_spectrum};
    }

    // ----- Angle grid (1° resolution) --------------------------------------
    vector<double> angle_grid(360);
    for (int i = 0; i < 360; ++i) angle_grid[i] = i;

    const double c = 343.0;
    vector<vector<double>> spectra;
    spectra.reserve(freq_indices.size());

    // ----- Storage for coarse‑angle estimation (best frequencies) ----------
    struct FreqInfo {
        double freq;
        double snr;
        complex<double> phase_x;   // cross‑spectrum mic0–mic2
        complex<double> phase_y;   // cross‑spectrum mic1–mic3
    };
    vector<FreqInfo> best_infos;   // keep top 5 by SNR

    // ----- Process each frequency bin --------------------------------------
    for (int freq_idx : freq_indices) {
        double freq_hz = freq_idx * (double(fs) / nfft);
        if (freq_hz >= fs / 2) continue;

        auto cov = computeCovariance(stft_data, freq_idx);

        // Eigendecomposition for MUSIC
        vector<double> eigvals;
        vector<vector<complex<double>>> eigvecs;
        eigendecomposition(cov, eigvals, eigvecs);

        // SNR estimate
        double snr = eigvals[n_mics-1] / (eigvals[0] + 1e-12);

        // Keep best frequencies for phase‑based ambiguity resolution
        if (snr > 0.5) {   // only if reasonable
            best_infos.push_back({freq_hz, snr, cov[0][2], cov[1][3]});
            // Keep only top 5 by SNR
            sort(best_infos.begin(), best_infos.end(),
                 [](const FreqInfo& a, const FreqInfo& b) { return a.snr > b.snr; });
            if (best_infos.size() > 5) best_infos.pop_back();
        }

        // ----- MUSIC spectrum ----------------------------------------------
        int n_sources = 1;
        vector<int> idx(n_mics);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(),
             [&](int i, int j) { return eigvals[i] < eigvals[j]; });

        int n_noise = n_mics - n_sources;
        if (n_noise < 1) n_noise = 1;
        vector<vector<complex<double>>> noise_subspace(n_mics,
                                                       vector<complex<double>>(n_noise));
        for (int i = 0; i < n_mics; ++i)
            for (int j = 0; j < n_noise; ++j)
                noise_subspace[i][j] = eigvecs[i][idx[j]];

        vector<double> spectrum(angle_grid.size(), 0.0);
        for (size_t a = 0; a < angle_grid.size(); ++a) {
            double theta = angle_grid[a] * M_PI / 180.0;
            double dir_x = cos(theta), dir_y = sin(theta);

            vector<complex<double>> steering(n_mics);
            for (int mic = 0; mic < n_mics; ++mic) {
                double mx = mic_positions[0][mic] - array_center[0];
                double my = mic_positions[1][mic] - array_center[1];
                double delay = -(mx * dir_x + my * dir_y) / c;
                steering[mic] = exp(complex<double>(0, 2 * M_PI * freq_hz * delay));
            }

            complex<double> proj = 0.0;
            for (int i = 0; i < n_noise; ++i) {
                complex<double> dot = 0.0;
                for (int j = 0; j < n_mics; ++j)
                    dot += conj(noise_subspace[j][i]) * steering[j];
                proj += dot * conj(dot);
            }
            spectrum[a] = 1.0 / (abs(proj) + 1e-10);
        }

        // Normalise
        double max_val = *max_element(spectrum.begin(), spectrum.end());
        if (max_val > 1e-10)
            for (auto& v : spectrum) v /= max_val;

        // Frequency weighting (favour mid‑range)
        double weight = 1.0 / (1.0 + abs(freq_hz - 1500.0) / 1000.0);
        for (auto& v : spectrum) v *= weight;

        spectra.push_back(spectrum);
    }

    if (spectra.empty()) {
        vector<double> empty_spectrum(360, 0.0);
        return {0.0, empty_spectrum};
    }

    // ----- Average spectra -------------------------------------------------
    vector<double> avg_spectrum(angle_grid.size(), 0.0);
    for (const auto& spec : spectra)
        for (size_t i = 0; i < avg_spectrum.size(); ++i)
            avg_spectrum[i] += spec[i];
    for (auto& v : avg_spectrum) v /= spectra.size();

    // ----- Find peaks ------------------------------------------------------
    vector<pair<double, double>> peaks;
    for (size_t i = 1; i < avg_spectrum.size() - 1; ++i) {
        if (avg_spectrum[i] > avg_spectrum[i-1] && avg_spectrum[i] > avg_spectrum[i+1])
            peaks.push_back({avg_spectrum[i], angle_grid[i]});
    }
    sort(peaks.begin(), peaks.end(),
         [](const auto& a, const auto& b) { return a.first > b.first; });

    if (peaks.empty()) {
        vector<double> result_spectrum(360, 0.0);
        copy(avg_spectrum.begin(), avg_spectrum.end(), result_spectrum.begin());
        return {0.0, result_spectrum};
    }

    // ----- Ambiguity resolution using phase differences --------------------
    double chosen_angle = peaks[0].second;   // fallback

    // Get the two dominant peaks (should be ~180° apart)
    double peak1 = peaks[0].second;
    double peak2 = (peaks.size() > 1) ? peaks[1].second : fmod(peak1 + 180.0, 360.0);
    // Ensure they are exactly 180° apart (use the stronger one as reference)
    double diff = fmod(peak2 - peak1 + 180.0, 360.0) - 180.0;
    if (abs(diff) > 10.0) peak2 = fmod(peak1 + 180.0, 360.0);

    // Compute coarse angle from best frequencies (if any)
    if (!best_infos.empty()) {
        // Compute radius of array
        double radius = 0.0;
        for (int i = 0; i < n_mics; ++i) {
            double dx = mic_positions[0][i] - array_center[0];
            double dy = mic_positions[1][i] - array_center[1];
            radius = max(radius, sqrt(dx*dx + dy*dy));
        }

        if (radius > 0.001) {
            // Weighted vector average of (cosθ, sinθ) over best frequencies
            double sum_cos = 0.0, sum_sin = 0.0, sum_w = 0.0;
            for (const auto& info : best_infos) {
                double f = info.freq;
                // phase_x = arg(X0 * conj(X2)) = -4π f r cosθ / c
                // phase_y = arg(X1 * conj(X3)) = -4π f r sinθ / c
                double ph_x = arg(info.phase_x);
                double ph_y = arg(info.phase_y);
                double cos_theta = -ph_x * c / (4.0 * M_PI * f * radius);
                double sin_theta = -ph_y * c / (4.0 * M_PI * f * radius);
                // Clamp to valid range
                cos_theta = max(-1.0, min(1.0, cos_theta));
                sin_theta = max(-1.0, min(1.0, sin_theta));
                double w = info.snr;   // weight by SNR
                sum_cos += w * cos_theta;
                sum_sin += w * sin_theta;
                sum_w  += w;
            }
            if (sum_w > 0) {
                double avg_cos = sum_cos / sum_w;
                double avg_sin = sum_sin / sum_w;
                double coarse = atan2(avg_sin, avg_cos) * 180.0 / M_PI;
                if (coarse < 0) coarse += 360.0;

                // Pick the peak closest to coarse (circular distance)
                double d1 = fmod(peak1 - coarse + 180.0, 360.0) - 180.0;
                double d2 = fmod(peak2 - coarse + 180.0, 360.0) - 180.0;
                chosen_angle = (abs(d1) < abs(d2)) ? peak1 : peak2;
            }
        }
    }

    // Ensure angle in [0, 360)
    chosen_angle = fmod(chosen_angle, 360.0);
    if (chosen_angle < 0) chosen_angle += 360.0;

    // ----- Prepare output spectrum (360 values) ----------------------------
    vector<double> result_spectrum(360, 0.0);
    copy(avg_spectrum.begin(), avg_spectrum.end(), result_spectrum.begin());

    return {chosen_angle, result_spectrum};
}