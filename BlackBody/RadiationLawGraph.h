#pragma once
#include <Siv3D.hpp>

void RadiationLawGraph()
{
	constexpr double h = 6.62607015 * 1e-34;	// プランク定数
	constexpr double c = 2.99792458 * 1e+8;		// 光速
	constexpr double k = 1.380649 * 1e-23;		// ボルツマン定数

	auto PlankLaw = [h, c, k](double lambda, double T)
		{
			double L2 = lambda * lambda;
			double L5 = L2 * L2 * lambda;
			double Term1 = 2.0 * h * c * c / L5;
			double exponent = Exp(h * c / (lambda * k * T));
			double Term2 = 1.0 / (exponent - 1.0);

			return Term1 * Term2;
		};

	auto WienLaw = [h, c, k](double lambda, double T)
		{
			double L2 = lambda * lambda;
			double L5 = L2 * L2 * lambda;
			double Term1 = 2.0 * h * c * c / L5;
			double Term2 = Exp(-h * c / (lambda * k * T));

			return Term1 * Term2;
		};

	auto RayleighJeansLaw = [h, c, k](double lambda, double T)
		{
			double L2 = lambda * lambda;
			double L4 = L2 * L2;
			return 2 * c * k * T / L4;
		};

	constexpr int iteration = 1000;
	constexpr double Offset = 70.0;
	constexpr double XMin = 0.0, XMax = 4.5 * 1e-6;
	constexpr double YMin = 0.0, YMax = 1.5 * 1e+13; // 高温度領域
	// constexpr double YMin = 0.0, YMax = 1.5 * 1e+12; // 低温度領域
	constexpr double TMin = 1000.0, TMax = 5000.0;

	constexpr double MinVisibleWaveLength = 380 * 1e-9; // 380nm
	constexpr double MaxVisibleWaveLength = 780 * 1e-9; // 780nm

	const auto SceneSize = Scene::Size();
	const auto SceneSizeMin = Vec2::One() * Offset;
	const auto SceneSizeMax = SceneSize - Vec2::One() * Offset;

	const Font font{ FontMethod::MSDF, 12 };
	Image image{ SceneSize, Palette::White };
	DynamicTexture texture{ SceneSize };

	auto DrawPlank = [&]()
		{
			image.fill(Palette::White);

			for (double T = TMin; T <= TMax; T += 1000)
			{
				ColorF color = Colormap01(T / 5000.0);

				double MaxPoint = std::numeric_limits<double>::lowest();
				Vec2 p;
				for (int i = 0; i < iteration; i++)
				{
					// Plankの法則を計算
					double lambdaCurrent = Math::Lerp(XMin, XMax, i / static_cast<double>(iteration));
					double lambdaNext = Math::Lerp(XMin, XMax, (i+1.0) / (iteration));
					double LCurrent = PlankLaw(lambdaCurrent, T);
					double LNext = PlankLaw(lambdaNext, T);

					if (isnan(LCurrent)) { LCurrent = 0.0; }
					if (isnan(LNext)) { LNext = 0.0; }

					// Lineを描画
					Vec2 current, next;
					current.x = Math::Lerp(SceneSizeMin.x, SceneSizeMax.x, i / static_cast<double>(iteration));
					next.x = Math::Lerp(SceneSizeMin.x, SceneSizeMax.x, (i+1) / static_cast<double>(iteration));
					current.y = Math::Lerp(SceneSizeMin.y, SceneSizeMax.y, (YMax - LCurrent) / (YMax - YMin));
					next.y = Math::Lerp(SceneSizeMin.y, SceneSizeMax.y, (YMax - LNext) / (YMax - YMin));

					Line{ current, next }.paint(image, color);

					if (MaxPoint < LCurrent)
					{
						p = current;
						MaxPoint = LCurrent;
					}
				}

				font(U"T=" + Format(T)).paint(image, p + Vec2::Up(15), Palette::Black);
			}

			// Line
			{
				double minWave = (MinVisibleWaveLength - XMin) / (XMax - XMin);
				minWave = Math::Lerp(SceneSizeMin.x, SceneSizeMax.x, minWave);
				double maxWave = (MaxVisibleWaveLength - XMin) / (XMax - XMin);
				maxWave = Math::Lerp(SceneSizeMin.x, SceneSizeMax.x, maxWave);

				Line{ Vec2{minWave, 0}, Vec2{minWave, SceneSize.y} }.paint(image, Palette::Black);
				Line{ Vec2{maxWave, 0}, Vec2{maxWave, SceneSize.y} }.paint(image, Palette::Black);
			}

			texture.fill(image);
		};

	auto DrawWien = [&]()
		{
			DrawPlank();

			for (double T = TMin; T <= TMax; T += 1000)
			{
				ColorF color = Colormap01(T / 5000.0);

				for (int i = 0; i < iteration; i+=20)
				{
					// Wienの放射法則を計算
					double lambdaCurrent = Math::Lerp(XMin, XMax, i / static_cast<double>(iteration));
					double LCurrent = WienLaw(lambdaCurrent, T);
					if (isnan(LCurrent)) { LCurrent = 0.0; }

					// Lineを描画
					Vec2 current;
					current.x = Math::Lerp(SceneSizeMin.x, SceneSizeMax.x, i / static_cast<double>(iteration));
					current.y = Math::Lerp(SceneSizeMin.y, SceneSizeMax.y, (YMax - LCurrent) / (YMax - YMin));

					Circle{ current, 1.0 }.paint(image, color);
				}
			}

			texture.fill(image);
		};

	auto DrawRayleighJeans = [&]()
		{
			DrawPlank();

			for (double T = TMin; T <= TMax; T += 1000)
			{
				ColorF color = Colormap01(T / 5000.0);

				for (int i = 0; i < iteration; i += 20)
				{
					// Wienの放射法則を計算
					double lambdaCurrent = Math::Lerp(XMin, XMax, i / static_cast<double>(iteration));
					double LCurrent = RayleighJeansLaw(lambdaCurrent, T);
					if (isnan(LCurrent)) { LCurrent = 0.0; }

					// Lineを描画
					Vec2 current;
					current.x = Math::Lerp(SceneSizeMin.x, SceneSizeMax.x, i / static_cast<double>(iteration));
					current.y = Math::Lerp(SceneSizeMin.y, SceneSizeMax.y, (YMax - LCurrent) / (YMax - YMin));

					Circle{ current, 1.0 }.paint(image, color);
				}
			}

			texture.fill(image);
		};

	DrawPlank();

	while (System::Update())
	{
		texture.draw();

		if (Key1.down()) { DrawWien(); }
		if (Key2.down()) { DrawRayleighJeans(); }
		if (Key3.down()) { DrawPlank(); }
	}
}
