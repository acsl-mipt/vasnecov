#ifndef VASNECOV_TEHNOLOGIST_H
#define VASNECOV_TEHNOLOGIST_H

#pragma GCC diagnostic ignored "-Weffc++"
#include "types.h"
#pragma GCC diagnostic warning "-Weffc++"

namespace Vasnecov
{
	// TODO: make normal loging
	int panic(const GLstring &text_problemy1, const GLstring &text_problemy2 = ""); // Функция сообщения и архивирования критических ошибок
	int panic(const GLstring &text_problemy1, int nomer_oshibki); // Функция сообщения и архивирования критических ошибок
	int panic(const GLstring &text_problemy1, float chislo); // Функция сообщения и архивирования критических ошибок

	int problem(const GLstring &text_problemy1, const GLstring &text_problemy2 = ""); // Функция сообщения и архивирования системных ошибок
	int problem(const GLstring &text_problemy1, int nomer_oshibki); // Функция сообщения и архивирования системных ошибок
	int problem(const GLstring &text_problemy1, float chislo); // Функция сообщения и архивирования системных ошибок

	float trimAngle(float deg);
}

inline float Vasnecov::trimAngle(float deg)
{
	if(deg > 0)
	{
		deg = fmod(deg, 360);
	}
	if(deg < 0)
	{
		deg = fmod(deg, 360) + 360;
	}
	return deg;
}

#pragma GCC diagnostic ignored "-Weffc++"
#endif // VASNECOV_TEHNOLOGIST_H
