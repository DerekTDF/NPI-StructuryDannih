#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>
#include <windows.h>
#include <cblas.h>
#include <omp.h>

class MatrixMultiplier {
private:
    int n;
    std::vector<float> A, B, C, BT; 
    
public:
    MatrixMultiplier(int size) : n(size) {
        A.resize(n * n);
        B.resize(n * n);
        C.resize(n * n);
        BT.resize(n * n);
    }
    
    // Генерация матриц + транспонирование B
    void generateRandomMatrices() {
        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dis(-1.0f, 1.0f); 
        
        for (int i = 0; i < n * n; ++i) {
            A[i] = dis(gen);
            B[i] = dis(gen);
        }
        
        // Транспонирование B для последовательного доступа к памяти
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                BT[i * n + j] = B[j * n + i];
            }
        }
        
        std::cout << "Матрицы сгенерированы (float), B транспонирована\n";
        std::cout << "Потоков: " << omp_get_max_threads() << "\n"; 
    }
    
    void clearResult() {
        std::fill(C.begin(), C.end(), 0.0f);  // float версия
    }
    
    // ========================================================================
    // ВАРИАНТ 1: Наивный алгоритм
    // ========================================================================
    double naiveMultiplication() {
        clearResult();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                for (int k = 0; k < n; ++k) {
                    C[i * n + j] += A[i * n + k] * B[k * n + j];
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }
    
    // ========================================================================
    // ВАРИАНТ 2: OpenBLAS
    // ========================================================================
    double blasMultiplication() {
        clearResult();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // cblas_sgemm для float (вместо cblas_dgemm для double)
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                    n, n, n, 1.0f, A.data(), n, B.data(), n, 0.0f, C.data(), n);
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }
    
    // ========================================================================
    // ВАРИАНТ 3: Оптимизированный алгоритм
    // ========================================================================
    double optimizedMultiplication() {
        clearResult();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                float sum = 0.0f;  // float версия
                
                for (int k = 0; k < n; ++k) {
                    sum += A[i * n + k] * BT[j * n + k];
                }
                
                C[i * n + j] = sum;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }
    
    double computeComplexity() {
        return 2.0 * std::pow(n, 3);
    }
    
    double computePerformance(double timeSeconds) {
        return (computeComplexity() / timeSeconds) * 1e-6;
    }
    
    bool verifyResult(const std::vector<float>& reference, float epsilon = 1e-3f) { 
        for (int i = 0; i < n * n; ++i) {
            if (std::abs(C[i] - reference[i]) > epsilon) {
                return false;
            }
        }
        return true;
    }
    
    const std::vector<float>& getResult() const { return C; }
};

void setupConsole() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

int main() {
    setupConsole();
    
    const int MATRIX_SIZE = 2048;
    
    std::cout << "==========================================================\n";
    std::cout << "   Умножение матриц " << MATRIX_SIZE << "x" << MATRIX_SIZE << " (float)\n";
    std::cout << "==========================================================\n\n";
    
    MatrixMultiplier m(MATRIX_SIZE);
    m.generateRandomMatrices();
    
    size_t mem_mb = (size_t)MATRIX_SIZE * MATRIX_SIZE * 4 / (1024 * 1024);
    std::cout << "Размер одной матрицы: " << mem_mb << " MB\n";
    std::cout << "Всего памяти: " << mem_mb * 4 << " MB\n\n";
    
    double complexity = m.computeComplexity();
    std::cout << "Сложность: " << std::scientific << complexity << " операций\n";
    
    // ВАРИАНТ 1
    std::cout << "\n[1/3] Наивный алгоритм...\n";
    double t1 = m.naiveMultiplication();
    double mf1 = m.computePerformance(t1);
    std::vector<float> r1 = m.getResult();
    std::cout << "  Время: " << std::fixed << t1 << " сек\n";
    std::cout << "  Производительность: " << mf1 << " MFlops\n";
    
    // ВАРИАНТ 2
    std::cout << "\n[2/3] BLAS (OpenBLAS)...\n";
    double t2 = m.blasMultiplication();
    double mf2 = m.computePerformance(t2);
    std::vector<float> r2 = m.getResult();
    std::cout << "  Время: " << t2 << " сек\n";
    std::cout << "  Производительность: " << mf2 << " MFlops\n";
    
    // ВАРИАНТ 3
    std::cout << "\n[3/3] Оптимизированный...\n";
    double t3 = m.optimizedMultiplication();
    double mf3 = m.computePerformance(t3);
    std::cout << "  Время: " << t3 << " сек\n";
    std::cout << "  Производительность: " << mf3 << " MFlops\n";
    
    // Проверка
    bool ok = m.verifyResult(r2, 1e-3f);
    std::cout << "\nПроверка: " << (ok ? "OK" : "FAIL") << "\n";
    
    // Итог
    double percent = (mf3 / mf2) * 100.0;
    std::cout << "\n==========================================================\n";
    std::cout << "ОТНОШЕНИЕ К BLAS: " << percent << "%\n";
    
    if (percent >= 30.0) {
        std::cout << "✓ ТРЕБОВАНИЕ ВЫПОЛНЕНО!\n";
    } else {
        std::cout << "✗ Требование не выполнено\n";
    }
    std::cout << "==========================================================\n";

    std::cout << "Лабораторная работа по структурам данных №2";
    std::cout << "Выполнил: Сиренко Владимир Владимирович 090304-РПИа-025";
    std::cout << "\nНажмите Enter...";
    std::cin.ignore();
    std::cin.get();
    
    return 0;

}

//g++ test.cpp -o test.exe -I"C:\WINDOWS\system32\vcpkg\installed\x64-mingw-dynamic\include\openblas" -L"C:\WINDOWS\system32\vcpkg\installed\x64-mingw-dynamic\lib" -lopenblas -fopenmp -mavx -mfma -O3  -march=native -mtune=native -funroll-loops -ffast-math 
