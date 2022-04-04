cmake_minimum_required(VERSION 3.22)
project(SO_2022 C)

set(CMAKE_C_STANDARD 11)

add_executable(FilterNoVocals lab6/a/set1/l6_a_s1_1_FilterNoVocals.c)
add_executable(MyCut lab6/a/set1/l6_a_s1_2_MyCut.c)

add_executable(ROT13 lab6/a/set2/l6_a_s2_1_ROT13.c)
add_executable(MyGrep lab6/a/set2/l6_a_s2_2_MyGrep.c)
