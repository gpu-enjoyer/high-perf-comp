
# High Performance Computing. <br>OpenMP Labs Pipeline.


## Запуск

```bash
# Все задачи
./main.sh

# Или только несколько
./main.sh omp3 omp9
```


## Структура проекта

```
src/       - исходники         common.hpp  + omp*.cpp
conf/      - конфиги           common.conf + omp*.conf
script/    - пайплайн          main.sh ... draw.py
build/     - бинарные файлы    omp*
result/    - вывод             omp*/ data.csv + plot.png
```


## CLI бинарных файлов

```bash
./build/omp1 <size> <mode> [--runs=R] [--threads=T] [--seed=S]

  # size: число или small (100K), medium (1M), large (10M)
  # mode: зависит от задачи (см. conf/omp*.conf)
```


## Параметры машины

```bash
# Ubuntu 25.10

nproc
# 12

g++ --version
# 13.3.0
```


## Условия задач

[**tasks.pdf**](tasks.pdf)

1. Минимальный (максимальный) элемент вектора.
2. Скалярное произведение двух векторов.
3. Определенный интеграл методом прямоугольников.
4. Максимальное значение среди минимальных элементов строк матрицы.
5. Предыдущая задача с матрицами специального типа.
6. Исследовать режимы: static, dynamic, guided.
7. Редукция: critical, atomic, lock, reduction.
8. Скалярное произведение набора векторов (sections).
9. Вложенный параллелизм.


## Формат исследования

**Параметры экспериментов**
- Число потоков относительно числа ядер (меньше, столько же, больше)
- Масштаб задачи (размеры массивов или матриц)

**Измерения**
- time
- speedup


## Отчет

[**report.md**](report.md)