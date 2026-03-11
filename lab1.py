
# Пусть g = НОД(A, B); Сумма A и B тоже делится на g => g делитель N.
# g можно представить как N/k - где d - это какое-то натуральное число. Чем меньше d, тем больше g. При этом деление должно выдавать целое число
# Итак: находим наименьший делитель числа (d), тогда g = N/d. В качестве A и B можно взять g и N-g


import math

def lab1():
    input_data = input()
    if not input_data:
        return
    
    n = int(input_data)

    min_divisor = n  # Останется таким если число простое
    
    if n % 2 == 0: #Чётное
        min_divisor = 2
    else:
        # Если нечетное, перебираем нечетные делители от 3 до квадратного корня этого числа (если у числа есть делитель то он точно будет меньше или равен ему)
        limit = int(math.isqrt(n)) 
        for i in range(3, limit + 1, 2):
            if n % i == 0:
                min_divisor = i
                break
    
    g = n // min_divisor
    
    a = g
    b = n - g
    
    print(a, b)

lab1()