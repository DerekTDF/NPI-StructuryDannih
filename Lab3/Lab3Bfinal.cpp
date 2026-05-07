#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <io.h>
#include <fcntl.h>
#include <stdexcept>

#pragma execution_character_set("utf-8")
#pragma warning(disable: 4996) // Чтобы MSVC не жаловался

using namespace std;
using namespace chrono;

// Инверсия типа ребра: К <-> О
inline wchar_t invert(wchar_t c) { return c == L'К' ? L'О' : L'К'; }

// Узел односвязного списка
struct Node {
    wchar_t data;
    Node* next;
    Node(wchar_t d, Node* n = nullptr) : data(d), next(n) {}
};

// Очистка памяти списка
void freeList(Node* h) { while (h) { Node* t = h; h = h->next; delete t; } }

// Валидация входной строки: только К/О и длина вида 2^n - 1
bool isValidCreaseStr(const wstring& s) {
    if (s.empty()) return false;
    size_t len = s.length();
    for (wchar_t c : s) if (c != L'К' && c != L'О') return false;
    size_t p = len + 1;
    return (p & (p - 1)) == 0; // Побитовая проверка на степень двойки
}

// 1: ДИНАМИЧЕСКИЙ МАССИВ
void runB_Array(const wstring& input) {
    auto t1 = steady_clock::now();
    size_t len = input.length();
    wchar_t* arr = new wchar_t[len];
    for (size_t i = 0; i < len; ++i) arr[i] = input[i];

    wstring folds; folds.reserve(len);
    size_t offset = 0, cur_len = len;
    bool valid = true;

    // Последовательное "вычленение" центра и усечение левой части через смещение указателя
    while (cur_len > 0) {
        size_t mid = cur_len / 2;
        wchar_t center = arr[offset + mid];
        folds += (center == L'О') ? L'П' : L'З';

        for (size_t i = 0; i < mid; ++i) {
            if (arr[offset + i] != invert(arr[offset + cur_len - 1 - i])) {
                valid = false; break;
            }
        }
        if (!valid) break;
        offset += mid + 1; // Переход к правой половине
        cur_len = mid;
    }
    delete[] arr;

    double ms = duration_cast<microseconds>(steady_clock::now() - t1).count() / 1000.0;
    if (valid) wcout << L"[Массив] " << ms << L" мс | Результат: " << folds << L"\n\n";
    else       wcout << L"[Массив] " << ms << L" мс | Ошибка: последовательность некорректна.\n\n";
}

// ФАЗА 2: СВЯЗАННЫЙ СПИСОК
void runB_List(const wstring& input) {
    auto t1 = steady_clock::now();
    Node* head = nullptr, * tail = nullptr;
    for (wchar_t c : input) {
        Node* n = new Node(c);
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
    }

    wstring folds; folds.reserve(input.length());
    size_t cur_len = input.length();
    bool valid = true;

    while (cur_len > 0 && head) {
        size_t mid = cur_len / 2;
        Node* center = head;
        for (size_t i = 0; i < mid; ++i) center = center->next; // Поиск центра

        folds += (center->data == L'О') ? L'П' : L'З';

        // Проверка симметрии: левая часть == инвертированная правая
        vector<wchar_t> right(mid);
        Node* pR = center->next;
        for (size_t i = 0; i < mid; ++i) { right[i] = pR->data; pR = pR->next; }

        bool sym = true;
        Node* pL = head;
        for (size_t i = 0; i < mid; ++i) {
            if (pL->data != invert(right[mid - 1 - i])) { sym = false; break; }
            pL = pL->next;
        }
        if (!sym) { valid = false; break; }

        // удаление левой части и центра, правая становится новой головой
        Node* new_head = center->next;
        while (head != new_head) { Node* tmp = head; head = head->next; delete tmp; }
        head = new_head;
        cur_len = mid;
    }
    freeList(head);

    double ms = duration_cast<microseconds>(steady_clock::now() - t1).count() / 1000.0;
    if (valid) wcout << L"[Связ.список] " << ms << L" мс | Результат: " << folds << L"\n\n";
    else       wcout << L"[Связ.список] " << ms << L" мс | Ошибка: последовательность некорректна.\n\n";
}

// ФАЗА 3: STL VECTOR
void runB_Vector(const wstring& input) {
    auto t1 = steady_clock::now();
    vector<wchar_t> creases(input.begin(), input.end());
    wstring folds; folds.reserve(creases.size());
    bool valid = true;

    while (!creases.empty()) {
        size_t mid = creases.size() / 2;
        wchar_t center = creases[mid];
        folds += (center == L'О') ? L'П' : L'З';

        // Проверка физической симметрии
        bool sym = true;
        for (size_t i = 0; i < mid; ++i) {
            if (creases[i] != invert(creases[creases.size() - 1 - i])) {
                sym = false; break;
            }
        }
        if (!sym) { valid = false; break; }

        // Переход к правой половине: erase сдвигает данные, оставляя нужный фрагмент
        creases.erase(creases.begin(), creases.begin() + mid + 1);
    }

    double ms = duration_cast<microseconds>(steady_clock::now() - t1).count() / 1000.0;
    if (valid) wcout << L"[STL Vector] " << ms << L" мс | Результат: " << folds << L"\n\n";
    else       wcout << L"[STL Vector] " << ms << L" мс | Ошибка: последовательность некорректна.\n\n";
}

int main() {
    // консоль в UTF-16 
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    wcout << L"Программа Б: последовательность рёбер -> последовательность сгибов\n";
    wcout << L"Выоплнил: РПИа-о25 Сиренко Владимир Владимирович\n";

    for (int phase = 0; phase < 3; ++phase) {
        if (phase == 0)      wcout << L"=== ФАЗА 1: ДИНАМИЧЕСКИЙ МАССИВ ===\n";
        else if (phase == 1) wcout << L"=== ФАЗА 2: СВЯЗАННЫЙ СПИСОК ===\n";
        else                 wcout << L"=== ФАЗА 3: STL VECTOR ===\n";

        wstring input;
        wcout << L"Введите последовательность рёбер (К/О): ";
        if (!(wcin >> input)) break;

        if (!isValidCreaseStr(input)) {
            wcout << L"Ошибка: пустая строка, недопустимые символы или длина не вида 2^n - 1.\n\n";
            continue;
        }

        try {
            if (phase == 0)      runB_Array(input);
            else if (phase == 1) runB_List(input);
            else                 runB_Vector(input);
        }
        catch (const bad_alloc&) {
            wcout << L"Ошибка: недостаточно памяти.\n\n";
        }
        catch (const exception& e) {
            wcout << L"Исключение: " << e.what() << L"\n\n";
        }
    }
    return 0;
}