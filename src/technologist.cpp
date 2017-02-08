#include "technologist.h"
#include <iostream>
#include <sstream>
#pragma GCC diagnostic warning "-Weffc++"


int Vasnecov::panic(const GLstring &text_problemy1, const GLstring &text_problemy2)
{
	std::cout << "********************************************************************************" << std::endl;
	std::cout << "Паника! Возможно аварийное завершение программы." << std::endl;
	std::cout << "Ошибка: " << text_problemy1 << text_problemy2 << std::endl;

	return 1;
}


int Vasnecov::panic(const GLstring &text_problemy1, int nomer_oshibki)
{
	std::ostringstream oss;
	oss << nomer_oshibki;
	return panic(text_problemy1, oss.str());
}


int Vasnecov::panic(const GLstring &text_problemy1, float chislo)
{
	std::ostringstream oss;
	oss << chislo;
	return panic(text_problemy1, oss.str());
}


int Vasnecov::problem(const GLstring &text_problemy1, const GLstring &text_problemy2)
{
	std::cout << "Ошибка: " << text_problemy1 << text_problemy2 << std::endl;

	return 1;
}


int Vasnecov::problem(const GLstring &text_problemy1, int nomer_oshibki)
{
	std::ostringstream oss;
	oss << nomer_oshibki;

	return problem(text_problemy1, oss.str());
}


int Vasnecov::problem(const GLstring &text_problemy1, float chislo)
{
	std::ostringstream oss;
	oss << chislo;

	return problem(text_problemy1, oss.str());
}

#pragma GCC diagnostic ignored "-Weffc++"
