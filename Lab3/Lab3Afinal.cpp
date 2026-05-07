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
    Node(wchar_t d, Node* n) : data(d), next(n) {}
};

// Очистка памяти списка
void freeList(Node* h) { while (h) { Node* t = h; h = h->next; delete t; } }

// 1. Обработка через динамический массив
void runArray(const wstring& folds, int k) {
    auto t1 = steady_clock::now();
    size_t len = 0; wchar_t* cur = nullptr;
    for (int i = folds.length() - 1; i >= 0; --i) {
        size_t nl = 2 * len + 1; wchar_t* nxt = new wchar_t[nl];
        wchar_t c = (folds[i] == L'П') ? L'О' : L'К';
        for (size_t j = 0; j < len; ++j) nxt[j] = invert(cur[len - 1 - j]);
        nxt[len] = c;
        for (size_t j = 0; j < len; ++j) nxt[len + 1 + j] = cur[j];
        delete[] cur; cur = nxt; len = nl;
    }
    wstring res(cur, len); delete[] cur;
    double ms = duration_cast<microseconds>(steady_clock::now() - t1).count() / 1000.0;

    if (k<1 || k>(int)res.size()) { wcout << L"Неверный номер ребра.\n\n"; return; }
    wcout << L"[Массив] " << ms << L" мс | №" << k << L": " << res[k - 1] << L"\n";
    wcout << L"Полная последовательность: " << res << L"\n\n";
}

// 2. Обработка через связанный список
void runList(const wstring& folds, int k) {
    auto t1 = steady_clock::now();
    Node* h = nullptr, * tail = nullptr; size_t len = 0;
    for (int i = folds.length() - 1; i >= 0; --i) {
        Node* oh = h; Node* nh = nullptr; Node* lt = nullptr;
        wchar_t c = (folds[i] == L'П') ? L'О' : L'К';
        // инвертированное зеркало (вставка в голову даёт обратный порядок)
        for (Node* p = oh; p; p = p->next) {
            Node* n = new Node(invert(p->data), nh); nh = n;
            if (!lt) lt = n;
        }
        Node* cn = new Node(c, oh); if (lt) lt->next = cn;
        h = nh ? nh : cn; if (!tail) tail = cn; len = 2 * len + 1;
    }
    wstring res; res.reserve(len); wchar_t tk = 0; size_t idx = 1;
    for (Node* p = h; p; p = p->next) {
        res += p->data; if (idx == k) tk = p->data; idx++;
    }
    freeList(h);
    double ms = duration_cast<microseconds>(steady_clock::now() - t1).count() / 1000.0;

    if (k<1 || k>(int)res.size()) { wcout << L"Неверный номер ребра.\n\n"; return; }
    wcout << L"[Связ.список] " << ms << L" мс | №" << k << L": " << tk << L"\n";
    wcout << L"Полная последовательность: " << res << L"\n\n";
}

// 3. Обработка через STL vector
void runVector(const wstring& folds, int k) {
    auto t1 = steady_clock::now();
    vector<wchar_t> seq;
    for (auto it = folds.rbegin(); it != folds.rend(); ++it) {
        size_t os = seq.size(); vector<wchar_t> ns; ns.reserve(2 * os + 1);
        for (auto rit = seq.rbegin(); rit != seq.rend(); ++rit) ns.push_back(invert(*rit));
        ns.push_back((*it == L'П') ? L'О' : L'К');
        ns.insert(ns.end(), seq.begin(), seq.end());
        seq = std::move(ns); // Сперемещение без копирования
    }
    wstring res(seq.begin(), seq.end());
    double ms = duration_cast<microseconds>(steady_clock::now() - t1).count() / 1000.0;

    if (k<1 || k>(int)res.size()) { wcout << L"Неверный номер ребра.\n\n"; return; }
    wcout << L"[STL Vector] " << ms << L" мс | №" << k << L": " << res[k - 1] << L"\n";
    wcout << L"Полная последовательность: " << res << L"\n\n";
}

int main() {
    // консоли в UTF-16 
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    wcout << L"Программа А: последовательность сгибов -> последовательность рёбер\n";
    wcout << L"Выоплнил: РПИа-о25 Сиренко Владимир Владимирович\n";

    for (int phase = 0; phase < 3; ++phase) {
        if (phase == 0)      wcout << L"=== ФАЗА 1: ДИНАМИЧЕСКИЙ МАССИВ ===\n";
        else if (phase == 1) wcout << L"=== ФАЗА 2: СВЯЗАННЫЙ СПИСОК ===\n";
        else                wcout << L"=== ФАЗА 3: STL VECTOR ===\n";

        wstring folds; int k;
        wcout << L"Введите последовательность сгибов (П/З): ";
        if (!(wcin >> folds)) break;
        wcout << L"Введите номер ребра (1..2^N-1): ";
        if (!(wcin >> k)) break;

        if (folds.empty()) { wcout << L"Пустая последовательность.\n\n"; continue; }

        try {
            if (phase == 0)      runArray(folds, k);
            else if (phase == 1) runList(folds, k);
            else                runVector(folds, k);
        }
        catch (const bad_alloc&) {
            wcout << L"Ошибка: недостаточно памяти (экспоненциальный рост).\n\n";
        }
        catch (const exception& e) {
            wcout << L"Исключение: " << e.what() << L"\n\n";
        }
    }
    return 0;
}