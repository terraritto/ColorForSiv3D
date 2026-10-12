#pragma once
#include <Siv3D.hpp>

void StefanBoltzmannGraph()
{
	Scene::SetBackground(Palette::White);

	auto f = [](const double T, const double epsilon = 1.0)
		{
			constexpr double sigma = 5.670374419 * 10e-8;
			const double T2 = T * T;
			const double T4 = T2 * T2;
			return Clamp(epsilon, 0.0, 1.0) * T4 * sigma;
		};

	Array<double> result;

	constexpr double TEMPERATURE_MIN = 1000.0;
	constexpr double TEMPERATURE_MAX = 10000.0;
	constexpr int Steps = 1000;

	for (int i = 0; i <= Steps; i++)
	{
		double T = Math::Lerp(TEMPERATURE_MIN, TEMPERATURE_MAX, i / static_cast<double>(Steps));
		result.push_back(f(T));
	}

	const double offset = 30;
	auto sceneSize = Scene::Size();
	Vec2 sceneMin = { offset, offset };
	Vec2 sceneMax = sceneSize - Vec2::One() * offset;

	auto ConvertAxisT = [&](double v)
		{
			auto threshold = (v - TEMPERATURE_MIN) / (TEMPERATURE_MAX - TEMPERATURE_MIN);
			return Math::Lerp(sceneMin.x, sceneMax.x, threshold);
		};

	const Font font{ FontMethod::SDF, 15 };

	while (System::Update())
	{
		// 軸
		// 一番下の線
		{
			Vec2 left{ sceneMin.x, sceneMax.y };
			Vec2 right{ sceneMax.x, sceneMax.y };

			Line{ left,right }.draw(Palette::Black);
		}

		// 1000K毎に線
		{
			for (int v = 1000; v <= 10000; v += 1000)
			{
				double x = ConvertAxisT(v);

				Vec2 top{ x, sceneMax.y - 5 };
				Vec2 bottom{ x, sceneMax.y };
				Line{ top, bottom }.draw(Palette::Black);

				font(Format(v)).drawAt(bottom + Vec2{0,15}, Palette::Black);
			}
		}

		// グラフ
		for (int i = 0; i < result.size() - 1; i++)
		{
			double T1 = Math::Lerp(TEMPERATURE_MIN, TEMPERATURE_MAX, i / static_cast<double>(Steps));
			double T2 = Math::Lerp(TEMPERATURE_MIN, TEMPERATURE_MAX, (i+1) / static_cast<double>(Steps));
			T1 = ConvertAxisT(T1);
			T2 = ConvertAxisT(T2);
			double E1 = Math::Lerp(sceneMin.y, sceneMax.y, 1.0 - result[i] / result.back());
			double E2 = Math::Lerp(sceneMin.y, sceneMax.y, 1.0 - result[i+1] / result.back());

			Line{ Vec2{T1,E1}, Vec2{T2,E2} }.draw(Palette::Red);
		}
	}
}
